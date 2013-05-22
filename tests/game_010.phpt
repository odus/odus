--TEST--
User blob -- migration test, write as 2.0 format
--SKIPIF--
<?php if (!extension_loaded("odus")) print "skip"; ?>
--INI--
odus.format_version=2
--FILE--
<?php 

error_reporting(E_ERROR);

$dir=dirname(__FILE__);

require_once("$dir/env.inc.php");
$r=file_get_contents("$dir/od_user");

$user = new ODWrapper($r);

$r1 = od_serialize($user);

$d = od_unserialize($r1);

$r2 = od_serialize($d);

$d2 = od_unserialize($r2);

echo $d==$d2?"same":"diff";

?>
--EXPECT--
same
