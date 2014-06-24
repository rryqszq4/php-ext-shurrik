<?php 



require_once("test_shurrik.php");

function test_aaa(){
	$abc = 123;
	return $abc;
}

function test_b($a,$b){
	return $a+$b;
}
echo test_b(1,2);

test_aaa();
test_aaa();

$foo = 'test';
print_r($foo);

?>
