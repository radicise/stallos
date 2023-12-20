CC ?= i686-linux-gnu-gcc
AS ?= i686-linux-gnu-as
LD ?= i686-linux-gnu-ld
STRIP ?= i686-linux-gnu-strip
OBJCOPY ?= i686-linux-gnu-objcopy

CFLAGS ?= -std=c99 -Wpedantic -m32 -march=i386 -nostartfiles -nostdlib -nodefaultlibs -static -c
ASFLAGS ?= -march=i386 --target=i686-elf
LDFLAGS ?= --no-dynamic-linker -Ttext=0x0

build stall.bin: stall.elf kernel.bin prgm.elf shell.elf
	${OBJCOPY} --dump-section .text=stall.bin stall.elf /dev/null
	dd if=kernel.bin of=stall.bin bs=512 skip=110 seek=48
	dd if=prgm.elf of=stall.bin bs=512 seek=66
	dd if=shell.elf of=stall.bin bs=512 seek=194
	dd if=/dev/zero of=stall.bin bs=512 count=1 seek=2879

run: build
	qemu-system-i386 -D log_qemu.txt -drive file=stall.bin,format=raw,index=0,if=floppy

shell.elf: shell-asm.elf shell-ul.elf
	${LD} $(LDFLAGS) -o shell.elf shell-ul.elf shell-asm.elf -lgcc

shell-asm.elf: os/shell.s
	${AS} $(ASFLAGS) -o shell-asm.elf os/shell.s

shell-ul.elf: os/shell.c
	${CC} $(CFLAGS) -o shell-ul.elf os/shell.c

prgm.elf: prgm-ul.elf prgm-asm.elf
	${LD} $(LDFLAGS) -o prgm.elf prgm-ul.elf prgm-asm.elf -lgcc

prgm-asm.elf: system-comp.s
	${AS} $(ASFLAGS) -o prgm-asm.elf system-comp.s

system-comp.s: os/system.s os/irupt_generic.s
	cp os/system.s system-comp.s
	awk '1{gsub(/NUM/, thenum, $$0);print($$0);}' thenum=70 os/irupt_generic.s thenum=71 os/irupt_generic.s thenum=72 os/irupt_generic.s thenum=73 os/irupt_generic.s thenum=74 os/irupt_generic.s thenum=75 os/irupt_generic.s thenum=76 os/irupt_generic.s thenum=77 os/irupt_generic.s thenum=78 os/irupt_generic.s thenum=79 os/irupt_generic.s thenum=7a os/irupt_generic.s thenum=7b os/irupt_generic.s thenum=7c os/irupt_generic.s thenum=7d os/irupt_generic.s thenum=7e os/irupt_generic.s thenum=7f os/irupt_generic.s >> system-comp.s

prgm-ul.elf: os/system.c
	${CC} $(CFLAGS) -o prgm-ul.elf os/system.c

kernel.bin: kernel.o sysc.elf irupts.o
	${LD} $(LDFLAGS) -o kernel.elf kernel.o sysc.elf irupts.o -lgcc
	cp kernel.elf kernel-copy.elf
	${STRIP} kernel-copy.elf	
	${OBJCOPY} --dump-section .text=kernel.bin kernel.elf /dev/null

sysc.elf: sys32/sys.c
	${CC} $(CFLAGS) -o sysc.elf sys32/sys.c

kernel.o: kernel-comp.s
	${AS} $(ASFLAGS) -o kernel.o kernel-comp.s

irupts.o: sys32/irupts.s
	${AS} $(ASFLAGS) -o irupts.o sys32/irupts.s

kernel-comp.s: sys32.dhulb
	dhulbpp - - < sys32.dhulb > kern32-comp.dhulb
	dhulbc 32 -tNGT < kern32-comp.dhulb > kernel-comp.s

stall.elf: stall.o
	${LD} $(LDFLAGS) -o stall.elf stall.o -lgcc

stall.o: stall-comp.s
	${AS} $(ASFLAGS) -o stall.o stall-comp.s

stall-comp.s: Salth.java stall.slth sys16.dhulb stall.s
	cp stall.s stall-comp.s
	java Salth n staltstd < stall.slth >> stall-comp.s
	printf ".if staltstd_str_commandline_addr\n  .err # The command line address is offset from the start of the shell's static text segment\n.endif\n" >> stall-comp.s
	cat sys16.dhulb | dhulbpp - - | dhulbc 16 -tNT >> stall-comp.s
	cat ${DHULB_PATH}/src/DLib/pc/io.s ${DHULB_PATH}/src/DLib/util_16.s shell.s ${DHULB_PATH}/src/DLib/stall/stack.s ${DHULB_PATH}/src/DLib/stall/sys.s ${DHULB_PATH}/src/DLib/dos/api_bindings.s kernel/int.s >> stall-comp.s

clean:
	rm -f stall.bin
	rm -f kernel.bin
	rm -f *.elf
	rm -f system-comp.s
	rm -f *.o
	rm -f kernel-comp.s
	rm -f kern32-comp.dhulb
	rm -f stall-comp.s
