.set RELOC , 0x40000
.text
_start:
.globl _start
movl %esp,(0x0007f7f4 - RELOC)
call systemEntry
lret
irupt_80h:# NOTE: This is under the assumption that %edx can never be used to return anything, which is contrary to the `syscall' Linux man-pages page; TODO resolve this conflict
.globl irupt_80h
#TODO Zero %ebp
pushl %eax
pushl %ebx
pushl %edx
movw %ss,%dx
roll $0x10,%edx
movw %ds,%dx
movl %cs:(0x0007f7f4 - RELOC),%ebx
movl %esp,%cs:-4(%ebx)
movl %edx,%cs:-8(%ebx)
movl %ecx,%cs:-12(%ebx)
popl %ecx
movl %ecx,%cs:-16(%ebx)
popl %ecx
movl %ecx,%cs:-20(%ebx)
popl %ecx
movl %ecx,%cs:-24(%ebx)
subl $0x18,%ebx
movl %ebx,%esp
movw %cs,%ax
movw %ax,%ds
movw %ax,%ss
popl %eax
popl %ebx
pushl %ebp
pushl %eax
pushl %ebp
xorl %ebp,%ebp
pushl %edi
pushl %esi
pushl %edx
pushl %ecx
pushl %ebx
call system_call# TODO Normalise argument structure if it had been re-formatted to fit the ABI
addl $0x1c,%esp
popl %ebp
popl %edx
popl %ecx
popw %ds
popw %ss
movl %cs:(%esp),%esp
iret
writePhysical:
.globl writePhysical
movw $0x10,%ax
movw %ax,%es
movl 4(%esp),%eax
movb 8(%esp),%cl
movb %cl,%es:(%eax)
ret
readPhysical:
.globl readPhysical
movw $0x10,%ax
movw %ax,%es
movl 4(%esp),%eax
movb %es:(%eax),%cl
xorl %eax,%eax
movb %cl,%al
ret
runELF:#int runELF(void* elfPhys, void* memAreaPhys, int* retVal)
.globl runELF
pushl %ebp
movl %esp,%ebp
pushl %ebx
pushl %esi
pushl %edi
movl 0x08(%ebp),%eax
movl 0x0c(%ebp),%edx
movl 0x10(%ebp),%ecx
lcall *(0x7f7f8 - RELOC)
popl %edi
popl %esi
popl %ebx
popl %ebp
ret
