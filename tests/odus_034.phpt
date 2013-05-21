--TEST--
Check for unset / od_reserialize with compact strings enabled.
--SKIPIF--
<?php if (!extension_loaded("odus")) print "skip"; ?>
--INI--
odus.format_version=2
--FILE--
<?php 

class AClass
{
	public $name1 = 'value';
}


$data=new ODWrapper(od_serialize(new AClass()));

echo isset($data->name)?"yes":"no";

echo "\n";

unset($data->name);

echo isset($data->a->name)?"yes":"no";

echo "\n";

$r= od_serialize($data);

// Strings will stay after unset.
echo strpos($r,'name')>0?"yes":"no";

echo "\n";

$r= od_reserialize($data);

// Strings will stay after unset.
echo strpos($r,'name')>0?"yes":"no";

echo "\n";

$r1 = od_serialize($data);

$d = od_unserialize($r1);

$r2 = od_serialize($d);


$d2 = od_unserialize($r2);

echo $d==$d2?"same":"diff";


?>
--EXPECT--
yes
no
yes
no
same

