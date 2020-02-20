--TEST--
Check if study_php_extension is loaded
--SKIPIF--
<?php
if (!extension_loaded('study_php_extension')) {
	echo 'skip';
}
?>
--FILE--
<?php
echo 'The extension "study_php_extension" is available';
?>
--EXPECT--
The extension "study_php_extension" is available
