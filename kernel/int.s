int20h:/*dhulbDoc-v301:function;u8 int20h();*/
movw %cs:0x16,%dx
movw %dx,%ds
movw 0x2e,%sp
movw 0x30,%dx
movw %dx,%ss
xorw %ax,%ax
clc
lretw
int21h:/*dhulbDoc-v301:function;u8 int21h();*/
pushw %dx
pushw %cx
pushw %ax
pushw %bx
pushw %si
pushw %di
pushw %dx
pushw %cx
pushw %bx
pushw %ax
callw handler21h#TODO use kernel stack
addw $0x08,%sp
popw %di
popw %si
popw %bx
popw %dx
movw %dh,%ah
popw %cx
popw %dx
iretw
