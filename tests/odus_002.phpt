--TEST--
Check for od_overwrite_function
--SKIPIF--
<?php if (!extension_loaded("odus")) print "skip"; ?>
--FILE--
<?php 

function test1() {
	echo "I am test1\n";
}

function test2() {
	echo "I am test2\n";
}

test1();
od_overwrite_function('test1','test2');
test1();

?>
--EXPECT--
I am test1
I am test2

