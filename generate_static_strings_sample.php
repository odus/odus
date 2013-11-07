<?php

/* This is sample code for generating the static string table.
 * Note:
 * To extract protected/private properties, you should either copy the
 * code into your class, or replace get_object_vars with your in-class
 * function to get full list of properties.
 */

private var $static_strings = array();
private function addString($str) {
	if (isset($strings[$str])) {
		$static_strings[$str] += 1;
	} else {
		$static_strings[$str] = 1;
	}
}

private function extractStrings($object) {
	$properties = get_object_vars($object);
	foreach ($properties as $key => $value) {
		if (is_string($key)) {
			addString(od_get_mangled_name($key));
		}
		extractStrings($value);
	}

	if (is_object($object)) {
		$this->getOneString($strings, get_class($object));
		// This will acctually only get public properties
		$properties = get_object_vars($object);
		foreach ($properties as $key => $value) {
			addString($key);
			extractStrings($value);
		}
	} else if (is_array($object)) {
		foreach ($object as $key => $value) {
			if (is_string($key)) {
				addString($key);
			}
			extractStrings($value);
		}
	} else if (is_string($object)) {
		addString($key);
	}
}

?>