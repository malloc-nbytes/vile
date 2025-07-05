#!/usr/local/bin/earl -xe

module Build

$"cc -c test.c -o test.o";
$"ar rcs libtest.a test.o";

let files = "main.c lexer.c smap.c grammar.c";

$f"cc -Iinclude -o main {files} -L. -ltest";
