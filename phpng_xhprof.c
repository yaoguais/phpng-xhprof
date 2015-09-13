/*
  +----------------------------------------------------------------------+
  | PHP Version 7                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2015 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:  Yaoguai (newtopdtdio@163.com)                               |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_phpng_xhprof.h"
#include "zend_extensions.h"

/* fix cpu properties */
#ifdef linux
# ifndef _GNU_SOURCE
# 	define _GNU_SOURCE
# endif
#endif
#include <sys/time.h>
#include <sys/resource.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef __FreeBSD__
# if __FreeBSD_version >= 700110
#   include <sys/resource.h>
#   include <sys/cpuset.h>
#   define cpu_set_t cpuset_t
#   define SET_AFFINITY(pid, size, mask) \
           cpuset_setaffinity(CPU_LEVEL_WHICH, CPU_WHICH_TID, -1, size, mask)
#   define GET_AFFINITY(pid, size, mask) \
           cpuset_getaffinity(CPU_LEVEL_WHICH, CPU_WHICH_TID, -1, size, mask)
# else
#   error "This version of FreeBSD does not support cpusets"
# endif
#elif __APPLE__
#    include <mach/mach_init.h>
#    include <mach/thread_policy.h>
#    define cpu_set_t thread_affinity_policy_data_t
#    define CPU_SET(cpu_id, new_mask) \
        (*(new_mask)).affinity_tag = (cpu_id + 1)
#    define CPU_ZERO(new_mask)                 \
        (*(new_mask)).affinity_tag = THREAD_AFFINITY_TAG_NULL
#   define SET_AFFINITY(pid, size, mask)       \
        thread_policy_set(mach_thread_self(), THREAD_AFFINITY_POLICY, mask, \
                          THREAD_AFFINITY_POLICY_COUNT)
#else
# include <sched.h>
# define SET_AFFINITY(pid, size, mask) sched_setaffinity(0, size, mask)
# define GET_AFFINITY(pid, size, mask) sched_getaffinity(0, size, mask)
#endif

/* define and structure */
#define XHPROF_MAX_IGNORED_FUNCTIONS  256
#define XHPROF_IGNORED_FUNCTION_FILTER_SIZE ((XHPROF_MAX_IGNORED_FUNCTIONS + 7)/8)

#define XHPROF_FLAGS_NO_BUILTINS   0x0001
#define XHPROF_FLAGS_CPU           0x0002
#define XHPROF_FLAGS_MEMORY        0x0004

#define INDEX_2_BYTE(index)  (index >> 3)
#define INDEX_2_BIT(index)   (1 << (index & 0x7));

#define XHPROF_MODE_HIERARCHICAL       1
#define XHPROF_MODE_SAMPLED            620002

#if !defined(uint64)
typedef unsigned long long uint64;
#endif
#if !defined(uint32)
typedef unsigned int uint32;
#endif
#if !defined(uint8)
typedef unsigned char uint8;
#endif

typedef struct hp_entry_t {
  char                  * name_hprof;
  int                     rlvl_hprof;
  uint64                  tsc_start;
  long int                mu_start_hprof;
  long int                pmu_start_hprof;
  struct rusage           ru_start_hprof;
  struct hp_entry_t     * prev_hprof;
  uint8                   hash_code;
} hp_entry_t;

typedef void (*hp_init_cb)           (TSRMLS_D);
typedef void (*hp_exit_cb)           (TSRMLS_D);
typedef void (*hp_begin_function_cb) (hp_entry_t **entries, hp_entry_t *current   TSRMLS_DC);
typedef void (*hp_end_function_cb)   (hp_entry_t **entries  TSRMLS_DC);

typedef struct hp_mode_cb {
  hp_init_cb             init_cb;
  hp_exit_cb             exit_cb;
  hp_begin_function_cb   begin_fn_cb;
  hp_end_function_cb     end_fn_cb;
} hp_mode_cb;

typedef struct hp_global_t {
  int              enabled;
  int              ever_enabled;
  zval             stats_count;
  int              profiler_level;
  hp_entry_t     * entries;
  hp_entry_t     * entry_free_list;
  hp_mode_cb       mode_cb;
  struct timeval   last_sample_time;
  uint64           last_sample_tsc;
  uint64           sampling_interval_tsc;
  double         * cpu_frequencies;
  uint32           cpu_num;
  cpu_set_t        prev_mask;
  uint32           cur_cpu_id;
  uint32           xhprof_flags;
  uint8            func_hash_counters[256];
  char          ** ignored_function_names;
  uint8            ignored_function_filter[XHPROF_IGNORED_FUNCTION_FILTER_SIZE];
} hp_global_t;

/* global static variables */
static hp_global_t       hp_globals;

/* static functions declare */
static void hp_register_constants(INIT_FUNC_ARGS);
/**
 * init global variable(no need malloc part),we call hp_init
 * and reset global variable,we call hp_clear
 * when user use  xhprof_enable/xhprof_sample_enable,
 * we call hp_begin(malloc all but once,reset emalloc variables)
 * when user use  xhprof_disable/xhprof_sample_disable,
 * we call hp_end(free all but once,destroy emalloc variables)
 */
static void hp_init(TSRMLS_D);
static void hp_clear(TSRMLS_D);
static void hp_begin(long level, long xhprof_flags TSRMLS_DC);
static void hp_end(TSRMLS_D);

static inline uint64 cycle_timer();
static double get_cpu_frequency();
static void clear_frequencies();
static void get_all_cpu_frequencies();

static void hp_free_the_free_list();
static hp_entry_t *hp_fast_alloc_hprof_entry();
static void hp_fast_free_hprof_entry(hp_entry_t *p);
static inline uint8 hp_inline_hash(char * str);
static long get_us_interval(struct timeval *start, struct timeval *end);
static void incr_us_interval(struct timeval *start, uint64 incr);

static void hp_get_ignored_functions_from_arg(zval *args);
static void hp_ignored_functions_filter_init();
static void hp_ignored_functions_filter_clear();

static inline zval  *hp_zval_at_key(char  *key,zval  *values);
static inline char **hp_strings_in_zval(zval  *values);
static inline void   hp_array_del(char **name_array);

/* function prototypes declare */
void hp_init_profiler_state(int level TSRMLS_DC);
void hp_clean_profiler_state(TSRMLS_D);
int restore_cpu_affinity(cpu_set_t * prev_mask);
int bind_to_cpu(uint32 cpu_id);

/* php callback functions */
PHP_MINIT_FUNCTION(phpng_xhprof)
{
	return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(phpng_xhprof)
{
	return SUCCESS;
}

PHP_RINIT_FUNCTION(phpng_xhprof)
{
#if defined(COMPILE_DL_PHPNG_XHPROF) && defined(ZTS)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif
	return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(phpng_xhprof)
{
	return SUCCESS;
}

PHP_MINFO_FUNCTION(phpng_xhprof)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "phpng_xhprof support", "enabled");
	php_info_print_table_end();

	DISPLAY_INI_ENTRIES();
}

const zend_function_entry phpng_xhprof_functions[] = {
	PHP_FE_END
};

zend_module_entry phpng_xhprof_module_entry = {
	STANDARD_MODULE_HEADER,
	"phpng_xhprof",
	phpng_xhprof_functions,
	PHP_MINIT(phpng_xhprof),
	PHP_MSHUTDOWN(phpng_xhprof),
	PHP_RINIT(phpng_xhprof),
	PHP_RSHUTDOWN(phpng_xhprof),
	PHP_MINFO(phpng_xhprof),
	PHP_PHPNG_XHPROF_VERSION,
	STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_PHPNG_XHPROF
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE();
#endif
ZEND_GET_MODULE(phpng_xhprof)
#endif

/* static functions implement */
static void hp_register_constants(INIT_FUNC_ARGS)
{
	REGISTER_LONG_CONSTANT("XHPROF_FLAGS_NO_BUILTINS", XHPROF_FLAGS_NO_BUILTINS,
													  CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XHPROF_FLAGS_CPU", XHPROF_FLAGS_CPU,
	                                                  CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("XHPROF_FLAGS_MEMORY", XHPROF_FLAGS_MEMORY,
	                                                  CONST_CS | CONST_PERSISTENT);
}

static void hp_init(TSRMLS_D)
{
	int i;
	hp_globals.cpu_num = sysconf(_SC_NPROCESSORS_CONF);

#ifndef __APPLE__
	if (GET_AFFINITY(0, sizeof(cpu_set_t), &hp_globals.prev_mask) < 0) {
		perror("getaffinity");
		return FAILURE;
	}
#else
	CPU_ZERO(&(hp_globals.prev_mask));
#endif

	hp_globals.enabled = 0;
	hp_globals.ever_enabled = 0;
	hp_globals.cpu_frequencies = NULL;
	hp_globals.cur_cpu_id = 0;
	hp_globals.stats_count = NULL;
	hp_globals.entry_free_list = NULL;

	for (i = 0; i < 256; i++) {
		hp_globals.func_hash_counters[i] = 0;
	}
	hp_ignored_functions_filter_clear();

#if defined(DEBUG)
	srand(0);
#endif

}

static void hp_clear(TSRMLS_D)
{
	if(!hp_globals.enabled)
	{
		return;
	}
	int   hp_profile_flag = 1;
	while (hp_globals.entries) {
		// TODO
		//END_PROFILING(&hp_globals.entries, hp_profile_flag);
	}
	restore_cpu_affinity(&hp_globals.prev_mask);
	hp_globals.enabled = 0;
}

static void hp_begin(long level, long xhprof_flags TSRMLS_DC)
{
	hp_globals.enabled      = 1;
	if(!hp_globals.ever_enabled)
	{
		hp_globals.ever_enabled = 1;
		hp_globals.entries = NULL;
	}
	hp_globals.xhprof_flags = (uint32)xhprof_flags;
	if (!(hp_globals.xhprof_flags & XHPROF_FLAGS_NO_BUILTINS)) {
		// TODO
	}
	hp_globals.mode_cb.init_cb     = hp_mode_dummy_init_cb;
	hp_globals.mode_cb.exit_cb     = hp_mode_dummy_exit_cb;
	hp_globals.mode_cb.begin_fn_cb = hp_mode_dummy_beginfn_cb;
	hp_globals.mode_cb.end_fn_cb   = hp_mode_dummy_endfn_cb;
	switch(level) {
	  case XHPROF_MODE_HIERARCHICAL:
		hp_globals.mode_cb.begin_fn_cb = hp_mode_hier_beginfn_cb;
		hp_globals.mode_cb.end_fn_cb   = hp_mode_hier_endfn_cb;
		break;
	  case XHPROF_MODE_SAMPLED:
		hp_globals.mode_cb.init_cb     = hp_mode_sampled_init_cb;
		hp_globals.mode_cb.begin_fn_cb = hp_mode_sampled_beginfn_cb;
		hp_globals.mode_cb.end_fn_cb   = hp_mode_sampled_endfn_cb;
		break;
	}
	if (!hp_globals.ever_enabled) {
	hp_globals.ever_enabled  = 1;
	hp_globals.entries = NULL;
	}
	hp_globals.profiler_level  = (int) level;

	/* Init stats_count */
	if (hp_globals.stats_count) {
	zval_dtor(hp_globals.stats_count);
	FREE_ZVAL(hp_globals.stats_count);
	}
	MAKE_STD_ZVAL(hp_globals.stats_count);
	array_init(hp_globals.stats_count);

	/* NOTE(cjiang): some fields such as cpu_frequencies take relatively longer
	* to initialize, (5 milisecond per logical cpu right now), therefore we
	* calculate them lazily. */
	if (hp_globals.cpu_frequencies == NULL) {
	get_all_cpu_frequencies();
	restore_cpu_affinity(&hp_globals.prev_mask);
	}

	/* bind to a random cpu so that we can use rdtsc instruction. */
	bind_to_cpu((int) (rand() % hp_globals.cpu_num));

	/* Call current mode's init cb */
	hp_globals.mode_cb.init_cb(TSRMLS_C);

	/* Set up filter of functions which may be ignored during profiling */
	hp_ignored_functions_filter_init();
}

static void hp_end(TSRMLS_D)
{

}

static inline uint64 cycle_timer()
{
	uint32 __a,__d;
	uint64 val;
	asm volatile("rdtsc" : "=a" (__a), "=d" (__d));
	(val) = ((uint64)__a) | (((uint64)__d)<<32);
	return val;
}

static double get_cpu_frequency()
{
	struct timeval start;
	struct timeval end;
	if (gettimeofday(&start, 0))
	{
		perror("gettimeofday");
		return 0.0;
	}
	uint64 tsc_start = cycle_timer();
	usleep(5000);
	if (gettimeofday(&end, 0)) {
		perror("gettimeofday");
		return 0.0;
	}
	uint64 tsc_end = cycle_timer();
	return (tsc_end - tsc_start) * 1.0 / (get_us_interval(&start, &end));
}

static void clear_frequencies()
{
	if (hp_globals.cpu_frequencies) {
		free(hp_globals.cpu_frequencies);
		hp_globals.cpu_frequencies = NULL;
	}
	restore_cpu_affinity(&hp_globals.prev_mask);
}

static void get_all_cpu_frequencies()
{
	int id;
	double frequency;
	hp_globals.cpu_frequencies = malloc(sizeof(double) * hp_globals.cpu_num);
	if (hp_globals.cpu_frequencies == NULL) {
		return;
	}
	for (id = 0; id < hp_globals.cpu_num; ++id) {
		if (bind_to_cpu(id)) {
			clear_frequencies();
			return;
		}
		usleep(0);
		frequency = get_cpu_frequency();
		if (frequency == 0.0) {
			clear_frequencies();
			return;
		}
		hp_globals.cpu_frequencies[id] = frequency;
	}
}

static void hp_free_the_free_list()
{

}

static hp_entry_t *hp_fast_alloc_hprof_entry()
{

}

static void hp_fast_free_hprof_entry(hp_entry_t *p)
{

}

static inline uint8 hp_inline_hash(char * str)
{

}

static long get_us_interval(struct timeval *start, struct timeval *end) {
	return (((end->tv_sec - start->tv_sec) * 1000000) + (end->tv_usec - start->tv_usec));
}

static void incr_us_interval(struct timeval *start, uint64 incr)
{

}

static void hp_get_ignored_functions_from_arg(zval *args)
{

}

static void hp_ignored_functions_filter_init()
{
	if (hp_globals.ignored_function_names != NULL) {
		int i = 0;
		for(; hp_globals.ignored_function_names[i] != NULL; i++) {
			char *str  = hp_globals.ignored_function_names[i];
			uint8 hash = hp_inline_hash(str);
			int   idx  = INDEX_2_BYTE(hash);
			hp_globals.ignored_function_filter[idx] |= INDEX_2_BIT(hash);
		}
	}
}

static void hp_ignored_functions_filter_clear()
{
	memset(hp_globals.ignored_function_filter, 0, XHPROF_IGNORED_FUNCTION_FILTER_SIZE);
}

static inline zval  *hp_zval_at_key(char  *key,zval  *values)
{

}

static inline char **hp_strings_in_zval(zval  *values)
{

}

static inline void   hp_array_del(char **name_array)
{

}

/* function prototypes implement */
void hp_init_profiler_state(int level TSRMLS_DC)
{
	if (!hp_globals.ever_enabled) {
		hp_globals.ever_enabled  = 1;
		hp_globals.entries = NULL;
	}
	hp_globals.profiler_level  = (int) level;
	array_init(&hp_globals.stats_count);
	if (hp_globals.cpu_frequencies == NULL) {
		get_all_cpu_frequencies();
		restore_cpu_affinity(&hp_globals.prev_mask);
	}
	bind_to_cpu((int) (rand() % hp_globals.cpu_num));
	hp_globals.mode_cb.init_cb(TSRMLS_C);
	hp_ignored_functions_filter_init();
}

void hp_clean_profiler_state(TSRMLS_D)
{

}

int restore_cpu_affinity(cpu_set_t * prev_mask)
{
	if (SET_AFFINITY(0, sizeof(cpu_set_t), prev_mask) < 0) {
		perror("restore setaffinity");
		return -1;
	}
	hp_globals.cur_cpu_id = 0;
	return 0;
}

int bind_to_cpu(uint32 cpu_id) {
	cpu_set_t new_mask;
	CPU_ZERO(&new_mask);
	CPU_SET(cpu_id, &new_mask);
	if (SET_AFFINITY(0, sizeof(cpu_set_t), &new_mask) < 0) {
		perror("setaffinity");
		return -1;
	}
	hp_globals.cur_cpu_id = cpu_id;
	return 0;
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
