#!/bin/sh

# this file ensures cross platform support of the Makefile

set +e
x86_64-linux-gnu-gcc > "/dev/null" 2>&1
set -e
if [ $? = 1 ]
then
export ASPRGM=i686-elf-as
export LDPRGM=i686-elf-ld
export STRIPPRGM=i686-elf-strip
export OBJCOPYPRGM=i686-elf-objcopy
fi
make run
