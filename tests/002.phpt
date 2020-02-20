--TEST--
study_php_extension_test1() Basic test
--SKIPIF--
<?php
if (!extension_loaded('study_php_extension')) {
	echo 'skip';
}
?>
--FILE--
<?php
$ret = study_php_extension_test1();

var_dump($ret);
?>
--EXPECT--
The extension study_php_extension is loaded and working!
NULL
