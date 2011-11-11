--TEST--
Check for od_serialize / od_unserialize
--SKIPIF--
<?php if (!extension_loaded("odus")) print "skip"; ?>
--FILE--
<?php 

$data = "test od_serialize and od_unserialize";

$r1 = od_serialize($data);

$d = od_unserialize($r1);

$r2 = od_serialize($d);

echo $r1==$r2?"same":"diff";

?>
--EXPECT--
same

