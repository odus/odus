--TEST--
Check that circular references don't trigger segfault in od_serialize
--SKIPIF--
<?php if (!extension_loaded("odus")) print "skip"; ?>
--FILE--
<?php 

function show($msg) {
	echo "$msg\n";
}

class AClass
{
	public $aval1 = 1;
	public $bclass;
	protected $aval2 = 2;
	private $aval3 = 3;

	public function __wakeup() {
		$this->bclass->setAClass($this);
	}
}

class BClass
{
	public $bval1 = 1;
	private $aclass;
	protected $bval2 = 2;
	private $bval3 = 3;

	public function setAClass($aclass) {
		$this->aclass = $aclass;
	}

	public function __sleep() {
		return array('bval1', 'bval2', 'bval3');

	}
}

$aclass = new AClass();
$bclass = new BClass();
$aclass->bclass = $bclass;
$bclass->setAClass($aclass);

$o1 = od_serialize($aclass);
if ($o1 == NULL) {
	show("NULL");
}
else {
	show("NON-NULL");
}
$d1 = new ODWrapper($o1);
$d1->aval1 = 2;
$d1->bclass->bval1 = 2;

// This used to go into an infinite loop and segfault.
$o2 = od_serialize($d1);
if ($o2 == NULL) {
	show("NULL");
}
else {
	show("NON-NULL");
}

?>
--EXPECT--
NON-NULL
NON-NULL
