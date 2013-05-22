--TEST--
User blob -- migration test, write as 1.0 format
--SKIPIF--
<?php if (!extension_loaded("odus")) print "skip"; ?>
--INI--
odus.format_version=1
--FILE--
<?php 

error_reporting(E_ERROR);

$dir=dirname(__FILE__);

require_once("$dir/env.inc.php");
$r=file_get_contents("$dir/od_user");

$user = new ODWrapper($r);

$r2 = od_serialize($user);

echo strcmp($r, $r2) == 0 ? "same":"diff";

?>
--EXPECT--
same
