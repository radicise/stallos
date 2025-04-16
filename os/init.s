.text
_exit:# void _exit(int)
.globl _exit
pushl %ebx
movl 8(%esp),%ebx
movl $1,%eax
int $0x80
popl %ebx
ret
write:
.globl write
pushl %ebx
movl 8(%esp),%ebx
movl 12(%esp),%ecx
movl 16(%esp),%edx
movl $4,%eax
int $0x80
popl %ebx
ret
read:
.globl read
pushl %ebx
movl 8(%esp),%ebx
movl 12(%esp),%ecx
movl 16(%esp),%edx
movl $3,%eax
int $0x80
popl %ebx
ret
time:
.globl time
pushl %ebx
movl 8(%esp),%ebx
movl $13,%eax
int $0x80
popl %ebx
ret
stime:
.globl stime
pushl %ebx
movl 8(%esp),%ebx
movl $25,%eax
int $0x80
popl %ebx
ret
brk:# void* brk(void*)
.globl brk
pushl %ebx
movl 8(%esp),%ebx
movl $45,%eax
int $0x80
popl %ebx
ret
fork:
.globl fork
movl $2,%eax
int $0x80
ret
gettid:
.globl gettid
movl $224,%eax
int $0x80
ret
testcall:
.globl testcall
pushl %ebx
movl 8(%esp),%ebx
movl $0x401,%eax
int $0x80
popl %ebx
ret
TNN:
.globl TNN
xorl %ebx,%ebx
tnnd:
movl $13,%eax
int $0x80
jmp tnnd
