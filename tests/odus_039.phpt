--TEST--
Check for assignment across object.
--SKIPIF--
<?php if (!extension_loaded("odus")) print "skip"; ?>
--FILE--
<?php 

class Storage
{
	public $val;
	public $position;

	public function __construct() {
		$this->position = array();
	}
}
$a = new Storage();
$a->position["y"] = 1;

$a1 = new ODWrapper(od_serialize($a));

echo od_is_wrapper($a1) ? "true" : "false";
echo "\n";
echo od_is_wrapper($a) ? "true" : "false";
echo "\n";

//print_r($d2);
?>
--EXPECT--
true
false
