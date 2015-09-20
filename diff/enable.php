<?php
function bar($x) {
  if ($x > 0) {
    bar($x - 1);
  }
}
function foo() {
  for ($idx = 0; $idx < 5; $idx++) {
    bar($idx);
    $x = strlen("abc");
  }
}
xhprof_enable(XHPROF_FLAGS_CPU | XHPROF_FLAGS_MEMORY);
include "lib.php";
$a = '$a="bbb\n";';
echo strlen($a);
echo str_replace($a,'bbb','aaa');
eval($a);
echo $a;
foo();
$xhprof_data = xhprof_disable();
array2file($xhprof_data,__DIR__.'/'.PHP_VERSION.'-enable-'.date('His').'.out');
