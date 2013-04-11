--TEST--
Test od_getobjectkeys_without_classname()
--SKIPIF--
<?php if (!extension_loaded("odus")) print "skip"; ?>
--FILE--
<?php 

function show($msg) {
	echo $msg . "\n";
}

function judgeEqual($key1,$key2)
{
	if(array_diff($key1,$key2) || array_diff($key2,$key1)){
		return false;
	}else{
		return true;
	}
}

class World
{
	public $v = 1;
	public $objects = null;
	public $customRides = array('zzzzzzz');
	public $element = null;
	protected $prot = "prot";
	private $priv = "priv";
	public $className = "World";

	public function callGetKeys() {
		$ps = get_object_vars($this);
		unset($ps["className"]);
		$keys1 = array_keys($ps);
		return $keys1;
	}

	public function callOdGetKeys() {
		$keys = od_getobjectkeys_without_classname($this);
		return $keys;
	}
}

class WorldElement
{
	public $v1 = 1;
	public $v2 = 2;
	public $v3 = 3;
	public $v4 = 4;
}

$w1 = new World();
$we1 = new WorldElement();
$w1->element = $we1;
$data1 = new ODWrapper(od_serialize($w1));

$data1->someStr = "someStr";
$data1->someStr2 = "someStr2";

$keys = od_getobjectkeys_without_classname($w1);
show(empty($keys)?"not support":"support");

$keys = od_getobjectkeys_without_classname($data1);
show(empty($keys)?"not support":"support");
show(print_r($keys, true));
$ps = get_object_vars($data1);
unset($ps["className"]);
$keys1 = array_keys($ps);
show(print_r($keys1, true));
show(judgeEqual($keys1, $keys)?"equal":"not equal");

$keys = od_getobjectkeys_without_classname("1234");
show(empty($keys)?"not support":"support");

for ($i=0;$i<3;$i++) {

$keys = od_getobjectkeys_without_classname($data1->element);
show(empty($keys)?"not support":"support");
//show(print_r($keys, true));
$ps = get_object_vars($data1->element);
unset($ps["className"]);
$keys1 = array_keys($ps);
//show(print_r($keys1, true));
show(judgeEqual($keys1, $keys)?"equal":"not equal");

}

$keys1 = $data1->callGetKeys();
$keys = $data1->callOdGetKeys();
show(print_r($keys1, true));
show(judgeEqual($keys1, $keys)?"equal":"not equal");

?>

--EXPECT--
not support
support
Array
(
    [0] => v
    [1] => objects
    [2] => customRides
    [3] => element
    [4] => someStr2
    [5] => someStr
)

Array
(
    [0] => v
    [1] => objects
    [2] => customRides
    [3] => element
    [4] => someStr2
    [5] => someStr
)

equal
not support
support
equal
support
equal
support
equal
Array
(
    [0] => v
    [1] => objects
    [2] => customRides
    [3] => element
    [4] => prot
    [5] => priv
    [6] => someStr2
    [7] => someStr
)

equal

