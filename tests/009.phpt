--TEST--
Check new study_standard_class
--SKIPIF--
<?php
if (!extension_loaded('study_extension')) {
	echo 'skip';
}
?>
--FILE--
<?php
study_extension_dump(new study_standard_class());
?>
--EXPECTF--
OBJECT(study_standard_class)#1 (0) {
  ["name"]=>
  uninitialized(string)
  ["number"]=>
  uninitialized(int)
}
