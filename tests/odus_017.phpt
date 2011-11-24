--TEST--
Check for od_serialize / od_unserialize -- isset
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
		$this->a[] = new B();
	}
}

$o = new ODWrapper(od_serialize(new A()));

$v = $o->a[0];

show(in_array($v,$o->a)?"yes":"no");

?>
--EXPECT--
yes
