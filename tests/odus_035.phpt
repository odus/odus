--TEST--
Check for assignment across object.
--SKIPIF--
<?php if (!extension_loaded("odus")) print "skip"; ?>
--INI--
odus.format_version=2
--FILE--
<?php 

class AClass
{
	public $name = "value";
}

class BClass
{
	public $id = "123";
}

class CClass
{
	public $x;

	public function __construct() {
		$this->x = new AClass();
	}
}

class DClass
{
	public $y;

	public function __construct() {
		$this->y = new BClass();
	}
}

$c = new CClass();
$d = new DClass();

file_put_contents("odus.o", od_serialize($c));

$c1 = new ODWrapper(od_serialize($c));
$d1 = new ODWrapper(od_serialize($d));

echo $c1->x->name;
echo "\n";

$c1->x = $d1->y;

$r1 = od_serialize($c1);

$c2 = new ODWrapper($r1);

echo $c2->x->id;
echo "\n";

$d = od_unserialize($r1);

$r2 = od_serialize($d);


$d2 = od_unserialize($r2);

echo $d==$d2?"same":"diff";


?>
--EXPECT--
value
123
same

