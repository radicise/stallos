shell:/*dhulbDoc-v300:function;s16 shell() call16;*/
pushw %bp
movw %sp,%bp
movw %sp,%ax
subw $1022,%sp
pushw $0x2f
subw $512,%ax
pushw %ax
subw $512,%ax
pushw %ax
callw shelld
movw %bp,%sp
popw %bp
retw
