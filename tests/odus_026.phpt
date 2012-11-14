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


$o = new stdClass;
$o->n = "only_one_string";
$o->r = &$o->n;

try {
    $result = od_serialize($o);
}
catch (Exception $e) {
   show("Exception caught");
}

?>
--EXPECT--
Exception caught
