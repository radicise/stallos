SimpleMutex_acquire:# void SimpleMutex_acquire(SimpleMutex* mutex)
.globl SimpleMutex_acquire
xorb %al,%al
incb %al
SimpleMutex_acquire__loop1:
lock xchgb %al,(%rdi)
testb %al,%al
jz SimpleMutex_acquire__end
nop
jmp SimpleMutex_acquire__loop1
SimpleMutex_acquire__end:
ret
SimpleMutex_release:# void SimpleMutex_release(SimpleMutex* mutex)
.globl SimpleMutex_release
xorq %rax,%rax
lock xchgb %al,(%rdi)
ret
SimpleMutex_tryAcquire:# int SimpleMutex_tryAcquire(SimpleMutex* mutex)
.globl SimpleMutex_tryAcquire
xorq %rax,%rax
incb %al
lock xchgb %al,(%rdi)
xorb $0x01,%al
ret
SimpleMutex_initUnlocked:# void SimpleMutex_initUnlocked(SimpleMutex* mutex)
.globl SimpleMutex_initUnlocked
xorq %rax,%rax
lock xchgb %al,(%rdi)
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
movq (%rdi),%rax# TODO URGENT Support processors that aren't Pentium-or-post-Pentium Intel processors
ret
AtomicULong_set:
.globl AtomicULong_set
movq %rsi,(%rdi)# TODO URGENT Support processors that aren't Pentium-or-post-Pentium Intel processors
ret
AtomicULong_inc:
.globl AtomicULong_inc
lock incq (%rdi)
ret
AtomicULong_dec:
.globl AtomicULong_dec
lock decq (%rdi)
ret
bus_out_u32:
.globl bus_out_u32
movq %rdi,%rdx
movq %rsi,%rax
outl %eax,%dx
ret
bus_out_u16:
.globl bus_out_u16
movq %rdi,%rdx
movq %rsi,%rax
outw %ax,%dx
ret
bus_out_u8:
.globl bus_out_u8
movq %rdi,%rdx
movq %rsi,%rax
outb %al,%dx
ret
bus_in_u32:
.globl bus_in_u32
movq %rdi,%rdx
xorq %rax,%rax
inl %dx,%eax
ret
bus_in_u16:
.globl bus_in_long
movq %rdi,%rdx
xorq %rax,%rax
inw %dx,%ax
ret
bus_in_u8:
.globl bus_in_u8
movq %rdi,%rdx
xorq %rax,%rax
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
