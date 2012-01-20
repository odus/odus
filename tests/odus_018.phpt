--TEST--
Check for isset and array
--SKIPIF--
<?php if (!extension_loaded("odus")) print "skip"; ?>
--FILE--
<?php 

function show($msg) {
	echo $msg . "\n";
}

class B
{
	public $bval="bval";
}

class A
{
	public $a;

	public function __construct() {
		$this->a = array();
		$this->a[] = new B();
	}
}

$o = new ODWrapper(od_serialize(new A()));

if(isset($o->a)) {
	$o->a[0]->bval="newbval";
}

$r = od_serialize($o);

echo strpos($r,'newbval')>0?"yes":"no";

?>
--EXPECT--
yes
