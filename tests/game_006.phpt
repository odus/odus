--TEST--
User blob -- unset one of user world objects 
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

show(count($user->world->objects));

$objects = $user->world->objects;

show($objects['w110']->position['x']);
show($objects['w110']->position['y']);

file_put_contents("$dir/user",od_serialize($user));

//od_unserialize(file_get_contents("$dir/user"));
--EXPECT--
1203
123
234
