--TEST--
Verify od_wrapper_get_property_ptr_ptr() bug fix, part 2
--SKIPIF--
<?php if (!extension_loaded("odus")) print "skip"; ?>
--FILE--
<?php 

class World
{
	public $v = 1;
	public $objects = null;
	public $customRides = array('zzzzzzz');

	public function __get($fld) {
		return null;
	}

}

$w = new World();
$data = new ODWrapper(od_serialize($w));

// This invokes od_wrapper_get_property_ptr_ptr().  Another variation of the
// od_wrapper_get_property_ptr_ptr bug.
//
// The Ville worked around this bug by accessing $data->customRides first,
// like so:
//
// $data->customRides;
$data->customRides[] = 'abcdefghijk';

$s1 = od_serialize($data);
echo strpos($s1, "abcdefghijk") > 0 ? "yes" : "no";
echo "\n";
echo strpos($s1, "zzzzzzz") > 0 ? "yes" : "no";
echo "\n";
--EXPECT--
yes
yes
