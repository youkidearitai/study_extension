--TEST--
Check study_extension_print_backtrace
--SKIPIF--
<?php
if (!extension_loaded('study_extension')) {
	echo 'skip';
}
?>
--FILE--
<?php

class y {
	function x() {
		sab();
	}

	static function b() {
		sab();
	}

	static function bb() {
		self::b();
	}
}

function x() { study_extension_print_backtrace(); echo PHP_EOL; }

function sab() { study_extension_print_backtrace(); echo PHP_EOL; }
sab();

(new y())->x();
y::bb();

x();

eval("y::bb();");

include __DIR__ . DIRECTORY_SEPARATOR . "include.inc";
require __DIR__ . DIRECTORY_SEPARATOR . "require.inc";

?>
--EXPECTF--
function: sab	filename: %s:%d

function: sab	filename: %s:%d
function: y->x	filename: %s:%d

function: sab	filename: %s:%d
function: y::b	filename: %s:%d
function: y::bb	filename: %s:%d

function: x	filename: %s:%d

function: sab	filename: %s:%d
function: y::b	filename: %s:%d
function: y::bb	filename: %s:%d
function: eval()	%d

function: include_x_study_extension	filename: %s:%d
function: include()	%d

function: require_x_study_extension	filename: %s:%d
function: require()	%d
