--TEST--
Check that odus.throw_exception=1/reduce_fatals=1 prevents PHP Fatal and causes od_serialize to throw an exception
--SKIPIF--
<?php if (!extension_loaded("odus")) print "skip"; ?>
--INI--
odus.reduce_fatals=1
odus.throw_exceptions=1
--FILE--
<?php 

function show($msg) {
	echo "$msg\n";
}

class Sharedata {
}
$shareddata = new Sharedata();

class AClass {
}

$arr = new AClass();
$arr->bloo = $shareddata;
$arr->blah = $shareddata;

try {
    $result = od_serialize($arr);
}
catch (Exception $e) {
   show("Exception caught");
}

?>
--EXPECT--
Exception caught
