irupt_NUMh:# TODO Correctly handle any "spurious" IRQ
.globl irupt_NUMh
.if (0xNUM == 0x70)
pushl %eax
movw %cs,%ax
addw $0x08,%ax
roll $16,%eax
movw %ds,%ax
roll $16,%eax
movw %ax,%ds
roll $16,%eax
movw %ax,(0x007fff2a - RELOC)
popl %eax
movl %eax,(0x007fff00 - RELOC)
movl %ebx,(0x007fff04 - RELOC)
movl %ecx,(0x007fff08 - RELOC)
movl %edx,(0x007fff0c - RELOC)
movl %ebp,(0x007fff10 - RELOC)
movl %esp,(0x007fff14 - RELOC)
addl $0x0c,(0x007fff14 - RELOC)
movl %esi,(0x007fff18 - RELOC)
movl %edi,(0x007fff1c - RELOC)
movl (%esp),%eax
movl %eax,(0x007fff20 - RELOC)
movl 8(%esp),%eax
movl %eax,(0x007fff24 - RELOC)
movw 4(%esp),%ax
movw %ax,(0x007fff28 - RELOC)
movw %ss,(0x007fff2c - RELOC)
movw %es,(0x007fff2e - RELOC)
movw %fs,(0x007fff30 - RELOC)
movw %gs,(0x007fff32 - RELOC)
movl (0x007fff00 - RELOC),%eax
movw (0x007fff2a - RELOC),%ds
.endif
pushal
movw %ss,%bx
roll $0x10,%ebx
movw %ds,%bx
movl %esp,%esi
movw %cs,%ax
addw $0x08,%ax
movw %ax,%ds
movw %ax,%ss
movl $(0x007fff00 - RELOC),%esp
pushw %es
pushw %fs
pushw %gs
xorl %ebp,%ebp
cld
movl $0x00000001,handlingIRQ
.if (0xNUM == 0x70)
pushl $(0x007fff00 - RELOC)
.endif
call irupt_handler_NUMh
.if (0xNUM == 0x70)
addl $0x04,%esp
.endif
movl $0x00000000,handlingIRQ
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
.endif
outb %al,$0x20
popal
iretl
