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
study_php_extension_dump(3.1415);
study_php_extension_dump(2.0);
study_php_extension_dump(-2.0);
study_php_extension_dump("Hello World");
study_php_extension_dump("");
study_php_extension_dump(fopen("/dev/null", "r"));
study_php_extension_dump(array());
study_php_extension_dump(array(1, 2, 3, 4, 5));
study_php_extension_dump(array(1, 2, array(3), array(4), 5));
study_php_extension_dump(new stdClass());
?>
--EXPECT--
NULL: null
BOOL: true
BOOL: false
LONG: 1
LONG: -30
DOUBLE: 3.1415000000000002
DOUBLE: 2.0
DOUBLE: -2.0
STRING: value="Hello World", length=11
STRING: value="", length=0
RESOURCE: id=5 type=stream
ARRAY(0) {
}
ARRAY(5) {
  [0]=>
  LONG: 1
  [1]=>
  LONG: 2
  [2]=>
  LONG: 3
  [3]=>
  LONG: 4
  [4]=>
  LONG: 5
}
ARRAY(5) {
  [0]=>
  LONG: 1
  [1]=>
  LONG: 2
  [2]=>
  ARRAY(1) {
    [0]=>
    LONG: 3
  }
  [3]=>
  ARRAY(1) {
    [0]=>
    LONG: 4
  }
  [4]=>
  LONG: 5
}
OBJECT(stdClass)#1 (0) {
}
