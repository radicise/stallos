loadGDT:# void loadGDT(unsigned long, unsigned long);
.globl loadGDT
pushw $0x0000
pushl 6(%esp)
pushw 14(%esp)
lgdt (%esp)
addl $8,%esp
ret
loadIDT:# void loadIDT(unsigned long, unsigned long);
.globl loadIDT
pushw $0x0000
pushl 6(%esp)
pushw 14(%esp)
lidt (%esp)
addl $8,%esp
ret
loadTS:# void loadTS(unsigned long);
.globl loadTS
movw 4(%esp),%ax
ltr %ax
ret
storeTS:# unsigned long storeTS(void);
.globl storeTS
xorl %eax,%eax
str %ax
ret
loadLDT:# void loadLDT(unsigned long);
.globl loadLDT
movw 4(%esp),%ax
lldt %ax
ret
setNT:# void setNT(void);
.globl setNT
pushfl
orl $0x00004000,(%esp)
popfl
ret
