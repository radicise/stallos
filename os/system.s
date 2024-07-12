.set RELOC , 0x40000
.text
_start:
.globl _start
movl %esp,(0x0007f7f4 - RELOC)
call systemEntry
lret
irupt_80h:# NOTE: This is under the assumption that %edx can never be used to return anything, which is contrary to the `syscall' Linux man-pages page; TODO resolve this conflict
.globl irupt_80h
cld# Preventing some consequences of malicious / bad drivers
call mem_barrier
xorl %eax,%eax
subl $RELOC,%eax
addl $0xd80,%eax
pushl 0x28(%eax)
pushl $0x00000000
pushl 0x3c(%eax)
pushl 0x44(%eax)
pushl 0x40(%eax)
pushl 0x30(%eax)
pushl 0x2c(%eax)
pushl 0x34(%eax)
call system_call# TODO URGENT Normalise argument structure if it had been re-formatted to fit the ABI
addl $32,%esp
movl PerThread_context,%ebx
movl (%ebx),%ebx
testl %ebx,%ebx
jz irupt_80h__noError
movl %ebx,%eax
notl %eax
incl %eax
irupt_80h__noError:

/*
nop
nop
nop
nop
nop
incl %ebx
cmpl $0x07ffffff,%ebx
jnz irupt_80h__noError
*/# NRW


xorl %ebx,%ebx
subl $RELOC,%ebx
addl $0xd80,%ebx
movl %eax,0x28(%ebx)
irupt_80h__iret:
iret
jmp irupt_80h
irupt_80h_sequenceEntry:
.globl irupt_80h_sequenceEntry
xorl %ebp,%ebp
jmp irupt_80h__iret
/*
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
movl %eax,12(%esp)
lret
*/
irupt_fail:
.globl irupt_fail
pushl $0x1badfa17
call bugCheckNum
iret
jmp irupt_fail
irupt_noprocess:
.globl irupt_noprocess
iret
jmp irupt_noprocess
writeLongLinear:# void writeLongLinear(u32 addr, unsigned long dat)
.globl writeLongLinear
movw $0x10,%ax
movw %ax,%es# TODO Does %es need to be saved?
movl 4(%esp),%eax
movl 8(%esp),%ecx
movl %ecx,%es:(%eax)
ret
readLongLinear:# unsigned long readLongLinear(u32 addr)
.globl readLongLinear
movw $0x10,%ax
movw %ax,%es# TODO Does %es need to be saved?
movl 4(%esp),%eax
movl %es:(%eax),%ecx
movl %ecx,%eax
ret
/*
runELF:# int runELF(void* elfPhys, void* memAreaPhys, struct Thread_state* state)
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
*/
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
popl %eax
pushl $0xffffffff
pushl %eax# TODO dec
bugCheckNum:# void bugCheckNum(unsigned long)
.globl bugCheckNum
pushl %ebp
movl %esp,%ebp
movl $12,%ecx
movl %esp,%edx
bugCheckNum__frameCheck:
testl %edx,%edx
jz bugCheckNum__framesFinished
pushl 4(%edx)
movl (%edx),%edx
loop bugCheckNum__frameCheck
bugCheckNum__framesFinished:
jecxz bugCheckNum__traceFinished
bugCheckNum__frameFiller:
pushl $0xffffffff
loop bugCheckNum__frameFiller
bugCheckNum__traceFinished:
pushl %esp
movl 8(%ebp),%eax
pushl %eax
call bugCheckNumWrapped
movl 8(%ebp),%eax
movl %ebp,%esp
popl %ebp
notl %eax
testl %eax,%eax
jnz bugCheckNum__noAdjustment
popl %eax
addl $4,%esp
pushl %eax
bugCheckNum__noAdjustment:
ret
/*
Thread_restore:# void Thread_restore(struct Thread_state*, long)
.globl Thread_restore# Only invoke this from interrupt handlers called because of IRQ interrupts
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
movl %eax,12(%esp)
lret
*/
/*
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
lret
*/
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
loadSegs:# void loadSegs(void);
.globl loadSegs
movw %ds,%ax
movw %ax,%ss
movw %ax,%es
movw %ax,%fs
movw %ax,%gs
ret
irupt_yield:
.globl irupt_yield
movl $0x00000001,handlingIRQ
cld
call irupt_handler_yield
movl $0x00000000,handlingIRQ
iret
jmp irupt_yield
yield_iruptCall:# void yield_iruptCall(void)
.globl yield_iruptCall
int $0x40
ret
getEFL:# u32 getEFL(void)
.globl getEFL
pushfl# TODO URGENT Confirm mnemonic
popl %eax
ret
