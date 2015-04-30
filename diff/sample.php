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

xhprof_sample_enable();
$i = 0;
while($i<300){
	if(rand(0,99)>50){
		usleep(10000);
		++$i;
	}
	foo();
	bar(50);
	for($j=0;$j<1000;++$j){
		str_replace('a','a','b');
	}
}

$xhprof_data = xhprof_sample_disable();

include "lib.php";

array2file($xhprof_data,__DIR__.'/'.PHP_VERSION.'-sample-'.date('His').'.out');
