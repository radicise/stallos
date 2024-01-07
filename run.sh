#!/bin/sh
set -eu
qemu-system-i386 -boot a -drive file=StallOS/stallos.bin,format=raw,index=0,if=floppy -drive file=StallOS/hda.bin,format=raw,index=0,if=ide
exit 0
