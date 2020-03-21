<?php

function include_x_builtin() {
	debug_print_backtrace();
}

function include_x_study_extension() {
	study_extension_print_backtrace();
    echo PHP_EOL;
}

include_x_study_extension();
//include_x_builtin();
