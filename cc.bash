#!/bin/bash
set -eu
alias x86_64-linux-gnu-gcc=$HOME/homebrew/bin/gcc-13
if [ $TARGETNUM == 0 ]
then
i686-linux-gnu-gcc -D TARGET=x86_32 -D TARGETNUM=0 -D __TESTING__=0 -O0 -std=c99 -pedantic -nostartfiles -nostdlib -nodefaultlibs -static -c -march=i386 -mabi=sysv "$@"
elif [ $TARGETNUM == 1 ]
then
# x86_64-linux-gnu-gcc -D TARGET=x86_64 -D TARGETNUM=1 -D __TESTING__=0 -O0 -std=c99 -pedantic -nostartfiles -nostdlib -nodefaultlibs -static -c -march=x86-64 -mabi=sysv "$@"
$HOME/homebrew/bin/gcc-13 -D TARGET=x86_64 -D TARGETNUM=1 -D __TESTING__=0 -O0 -std=c99 -pedantic -nostartfiles -nostdlib -nodefaultlibs -static -c -march=x86-64 -mabi=sysv "$@"
else
exit 1
fi
exit
