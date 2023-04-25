shell:/*dhulbDoc-v300:function;s16 shell() call16;*/
pushw %bp
movw %sp,%bp
subw $510,%sp
pushw $0x2f
movw %sp,%ax
pushw %ax
callw shelld
movw %bp,%sp
popw %bp
retw
