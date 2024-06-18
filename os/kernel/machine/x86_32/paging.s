CR3Load:#void CR3Load(unsigned long);
.globl CR3Load
movl 4(%esp),%eax
movl %eax,%cr3
ret
WPPGSetup:#void WPPGSetup(void);
.globl WPPGSetup
movl %cr0,%eax
orl $0x80000000,%eax
andl $0xfffeffff,%eax
movl %eax,%cr0
ret
TLBReload:#void TLBReload(void);
.globl TLBReload
movl %cr3,%eax
movl %eax,%cr3
ret
