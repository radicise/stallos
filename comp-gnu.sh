#!/bin/sh
set -eu
cp stall.s stall-comp.s
java Salth n staltstd < stall.slth >> stall-comp.s
printf ".if staltstd_str_commandline_addr\n  .err # The command line address is offset from the start of the shell's static text segment\n.endif\n" >> stall-comp.s
#printf ".set sys_off, .\n" >> stall-comp.s
cat sys16.dhulb | dhulbpp - - | dhulbc 16 -tNT >> stall-comp.s
cat ${DHULB_PATH}/src/DLib/pc/io.s ${DHULB_PATH}/src/DLib/util_16.s shell.s ${DHULB_PATH}/src/DLib/stall/stack.s ${DHULB_PATH}/src/DLib/stall/sys.s ${DHULB_PATH}/src/DLib/dos/api_bindings.s kernel/int.s >> stall-comp.s
as -o stall.o stall-comp.s
ld -T ./newf -o stall.elf stall.o
strip stall.elf
objcopy stall.elf /dev/null --dump-section .text=stall.bin
#cp kernel.s kernel-comp.s
#as -m32 -o kernel.o kernel-comp.s
#strip kernel.o
#FILEOFF=$(otool -l kernel.o | grep "fileoff" | grep -oE '[^ ]+$')
#dd if=kernel.o of=stall-inter.bin bs=${FILEOFF} skip=1
#FILEOFF=$(otool -l kernel.o | grep " size" | grep -oE '[^ ]+$')
#FILEOFF=$(printf "%d" ${FILEOFF})
#dd if=stall-inter.bin of=kernel.bin bs=${FILEOFF} count=1
#cat kernel.bin >> stall.bin
#dd if=/dev/zero of=stall.bin bs=16 count=1 seek=511
#clang -c -fno-asynchronous-unwind-tables -target i386-pc-linux-elf -Wall -o kern-ul.elf kern.c
#ld.lld kern-ul.elf -o kern.elf
#cat kern.elf >> stall.bin
dd if=/dev/zero of=stall.bin bs=512 count=1 seek=1439
cat fs.bin >> stall.bin
dd if=/dev/zero of=stall.bin bs=512 count=1 seek=2879
qemu-system-i386 -D log_qemu.txt -drive file=stall.bin,format=raw,index=0,if=floppy
