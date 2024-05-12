irupt_NUMh:# TODO Correctly handle any "spurious" IRQ
.globl irupt_NUMh
cld
call irupt_handler_NUMh
movb $0x20,%al
.if (0xNUM >= 0x78)
outb %al,$0xa0
.endif
outb %al,$0x20
iret
jmp irupt_NUMh
