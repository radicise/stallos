#!/bin/sh
export GCCPATH=$(which "gcc-13")
# eval "$GCCPATH" -D TARGET=x86_64 -D TARGETNUM=2 -D __TESTING__=0 -O0 -std=c99 -pedantic -Wno-builtin-declaration-mismatch -nostdlib -c os/kernel/filesystems/tsfs.c -o bin/tsfs.out
./cc.bash -o bin/tsfs.out os/kernel/filesystems/tsfs.c
eval "$GCCPATH" -D TARGET=x86_64 -D TARGETNUM=2 -D __TESTING__=0 -O0 -std=c99 -pedantic fstesting.c bin/tsfs.out -o bin/fstesting.bin
