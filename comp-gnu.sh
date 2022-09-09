#!/bin/sh
PROL="stall"
cp stall.s stall-comp.s
LEC=${?}
if [ ${LEC} -ne 0 ];then
    printf "cp failed with code ${LEC}\n"
    exit ${LEC}
fi
printf "asm_copy:       succeeded\n"
java Salth n staltstd < ${PROL}.slth >> ${PROL}-comp.s
LEC=${?}
if [ ${LEC} -ne 0 ];then
    printf "java failed with code ${LEC}\n"
    exit ${LEC}
fi
printf "salth_compile:  succeeded\n"
printf ".if staltstd_str_commandline_addr\n  .err # The command line address is offset from the start of the shell's static text segment\n.endif\n.set dist_end141, . - _start\n.space 1474560 - dist_end141\n" >> ${PROL}-comp.s
LEC=${?}
if [ ${LEC} -ne 0 ];then
    printf "printf failed with code ${LEC}\n"
    exit ${LEC}
fi
printf "src_141MBpad:   succeeded\n"
as -o ${PROL}.o ${PROL}-comp.s
LEC=${?}
if [ ${LEC} -ne 0 ];then
    printf "as failed with code ${LEC}\n"
    exit ${LEC}
fi
printf "assembly:       succeeded\n"
ld -T "./newf" -o ${PROL}.elf ${PROL}.o
LEC=${?}
if [ ${LEC} -ne 0 ];then
    printf "ld failed with code ${LEC}\n"
    exit ${LEC}
fi
printf "linking:        succeeded\n"
objcopy -O binary ${PROL}.elf ${PROL}.bin
LEC=${?}
if [ ${LEC} -ne 0 ];then
    printf "objcopy failed with code ${LEC}\n"
    exit 255
fi
qemu-system-i386 -drive file=${PROL}.bin,index=0,if=floppy,format=raw
LEC=${?}
if [ ${LEC} -ne 0 ];then
    printf "remove failed with code ${LEC}\n"
    exit 255
fi
printf "execution:      succeeded\n"
exit 0
