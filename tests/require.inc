<?php

function require_x_builtin() {
	debug_print_backtrace();
}

function require_x_study_extension() {
	study_extension_print_backtrace();
    echo PHP_EOL;
}

require_x_study_extension();
