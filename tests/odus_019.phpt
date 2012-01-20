--TEST--
Check for repeat unset and set
--SKIPIF--
<?php if (!extension_loaded("odus")) print "skip"; ?>
--FILE--
<?php 

function show($msg) {
	echo $msg . "\n";
}

class A
{
	public $a = "value";
}

$o = new ODWrapper(od_serialize(new A()));

unset($o->a);

$o->a = "bad";

unset($o->a);

$r = od_serialize($o);

echo strpos($r,'value')>0?"yes":"no";

?>
--EXPECT--
no
