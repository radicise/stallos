#!/bin/sh
PROL="stall"
as -o $PROL.o $PROL.s
LEC=$?
if [ $LEC -ne 0 ];then
    printf "as failed with code $LEC\n"
    exit $LEC
fi
printf "assembly:       succeeded\n"
ld -T "./newf" -o $PROL.elf $PROL.o
LEC=$?
if [ $LEC -ne 0 ];then
    printf "ld failed with code $LEC\n"
    exit $LEC
fi
printf "linking:        succeeded\n"
objcopy -O binary $PROL.elf $PROL.bin
LEC=$?
if [ $LEC -ne 0 ];then
    printf "objcopy failed with code $LEC\n"
    exit 255
fi
printf "objcopy:        succeeded\n"
if [ -e "./stal.flab" ];then
    rm ./$PROL.flab
    LEC=$?
    if [ $LEC -ne 0 ];then
        printf "rm failed with code $LEC\n"
        exit 255
    fi
    printf "remove:         succeeded\n"
fi
qemu-system-i386 -drive file=$PROL.bin,index=0,if=floppy,format=raw
LEC=$?
if [ $LEC -ne 0 ];then
    printf "remove failed with code $LEC\n"
    exit 255
fi
printf "execution:      succeeded\n"
exit 0
