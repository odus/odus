--TEST--
User blob -- read and wirte simple properties
--SKIPIF--
<?php if (!extension_loaded("odus")) print "skip"; ?>
--FILE--
<?php 

function show($msg) {
	echo $msg . "\n";
}

$dir=dirname(__FILE__);

system("cp $dir/od_user $dir/user");

require_once("$dir/env.inc.php");

$data = file_get_contents("$dir/user");

$user = new ODWrapper($data);

show($user->id);
show($user->player->xp);
show($user->player->gold);
show($user->player->energy);

$user->id = 1314;
$user->player->xp=234;
$user->player->gold=567;
$user->player->energy=7788;

file_put_contents("$dir/user",od_serialize($user));

//od_unserialize(file_get_contents("$dir/user"));
?>
--EXPECT--
31874718299
42444
1128926
27
