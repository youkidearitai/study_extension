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
study_extension_dump(study_standard_class_create());
?>
--EXPECTF--
OBJECT(study_standard_class)#1 (0) {
  ["name"]=>
  uninitialized(string)
  ["number"]=>
  uninitialized(int)
}
OBJECT(study_standard_class)#1 (2) {
  ["name"]=>
  STRING: value="hoge", length=4
  ["number"]=>
  LONG: 1000000
}
