.text
irupt_discall:
.globl irupt_discall
pushw %ds
movw %cs,%ax
movw %ax,%ds
xorb %al,%al
irupt_discall__loop1:
movb %al,0x000b8000
incb %al
jmp irupt_discall__loop1
bugCheck:
.globl bugCheck
movw $0x4720,%ax
movl $2000,%ecx
movl $0x000b8000,%edx
bugCheck__loop2:
movw %ax,%cs:(%edx)
incl %edx
incl %edx
loop bugCheck__loop2
movl $0x47414746, %cs:0x000b8000
movl $0x47414754, %cs:0x000b8004
movl $0x4720474c, %cs:0x000b8008
movl $0x47594753, %cs:0x000b800c
movl $0x47544753, %cs:0x000b8010
movl $0x474d4745, %cs:0x000b8014
movl $0x47454720, %cs:0x000b8018
movl $0x47524752, %cs:0x000b801c
movl $0x4752474f, %cs:0x000b8020
bugCheck__loop1:
hlt
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
jmp bugCheck__loop1
