<?php
//shurrik_work();

/*foreach($GLOBALS as $k=>$v){
    if($k == 'GLOBALS'){
        #var_dump($v['_ENV']);
    }
}*/




//$test_array = array('c'=>'c','0','key'=>'value',array('abc'=>'haha','test'=>'test','efg',array('123',array("a"=>"b","c"=>"c"))));

function c(){
	echo 123;
}

class TestA{

	public function __construct(){
		$abc = '123';
		//$this->t();
	}

	public function t(){
		$abc = 123;
		//c();
	}

	static public function u(){
		$aaa = "234";
	}
}

$test_a = new TestA();
$test_a->t();
//TestA::u();

/*function test_b(){
	$test_bb = 123;
	return $test_bb;

}*/

/*function test_c($a = '123'){
	$b = 'abc';
	return $a + $b;
}*/


//test_b();
?>
