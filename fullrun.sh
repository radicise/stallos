#!/bin/sh

# this file ensures cross platform support of the Makefile

set +e
which i686-elf-gcc > "/dev/null" 2>&1
set -e
if [ $? = 0 ]
then
export ASPRGM=i686-elf-as
export LDPRGM=i686-elf-ld
export STRIPPRGM=i686-elf-strip
export OBJCOPYPRGM=i686-elf-objcopy
fi
make run
