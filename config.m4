dnl $Id$
dnl config.m4 for extension xhprof

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

PHP_ARG_WITH(xhprof, for xhprof support,
Make sure that the comment is aligned:
[  --with-xhprof             Include xhprof support])

dnl Otherwise use enable:

dnl PHP_ARG_ENABLE(xhprof, whether to enable xhprof support,
dnl Make sure that the comment is aligned:
dnl [  --enable-xhprof           Enable xhprof support])

if test "$PHP_XHPROF" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-xhprof -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/xhprof.h"  # you most likely want to change this
  dnl if test -r $PHP_XHPROF/$SEARCH_FOR; then # path given as parameter
  dnl   XHPROF_DIR=$PHP_XHPROF
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for xhprof files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       XHPROF_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$XHPROF_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the xhprof distribution])
  dnl fi

  dnl # --with-xhprof -> add include path
  dnl PHP_ADD_INCLUDE($XHPROF_DIR/include)

  dnl # --with-xhprof -> check for lib and symbol presence
  dnl LIBNAME=xhprof # you may want to change this
  dnl LIBSYMBOL=xhprof # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $XHPROF_DIR/$PHP_LIBDIR, XHPROF_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_XHPROFLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong xhprof lib version or lib not found])
  dnl ],[
  dnl   -L$XHPROF_DIR/$PHP_LIBDIR -lm
  dnl ])
  dnl
  dnl PHP_SUBST(XHPROF_SHARED_LIBADD)

  PHP_NEW_EXTENSION(xhprof, xhprof.c, $ext_shared,, -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1)
fi
