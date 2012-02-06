--TEST--
Check for od_serialize removing default properties 
--SKIPIF--
<?php if (!extension_loaded("odus")) print "skip"; ?>
--INI--
odus.remove_default=1
--FILE--
<?php 

function show($msg) {
	echo "$msg\n";
}

function show_bool($b) {
	show($b?"yes":"no");
}

function has($s,$v) {
	show_bool(strpos($s,$v)>0);
}

class AClass
{
	public $nval;
	public $ival=123;
	public $dval=123.456;
	public $sval='pvalue';
	public $aval=array('abcde');
}

$r = od_serialize(new AClass());

has($r,'nval');
has($r,'ival');
has($r,'dval');
has($r,'sval');
has($r,'aval');

?>
--EXPECT--
no
no
no
no
yes
