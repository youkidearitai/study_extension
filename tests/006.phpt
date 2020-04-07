--TEST--
Check error handler study_extension_print_backtrace 
--SKIPIF--
<?php
if (!extension_loaded('study_extension')) {
	echo 'skip';
}
?>
--FILE--
<?php
if (!extension_loaded("study_extension")) { exit("no study extension"); }
error_reporting(E_ALL);

set_error_handler("study_error_handler");
ini_set("display_errors", "on");

function study_error_handler($errorcode, $errorstring, $filename, $line, $symbols) {
	error_handler();
}

function error_handler() {
	xx_error_handler();
}

function xx_error_handler() {
	study_extension_print_backtrace(); echo PHP_EOL;
}

$a = $aa;

?>
--EXPECTF--
function: xx_error_handler	filename: %s:%d
function: error_handler	filename: %s:%d
function: study_error_handler	filename: %s:%d
