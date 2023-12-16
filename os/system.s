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
movl %ebp,errno
call system_call# TODO Normalise argument structure if it had been re-formatted to fit the ABI
movl errno,%ebp
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
writePhysical:# void writePhysical(u32 byteAddr, u32 byte)
.globl writePhysical
movw $0x10,%ax
movw %ax,%es
movl 4(%esp),%eax
movb 8(%esp),%cl
movb %cl,%es:(%eax)
ret
readPhysical:# u32 readPhysical(u32 byteAddr)
.globl readPhysical
movw $0x10,%ax
movw %ax,%es
movl 4(%esp),%eax
movb %es:(%eax),%cl
xorl %eax,%eax
movb %cl,%al
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
bus_out_u32:
.globl bus_out_u32
bus_out_long:
.globl bus_out_long
movw 4(%esp),%dx
movl 8(%esp),%eax
outl %eax,%dx
ret
bus_out_u16:
.globl bus_out_u16
movw 4(%esp),%dx
movw 8(%esp),%ax
outw %ax,%dx
ret
bus_out_u8:
.globl bus_out_u8
movw 4(%esp),%dx
movb 8(%esp),%al
outb %al,%dx
ret
bus_in_u32:
.globl bus_in_u32
bus_in_long:
.globl bus_in_long
movw 4(%esp),%dx
inl %dx,%eax
ret
bus_in_u16:
.globl bus_in_u16
movw 4(%esp),%dx
inw %dx,%ax
ret
bus_in_u8:
.globl bus_in_u8
movw 4(%esp),%dx
inb %dx,%al
ret
bus_wait:
.globl bus_wait
nop
nop
nop
nop
nop
ret
bus_outBlock_u32:
.globl bus_outBlock_u32
bus_outBlock_long:/* void bus_outBlock_long(u16, const long*, unsigned long) */
.globl bus_outBlock_long
movw 4(%esp),%dx
movl 12(%esp),%ecx
pushl %esi
movl 12(%esp),%esi
rep outsl
popl %esi
ret
bus_outBlock_u16:
.globl bus_outBlock_u16
movw 4(%esp),%dx
movl 12(%esp),%ecx
pushl %esi
movl 12(%esp),%esi
rep outsw
popl %esi
ret
bus_outBlock_u8:
.globl bus_outBlock_u8
movw 4(%esp),%dx
movl 12(%esp),%ecx
pushl %esi
movl 8(%esp),%esi
rep outsb
popl %esi
ret
bus_inBlock_u32:
.globl bus_inBlock_u32
bus_inBlock_long:/* void bus_inBlock_long(u16, long*, unsigned long) */
.globl bus_inBlock_long
movw 4(%esp),%dx
movl 12(%esp),%ecx
pushl %edi
movl 12(%esp),%edi
pushw %es
movw %ds,%ax
movw %ax,%es
rep insl
popw %es
popl %edi
ret
bus_inBlock_u16:
.globl bus_inBlock_u16
movw 4(%esp),%dx
movl 12(%esp),%ecx
pushl %edi
movl 12(%esp),%edi
pushw %es
movw %ds,%ax
movw %ax,%es
rep insw
popw %es
popl %edi
ret
bus_inBlock_u8:
.globl bus_inBlock_u8
movw 4(%esp),%dx
movl 12(%esp),%ecx
pushl %edi
movl 12(%esp),%edi
pushw %es
movw %ds,%ax
movw %ax,%es
rep insb
popw %es
popl %edi
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
Mutex_acquire:# void Mutex_acquire(Mutex* mutex)
.globl Mutex_acquire
movl 4(%esp),%edx
xorb %al,%al
incb %al
Mutex_acquire__loop1:
lock xchgb %al,(%edx)
testb %al,%al
jz Mutex_acquire__end
nop
jmp Mutex_acquire__loop1
Mutex_acquire__end:
ret
Mutex_release:# void Mutex_release(Mutex* mutex)
.globl Mutex_release
movl 4(%esp),%edx
xorb %al,%al
lock xchgb %al,(%edx)
ret
Mutex_tryAcquire:# int Mutex_tryAcquire(Mutex* mutex)
.globl Mutex_tryAcquire
movl 4(%esp),%edx
xorb %al,%al
incb %al
lock xchgb %al,(%edx)
xorb $0x01,%al
ret
Mutex_initUnlocked:# void Mutex_initUnlocked(Mutex* mutex)
.globl Mutex_initUnlocked
movl 4(%esp),%edx
xorb %al,%al
lock xchgb %al,(%edx)
ret
Mutex_wait:
.globl Mutex_wait
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
nop
nop
nop
nop
nop
nop
ret
timeIncrement:
.globl timeIncrement
lock incl currentTime
ret
timeFetch:
.globl timeFetch
movl currentTime,%eax# TODO Ensure atomic memory reads
ret
timeStore:
.globl timeStore
movl 4(%esp),%eax
movl %eax,currentTime# TODO Ensure atomic memory writes
ret
