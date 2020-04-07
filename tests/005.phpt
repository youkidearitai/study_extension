--TEST--
Check closure study_extension_print_backtrace 
--SKIPIF--
<?php
if (!extension_loaded('study_extension')) {
	echo 'skip';
}
?>
--FILE--
<?php
if (!extension_loaded("study_extension")) { exit("no study extension"); }

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

preg_replace_callback("/a/", function () { x(); }, 'a');
preg_replace_callback("/a/", function () { $b = function () { y::bb(); }; $b(); }, 'a');

x();

?>
--EXPECTF--
function: sab	filename: %s:%d

function: sab	filename: %s:%d
function: y->x	filename: %s:%d

function: sab	filename: %s:%d
function: y::b	filename: %s:%d
function: y::bb	filename: %s:%d

function: x	filename: %s:%d
function: {closure}	
function: preg_replace_callback	filename: %s:%d

function: sab	filename: %s:%d
function: y::b	filename: %s:%d
function: y::bb	filename: %s:%d
function: {closure}	filename: %s:%d
function: {closure}	
function: preg_replace_callback	filename: %s:%d

function: x	filename: %s:%d
