--TEST--
Verify od_wrapper_get_property_ptr_ptr() bug fix
--SKIPIF--
<?php if (!extension_loaded("odus")) print "skip"; ?>
--FILE--
<?php 

class World
{
	public $v = 1;
	public $objects = null;
	public $customRides = null;

	public function __get($fld) {
		return null;
	}

}

function addCustomRides(&$storage, $rideId) {
	if ($storage == null) {
		$storage = array();
	}
	$storage[] = $rideId;
}

$w = new World();
$data = new ODWrapper(od_serialize($w));

// This invokes od_wrapper_get_property_ptr_ptr(). There was a bug in
// there that would get triggered if the property being accessed had
// not been unserialized (i.e., it was in the OD serialized string,
// but not in the OD property hash), and if the object being accessed
// had a __get() function defined. In this case, it did not unserialize
// the property from the OD serialized string, and just returned a null
// zval. This has since been fixed.
addCustomRides($data->customRides, 'abcdefghijk');

$s1 = od_serialize($data);
echo strpos($s1, "abcdefghijk") > 0 ? "yes" : "no";
--EXPECT--
yes
