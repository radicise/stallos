#!/bin/sh
# unified way to compile the fs across intel and arm architecture

arch=`uname -m`
TAR=0

case "$arch" in
    x86_64|amd64)
        TAR=2;;
    i?86)
        TAR=1;;
    arm*)
        TAR=3;;
esac

if [ $TAR = 0 ]
then
    printf "UNSUPPORTED ARCHITECTURE\n"
    exit 1
fi

echo $TAR

if [ $TAR = 3 ]
then
    ./cfst.sh 1
else
    export TARGETNUM=$TAR
    ./cfst.sh 0
fi
exit
