#!/usr/local/bin/earl -xe

module Build

import "std/system.rl"; as sys
import "std/datatypes/list.rl";

let install, uninstall = (false, false);
try { install = argv()[1] == "install"; }
try { uninstall = argv()[1] == "uninstall"; }

# $"cc -c test.c -o test.o";
# $"ar rcs libtest.a test.o";

let files = List::to_str(
    sys::get_all_files_by_ext(".", "c")
        .filter(|f| { f != "./test.c" && f != "./.vile.c"; })
);

$f"cc -ggdb -Iinclude -o vile {files} -lforge";

if install   { $"sudo cp ./vile /usr/local/bin"; }
if uninstall { $"sudo rm /usr/local/bin/vile"; }
