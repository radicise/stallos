trace:/*dhulbDoc-v301:function;u16 trace(a16*StackTracer) call16;*/
pushw %bp
movw %sp,%bp
pushw 4(%bp)
pushw %bp
callw trace_stack
movw %bp,%sp
popw %bp
retw
trace_stack:/*dhulbDoc-v301:function;u16 trace_stack(a16, a16*StackTracer) call16;*/
pushw %bp
movw %sp,%bp
pushw 6(%bp)
movw 4(%bp),%bx
xorw %ax,%ax
movw %ax,6(%bp)
trace_stack__loop:
testw %bx,%bx
jz trace_stack__end
incw 6(%bp)
movw %bx,4(%bp)
pushw %bx
pushw 2(%bx)
callw stack_printer
addw $0x04,%sp
movw 4(%bp),%bx
movw (%bx),%bx
jmp trace_stack__loop
trace_stack__end:
movw 6(%bp),%ax
movw %bp,%sp
popw %bp
retw
