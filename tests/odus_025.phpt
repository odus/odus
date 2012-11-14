--TEST--
Check that odus.reduce_fatals=1 prevents PHP Fatal and causes od_serialize to return NULL on failure
--SKIPIF--
<?php if (!extension_loaded("odus")) print "skip"; ?>
--INI--
odus.reduce_fatals=1
odus.throw_exceptions=0
--FILE--
<?php 

function show($msg) {
	echo "$msg\n";
}

$o = new stdClass;
$o->n = "only_one_string";
$o->r = &$o->n;

$result = od_serialize($o);

if ($result == NULL) {
   show("NULL");
}
else {
   show("NONNULL");
}

?>
--EXPECT--
NULL
