SimpleMutex_acquire:# void SimpleMutex_acquire(SimpleMutex* mutex)
.globl SimpleMutex_acquire
movl 4(%esp),%edx
xorb %al,%al
incb %al
SimpleMutex_acquire__loop1:
lock xchgb %al,(%edx)
testb %al,%al
jz SimpleMutex_acquire__end
nop
jmp SimpleMutex_acquire__loop1
SimpleMutex_acquire__end:
ret
SimpleMutex_release:# void SimpleMutex_release(SimpleMutex* mutex)
.globl SimpleMutex_release
movl 4(%esp),%edx
xorl %eax,%eax
lock xchgb %al,(%edx)
ret
SimpleMutex_tryAcquire:# int SimpleMutex_tryAcquire(SimpleMutex* mutex)
.globl SimpleMutex_tryAcquire
movl 4(%esp),%edx
xorl %eax,%eax
incb %al
lock xchgb %al,(%edx)
xorb $0x01,%al
ret
SimpleMutex_initUnlocked:# void SimpleMutex_initUnlocked(SimpleMutex* mutex)
.globl SimpleMutex_initUnlocked
movl 4(%esp),%edx
xorb %al,%al
lock xchgb %al,(%edx)
ret
SimpleMutex_wait:# TODO URGENT Account for differing CPU clock speeds
.globl SimpleMutex_wait
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
AtomicULong_get:
.globl AtomicULong_get
movl 4(%esp),%eax
movl (%eax),%eax# TODO URGENT Support processors that aren't Intel486-or-post-Intel486 Intel processors
ret
AtomicULong_set:
.globl AtomicULong_set
movl 4(%esp),%eax
movl 8(%esp),%edx
movl %edx,(%eax)# TODO URGENT Support processors that aren't Intel486-or-post-Intel486 Intel processors
ret
AtomicULong_inc:
.globl AtomicULong_inc
movl 4(%esp),%eax
lock incl (%eax)
ret
AtomicULong_dec:
.globl AtomicULong_dec
movl 4(%esp),%eax
lock decl (%eax)
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
bus_wait:# TODO URGENT Is more waiting needed?
.globl bus_wait
nop
nop
nop
nop
nop
ret
bus_outBlock_u32:
.globl bus_outBlock_u32
bus_outBlock_long:/* void bus_outBlock_long(unsigned long, const long*, unsigned long) */
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
bus_inBlock_long:/* void bus_inBlock_long(unsigned long, long*, unsigned long) */
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
mem_barrier:
.globl mem_barrier
subl $4,%esp
lock xchgl %eax,(%esp)# TODO Change for 80486 and newer
popl %eax
ret
