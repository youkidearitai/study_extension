--TEST--
study_php_extension_test2() Basic test
--SKIPIF--
<?php
if (!extension_loaded('study_php_extension')) {
	echo 'skip';
}
?>
--FILE--
<?php
var_dump(study_php_extension_test2());
var_dump(study_php_extension_test2('PHP'));
?>
--EXPECT--
string(11) "Hello World"
string(9) "Hello PHP"
