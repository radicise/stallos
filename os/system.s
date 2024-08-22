.set RELOC , 0x40000
.text
_start:
.globl _start
xorl %ebp,%ebp
movl %esp,(0x0007f7f4 - RELOC)
call systemEntry
lret
irupt_80h:# NOTE: This is under the assumption that %edx can never be used to return anything, which is contrary to the `syscall' Linux man-pages page; TODO resolve this conflict
.globl irupt_80h
cld# Preventing some consequences of malicious / bad drivers; Is this necessary
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
andl $0xfffffff0,%esp
xorl %ebp,%ebp
jmp irupt_80h__iret
irupt_noprocess:
.globl irupt_noprocess
iret
jmp irupt_noprocess
writeLongLinear:# void writeLongLinear(u32 addr, unsigned long dat)
.globl writeLongLinear
movl 4(%esp),%ecx
movl 8(%esp),%eax
movl %eax,-RELOC(%ecx)
ret
readLongLinear:# unsigned long readLongLinear(u32 addr)
.globl readLongLinear
movl 4(%esp),%eax
movl -RELOC(%eax),%eax
ret
int_enable:# TODO How much waiting needs to be done?
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
int_disable:# TODO How much waiting needs to be done?
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
pushl $0x00000000
loop bugCheckNum__frameFiller
bugCheckNum__traceFinished:
pushl %esp
movl 8(%ebp),%eax
pushl %eax
call bugCheckNumWrapped
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
irupt_kfunc:
.globl irupt_kfunc
movl $0x00000001,handlingIRQ
cld
call irupt_handler_kfunc
movl $0x00000000,handlingIRQ
iret
jmp irupt_kfunc
kfunc_iruptCall:# void kfunc_iruptCall(void)
.globl kfunc_iruptCall
int $0x40
ret
getEFL:# u32 getEFL(void)
.globl getEFL
pushfl# TODO URGENT Confirm mnemonic
popl %eax
ret
halt_and_catch_fire:# void halt_and_catch_fire(void)
.globl halt_and_catch_fire
subl $12,%esp
call int_disable
halt_and_catch_fire__loop:
hlt
jmp halt_and_catch_fire__loop
kfunc:
.globl kfunc
pushl %ebp
movl %esp,%ebp
pushl 40(%ebp)
pushl 36(%ebp)
pushl 32(%ebp)
pushl 28(%ebp)
pushl 24(%ebp)
pushl 20(%ebp)
pushl 16(%ebp)
pushl 12(%ebp)
pushl 8(%ebp)
call kfct
movl %ebp,%esp
popl %ebp
ret
