CCPRGM ?= ./cc.sh
ASPRGM ?= i686-linux-gnu-as
LDPRGM ?= i686-linux-gnu-ld
STRIPPRGM ?= i686-linux-gnu-strip
OBJCOPYPRGM ?= i686-linux-gnu-objcopy

TARGETNUM ?= 1
export TARGETNUM
TARGETMACHINE ?= x86_32

ASFLAGS ?= -march=i386
LDFLAGS ?= --no-dynamic-linker


build Stallos/stallos.bin : build/stall.bin
	cp build/stall.bin Stallos/stallos.bin

build/stall.bin : build/stall.elf build/loader.bin build/kernel.elf build/init.elf
	${OBJCOPYPRGM} --dump-section .text=build/stall.bin build/stall.elf /dev/null
	dd if=build/loader.bin of=build/stall.bin bs=512 skip=110 seek=48
	dd if=build/init.elf of=build/stall.bin bs=512 seek=66
	dd if=build/kernel.elf of=build/stall.bin bs=512 seek=194
	dd if=/dev/zero of=build/stall.bin bs=512 count=1 seek=2879
	# Beware of overrunning allotted space and thus having corrupted binaries in memory

run : Stallos/stallos.bin
	./run.sh

build/init.elf : build/init-asm.elf build/init-ul.elf
	${LDPRGM} ${LDFLAGS} -o build/init.elf build/init-ul.elf build/init-asm.elf -lgcc

build/init-asm.elf : os/init.s
	${ASPRGM} ${ASFLAGS} -o build/init-asm.elf os/init.s

build/init-ul.elf : os/init.c
	${CCPRGM} -o build/init-ul.elf os/init.c

build/kernel.elf : build/kernelbase-ul.elf build/kernel-asm.elf build/tsfs.out
	${LDPRGM} ${LDFLAGS} -Ttext=0x0 -o build/kernel.elf build/kernelbase-ul.elf build/kernel-asm.elf -lgcc

build/kernel-asm.elf : build/system-comp.s
	${ASPRGM} ${ASFLAGS} -o build/kernel-asm.elf build/system-comp.s

build/system-comp.s : os/system.s os/irupt_generic.s os/kernel/machine/${TARGETMACHINE}/*.s
	cp os/system.s build/system-comp.s
	awk '1{gsub(/NUM/, thenum, $$0);print($$0);}' thenum=70 os/irupt_generic.s thenum=71 os/irupt_generic.s thenum=72 os/irupt_generic.s thenum=73 os/irupt_generic.s thenum=74 os/irupt_generic.s thenum=75 os/irupt_generic.s thenum=76 os/irupt_generic.s thenum=77 os/irupt_generic.s thenum=78 os/irupt_generic.s thenum=79 os/irupt_generic.s thenum=7a os/irupt_generic.s thenum=7b os/irupt_generic.s thenum=7c os/irupt_generic.s thenum=7d os/irupt_generic.s thenum=7e os/irupt_generic.s thenum=7f os/irupt_generic.s >> build/system-comp.s
	cat os/kernel/machine/${TARGETMACHINE}/*.s >> build/system-comp.s # TODO Have this not fail if there are no /*.s/ files present for the machine

build/tsfs.out : os/kernel/*.h os/kernel/machine/${TARGETMACHINE}/*.h os/kernel/driver/*.h os/kernel/obj/*.h os/kernel/object/*.h os/kernel/filesystems/*
	${CCPRGM} -o build/tsfs.out os/kernel/filesystems/tsfs.c

build/kernelbase-ul.elf : os/system.c os/kernel/*.h os/kernel/machine/${TARGETMACHINE}/*.h os/kernel/driver/*.h os/kernel/obj/*.h os/kernel/object/*.h
	${CCPRGM} -o build/kernelbase-ul.elf os/system.c

build/loader.bin : build/loader.o build/sysc.elf build/irupts.o
	${LDPRGM} ${LDFLAGS} -Ttext=0x0 -o build/loader.elf build/loader.o build/sysc.elf build/irupts.o -lgcc
	cp build/loader.elf build/loader-copy.elf
	${STRIPPRGM} build/loader-copy.elf	
	${OBJCOPYPRGM} --dump-section .text=build/loader.bin build/loader.elf /dev/null

build/sysc.elf : sys32/sys.c
	${CCPRGM} -o build/sysc.elf sys32/sys.c

build/loader.o : build/loader-comp.s
	${ASPRGM} ${ASFLAGS} -o build/loader.o build/loader-comp.s

build/irupts.o : sys32/irupts.s
	${ASPRGM} ${ASFLAGS} -o build/irupts.o sys32/irupts.s

build/loader-comp.s : sys32.dhulb sys32/kern32.dhulb sys32/elfExec.dhulb
	dhulbpp - - < sys32.dhulb > build/kern32-comp.dhulb
	dhulbc 32 -tNGTw < build/kern32-comp.dhulb > build/loader-comp.s

build/stall.elf : build/stall.o
	${LDPRGM} ${LDFLAGS} -Ttext=0x0 -o build/stall.elf build/stall.o -lgcc

build/stall.o : build/stall-comp.s
	${ASPRGM} ${ASFLAGS} -o build/stall.o build/stall-comp.s

build/stall-comp.s : build/Salth.class stall.slth stall.s
	cp stall.s build/stall-comp.s
	java -cp build Salth n staltstd < stall.slth >> build/stall-comp.s
	printf ".if staltstd_str_commandline_addr\n  .err # The command line address is offset from the start of the shell's static text segment\n.endif\n" >> build/stall-comp.s


build/Salth.class : Salth.java
	javac -d build Salth.java

clean :
	rm -f build/*
	rm -f bin/*

build/tsfs-ul.elf : os/kernel/filesystems/tsfs.c os/kernel/filesystems/*.h
	# TODO Revise "tsfs" file organisation
	${CCPRGM} -o build/tsfs-ul.elf os/kernel/filesystems/tsfs.c
