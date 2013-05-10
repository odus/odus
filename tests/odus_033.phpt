--TEST--
Check for od_reserialize
--SKIPIF--
<?php if (!extension_loaded("odus")) print "skip"; ?>
--FILE--
<?php 

class AClass
{
	public $pval='val';
}

class BClass
{
	public $a;

	public function __construct() {
		$this->a = new AClass();
	}
}

$s=od_serialize(new BClass());

$data=new ODWrapper($s);

$r1 = od_reserialize($data);

$d = od_unserialize($r1);

$r2 = od_serialize($d);


$d2 = od_unserialize($r2);

echo $d==$d2?"same":"diff";

?>
--EXPECT--
same

