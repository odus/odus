--TEST--
Check for od_serialize / od_unserialize -- sleep 
--SKIPIF--
<?php if (!extension_loaded("odus")) print "skip"; ?>
--INI--
odus.remove_default=0
--FILE--
<?php 

class AClass
{
	public $pval='pvalue';

	public $arr;

	public function __sleep() {
		global $flag;
		return $flag?array('arr'):array('pval','arr');
	}
}

class BClass
{
	public $a;

	public function __construct() {
		$this->a = new AClass();
	}
}

$flag = 0;

$data=new ODWrapper(od_serialize(new BClass()));

$flag = 1;

$r = od_serialize($data);

echo strpos($r,"pvalue")>0?"yes":"no";

echo "\n";

$data->a->arr['noval']='noval';
$data->a->pval='newpvalue';

$r = od_serialize($data);

echo strpos($r,"newpvalue")>0?"yes":"no";

echo "\n";

echo strpos($r,"arr")>0?"yes":"no";

echo "\n";

echo strpos($r,"noval")>0?"yes":"no";

echo "\n";

$data->a->noval = 'anoval';

echo $data->a->noval . "\n";

$r = od_serialize($data);

echo strpos($r,"anoval")>0?"yes":"no";

?>
--EXPECT--
yes
no
yes
yes
anoval
no
