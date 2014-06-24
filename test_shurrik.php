<?php
//shurrik_work();

foreach($GLOBALS as $k=>$v){
    if($k == 'GLOBALS'){
        #var_dump($v['_ENV']);
    }
}




$test_array = array('c'=>'c','0','key'=>'value',array('abc'=>'haha','test'=>'test','efg',array('123',array("a"=>"b","c"=>"c"))));



class TestA{

	public function __construct(){
		$abc = 123;
		//echo $abc;
	}
}

$test_a = new TestA();

function test_b(){
	$test_bb = 123;
	return $test_bb;

}

function test_c($a = '123'){
	$b = 'abc';
	return $a + $b;
}


test_b();
?>
