PHP_ARG_WITH(phpng_xhprof, for phpng_xhprof support, [--with-phpng_xhprof Include phpng_xhprof support])
if test "$PHP_PHPNG_XHPROF" != "no"; then
  PHP_NEW_EXTENSION(phpng_xhprof, phpng_xhprof.c, $ext_shared,, -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1)
fi