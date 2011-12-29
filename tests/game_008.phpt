--TEST--
User blob -- memory test 
--SKIPIF--
<?php if (!extension_loaded("odus")) print "skip"; ?>
--FILE--
<?php 

error_reporting(E_ERROR);

$dir=dirname(__FILE__);

require_once("$dir/env.inc.php");
$r=file_get_contents("$dir/od_user");

$user = new ODWrapper($r);

foreach($user->world->objects as $obj) {
	$obj->position['x']+=1;
	$obj->position['y']+=1;
}

serialize($user);

od_serialize($user);

od_unserialize($r);

echo "good\n";
?>
--EXPECT--
good
