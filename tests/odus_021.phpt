--TEST--
Check for ODWrapper removing default properties 
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
	public $sval='svalue';
	public $aval=array('abcde');
}

$o = new ODWrapper(od_serialize(new AClass()));

show($o->ival);
show($o->sval);
show($o->dval);
$o->dval = 123.456;
unset($o->nval);
unset($o->ival);

$r = od_serialize($o);

has($r,'nval');
has($r,'ival');
has($r,'dval');
has($r,'sval');
has($r,'aval');

?>
--EXPECT--
123
svalue
123.456
no
no
yes
no
yes
