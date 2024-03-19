/*
 *
 * stall.s
 * File created on 2022-08-22 by root
 *
 */
.text
.code16
_start:
.globl _start
    ljmpw $0x07c0,$0x0005
    movw $0x07c0,%ax
    movw %ax,%ss
    movw $510,%bp
    movw $510,%sp
    movw $0x50,%ax
    movw %ax,%ds
    movb %dl,0
    movw $0x03,%ax
    int $0x10
    movb $80,0x02
    movb $25,0x04
    movw $0x00,0x06
    movb $0x01,0x0c
    movw $0xb800,0x0a # TODO support Hercules-MDA graphics area
    movb $0x01,%ah
    movw $0x2706,%cx
    int $0x10
    movb $0x07,0x08
    movw $0x07c0,%ax
    movw %ax,%ds
    # call _dishe
    xorb %ah,%ah
    xorb %dl,%dl
    int $0x13
    movw %ds,%ax
    movw %ax,%es
    movw $0x0200,%bx
    movw $1,%ax
    kree:
    movw $0x01,%dx
    call _cann
    incw %ax
    movw %es,%si
    addw $0x0020,%si
    movw %si,%es
    cmpw $0x242,%ax
    jnz kree
    # call _dishe
    # call test
    movw $0x100,%ax
    movw %ax,%ss
    movw $0x6c00,%ax
    movw %ax,%bp
    movw %ax,%sp
    movw $0x50,%ax
    movw %ax,%ds
    ljmpw $0x07c0,$0x0200
_cahh: # TODO read and write files using numbers (terminal in / out will also have [a] number[s])
    cmpw $0x01,%dx
    jz lad
    cmpw $0x02,%dx
    jz lam
    cmpw $0x03,%dx
    jz lak
    cmpw $0x04,%dx
    jz lao
    ret
    lad:
    call _diwr
    ret
    lam:
    call _print
    ret
    lak:
    call _read
    ret
    lao:
    call _boot_kernel32
    ret
_cann:
    # doc:
    # Saves %ax, %bx, %cx, %si, %di, %ds, and %es in the stack before calling the service specified in %ds, popping the saved registers back afterwards
    # Does not make service call if the service number is not valid
    # Does not check if the service is loaded in memory
    # %dx=0x0001 - bootedDiskette144_service
    # %dx=0x0002 - print_service
    # %dx=0x0003 - read_service
    # %dx=0x0004 - bootKernel32_service
    pushw %ax
    pushw %bx
    pushw %cx
    pushw %si
    pushw %di
    pushw %ds
    pushw %es
    call _cahh
    popw %es
    popw %ds
    popw %di
    popw %si
    popw %cx
    popw %bx
    popw %ax
    ret
_dishe:
    pushw %ax
    pushw %es
    pushw %bx
    pushw %cx
    pushw %ds
    pushw %di
    movw $0xb800,%ax
    movw %ax,%es
    movw $0x07c0,%ax
    movw %ax,%ds
    movw $0x1fff,%bx
    movw $0x0fff,%cx
    movw $0x0fff,%di
    bee:
    movb $0x07,%es:(%bx)
    decw %bx
    decw %di
    movb %ds:(%di),%al
    movb %al,%es:(%bx)
    decw %bx
    loop bee
    popw %di
    popw %ds
    popw %cx
    popw %bx
    popw %es
    popw %ax
    jmp test
    ret
    test:
    movw $0xb800,%bx
    movw %bx,%es
    xorw %bx,%bx
    trep:
    incb %al
    movb %al,%es:(%bx)
    nop
    nop
    nop
    jmp trep
.set dist_twty, . - _start
.set dist_twtyar, 256 - dist_twty
.space dist_twtyar
_diwr:
    # TODO implement as an interrupt
    # doc:
    # %ax - Sector number (0-indexed)
    # %es:(%bx) - Storage address
    # Cannot cross segment boundary of %es
    # Several general-purpose registers are scratched, as well as %ds
    movw $0x50,%cx
    movw %cx,%ds
    xorb %ch,%ch
    movb $18,%cl
    divb %cl
    xorb %dh,%dh
    movb %ah,%cl
    incb %cl
    xorb %ah,%ah
    movb $2,%ch
    divb %ch
    movb %al,%ch
    movb %ah,%dh
    movb $1,%al
    movb $2,%ah
    # movb drive,%dl
    movb 0,%dl
    # jmp gtre
    clc
    # call _dishe
    int $0x13
    jc _dishe # TODO proper error handling
    # jmp gtre
    # movb %,%dh
    # call _dishe
    ret
.set dist_ttqp, . - _start
.space 510 - dist_ttqp
.byte 0x55
.byte 0xaa
_seac:
    jmp shesh
    inpc:
    .byte 0
    .set iinter, inpc - _start
    .set isec, (iinter / 0x10) + 0x07c0
    .set iadd, iinter % 0x10
    prompt:
    .ascii ":# "
    .set plen, (.-prompt)
    .set pinter, prompt - _start
    .set psec, (pinter / 0x10) + 0x07c0
    .set padd, pinter % 0x10
    shesh:
    movw $psec,%ax
    movw %ax,%es
    movw $padd,%bx
    movw $plen,%cx
    movw $0x02,%dx
    call _cann
    movw $isec,%ax
    movw %ax,%es
    movw $iadd,%bx
    movw $0x01,%cx
    xorb %al,%al
    xorw %di,%di
    .set shell_seg, (shell_offset / 16) + 0x7c0
    .set commstart_seg, (shell_offset / 16) + 0x7c0 + staltstd_RESrmstroff
    movw $commstart_seg,%si
    steel:
    movw $0x03,%dx
    call _cann
    cmpb $0x0a,%es:(%bx)
    jz holk
    cmpb $0x0d,%es:(%bx)
    jz holk
    cmpw $0xff,%di
    jz healk
    cmpb $0x20,%es:(%bx)
    jl healk
    cmpb $0x7f,%es:(%bx)
    jz healk
    movb %es:(%bx),%ah
    pushw %es
    movw %si,%es
    movb %ah,%es:(%di)
    incw %di
    popw %es
    healk:
    movw $2,%dx
    call _cann
    jmp steel
    holk:
    movb $0x0a,%es:(%bx)
    movw $2,%dx
    call _cann
    movw %si,%es
    movw $0x00,%es:(%di)
    movw %di,%bx
    pushw $shell_seg
    pushw $0x0000
    movw %sp,%di
    pushw %ds
    movw $0x0080,%ax
    movw %ax,%ds
    movw %bx,%ds:0x1c
    lcall *%ss:(%di)
    popw %ds
    popw %ax
    popw %ax
    jmp shesh
.set dist_tqpwr, . - _start
.space 1024 - dist_tqpwr
_print:
    # doc:
    # %es:%bx - Location of text
    # %cx - Length in bytes
    # Cannot cross segment boundary of %es
    jcxz preq
    movw %cx,%ax
    addw %bx,%ax
    jc _dishe # TODO proper error handling
    movw $0x50,%ax
    movw %ax,%ds
    movw 0x06,%di
    movb 0x08,%dh
    movb 0x02,%ah
    movb 0x04,%al
    movw %ax,%si
    movw 0x0a,%ax
    movw %ax,%ds
    shlw $1,%di
    xchgw %bx,%si
    movb %bh,%al
    mulb %bl
    xchgw %bx,%si
    shlw $1,%ax
    prii:
    movb %es:(%bx),%dl
    cmpb $0x7f,%dl
    jnc steev
    cmpb $0x20,%dl
    jb steew
    movb %dl,%ds:(%di)
    incw %di
    movb %dh,%ds:(%di)
    incw %di
    cmpw %di,%ax
    jz trog
    jmp kroe
    preq:
    jmp pren
    trog:
    xchg %bx,%si
    call allo
    xchg %bx,%si
    kroe:
    jmp steev
    steew:
    cmpb $0x0a,%dl
    jz knell
    jmp steev
    knell:
    pushw %ax
    pushw %ds
    pushw %si
    xchgw %bx,%si
    movw %di,%ax
    shrw $1,%ax
    divb %bh
    incb %al
    cmpb %al,%bl
    jz adli
    jmp aghi
    adli:
    call allo
    aghi:
    movb %bh,%bl
    xorb %bh,%bh
    shrw $1,%di
    addw %bx,%di
    movw $0x50,%bx
    movw %bx,%ds
    popw %si
    cmpb $0x00,0x0c
    jz kreh
    movw %di,%ax
    movw %si,%bx
    divb %bh
    movb %ah,%al
    xorb %ah,%ah
    subw %ax,%di
    kreh:
    shlw $1,%di
    popw %ds
    popw %ax
    jmp steev
    steev:
    incw %bx
    loop prii
    shrw $1,%di
    movw $0x50,%ax
    movw %ax,%ds
    movw %di,0x06
    # cmp
    pren:
    ret
    allo:
    pushw %cx
    pushw %es
    pushw %si
    pushw %bx
    movb %bh,%bl
    xorb %bh,%bh
    shrw $1,%di
    subw %bx,%di
    shlw $1,%di
    xchgw %sp,%bp
    movw (%bp),%bx
    xchgw %sp,%bp
    pushw %di
    decb %bl
    movb %bh,%al
    mulb %bl
    movw %ax,%cx
    xorw %di,%di
    movb %bh,%bl
    xorb %bh,%bh
    movw %ds,%si
    movw %si,%es
    shlw $1,%bx
    movw %bx,%si
    movw %cx,%ax
    cld
    es
    rep
    movsw
    movw %ax,%di
    shlw $1,%di
    shrw $1,%bx
    movw %bx,%cx
    greth:
    movb $0x00,%es:(%di)
    incw %di
    movb %dh,%es:(%di)
    incw %di
    loop greth
    popw %di
    popw %bx
    popw %si
    popw %es
    popw %cx
    ret
.set dist_tqtey, . - _start
.space 1536 - dist_tqtey
_read:
    # doc:
    # %al - Read ASCII or 16-bit code (al=0x00: ASCII, ax!=0x00: 16-bit code)
    # %es:(%bx) - Destination location start
    # %cx - Length in units
    # Cannot cross segment boundary of %es
    jcxz reand
    testb %al,%al
    jz rheat
    coalm:
    xorb %ah,%ah
    int $0x16
    movw %ax,%es:(%bx)
    addw $2,%bx
    loop coalm
    jmp reand
    rheat:
    movb $0x00,%ah
    int $0x16
    movb %al,%es:(%bx)
    incw %bx
    loop rheat
    reand:
    ret
.set dist_twpri, . - _start
.space 2048 - dist_twpri
    call _cann
    lretw
.set dist_twtyp, . - _start
.space 2560 - dist_twtyp
    call _cahh
    lretw
.set dist_tepuw, . - _start
.space 3072 - dist_tepuw
_boot_kernel32:
    jmp a20_enable
    disint:
    inb $0x70,%al
    orb $0x80,%al
    outb %al,$0x70
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    inb $0x71
    cli
    jmp afdis
    a20_enable:
    movw $0x2403,%ax
    clc
    int $0x15
    jc a20_failure
    testb %ah,%ah
    jnz a20_failure
    movw $0x2402,%ax
    clc
    int $0x15
    jc a20_failure
    testb %ah,%ah
    jnz a20_failure
    cmpb $0x01,%al
    jz a20_isset
    movw $0x2401,%ax # TODO maybe wait for a20 for a bit? does the bios only return after it has been set fully?
    clc
    int $0x15
    jc a20_failure
    testb %ah,%ah
    jnz a20_failure
    a20_isset:
    jmp disint
    afdis:
    movw $0x00,%ax
    movw %ax,%ss
    movw $0x818,%sp
    pushw $0x00cf
    pushw $0x9200
    pushw $0x0000
    pushw $0xffff
    pushw $0x00cf
    pushw $0x9a00
    pushw $0x0000
    pushw $0xffff
    pushw $0x0000
    pushw $0x0000
    pushw $0x0000
    pushw $0x0000
    movw $0x0000,%ax
    movw %ax,%ds
    xorl %eax,%eax
    movw %ss,%ax
    shll $4,%eax
    xorl %ebx,%ebx
    movw %sp,%bx
    addl %ebx,%eax
    movl %eax,%edi
    movw %di,%ds:0x0510
    shrl $16,%edi
    movw %di,%ds:0x0512
    pushl %eax
    pushw $23
    subl $6,%eax
    movl %eax,%ebx
    pushl $0xdc00
    pushw $8
    subl $0x06,%eax
    movw $0x01,%cx
    movw %cx,%es
    movl %ebx,%edx
    movl %eax,%ebx
    lgdtl %ds:(%edx)
    movl %cr0,%edx
    orl $0x01,%edx
    movl %edx,%cr0
    xorl %edx,%edx
    incw %dx
    shll $0x10,%edx
    movl %edx,%esp
    movl $0x10,%ebx
    movw %bx,%ss
    movw %bx,%ds
    xorl %ebp,%ebp
    ljmp $0x08,$0xdc00
    a20_failure:
    ret
.set shell_offset, 4096
.set dist_tshell, . - _start
.space shell_offset - dist_tshell
