dnl $Id$
dnl config.m4 for extension phpng_xhprof

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

PHP_ARG_WITH(phpng_xhprof, for phpng_xhprof support,
dnl Make sure that the comment is aligned:
[  --with-phpng_xhprof             Include phpng_xhprof support])

dnl Otherwise use enable:

dnl PHP_ARG_ENABLE(phpng_xhprof, whether to enable phpng_xhprof support,
dnl Make sure that the comment is aligned:
dnl [  --enable-phpng_xhprof           Enable phpng_xhprof support])

if test "$PHP_PHPNG_XHPROF" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-phpng_xhprof -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/phpng_xhprof.h"  # you most likely want to change this
  dnl if test -r $PHP_PHPNG_XHPROF/$SEARCH_FOR; then # path given as parameter
  dnl   PHPNG_XHPROF_DIR=$PHP_PHPNG_XHPROF
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for phpng_xhprof files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       PHPNG_XHPROF_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$PHPNG_XHPROF_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the phpng_xhprof distribution])
  dnl fi

  dnl # --with-phpng_xhprof -> add include path
  dnl PHP_ADD_INCLUDE($PHPNG_XHPROF_DIR/include)

  dnl # --with-phpng_xhprof -> check for lib and symbol presence
  dnl LIBNAME=phpng_xhprof # you may want to change this
  dnl LIBSYMBOL=phpng_xhprof # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $PHPNG_XHPROF_DIR/$PHP_LIBDIR, PHPNG_XHPROF_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_PHPNG_XHPROFLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong phpng_xhprof lib version or lib not found])
  dnl ],[
  dnl   -L$PHPNG_XHPROF_DIR/$PHP_LIBDIR -lm
  dnl ])
  dnl
  dnl PHP_SUBST(PHPNG_XHPROF_SHARED_LIBADD)

  PHP_NEW_EXTENSION(phpng_xhprof, phpng_xhprof.c, $ext_shared,, -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1)
fi
