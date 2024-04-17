#!/bin/bash
gcc fstesting.c -Wpedantic -o fstesting
if [ $? == 0 ]
then
./fstesting $@
fi
