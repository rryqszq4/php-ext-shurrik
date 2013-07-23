<?php
//shurrik_work();

foreach($GLOBALS as $k=>$v){
    if($k == 'GLOBALS'){
        var_dump($v['_ENV']);
    }
}


$test_array = array('c'=>'c','0','key'=>'value',array('abc'=>'haha','test'=>'test','efg',array('123',array("a"=>"b","c"=>"c"))));
