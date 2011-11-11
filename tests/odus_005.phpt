--TEST--
Check for od_serialize / od_unserialize -- read
--SKIPIF--
<?php if (!extension_loaded("odus")) print "skip"; ?>
--FILE--
<?php 

class AClass
{
	public $pval;

	public function __construct() {
		$this->pval = 'original';
	}
}

$data=new ODWrapper(od_serialize(new AClass()));

echo get_class($data) . "\n";
echo $data->pval . "\n";

$r1 = od_serialize($data);

$d = od_unserialize($r1);

$r2 = od_serialize($d);

$d2 = od_unserialize($r2);

echo $d==$d2?"same":"diff";



?>
--EXPECT--
AClass
original
same
