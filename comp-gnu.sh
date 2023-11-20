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
objcopy --dump-section .text=stall.bin stall.elf /dev/null
dhulbpp - - < sys32.dhulb > kern32-comp.dhulb
dhulbc 32 -tNGT < kern32-comp.dhulb > kernel-comp.s
#cp kernel.s kernel-comp.s
i686-linux-gnu-as -march=i386 -o kernel.o kernel-comp.s
i686-linux-gnu-as -march=i386 -o irupts.o sys32/irupts.s
gcc -std=c99 -m32 -march=i386 -nostartfiles -nostdlib -nodefaultlibs -static -c -o sysc.elf sys32/sys.c
i686-linux-gnu-ld --no-dynamic-linker -T ./newf386 -o kernel.elf kernel.o sysc.elf irupts.o
cp kernel.elf kernel-copy.elf
i686-linux-gnu-strip kernel-copy.elf
i686-linux-gnu-objcopy --dump-section .text=kernel.bin kernel.elf /dev/null
dd if=kernel.bin of=stall.bin bs=512 skip=110 seek=48
gcc -std=c99 -m32 -march=i386 -nostartfiles -nostdlib -nodefaultlibs -static -c -o prgm-ul.elf os/system.c
i686-linux-gnu-as -march=i386 -o prgm-asm.elf os/system.s
i686-linux-gnu-ld --no-dynamic-linker -T ./newf386 -o prgm.elf prgm-ul.elf prgm-asm.elf
gcc -std=c99 -m32 -march=i386 -nostartfiles -nostdlib -nodefaultlibs -static -c -o shell-ul.elf os/shell.c
i686-linux-gnu-as -march=i386 -o shell-asm.elf os/shell.s
i686-linux-gnu-ld --no-dynamic-linker -T ./newf386 -o shell.elf shell-ul.elf shell-asm.elf
dd if=prgm.elf of=stall.bin bs=512 seek=66
#clang -c -fno-asynchronous-unwind-tables -target i386-pc-linux-elf -Wall -o kern-ul.elf kern.c
#ld.lld kern-ul.elf -o kern.elf
#cat kern.elf >> stall.bin
#dd if=/dev/zero of=stall.bin bs=512 count=1 seek=1439
#cat fs.bin >> stall.bin
dd if=/dev/zero of=stall.bin bs=512 count=1 seek=2879
qemu-system-i386 -D log_qemu.txt -drive file=stall.bin,format=raw,index=0,if=floppy
