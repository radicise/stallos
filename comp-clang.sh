#!/bin/sh
set -e
cp stall.s stall-comp.s
java Salth n staltstd < stall.slth >> stall-comp.s
printf ".if staltstd_str_commandline_addr\n  .err # The command line address is offset from the start of the shell's static text segment\n.endif\n.set dist_end141, . - _start\n.space 1474560 - dist_end141\n" >> stall-comp.s
as -m16 -o stall.o stall-comp.s
strip stall.o
FILEOFF=$(otool -l stall.o | grep "fileoff" | grep -oE '[^ ]+$')
dd if=stall.o of=stall-inter.bin bs=${FILEOFF} skip=1
FILEOFF=$(otool -l stall.o | grep " size" | grep -oE '[^ ]+$')
FILEOFF=$(printf "%d" ${FILEOFF})
dd if=stall-inter.bin of=stall.bin bs=${FILEOFF} count=1
qemu-system-i386 -drive file=stall.bin,index=0,if=floppy,format=raw
