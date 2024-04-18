#!/bin/bash
set -eu
i686-linux-gnu-gcc -D TARGET=x86_32 -D TARGETNUM=0 -D __TESTING__=0 -O0 -std=c99 -Wpedantic -nostartfiles -nostdlib -nodefaultlibs -static -c -march=i386 -mabi=sysv "$@"
exit
