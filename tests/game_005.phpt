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

unset($user->world->objects['w109']);

$objects = $user->world->objects;

$objects['w110']->position['x']=123;
$objects['w110']->position['y']=234;

show($user->world->objects['w110']->position['x']);
show($user->world->objects['w110']->position['y']);

file_put_contents("$dir/user",od_serialize($user));

//od_unserialize(file_get_contents("$dir/user"));
--EXPECT--
123
234
