#!/bin/sh

# driver for unifscc.sh

./unifscc.sh
./bin/fstesting.bin ./FSMOCKFILE.mock "$@"