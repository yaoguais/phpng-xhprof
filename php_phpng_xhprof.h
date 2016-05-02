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

#ifndef PHP_XHPROF_H
#define PHP_XHPROF_H

extern zend_module_entry xhprof_module_entry;
#define phpext_xhprof_ptr &xhprof_module_entry

#ifdef PHP_WIN32
#	define PHP_XHPROF_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_XHPROF_API __attribute__ ((visibility("default")))
#else
#	define PHP_XHPROF_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_MINIT_FUNCTION(xhprof);
PHP_MSHUTDOWN_FUNCTION(xhprof);
PHP_RINIT_FUNCTION(xhprof);
PHP_RSHUTDOWN_FUNCTION(xhprof);
PHP_MINFO_FUNCTION(xhprof);

PHP_FUNCTION(xhprof_enable);
PHP_FUNCTION(xhprof_disable);
PHP_FUNCTION(xhprof_sample_enable);
PHP_FUNCTION(xhprof_sample_disable);
PHP_FUNCTION(xhprof_for_test);



#endif	/* PHP_XHPROF_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
