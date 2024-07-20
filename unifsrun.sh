#!/bin/sh

# driver for unifscc.sh

./unifscc.sh
if [ $? != 0 ]
then
    exit
fi
TAR="./Stallos/hda.bin"
if [ "$1" == "--target" ]
then
    TAR="$2"
    shift 2
fi
./bin/fstesting.bin $TAR "$@"
# ./bin/fstesting.bin ./FSMOCKFILE.mock "$@"
# ./bin/fstesting.bin ./Stallos/hda.bin "$@"
