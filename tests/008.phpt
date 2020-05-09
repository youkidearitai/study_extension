--TEST--
Check study_extension_dump is_ref
--SKIPIF--
<?php
if (!extension_loaded('study_extension')) {
	echo 'skip';
}
?>
--FILE--
<?php
$array = [];
$x = 30;
$y = 3.3;
$z = "Hello World";
$xx = true;
$xy = false;
$xz = null;
$yx = tmpfile();
$yy = array();
$yz = new stdClass();
$array[1] = &$x;
$array[2] = &$y;
$array[3] = &$z;
$array[4] = &$xx;
$array[5] = &$xy;
$array[6] = &$xz;
$array[7] = &$yx;
$array[8] = &$yy;
$array[9] = &$yz;
study_extension_dump($array);
?>
--EXPECTF--
ARRAY(9) {
  [1]=>
  &LONG: 30
  [2]=>
  &DOUBLE: 3.3
  [3]=>
  &STRING: value="Hello World", length=11
  [4]=>
  &BOOL: true
  [5]=>
  &BOOL: false
  [6]=>
  &NULL: null
  [7]=>
  &RESOURCE: id=%d type=%s
  [8]=>
  &ARRAY(0) {
  }
  [9]=>
  &OBJECT(stdClass)#%d (0) {
  }
}
