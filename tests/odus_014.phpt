--TEST--
Check for od_serialize / od_unserialize -- simple values 
--SKIPIF--
<?php if (!extension_loaded("odus")) print "skip"; ?>
--FILE--
<?php 

class AClass
{
	public $empty = "";
	public $bfalse;
	public $btrue;
	public $lval;
	public $fval;

	public function __construct() {
		$this->bfalse = 0;
		$this->btrue = 1;
		$this->lval = intval(12345);
		$this->fval = floatval(12345.567);
	}
}

function show($msg) {
	echo $msg."\n";
}

$data=new ODWrapper(od_serialize(new AClass()));

show($data->fval);
show($data->lval);
show($data->btrue);
show($data->bfalse);
show($data->empty);

?>
--EXPECT--
12345.567
12345
1
0

