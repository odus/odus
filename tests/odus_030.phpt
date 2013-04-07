--TEST--
Test od_refresh_odwrapper()
--SKIPIF--
<?php if (!extension_loaded("odus")) print "skip"; ?>
--FILE--
<?php 

function show($msg) {
	echo $msg . "\n";
}

class World
{
	public $v = 1;
	public $objects = null;
	public $customRides = array('zzzzzzz');
	public $element = null;
}

class WorldElement
{
	public $v1 = 1;
	public $v2 = 2;
	public $v3 = 3;
	public $v4 = 4;
}

$w1 = new World();
$we1 = new WorldElement();
$w1->element = $we1;
$data1 = new ODWrapper(od_serialize($w1));
$data1->element->v3 = 5;
show($data1->element->v3);

$w2 = new World();
$we2 = new WorldElement();
$w2->element = $we1;
$data2 = new ODWrapper(od_serialize($w2));
$data2->element->v3 = 6;
show($data2->element->v3);

if (od_refresh_odwrapper($data1, $data2))
{
show("refresh success");
}
show($data1->element->v3);
show($data1->element->v3 == 3 ? "yes" : "no");

$data3 =  new ODWrapper(od_serialize($data2));
show($data3->element->v3);

od_refresh_odwrapper($data1, $data3);
show($data1->element->v3);
show($data1->element->v3 == 6 ? "yes" : "no");

if (od_refresh_odwrapper($w1, $w2)) {
show("refresh success");
} else {
show("refresh fail");
}

if (od_refresh_odwrapper(123, 'abc')) {
show("refresh success");
} else {
show("refresh fail");
}

--EXPECT--
5
6
refresh success
3
yes
6
6
yes
refresh fail
refresh fail