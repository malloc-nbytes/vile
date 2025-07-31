#!/bin/bash

set -xe

files=$(find . -type f -name '*.c')

cc -o ViLe $files -Iinclude/ $(forge lib)
