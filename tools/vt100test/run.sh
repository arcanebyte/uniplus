#!/bin/sh
# Build the console VT100 emulator (v1.5/sys/vt100.c) on the Mac against a
# character-grid model of the bitmap routines in bm.c, and run the checks.
set -e
cd "$(dirname "$0")"
top=../..
tmp=${TMPDIR:-/tmp}/vt100test.$$
mkdir -p "$tmp"
cp $top/v1.5/include/sys/l2.h inc/sys/l2.h
clang -w -std=gnu89 -Wno-implicit-function-declaration -Wno-implicit-int -Wno-return-type \
	-I inc -c $top/v1.5/sys/vt100.c -o "$tmp/vt100.o"
clang -w -c stubs.c -o "$tmp/stubs.o"
clang -w -c test.c -o "$tmp/test.o"
clang -o "$tmp/vttest" "$tmp/vt100.o" "$tmp/stubs.o" "$tmp/test.o"
"$tmp/vttest"
status=$?
rm -rf "$tmp" inc/sys/l2.h
exit $status
