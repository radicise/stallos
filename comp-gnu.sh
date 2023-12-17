#!/bin/sh
set -eu
mkdir -p build
cp stall.s build/stall-comp.s
java Salth n staltstd < stall.slth >> build/stall-comp.s
printf ".if staltstd_str_commandline_addr\n  .err # The command line address is offset from the start of the shell's static text segment\n.endif\n" >> build/stall-comp.s
#printf ".set sys_off, .\n" >> stall-comp.s
cat sys16.dhulb | dhulbpp - - | dhulbc 16 -tNTw >> build/stall-comp.s
cat ${DHULB_PATH}/src/DLib/pc/io.s ${DHULB_PATH}/src/DLib/util_16.s shell.s ${DHULB_PATH}/src/DLib/stall/stack.s ${DHULB_PATH}/src/DLib/stall/sys.s ${DHULB_PATH}/src/DLib/dos/api_bindings.s kernel/int.s >> build/stall-comp.s
as -o build/stall.o build/stall-comp.s
ld -T ./newf -o build/stall.elf build/stall.o
strip build/stall.elf
objcopy --dump-section .text=build/stall.bin build/stall.elf /dev/null
dhulbpp - - < sys32.dhulb > build/kern32-comp.dhulb
dhulbc 32 -tNGTw < build/kern32-comp.dhulb > build/kernel-comp.s
#cp kernel.s kernel-comp.s
i686-linux-gnu-as -march=i386 -o build/kernel.o build/kernel-comp.s
i686-linux-gnu-as -march=i386 -o build/irupts.o sys32/irupts.s
i686-linux-gnu-gcc -std=c99 -Wpedantic -m32 -march=i386 -nostartfiles -nostdlib -nodefaultlibs -static -c -o build/sysc.elf sys32/sys.c
i686-linux-gnu-ld --no-dynamic-linker -T ./newf386 -o build/kernel.elf build/kernel.o build/sysc.elf build/irupts.o -lgcc
cp build/kernel.elf build/kernel-copy.elf
i686-linux-gnu-strip build/kernel-copy.elf
i686-linux-gnu-objcopy --dump-section .text=build/kernel.bin build/kernel.elf /dev/null
dd if=build/kernel.bin of=build/stall.bin bs=512 skip=110 seek=48
i686-linux-gnu-gcc -std=c99 -Wpedantic -m32 -march=i386 -nostartfiles -nostdlib -nodefaultlibs -static -c -o build/prgm-ul.elf os/system.c
cp os/system.s build/system-comp.s
awk '1{gsub(/NUM/, thenum, $0);print($0);}' thenum=70 os/irupt_generic.s thenum=71 os/irupt_generic.s thenum=72 os/irupt_generic.s thenum=73 os/irupt_generic.s thenum=74 os/irupt_generic.s thenum=75 os/irupt_generic.s thenum=76 os/irupt_generic.s thenum=77 os/irupt_generic.s thenum=78 os/irupt_generic.s thenum=79 os/irupt_generic.s thenum=7a os/irupt_generic.s thenum=7b os/irupt_generic.s thenum=7c os/irupt_generic.s thenum=7d os/irupt_generic.s thenum=7e os/irupt_generic.s thenum=7f os/irupt_generic.s >> build/system-comp.s
i686-linux-gnu-as -march=i386 -o build/prgm-asm.elf build/system-comp.s
i686-linux-gnu-ld --no-dynamic-linker -T ./newf386 -o build/prgm.elf build/prgm-ul.elf build/prgm-asm.elf -lgcc
dd if=build/prgm.elf of=build/stall.bin bs=512 seek=66
i686-linux-gnu-gcc -std=c99 -Wpedantic -m32 -march=i386 -nostartfiles -nostdlib -nodefaultlibs -static -c -o build/shell-ul.elf os/shell.c
i686-linux-gnu-as -march=i386 -o build/shell-asm.elf os/shell.s
i686-linux-gnu-ld --no-dynamic-linker -T ./newf386 -o build/shell.elf build/shell-ul.elf build/shell-asm.elf -lgcc
dd if=build/shell.elf of=build/stall.bin bs=512 seek=194
#clang -c -fno-asynchronous-unwind-tables -target i386-pc-linux-elf -Wall -o kern-ul.elf kern.c
#ld.lld kern-ul.elf -o kern.elf
#cat kern.elf >> stall.bin
#dd if=/dev/zero of=stall.bin bs=512 count=1 seek=1439
#cat fs.bin >> stall.bin
dd if=/dev/zero of=build/stall.bin bs=512 count=1 seek=2879
mkdir -p StallOS
cp build/stall.bin StallOS/stallos.bin
cp build/stall.bin StallOS/hda.bin
qemu-system-i386 -D log_qemu.txt -boot a -drive file=StallOS/stallos.bin,format=raw,index=0,if=floppy -drive file=StallOS/hda.bin,format=raw,index=0,if=ide
exit 0
