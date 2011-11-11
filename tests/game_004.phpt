--TEST--
User blob -- check user world objects position 
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

$x=0;
$y=0;

foreach($user->world->objects as $obj) {
	$x+=$obj->position['x'];
	$y+=$obj->position['y'];
}

show($x);
show($y);

file_put_contents("$dir/user",od_serialize($user));

//od_unserialize(file_get_contents("$dir/user"));
--EXPECT--
1204
16670
28540
