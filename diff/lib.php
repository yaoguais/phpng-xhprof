<?php

function a(){
	echo "a\n";
}

function b(){
	echo "b\n";
}

function array2file($arr,$file){
	$arrStr = var_export($arr,true);
	file_put_contents($file,$arrStr);
}
