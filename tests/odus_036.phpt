--TEST--
Check for static strings compact.
--SKIPIF--
<?php if (!extension_loaded("odus")) print "skip"; ?>
--INI--
odus.format_version=2
--FILE--
<?php 

class AClass
{
	// "x" is in static string table.
	public $name = "x";
}


$data = new ODWrapper(od_serialize(new AClass()));

$r = od_serialize($data);

$data2 = new ODWrapper($r);

echo $data2->name;
echo "\n";


$r1 = od_serialize($data);

$d = od_unserialize($r1);

$r2 = od_serialize($d);


$d2 = od_unserialize($r2);

echo $d==$d2?"same":"diff";


?>
--EXPECT--
x
same

