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
$x = new study_standard_class();
$x->increment_number();
study_extension_dump($x);
$x->increment_number();
study_extension_dump($x);
$y = study_standard_class_create();
$y->increment_number();
study_extension_dump($y);
$y->increment_number();
study_extension_dump($y);
?>
--EXPECTF--
OBJECT(study_standard_class)#1 (1) {
  ["name"]=>
  uninitialized(string)
  ["number"]=>
  LONG: 1
}
OBJECT(study_standard_class)#1 (1) {
  ["name"]=>
  uninitialized(string)
  ["number"]=>
  LONG: 2
}
OBJECT(study_standard_class)#2 (2) {
  ["name"]=>
  STRING: value="hoge", length=4
  ["number"]=>
  LONG: 1000001
}
OBJECT(study_standard_class)#2 (2) {
  ["name"]=>
  STRING: value="hoge", length=4
  ["number"]=>
  LONG: 1000002
}
