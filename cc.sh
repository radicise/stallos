#!/bin/sh
set -eu
#alias x86_64-linux-gnu-gcc=$HOME/homebrew/bin/gcc-13
if [ ${TARGETNUM} = 1 ]
then
	i686-linux-gnu-gcc -D TARGET=x86_32 -D TARGETNUM=1 -D __TESTING__=1 -O0 -std=c99 -pedantic -ffreestanding -nostartfiles -nostdlib -nodefaultlibs -static -c -march=i386 -mabi=sysv "$@"
elif [ ${TARGETNUM} = 2 ]
then
    # check for presence of symlink'd name
    set +e
    x86_64-linux-gnu-gcc > "/dev/null" 2>&1
    set -e
    if [ $? = 1 ]
    then
	    x86_64-linux-gnu-gcc -D TARGET=x86_64 -D TARGETNUM=2 -D __TESTING__=1 -O0 -std=c99 -pedantic -ffreestanding -nostartfiles -nostdlib -nodefaultlibs -static -c -march=x86-64 -mabi=sysv "$@"
    else
        # fallback to explicit path
        eval "$GCCPATH" -D TARGET=x86_64 -D TARGETNUM=2 -D __TESTING__=1 -O0 -std=c99 -pedantic -Wno-builtin-declaration-mismatch -ffreestanding -nostartfiles -nostdlib -nodefaultlibs -static -c -march=x86-64 -mabi=sysv "$@"
    fi
else
    exit 1
fi
exit
