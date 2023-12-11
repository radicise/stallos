irupt_NUMh:
.globl irupt_NUMh
pushal
movw %ss,%bx
roll $0x10,%ebx
movw %ds,%bx
movl %esp,%esi
movw %cs,%ax
addw $0x08,%ax
movw %ax,%ds
movw %ax,%ss
movl $(0x00800000 - RELOC),%esp
pushw %es
pushw %fs
pushw %gs
xorl %ebp,%ebp
cld
call irupt_handler_NUMh
popw %gs
popw %fs
popw %es
movw %bx,%ds
roll $0x10,%ebx
movw %bx,%ss
movl %esi,%esp
movb $0x20,%al
.if (0xNUM >= 0x78)
outb %al,$0xa0
nop
nop
nop
nop
nop
.endif
outb %al,$0x20
nop
nop
nop
nop
nop
popal
iretl
