--TEST--
Check study_php_extension_dump function print value
--SKIPIF--
<?php
if (!extension_loaded('study_php_extension')) {
	echo 'skip';
}
?>
--FILE--
<?php
study_php_extension_dump(null);
study_php_extension_dump(true);
study_php_extension_dump(false);
study_php_extension_dump(1);
study_php_extension_dump(-30);
?>
--EXPECT--
NULL: null
BOOL: true
BOOL: false
LONG: 1
LONG: -30
