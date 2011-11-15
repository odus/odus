--TEST--
Check for odus presence
--SKIPIF--
<?php if (!extension_loaded("odus")) print "skip"; ?>
--FILE--
<?php 
function show($msg) {
	echo $msg . "\n";
}

show("odus extension is available");
$class="ODWrapper";show("class $class ".(class_exists($class)?"exists":"doesn't exist"));
$func="od_serialize";show("function $func ".(function_exists($func)?"exists":"doesn't exist"));
$func="od_unserialize";show("function $func ".(function_exists($func)?"exists":"doesn't exist"));
$func="od_version";show("function $func ".(function_exists($func)?"exists":"doesn't exist"));
$func="od_format_version";show("function $func ".(function_exists($func)?"exists":"doesn't exist"));
$func="od_format_match";show("function $func ".(function_exists($func)?"exists":"doesn't exist"));

show(od_format_match(od_serialize("test"))?"match":"mismatch");

?>
--EXPECT--
odus extension is available
class ODWrapper exists
function od_serialize exists
function od_unserialize exists
function od_version exists
function od_format_version exists
function od_format_match exists
match
