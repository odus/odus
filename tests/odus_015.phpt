--TEST--
Check for od_serialize / od_unserialize -- default properties
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
	public    $v2 = 'val2';
	protected $v3 = 'val3';
	private   $v4 = 'val4';
	public    $v5 = 'val5';

	public function say() {
		show($this->v5);
		show($this->v2);
		show($this->v3);
		show($this->v4);
		show($this->v1);
	}
}


$dir=dirname(__FILE__);
$data=new ODWrapper(file_get_contents("$dir/empty_class"));

$data->say();

?>
--EXPECT--
val5
val2
val3
val4
val1
