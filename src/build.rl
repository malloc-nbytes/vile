#!/usr/local/bin/earl -xe

module Build

import "std/system.rl"; as sys
import "std/datatypes/list.rl";

$"cc -c test.c -o test.o";
$"ar rcs libtest.a test.o";

let files = List::to_str(sys::get_all_files_by_ext(".", "c"));

$f"cc -ggdb -Iinclude -o main {files} -lforge";
