irupt_NUMh:# TODO URGENT Correctly handle any "spurious" IRQ
.globl irupt_NUMh
movl $0x00000001,handlingIRQ
cld
call irupt_handler_NUMh
movb $0x20,%al
.if (0xNUM >= 0x78)
outb %al,$0xa0
.endif
outb %al,$0x20
movl $0x00000000,handlingIRQ
iret
jmp irupt_NUMh
