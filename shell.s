shell:/*dhulbDoc-v300:function;s16 shell(s16, a16*a16*u8) call16;*/
pushw %bp
movw %sp,%bp
movw %sp,%ax
subw $702,%sp
pushw $0x2f
subw $64,%ax
pushw %ax
subw $128,%ax
pushw %ax
subw $512,%ax
pushw %ax
callw shelld
movw %bp,%sp
popw %bp
retw
shell_do:/*dhulbDoc-v301:function;s16 shell_do(a16*u8, a16*u8, a16*Map, a16*s16) call16;*/
pushw %bp
movw %sp,%bp
subw $32,%sp
movw %sp,%ax
pushw %ax
pushw %ax
pushw %ax
pushw %ax
pushw $15
subw $0x04,%ax
pushw %ax
pushw 4(%bp)
callw parse_args
addw $8,%sp
popw %bx
testw $0xffff,-4(%bx)
movb $0x00,%al
jz shell_do__didntDo
cmpw $15,-4(%bx)
movb $0x03,%al
jz shell_do__didntDo
subw $0x08,%sp
movw %sp,%ax
pushw %ax
pushw %ax
pushw -32(%bp)
call ObjStr_init
addw $0x04,%sp
popw %ax
pushw 8(%bp)
pushw %ax
movw 6(%bp),%bx
movb $0x01,(%bx)
movw 8(%bp),%bx
addw $0x0a,%bx
movw (%bx),%bx
callw *%bx
addw $0x0c,%sp # TODO kill when that is added
movw %ax,%bx
testw %ax,%ax
movb $0x02,%al
jz shell_do__didntDo
callw *%bx # (s16, a16*u8)
movw 10(%bp),%bx
movw %ax,(%bx)
movw %bp,%sp
popw %bp
retw
shell_do__didntDo:
movw 6(%bp),%bx
movb %al,(%bx)
xorw %ax,%ax
movw %bp,%sp
popw %bp
retw
