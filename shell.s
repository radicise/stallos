#needs a16 alloc(u16) call16
#needs a16 inLine() call16
readArgs:/*dhulbDoc-v202:function;u8 readArgs(a16*ArgStruct) call16;*/
	pushw %si
	pushw %bx
	pushw %di
	call inLine
	pushw %ax
	movw %ax,%si
	xorw %dx,%dx
	readArgs_loop:
	lodsb
	testb %al,%al
	jz readArgs_step2
	testb $0x20,%al
	jnz readArgs_loop
	incw %dx
	jmp readArgs_loop
	readArgs_step2:
	addw $0x02,%dx
	testw $0x8000,%dx
	jnz readArgs_failure
	shlw %dx
	pushw %dx
	call alloc
	popw %bx
	shrw %bx
	decw %bx
	popw %di
	movw %bx,%dx
	movw %ds,%bx
	movw %bx,%es
	movw %ax,%bx
	xorb %si,%si
	movw $0x20,%ax
	xorw %cx,%cx
	notw %cx
	readArgs_loop2:
	movw %di,(%bx,%si)
	addw $0x02,%si
	decw %dx
	testw %dx,%dx
	jz end
	repnz
	scasb
	jmp readArgs_loop2
	end:
	movw $0x00,(%bx,%si)
	movb %bx,%ax
	popw %di
	popw %bx
	popw %si
	retw
