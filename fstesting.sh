#!/bin/sh
clang fstesting.c -o fstesting
if [ $? == 0 ]
then
./fstesting $@
fi