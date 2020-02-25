--TEST--
Check if study_extension is loaded
--SKIPIF--
<?php
if (!extension_loaded('study_extension')) {
	echo 'skip';
}
?>
--FILE--
<?php
echo 'The extension "study_extension" is available';
?>
--EXPECT--
The extension "study_extension" is available
