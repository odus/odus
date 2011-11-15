--TEST--
Check for od_serialize / od_unserialize -- isset
--SKIPIF--
<?php if (!extension_loaded("odus")) print "skip"; ?>
--FILE--
<?php 

function show($msg) {
	echo $msg."\n";
}

class AClass
{
	public    $v1 = 'val1';
}


$dir=dirname(__FILE__);

$a = new AClass();
$r = od_serialize($a);

$data=new ODWrapper($r);

if(isset($data->v1)) {
	$data->v1 = "newval";
}

$nr = od_serialize($data);

show(strpos($nr,"newval")>0?"yes":"no");

?>
--EXPECT--
yes
