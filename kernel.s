.globl _start
.text
.code32
_start:
movw $0x10,%ax
movw %ax,%ss
movl $0x1fffff0,%esp
movw %ax,%ds
# TODO remap PIC so that hardware interrupts do not overlap with CPU exception interrupts, make interrupt handlers, and set the IDT so that interrupts can be re-enabled and go to the handlers, then re-enable both NMI interrupts dependent on IF
# sti
# inb $0x70,%al
# andb $0x7f,%al
# outb %al,$0x70
# inb $0x71,%al
pushl $0x00
pushl $0x00
pushl $1024
pushl $0x9800
movl %esp,%ebp
subl $0x08,%ebp
xorl %eax,%eax
call executeELFKernelSpace
jmp n
movw %ds:0x0514,%cx
xorl %ebx,%ebx
lop:
movw %cx,%ds:0xb8000(%ebx)
addw $0x0100,%cx
addw $0x02,%bx
testw $0x07ff,%bx
jz reh
jmp lop
reh:
nop
nop
jmp reh
giveMem:
# doc:
# kernel-service
# giveMem
# arg0 - Length in 4KiB chunks
# arg1 - First in-lower-bounds chunk, 0-indexed
# arg2 - First out-of-upper-bounds chunk, 0-indexed
# Returns 0 on error
# TODO save registers which need saving
# TODO check if %esp is large enough
pushl %ebx
pushl %esi
pushl %edi
movl %ss:8(%ebp),%edi
movl %ss:12(%ebp),%eax
movl %ss:16(%ebp),%edx
subl %edi,%edx
movl $0x1fffff,%ebx
movl %eax,%esi
shrl $3,%esi
addl %esi,%ebx
andb $0x07,%al
movl %ecx,%esi
movb %al,%cl
movb $0x01,%al
shlb %cl,%al
xorl %esi,%esi
movb %al,%cl
movl %edx,%eax
shll $3,%eax
addl $0x200000,%eax
jmp fmem_entr2
fmem_loop:
xorl %esi,%esi
fmem_loop2:
incl %ebx
cmpl %eax,%ebx
jnc fmem_zer
movb $0x01,%cl
fmem_entr2:
cmpb $0xff,%ds:(%ebx)
jz fmem_loop
fmem_testb:
test %cl,%ds:(%ebx)
jnz fmem_j4
incl %esi
cmpl %esi,%edi
jz fmem_end
jmp fmem_j3
fmem_j4:
xorl %esi,%esi
fmem_j3:
shlb %cl
testb %cl,%cl
jz fmem_loop2
fmem_end:
subl $0x200000,%ebx
shll $3,%ebx
fmem_eah:
shrb $1,%cl
testb %cl,%cl
jz fmem_rend
incl %ebx
jmp fmem_eah
fmem_rend:
cmpl %edx,%ebx
jnc fmem_zer
movl %ebx,%ecx
movl %ebx,%eax
shrl $3,%ebx
addl $0x200000,%ebx
orl $0x07,%ecx
movl $0x01,%edx
shlb %cl,%dl
decl %ebx
movl %eax,%ecx
fmem_reah:
incl %ebx
fmem_reed:
jecxz fmem_reas
orb %dl,%ds:(%ebx)
decl %ecx
rolb %dl
cmpb $0x01,%dl
jz fmem_reah
jmp fmem_reed
fmem_reas:
popl %edi
popl %esi
popl %edx
ret
fmem_zer:
xorl %eax,%eax
popl %edi
popl %esi
popl %ebx
ret
executeELFKernelSpace:
# doc:
# kernel-service
# executeELFKernelSpace
# arg0 - Physical / absolute address of ELF in memory
# arg1 - Length of ELF file in memory, in bytes
# TODO check if %esp gives enough space
# TODO push / otherwise save registers, make this call SystemVABI-compatible
# TODO return appropriate values for loading failures and make sure that they are distinguishable from program exit codes and run-time failures by the user space caller
# TODO make sure that no memory is read outside of the passed length
movl %ss:8(%ebp),%ebx
movl %ss:12(%ebp),%edx
# TODO make sure that everything fits within file size
cmpl $0x464c457f,%ds:(%ebx)
movw $0x0001,%ds:0x0514
jnz elfLoad_failure # invalid magic
cmpb $0x01,%ds:4(%ebx)
movw $0x0002,%ds:0x0514
jnz elfLoad_failure # wrong platform size
cmpb $0x01,%ds:5(%ebx)
movw $0x0003,%ds:0x0514
jnz elfLoad_failure # wrong endian-ness
# TODO test header version
cmpb $0x00,%ds:7(%ebx)
movw $0x0004,%ds:0x0514
jnz elfLoad_failure # wrong ABI
cmpw $0x02,%ds:16(%ebx)
movw $0x0005,%ds:0x0514
jnz elfLoad_failure # not of executable type
movw %ds:18(%ebx),%ax
testw %ax,%ax
jz elf_goodArch
cmpw $3,%ax
jz elf_goodArch
movw $0x06,%ds:0x0514
jmp elfLoad_failure # incompatible architecture
elf_goodArch:
movl $0x00,%ds:(%ebx)
elf_goo:
movl %ds:8(%ebp),%ebx
movl %ds:28(%ebx),%edi
movl %ds:(%ebx),%eax
shll $5,%eax
addl %eax,%edi
addl %ebx,%edi
cmpw $0x20,%ds:42(%ebx)
movw $0x07,%ds:0x0514
jnz elfLoad_failure # unsupported program header size
xorl %ecx,%ecx
movw %ds:44(%ebx),%cx
xchgl %ebx,%edi
jecxz elf_almEnd
movl %ds:(%ebx),%eax
cmpl $0x01,%eax
movw $0x09,%ds:0x0514
jnz theNS # not a loadable segment
movl %edx,%esi
xorl %ecx,%ecx
pushl %ebp
elf_padLoop:
testl $0x0f,%esp
jz elf_skippad
pushl $0x00
incl %ecx
jmp elf_padLoop
elf_skippad:
pushl %ecx
pushl $0x01f00000
pushl $0x00800000
movl %ds:20(%ebx),%ecx
shrl $12,%ecx
incl %ecx
pushl %ecx
movl %esp,%ecx
subl $0x08,%ecx
movl %ecx,%esp
movl %esp,%ebp
call giveMem
popl %edx
popl %edx
popl %edx
popl %edx
popl %edx
popl %ecx
jecxz elf_noUnpad
jmp elf_unPad
elf_almEnd:
jmp elf_end
elf_unPad:
popl %edx
loop elf_unPad
popl %ebp
elf_noUnpad:
testl %eax,%eax
movw $0x0a,%ds:0x0514
jz elfLoad_failure # memory allocation failure
movw $0x10,%cx
movw %cx,%es
movl %edi,%ebx
movl %esi,%edx
movl %ebx,%ecx
movl %ds:(%ebx),%ecx
shll $5,%ebx
addl %ds:28(%ebx),%ecx
addl %ebx,%ecx
movl %ecx,%ebx
addl %ds:4(%ebx),%ecx
movl %ecx,%edi
movl %eax,%esi
movl %ds:16(%ebx),%ecx
rep movsb
addl %eax,%ecx
movl %ecx,%edi
movl %ds:20(%ebx),%ecx
subl %ds:16(%ebx),%ecx
jecxz elf_afcl
elf_clearloop: # TODO optimize
movb $0x00,%ds:(%edi)
incl %edi
loop elf_clearloop
elf_afcl:
subl %ds:8(%ebx),%eax
movl %ds:8(%ebx),%edi
movl %ds:24(%ebx),%ecx
movl %ds:8(%ebp),%ebx
movl %ds:(%ebx),%ebx
shll $3,%ebx
addl %ds:0x0510,%ebx
addl $0x18,%ebx
movw $0xffff,%ds:(%ebx)
movl %eax,%ds:2(%ebx)
testl $0x01,%ecx
movb $0x92,%ds:5(%ebx)
jz elf_noExBF
movb $0x9a,%ds:5(%ebx)
movl %ds:8(%ebp),%esi
movl %edi,%ds:4(%esi)
movl %ds:(%esi),%edi
movl %edi,%ds:8(%esi)
elf_noExBF:
movb $0xcf,%ds:6(%ebx)
movl %eax,%ecx
shrl $0x18,%ecx
movb %cl,%ds:7(%ebx)
jmp theNS
theNS:
movl %ds:8(%ebp),%ebx
movl %ds:(%ebx),%eax
movl %ebx,%ds:0xb80a0(,%eax,4)
incl %eax
movl %eax,%ds:(%ebx)
movw $0x0741,%ds:0xb8000(,%eax,2)
xorl %edx,%edx
movw %ds:44(%ebx),%dx
movw $0x0742,%ds:0xb8000(,%edx,2)
xorl %ecx,%ecx
movw %ds:4(%ebx),%cx
movl $0x07000700,%ds:0xb8000(,%ecx,2)
movb %cl,%ds:0xb8000(,%ecx,2)
movb %ch,%ds:0xb8002(,%ecx,2)
cmpl %edx,%eax
jnc elf_launch
jmp elf_goo
elf_launch:
movl %ds:8(%ebp),%ebx
movl %ds:8(%ebx),%edi
shl $3,%edi
movw %di,%ds:2(%ebx)
pushl $0x00
pushw $0x00
bp_ent:
pushw $0x00
testl $0x0f,%esp
jnz bp_ent
movl %esp,%ebp
subl $0x08,%ebp
lcall *%ds:2(%ebx)
jmp elf_end
# TODO check ELF version
# TODO avoid kernel stack overflow
elfLoad_failure:
movl $0x41,%eax
elf_end:
movw $0x10,%cx
movw %cx,%ds
ret
n:
nop
nop
nop
jmp n
