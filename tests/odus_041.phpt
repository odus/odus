--TEST--
Test for od_get_mangled_name
--SKIPIF--
<?php if (!extension_loaded("odus")) print "skip"; ?>
--INI--
odus.format_version=2
--FILE--
<?php

class A
{
    private $pri_name = "pri_value";
    protected $pro_name = "pro_value";
    public $pub_name = "public_value";

    public function getPrivateName() {
        $name = od_get_mangled_name($this, "pri_name");
        return $name;
    }

    public function getProtectedName() {
        $name = od_get_mangled_name($this, "pro_name");
        return $name;
    }

    public function getPublicName() {
        $name = od_get_mangled_name($this, "pub_name");
        return $name;
    }
}

$a = new A();

echo $a->getPrivateName() . "\n";

echo $a->getProtectedName() . "\n";

echo $a->getPublicName() . "\n";

?>
--EXPECT--
\0A\0pri_name
\0*\0pro_name
pub_name
