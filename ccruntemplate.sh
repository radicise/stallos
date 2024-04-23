#!/bin/bash
# runs the cc.bash script with explicit gcc path
TARGETNUM=2
export GCCPATH="path-to-gcc"
bash "$PWD/cc.bash" "$@"