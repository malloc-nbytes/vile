#!/bin/bash

set -xe

files=$(find . -type f -name '*.c')
cc="cc"
name="vile"

$cc -o $name $files -Iinclude/ $(forge lib)
