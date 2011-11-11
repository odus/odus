--TEST--
Check for od_serialize / od_unserialize -- add new
--SKIPIF--
<?php if (!extension_loaded("odus")) print "skip"; ?>
--FILE--
<?php 

class AClass
{
	public $pval;
}

$data=new ODWrapper(od_serialize(new AClass()));

$data->newpval = "newpval";

echo $data->newpval . "\n";

$r = od_serialize($data);

echo strpos($r,$data->newpval)>0 ? "yes":"no";

echo "\n";

$r1 = od_serialize($data);

$d = od_unserialize($r1);

$r2 = od_serialize($d);

$d2 = od_unserialize($r2);

echo $d==$d2?"same":"diff";


?>
--EXPECT--
newpval
yes
same
