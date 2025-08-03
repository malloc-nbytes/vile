#!/bin/bash

set -xe

files=$(earl \
            -i 'std/system.rl' \
            -i 'std/datatypes/list.rl' \
            -O 'List::to_str(System::get_all_files_by_ext(".", "c").filter(!= "./test.c"));')
cc="cc"
name="vile"
flags=""

$cc $flags -o $name $files -Iinclude/ $(forge lib)
