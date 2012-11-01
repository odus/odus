--TEST--
Check that odus.reduce_fatals=1 prevents PHP Fatal and causes od_serialize to return NULL on failure
--SKIPIF--
<?php if (!extension_loaded("odus")) print "skip"; ?>
--INI--
odus.reduce_fatals=1
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

$result = od_serialize($arr);

if ($result == NULL) {
   show("NULL");
}
else {
   show("NONNULL");
}

?>
--EXPECT--
NULL
