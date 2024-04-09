.text
write:# TODO Capture errors of all of the kernel wrapper functions
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
testcall:
.globl testcall
pushl %ebx
movl 8(%esp),%ebx
movl $0x401,%eax
int $0x80
popl %ebx
ret
