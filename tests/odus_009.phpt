--TEST--
Check for od_serialize / od_unserialize -- unset
--SKIPIF--
<?php if (!extension_loaded("odus")) print "skip"; ?>
--FILE--
<?php 

class AClass
{
	public $pval='val';

	public function __unset($key) {
		echo "in __unset\n";
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

echo isset($data->a->pval)?"yes":"no";

echo "\n";

unset($data->a->pval);

echo isset($data->a->pval)?"yes":"no";

echo "\n";

$newval='newpval';

$data->a->pval = $newval;

echo isset($data->a->pval)?"yes":"no";

echo "\n";

$r= od_serialize($data);

echo strpos($r,$newval)>0?"yes":"no";

echo "\n";

unset($data->a->noval);

$r1 = od_serialize($data);

$d = od_unserialize($r1);

$r2 = od_serialize($d);


$d2 = od_unserialize($r2);

echo $d==$d2?"same":"diff";


?>
--EXPECT--
yes
no
yes
yes
in __unset
same

