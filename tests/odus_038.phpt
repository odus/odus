--TEST--
Check for assignment across object.
--SKIPIF--
<?php if (!extension_loaded("odus")) print "skip"; ?>
--INI--
odus.format_version=2
--FILE--
<?php 

class Storage
{
	public $val;
	public $position;

	public function __construct() {
		$this->position = array();
	}
}

class BClass
{
	public $id = "123";
}

class CClass
{
	public $x;

}

class DClass
{
	public $y;

}

$c = new CClass();
$d = new DClass();

$a = new Storage();
$a->position["y"] = 1;
$d->y = $a;


$c1 = new ODWrapper(od_serialize($c));
$d1 = new ODWrapper(od_serialize($d));

$a1 = $d1->y;
$c1->x = $a1;

// To trigger get_properties
od_serialize($c1);


$d2 = new ODWrapper(od_serialize($d1));

$d2->y = $c1->x;

$d2->y->position["y"] = 2;
$d2->y->val = NULL;


$r2 = od_serialize($d2);

$d3 = new ODWrapper($r2);

echo $d3->y->position["y"];
echo "\n";

//print_r($d2);
?>
--EXPECT--
2

