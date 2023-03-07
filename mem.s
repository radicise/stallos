.text
.code16
alloc:/*dhulbDoc-v202:function;a16 alloc(u16) call16;*/
	pushw %bp
	movw %sp,%bp
	pushw %ebx
	movw 4(%bp),%dx
	xorb %ah,%ah
	testw $0x0f,%dx
	setnz %al
	shrw $0x04,%dx
	addw %al,%dx
	xorw %cx,%cx
	movb %cl,%al
	incw %cx
	movw $0xfe00,%bx
	alloc_loop:
	incw %ax
	testb (%bx),%cl
	jnz alloc_nex
	xorw %ax,%ax
	alloc_nex:
	cmpw %ax,%dx
	jz alloc_solv
	testw %bx,%bx
	jz alloc_fail
	shlb %cl
	jcxz alloc_inc
	jmp alloc_loop
	alloc_inc:
	incw %bx
	incb %cl
	jmp alloc_loop
	alloc_fail:
	xorw %ax,%ax
	jmp alloc_end
	alloc_solv:
	testw %dx,%dx
	jz alloc_sl
	orb %cl,(%bx)
	shrb %cl,$1
	jcxz alloc_uinc
	jmp alloc_solv
	allov_uinc:
	movb $0x80,%cl
	decw %dx
	jmp alloc_solv
	alloc_sl:
	shlw %bx,$3
	bsrw %cx,%cx
	addw %cx,%bx
	shlw %bx,$4
	alloc_end:
	popw %dx
	movb %bp,%sp
	popw %bp
	retw
