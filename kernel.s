.globl _start
.text
.code32
_start:
# TODO make interrupt handlers and set the IDT so that interrupts can be re-enabled, then re-enable interrupts
# sti
# inb $0x70,%al
# andb $0x7f,%al
# outb %al,$0x70
# inb $0x71,%al
movl $0x10,%eax
movw %ax,%ds
movw $0x7070,%ds:0xb8000
lop:
nop
nop
jmp lop
