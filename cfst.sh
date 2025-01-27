#!/bin/sh
export GCCPATH=$(which "gcc-13")
if [ $? != 0 ]
then
export GCCPATH=$(which "gcc")
fi
# eval "$GCCPATH" -D TARGET=x86_64 -D TARGETNUM=2 -D __TESTING__=0 -O0 -std=c99 -pedantic -Wno-builtin-declaration-mismatch -nostdlib -c os/kernel/filesystems/tsfs.c -o bin/tsfs.out
set -e
if [ ${1:-0} = 0 ]
then
    ./cc.sh -o bin/tsfs.out os/kernel/filesystems/tsfs.c
    eval "$GCCPATH" -D TARGET=x86_64 -D TARGETNUM=2 -D __TESTING__=0 -D FST=1 -O0 -pedantic fstesting.c bin/tsfs.out -o bin/fstesting.bin
else
    # gcc -D TARGET=x86_64 -D TARGETNUM=3 -D __TESTING__=0 -O0 -pedantic -Wno-builtin-declaration-mismatch -ffreestanding -nostartfiles -nostdlib -nodefaultlibs -static -c -o ./bin/tsfs.out ./os/kernel/filesystems/tsfs.c
    gcc -D TARGET=x86_64 -D TARGETNUM=3 -D __TESTING__=0 -D FST=1 -O0 -pedantic -ffreestanding -nostdlib -nodefaultlibs -static -c -o ./bin/tsfs.out ./os/kernel/filesystems/tsfs.c
    # eval "$GCCPATH" -D TARGET=x86_64 -D TARGETNUM=3 -D __TESTING__=0 -O0 -std=c99 -pedantic -Wno-builtin-declaration-mismatch -ffreestanding -nostartfiles -nostdlib -nodefaultlibs -static -c fstesting.c bin/tsfs.out -o bin/fstesting.bin
    gcc -D TARGET=x86_64 -D TARGETNUM=3 -D __TESTING__=0 -O0 -pedantic ./fstesting.c ./bin/tsfs.out -o ./bin/fstesting.bin
    #-march=x86-64 -mabi=sysv
fi
exit
