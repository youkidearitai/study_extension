--TEST--
Check arginfo study_extension_dump
--SKIPIF--
<?php
if (!extension_loaded('study_extension')) {
	echo 'skip';
}
?>
--FILE--
<?php
if (!extension_loaded("study_extension")) { exit("no study extension"); }

$refrections = new ReflectionFunction("study_extension_dump");
foreach ($refrections->getParameters() as $refrection) {
    echo $refrection . PHP_EOL;
}
?>
--EXPECTF--
Parameter #0 [ <required> ...$vars ]
