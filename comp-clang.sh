#!/bin/sh
set -e
cp stall.s stall-comp.s
java Salth n staltstd < stall.slth >> stall-comp.s
printf ".if staltstd_str_commandline_addr\n  .err # The command line address is offset from the start of the shell's static text segment\n.endif\n.set dist_end6144, . - _start\n.space 6144 - dist_end6144\n" >> stall-comp.s
as -m16 -o stall.o stall-comp.s
strip stall.o
FILEOFF=$(otool -l stall.o | grep "fileoff" | grep -oE '[^ ]+$')
dd if=stall.o of=stall-inter.bin bs=${FILEOFF} skip=1
FILEOFF=$(otool -l stall.o | grep " size" | grep -oE '[^ ]+$')
FILEOFF=$(printf "%d" ${FILEOFF})
dd if=stall-inter.bin of=stall.bin bs=${FILEOFF} count=1
cp kernel.s kernel-comp.s
as -m32 -o kernel.o kernel-comp.s
strip kernel.o
FILEOFF=$(otool -l kernel.o | grep "fileoff" | grep -oE '[^ ]+$')
dd if=kernel.o of=stall-inter.bin bs=${FILEOFF} skip=1
FILEOFF=$(otool -l kernel.o | grep " size" | grep -oE '[^ ]+$')
FILEOFF=$(printf "%d" ${FILEOFF})
dd if=stall-inter.bin of=kernel.bin bs=${FILEOFF} count=1
cat kernel.bin >> stall.bin
dd if=/dev/zero of=stall.bin bs=16 count=1 seek=447
clang -c -fno-asynchronous-unwind-tables -target i386-pc-linux-elf -Wall -o kern-ul.elf kern.c
ld.lld kern-ul.elf -o kern.elf
cat kern.elf >> stall.bin
dd if=/dev/zero of=stall.bin bs=512 count=1 seek=2879
qemu-system-i386 -D log_qemu.txt -drive file=stall.bin,format=raw,index=0,media=disk
