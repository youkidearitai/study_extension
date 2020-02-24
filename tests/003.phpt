--TEST--
Check study_php_extension_dump recursion protection
--SKIPIF--
<?php
if (!extension_loaded('study_php_extension')) {
	echo 'skip';
}
?>
--FILE--
<?php
$a[] = &$a;
study_php_extension_dump($a);
$s = new stdClass();
$s->str = "abc";
$s->x = 10;
$k = $s;
$s->x = $k;
study_php_extension_dump($s);
?>
--EXPECT--
ARRAY(1) {
  [0]=>
  ARRAY(1) {
    [0]=>
    *RECURSION*
  }
}
OBJECT(stdClass)#1 (2) {
  ["str"]=>
  STRING: value="abc", length=3
  ["x"]=>
  *RECURSION*
}

