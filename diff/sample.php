<?php
function bar($x) {
  if ($x > 0) {
    bar($x - 1);
  }
}
function foo() {
  $i = 0;
  for ($idx = 0; $idx < 5; $idx++) {
    bar($idx);
    $i += strlen($idx);
  }
  return $i;
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
		$ret = str_replace('a','a',$j > 500 ? 'b' : 'bbbb');
	}
}
echo $ret;
$xhprof_data = xhprof_sample_disable();
file_put_contents(__DIR__.'/'.PHP_VERSION.'-sample-'.date('His').'.out', var_export($xhprof_data,true));
