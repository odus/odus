--TEST--
Check for od_serialize / od_unserialize -- get 
--SKIPIF--
<?php if (!extension_loaded("odus")) print "skip"; ?>
--FILE--
<?php 

class AClass
{
	public $pval='val';

	public function __get($key) {
		return $key;
	}
}

class BClass
{
	public $a;

	public function __construct() {
		$this->a = new AClass();
	}
}

$data=new ODWrapper(od_serialize(new BClass()));

echo $data->a->pval . "\n";
echo $data->a->noval . "\n";

$r1 = od_serialize($data);

$d = od_unserialize($r1);

$r2 = od_serialize($d);

$d2 = od_unserialize($r2);

echo $d==$d2?"same":"diff";


?>
--EXPECT--
val
noval
same
