#!/bin/sh
set -eu
qemu-system-i386 -cpu 486-v1 -m 64M -boot a -drive file=Stallos/stallos.bin,format=raw,index=0,if=floppy -drive file=Stallos/hda.bin,format=raw,index=0,if=ide
# qemu-system-i386 -d int -cpu 486-v1 -m 64M -boot a -drive file=Stallos/stallos.bin,format=raw,index=0,if=floppy -drive file=Stallos/hda.bin,format=raw,index=0,if=ide
exit 0
