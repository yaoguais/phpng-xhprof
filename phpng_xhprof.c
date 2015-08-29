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
  | Author:  Copyright (c) 2009 Facebook                                 |
  |          newtopstdio@163.com                                         |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef linux
/* To enable CPU_ZERO and CPU_SET, etc.     */
# define _GNU_SOURCE
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_phpng_xhprof.h"
#include "zend_extensions.h"
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
# endif /* __FreeBSD_version */
#elif __APPLE__


/* global structure declare */
typedef struct hp_entry_t {
  char                   *name_hprof;
  int                     rlvl_hprof;
  uint64                  tsc_start;
  long int                mu_start_hprof;
  long int                pmu_start_hprof;
  struct rusage           ru_start_hprof;
  struct hp_entry_t      *prev_hprof;
  uint8                   hash_code;
} hp_entry_t;

typedef struct hp_global_t {
  int          enabled;           /* Indicates if xhprof is currently enabled */
  int          ever_enabled;      /* Indicates if xhprof was ever enabled during this request */
  zval        *stats_count;       /* Holds all the xhprof statistics */
  int          profiler_level;    /* Indicates the current xhprof mode or level */
  hp_entry_t  *entries;           /* Top of the profile stack */
  hp_entry_t  *entry_free_list;   /* freelist of hp_entry_t chunks for reuse... */
  hp_mode_cb   mode_cb;           /* Callbacks for various xhprof modes */


  struct timeval   last_sample_time;       /* Global to track the time of the last sample in time and ticks */
  uint64           last_sample_tsc;
  uint64           sampling_interval_tsc;  /* XHPROF_SAMPLING_INTERVAL in ticks */

  double *cpu_frequencies;
  uint32 cpu_num;                /* The number of logical CPUs this machine has. */
  cpu_set_t prev_mask;           /* The saved cpu affinity. */
  uint32 cur_cpu_id;             /* The cpu id current process is bound to. (default 0) */
  uint32 xhprof_flags;           /* XHProf flags */
  uint8  func_hash_counters[256];/* counter table indexed by hash value of function names. */
  char  **ignored_function_names;/* Table of ignored function names and their filter */
  uint8   ignored_function_filter[XHPROF_IGNORED_FUNCTION_FILTER_SIZE];
} hp_global_t;

/* Various types for XHPROF callbacks       */
typedef void (*hp_init_cb)           (TSRMLS_D);
typedef void (*hp_exit_cb)           (TSRMLS_D);
typedef void (*hp_begin_function_cb) (hp_entry_t **entries, hp_entry_t *current   TSRMLS_DC);
typedef void (*hp_end_function_cb)   (hp_entry_t **entries  TSRMLS_DC);

/* Struct to hold the various callbacks for a single xhprof mode */
typedef struct hp_mode_cb {
  hp_init_cb             init_cb;
  hp_exit_cb             exit_cb;
  hp_begin_function_cb   begin_fn_cb;
  hp_end_function_cb     end_fn_cb;
} hp_mode_cb;



PHP_INI_BEGIN()
	PHP_INI_ENTRY("xhprof.output_dir", "", PHP_INI_ALL, NULL)
PHP_INI_END()

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(phpng_xhprof)
{

	REGISTER_INI_ENTRIES();
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(phpng_xhprof)
{

	UNREGISTER_INI_ENTRIES();
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(phpng_xhprof)
{
#if defined(COMPILE_DL_PHPNG_XHPROF) && defined(ZTS)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(phpng_xhprof)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(phpng_xhprof)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "phpng_xhprof support", "enabled");
	php_info_print_table_end();

	DISPLAY_INI_ENTRIES();
}
/* }}} */

/* {{{ phpng_xhprof_functions[]
 *
 * Every user visible function must have an entry in phpng_xhprof_functions[].
 */
const zend_function_entry phpng_xhprof_functions[] = {
	PHP_FE_END	/* Must be the last line in phpng_xhprof_functions[] */
};
/* }}} */

/* {{{ phpng_xhprof_module_entry
 */
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
/* }}} */

#ifdef COMPILE_DL_PHPNG_XHPROF
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE();
#endif
ZEND_GET_MODULE(phpng_xhprof)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
