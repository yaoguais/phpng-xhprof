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

#ifndef PHP_PHPNG_XHPROF_H
#define PHP_PHPNG_XHPROF_H

extern zend_module_entry phpng_xhprof_module_entry;
#define phpext_phpng_xhprof_ptr &phpng_xhprof_module_entry



#ifdef PHP_WIN32
#	define PHP_PHPNG_XHPROF_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_PHPNG_XHPROF_API __attribute__ ((visibility("default")))
#else
#	define PHP_PHPNG_XHPROF_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

/* global data types and type defines */
#define PHP_PHPNG_XHPROF_VERSION   "0.9.5"
#define ROOT_SYMBOL                "main()"
#define SCRATCH_BUF_LEN            512
#define XHPROF_MODE_HIERARCHICAL   1
#define XHPROF_MODE_SAMPLED        620002         /* Rockfort's zip code           */
#define XHPROF_FLAGS_NO_BUILTINS   0x0001         /* do not profile builtins       */
#define XHPROF_FLAGS_CPU           0x0002         /* gather CPU times for funcs    */
#define XHPROF_FLAGS_MEMORY        0x0004         /* gather memory usage for funcs */
#define XHPROF_SAMPLING_INTERVAL   100000         /* In microsecs                  */
#define XHPROF_MAX_IGNORED_FUNCTIONS  256
#define XHPROF_IGNORED_FUNCTION_FILTER_SIZE       ((XHPROF_MAX_IGNORED_FUNCTIONS + 7)/8)

#if !defined(uint64)
typedef unsigned long long uint64;
#endif
#if !defined(uint32)
typedef unsigned int uint32;
#endif
#if !defined(uint8)
typedef unsigned char uint8;
#endif

/*  php function declare */
PHP_MINIT_FUNCTION(xhprof);
PHP_MSHUTDOWN_FUNCTION(xhprof);
PHP_RINIT_FUNCTION(xhprof);
PHP_RSHUTDOWN_FUNCTION(xhprof);
PHP_MINFO_FUNCTION(xhprof);

PHP_FUNCTION(xhprof_enable);
PHP_FUNCTION(xhprof_disable);
PHP_FUNCTION(xhprof_sample_enable);
PHP_FUNCTION(xhprof_sample_disable);


#if defined(ZTS) && defined(COMPILE_DL_PHPNG_XHPROF)
ZEND_TSRMLS_CACHE_EXTERN();
#endif

#endif	/* PHP_PHPNG_XHPROF_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
