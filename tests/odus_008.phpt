--TEST--
Check for od_serialize / od_unserialize -- isset
--SKIPIF--
<?php if (!extension_loaded("odus")) print "skip"; ?>
--FILE--
<?php 

class AClass
{
	public $pval='val';

	public function __isset($key) {
		echo "in __isset\n";
		return 0;
	}
}

$data=new ODWrapper(od_serialize(new AClass()));

echo isset($data->pval)?"yes":"no";

echo "\n";

echo isset($data->noval)?"yes":"no";

echo "\n";

$r1 = od_serialize($data);

$d = od_unserialize($r1);

$r2 = od_serialize($d);


$d2 = od_unserialize($r2);

echo $d==$d2?"same":"diff";



?>
--EXPECT--
yes
in __isset
no
same
