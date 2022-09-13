.globl _start
.text
.code32
_start:
movw $0x10,%ax
movw %ax,%ss
movl $0x1ffffff,%esp
movw %ax,%ds
# TODO remap PIC so that hardware interrupts do not overlap with CPU exception interrupts, make interrupt handlers, and set the IDT so that interrupts can be re-enabled and go to the handlers, then re-enable both NMI interrupts dependent on IF
# sti
# inb $0x70,%al
# andb $0x7f,%al
# outb %al,$0x70
# inb $0x71,%al

xorw %cx,%cx
xorl %ebx,%ebx
lop:
movw %cx,%ds:0xb8000(%ebx)
incw %cx
incb %bl
testb %bl,%bl
jz reh
jmp lop
reh:
nop
nop
jmp reh
giveMem:
# doc:
# kernel-service
# findMem
# arg0 - Length in 4KiB chunks
# arg1 - First in-lower-bounds chunk, 0-indexed
# arg2 - First out-of-upper-bounds chunk, 0-indexed
# Returns 0 on error
# TODO save registers which need saving
# TODO check if %esp is large enough
movl %ss:8(%ebp),%edi
movl %ss:12(%ebp),%eax
movl %ss:16(%ebp),%edx
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
cmpl %eax,%ebx
jnc fmem_zer
movb $0x01,%cl
fmem_entr2:
incl %ebx
cmpb $0xff,%ds:(%ebx)
jz fmem_loop
fmem_testb:
test %cl,%ds:(%ebx)
jnz fmem_j3
incl %esi
cmpl %esi,%edi
jz fmem_end
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
movl %ebx,%edx
movl %ebx,%eax
shrl $3,%ebx
addl $0x200000,%ebx
orl $0x07,%ecx
movl $0x01,%edx
shlb %cl,%dl
decl %ebx
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
leavel
ret
fmem_zer:
xorl %eax,%eax
leave
ret
executeELF:
# doc:
# kernel-service
# executeELF
# arg0 - Physical / absolute address of ELF in memory
# arg1 - Length of ELF file in memory, in bytes
# TODO check if %esp gives enough space
# TODO push / otherwise save registers, make this call SystemVABI-compatible
# TODO return appropriate values for loading failures and make sure that they are distinguishable from program exit codes and run-time failures by the user space caller
# TODO make sure that no memory is read outside of the passed length
movl %ss:8(%ebp),%ebx
cmpl $0x464c457f,%ds:(%ebx)
jnz elfLoad_failure # invalid magic
cmpb $0x01,%ds:4(%ebx)
jnz elfLoad_failure # wrong platform size
cmpb $0x01,%ds:5(%ebx)
jnz elfLoad_failure # wrong endian-ness
# TODO test header version
cmpb $0x00,%ds:7(%ebx)
jnz elfLoad_failure # wrong ABI
cmpw $0x02,%ds:16(%ebx)
jnz elfLoad_failure
movw %ds:18(%ebx),%ax
testw %ax,%ax
jz elf_goodArch
cmpw $3,%ax
jz elf_goodArch
jmp elfLoad_failure # incompatible architecture
elf_goodArch:
movl %ds:28(%ebx),%edi
cmpw $0x20,%ds:42(%ebx)
xorl %ecx,%ecx
movw %ds:44(%ebx),%cx
xchgl %ebx,%edi


jnz elfLoad_failure # unsupported program header size
# TODO check ELF version



elfLoad_failure:
