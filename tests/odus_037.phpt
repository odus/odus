--TEST--
Check for value len compress.
--SKIPIF--
<?php if (!extension_loaded("odus")) print "skip"; ?>
--INI--
odus.format_version=2
--FILE--
<?php

class AClass
{
	public $arr;

	public function __construct() {
		$this->arr = array();
	}
}


/* 1 Byte value len. */

$a = new AClass();

$a->arr["key0"] = "value0";

$a1 = new ODWrapper(od_serialize($a));

$r1 = od_serialize($a1);

$a2 = new ODWrapper($r1);

echo $a2->arr["key0"];
echo "\n";

$d = od_unserialize($r1);

$r2 = od_serialize($d);

$d2 = od_unserialize($r2);

echo $d==$d2?"same":"diff";
echo "\n";



/* 2 Bytes value len. */

$a = new AClass();

for($i = 0; $i < 100; $i++) {
	$a->arr["key" . $i] = "value" . $i;
}

$a1 = new ODWrapper(od_serialize($a));

$r1 = od_serialize($a1);

$a2 = new ODWrapper($r1);

echo $a2->arr["key99"];
echo "\n";

$d = od_unserialize($r1);

$r2 = od_serialize($d);

$d2 = od_unserialize($r2);

echo $d==$d2?"same":"diff";
echo "\n";



/* 4 Bytes value len. */

$a = new AClass();

for($i = 0; $i < 1000; $i++) {
	$a->arr["key" . $i] = "value" . $i;
}

$a1 = new ODWrapper(od_serialize($a));

$r1 = od_serialize($a1);

$a2 = new ODWrapper($r1);

echo $a2->arr["key999"];
echo "\n";

$d = od_unserialize($r1);

$r2 = od_serialize($d);

$d2 = od_unserialize($r2);

echo $d==$d2?"same":"diff";
echo "\n";



?>
--EXPECT--
value0
same
value99
same
value999
same
