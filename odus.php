<?php

/*
  +----------------------------------------------------------------------+
  | See COPYING file for further copyright information                   |
  +----------------------------------------------------------------------+
  | Author: Pai Deng <pdeng@zynga.com>                                   |
  | See CREDITS for contributors                                         |
  +----------------------------------------------------------------------+
*/

function load() {
	$br = (php_sapi_name() == "cli")? "":"<br>";

	if(!extension_loaded('odus')) {
		dl('odus.' . PHP_SHLIB_SUFFIX);
	}
	$module = 'odus';
	$functions = get_extension_funcs($module);
	echo "Functions available in the test extension:$br\n";
	foreach($functions as $func) {
	    echo $func."$br\n";
	}
}

load();

function show($msg) {
	echo $msg."\n";
}

function test(&$val) {
	$val = "changeval";
}

class D {
	public $dval="dval";
}

class C {
	public $cval="IamCVal";
	public $d;

	public function __construct() {
		$this->d = new D();
	}

	public function __unset($key) {
		echo "class C: in user unset for key $key\n";
	}

	public function tell() {
		echo "in C: cval: ".$this->cval."\n";
	}
}

class B {
	private $ival;
	protected $fval;
	public $bval = "IamInB";

	public function __construct() {
		$this->ival=intval(12);
		$this->fval="12.3";
	}
}

class AClass {

	public $c;
	public $pval;

	public function __construct() {
		$this->c = new C();
		$this->pval = 'pval';
	}

	public function __get($key) {
		echo "in getter $key\n";
	}

	public function __unset($key) {
		echo "in user unset for key $key\n";
	}


	public function say() {
		echo "say pval: $this->pval\n";
	}
}

function hello($obj)
{
	show(serialize($obj));
}

system("rm /tmp/buffer");

$data=new ODWrapper(od_serialize(new AClass()));

$data->pval = "pval";

$r1 = od_serialize($data);

$d = od_unserialize($r1);

$r2 = od_serialize($d);

echo $r1==$r2?"same":"diff";

?>
