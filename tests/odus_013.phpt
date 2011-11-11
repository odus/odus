--TEST--
Check for od_serialize / od_unserialize -- wakeup
--SKIPIF--
<?php if (!extension_loaded("odus")) print "skip"; ?>
--FILE--
<?php 

class AClass
{
	public $pval='origpval';

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
	}

	public function __wakeup() {
		$this->a = new AClass();
	}
}

$data=new ODWrapper(od_serialize(new BClass()));

echo $data->a->pval . "\n";

$r = od_serialize($data);

echo strpos($r,'origpval')>0?"yes":"no";

?>
--EXPECT--
origpval
yes
