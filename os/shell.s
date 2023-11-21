.text
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
