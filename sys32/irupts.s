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
irupt_hang:
.globl irupt_hang
cli# TODO Is this necessary?
irupt_hang__loop1:
jmp irupt_hang__loop1
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
movl $0x4745474b,%cs:0x000b8000
movl $0x474e4752,%cs:0x000b8004
movl $0x474c4745,%cs:0x000b8008
movl $0x47424720,%cs:0x000b800c
movl $0x47484745,%cs:0x000b8010
movl $0x47564741,%cs:0x000b8014
movl $0x474f4749,%cs:0x000b8018
movl $0x47204752,%cs:0x000b801c
movl $0x47484743,%cs:0x000b8020
movl $0x47434745,%cs:0x000b8024
movl $0x4720474b,%cs:0x000b8028
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
