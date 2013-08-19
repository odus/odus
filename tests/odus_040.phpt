--TEST--
Static string for protected/private property
--SKIPIF--
<?php if (!extension_loaded("odus")) print "skip"; ?>
--INI--
odus.format_version=2
odus.static_strings_file=static_strings_unit_test.ini
--FILE--
<?php

/* Use relative path for the static strings file, so just make sure you run in the main folder of ODUS. */

class A
{
    private $pri_name = "pri_value";
    protected $pro_name = "pro_value";
    public $pub_name = "pub_value";

    public function getProtected() {
    	return $this->pro_name;
    }

    public function setProtected($name) {
    	$this->pro_name = $name;
    }

    public function getPrivate() {
    	return $this->pri_name;
    }

    public function setPrivate($name) {
    	$this->pri_name = $name;
    }
}

$obj = new A();

$s = od_serialize($obj);

$o = new ODWrapper($s);

// protected
echo $o->getProtected() . "\n";

$o->setProtected("Hello Protected");

echo $o->getProtected() . "\n";

// private
echo $o->getPrivate() . "\n";

$o->setPrivate("Hello Private");

echo $o->getPrivate() . "\n";

?>
--EXPECT--
pro_value
Hello Protected
pri_value
Hello Private
