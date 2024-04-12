.set RELOC , 0x40000
.text
_start:
.globl _start
movl %esp,(0x0007f7f4 - RELOC)
call systemEntry
lret
irupt_80h:# NOTE: This is under the assumption that %edx can never be used to return anything, which is contrary to the `syscall' Linux man-pages page; TODO resolve this conflict
.globl irupt_80h
pushw %es
pushw %fs
pushw %gs
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
movl %cs:-12(%ebx),%ecx
subl $0x18,%ebx
movw %cs,%ax
addw $0x08,%ax
movw %ax,%ds
movw %ax,%ss
movl %ebx,%esp
popl %eax
popl %ebx
movl (%esp),%edx
pushl %ebp
pushl %eax
pushl $0x00
pushl %ebp
xorl %ebp,%ebp
pushl %edi
pushl %esi
pushl %edx
pushl %ecx
pushl %ebx
cld
call system_call# TODO Normalise argument structure if it had been re-formatted to fit the ABI
movl PerThread_context,%ebp
movl %ds:(%ebp),%ebp
testl %ebp,%ebp
jz irupt_80h__noError
movl %ebp,%eax
notl %eax
incl %eax
irupt_80h__noError:
addl $0x20,%esp
popl %ebp
popl %edx
popl %ecx
popw %ds
popw %ss
movl %cs:(%esp),%esp
addl $0x0c,%esp
popw %gs
popw %fs
popw %es
iretl
writeLongPhysical:# void writePhysical(u32 addr, unsigned long dat)
.globl writeLongPhysical
movw $0x10,%ax
movw %ax,%es# TODO Does %es need to be saved?
movl 4(%esp),%eax
movl 8(%esp),%ecx
movl %ecx,%es:(%eax)
ret
readLongPhysical:# unsigned long readPhysical(u32 addr)
.globl readLongPhysical
movw $0x10,%ax
movw %ax,%es# TODO Does %es need to be saved?
movl 4(%esp),%eax
movl %es:(%eax),%ecx
movl %ecx,%eax
ret
runELF:# int runELF(void* elfPhys, void* memAreaPhys, int* retVal)
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
int_enable:
.globl int_enable
inb $0x70,%al
andb $0x7f,%al
outb %al,$0x70
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
inb $0x71
nop
nop
nop
nop
nop
sti
ret
int_disable:
.globl int_disable
inb $0x70,%al
orb $0x80,%al
outb %al,$0x70
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
inb $0x71
nop
nop
nop
nop
nop
cli
ret
timeIncrement:
.globl timeIncrement
lock incl ___currentTime___
ret
timeFetch:
.globl timeFetch
movl ___currentTime___,%eax# TODO Ensure atomic memory reads
ret
timeStore:
.globl timeStore
movl 4(%esp),%eax
movl %eax,___currentTime___# TODO Ensure atomic memory writes
ret
bugCheck:# void bugCheck(void)
.globl bugCheck
movl (%esp),%eax
pushl %eax
call bugCheckWrapped
ret
bugCheckNum:# void bugCheckNum(unsigned long)
.globl bugCheckNum
movl (%esp),%eax
movl 4(%esp),%edx
pushl %eax
pushl %edx
call bugCheckNumWrapped
ret
Thread_restore:# void Thread_restore(struct Thread_state*, long)
.globl Thread_restore# Only invoke this from interrupt handlers called because pf IRQ interrupts
movl $0x00000000,handlingIRQ
movl 4(%esp),%ebx
movl 8(%esp),%eax
movw 0x2c(%ebx),%dx
movw %dx,%ss
movw 0x2e(%ebx),%dx
movw %dx,%es
movw 0x30(%ebx),%dx
movw %dx,%fs
movw 0x32(%ebx),%dx
movw %dx,%gs
movl 0x10(%ebx),%ebp
movl 0x14(%ebx),%esp
movl 0x18(%ebx),%esi
movl 0x1c(%ebx),%edi
pushl 0x24(%ebx)
pushw $0x00
pushw 0x28(%ebx)
pushl 0x20(%ebx)
pushl (%ebx)
pushl 0x04(%ebx)
pushl 0x08(%ebx)
pushl 0x0c(%ebx)
pushw 0x2a(%ebx)
popw %ds
popl %edx
popl %ecx
popl %ebx
testb $0x08,%al
movb $0x20,%al
jz Thread_restore__1
outb %al,$0xa0
Thread_restore__1:
outb %al,$0x20
popl %eax
iretl
Thread_run:# void Thread_run(struct Thread_state*)
.globl Thread_run# Do NOT invoke this from interrupt handlers called because of IRQ interrupts
movl 4(%esp),%ebx
movw 0x2c(%ebx),%dx
movw %dx,%ss
movw 0x2e(%ebx),%dx
movw %dx,%es
movw 0x30(%ebx),%dx
movw %dx,%fs
movw 0x32(%ebx),%dx
movw %dx,%gs
movl 0x10(%ebx),%ebp
movl 0x14(%ebx),%esp
movl 0x18(%ebx),%esi
movl 0x1c(%ebx),%edi
pushl 0x24(%ebx)
pushw $0x00
pushw 0x28(%ebx)
pushl 0x20(%ebx)
pushl (%ebx)
pushl 0x04(%ebx)
pushl 0x08(%ebx)
pushl 0x0c(%ebx)
pushw 0x2a(%ebx)
popw %ds
pushw $0x00# Zeroing the stack
popw %dx
popl %edx
popl %ecx
popl %ebx
popl %eax
iretl
strconcat:# const char* strconcat()
.globl strconcat# Returns a const char* `a' that can deallocated with `dealloc(strlen(a) + 1)'
pushl %ebp
movl %esp,%ebp
movl %esp,%eax
addl $0x08,%eax
pushl %eax
call strct
movl %ebp,%esp
popl %ebp
ret
