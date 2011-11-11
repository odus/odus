--TEST--
Check for od_serialize / od_unserialize -- set 
--SKIPIF--
<?php if (!extension_loaded("odus")) print "skip"; ?>
--FILE--
<?php 

class AClass
{
	public $pval='val';

	public $arr;

	public function __set($key,$val) {
		$this->arr =array();
		$this->arr[$key]=$val;
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

$data->a->pval='newp';

$data->a->noval='noval';

echo $data->a->pval . "\n";
echo $data->a->arr['noval'] . "\n";

$r1 = od_serialize($data);

$d = od_unserialize($r1);

$r2 = od_serialize($d);

$d2 = od_unserialize($r2);

echo $d==$d2?"same":"diff";



?>
--EXPECT--
newp
noval
same
