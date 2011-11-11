--TEST--
User blob -- modify all of user world objects 
--SKIPIF--
<?php if (!extension_loaded("odus")) print "skip"; ?>
--FILE--
<?php 

error_reporting(E_NOTICE || E_ERROR);

function show($msg) {
	echo $msg . "\n";
}

$dir=dirname(__FILE__);

require_once("$dir/env.inc.php");

$data = file_get_contents("$dir/user");

$user = new ODWrapper($data);

$objects = $user->world->objects;

foreach($objects as $obj) {
	$obj->position['x']=12;
	$obj->position['y']=12;
}

show($objects['w110']->position['x']);
show($objects['w110']->position['y']);

file_put_contents("$dir/user",od_serialize($user));

//od_unserialize(file_get_contents("$dir/user"));
--EXPECT--
12
12
