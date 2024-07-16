#!/bin/sh

# driver for unifscc.sh

./unifscc.sh
if [ $? != 0 ]
then
    exit
fi
# ./bin/fstesting.bin ./FSMOCKFILE.mock "$@"
./bin/fstesting.bin ./Stallos/hda.bin "$@"
