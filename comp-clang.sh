#!/bin/sh
set -e
cp stall.s stall-comp.s
java -jar salth.jar < stall.slth >> stall-comp.s
printf ".set dist_end141, . - _start\n.space 1474560 - dist_end141\n" >> stall-comp.s
as -m16 -o stall.o stall-comp.s
strip stall.o
FILEOFF=$(otool -l stall.o | grep "fileoff" | grep -oE '[^ ]+$')
dd if=stall.o of=stall-inter.bin bs=${FILEOFF} skip=1
FILEOFF=$(otool -l stall.o | grep " size" | grep -oE '[^ ]+$')
FILEOFF=$(printf "%d" ${FILEOFF})
dd if=stall-inter.bin of=stall.bin bs=${FILEOFF} count=1
