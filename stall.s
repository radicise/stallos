/*
  stall.s
  File created on 2022-08-22 by root
*/
.globl _start
.text
.code16
_start: # TODO migrate to protected mode
    movw $0x07c0,%ax
    movw %ax,%ss
    movw $510,%bp
    movw $510,%sp
    movw $0x50,%ax
    movw %ax,%ds
    movb %dl,0
    movb $0x00,%ah
    movb $0x03,%al
    int $0x10
    movb $80,0x02
    movb $25,0x04
    movw $0x00,0x06
    movb $0x01,0x0c
    movw $0xb800,0x0a # TODO support Hercules-MDA graphics area
    movb $0x01,%ah
    movw $0x1706,%cx
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
    movw $15,%cx
    kree:
    movw $0x01,%dx
    call _cann
    incw %ax
    addw $0x0200,%bx
    loop kree
    # call _dishe
    # call test
    movw $0x100,%ax
    movw %ax,%ss
    movw $0x6bff,%ax
    movw %ax,%bp
    movw %ax,%sp
    movw $0x50,%ax
    movw %ax,%ds
    jmp _seac # TODO Use a far jump with %cs=0x07c0 so that it is sure that code segment space does not run out
_cahh: # TODO read and write files using numbers (terminal in / out will also have [a] number[s])
    cmpw $0x01,%dx # TODO migrate to interrupts
    jz lad
    cmpw $0x02,%dx
    jz lam
    cmpw $0x03,%dx
    jz lak
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
_cann:
    # doc:
    # Saves %ax, %bx, %cx, %si, %di, %ds, and %es in the stack before calling the service specified in %ds, popping the saved registers back afterwards
    # Does not make service call if the service number is not valid
    # Does not check if the service is loaded in memory
    # %dx=0x0001 - bootedDiskette144_service
    # %dx=0x0002 - print_service
    # %dx=0x0003 - read_service
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
.space 256 - dist_twty
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
    jc test # TODO proper error handling
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
    xorw %ax,%ax
    steel:
    movw $0x03,%dx
    call _cann
    cmpb $0x0a,%es:(%bx)
    jz holk
    cmpb $0x0d,%es:(%bx)
    jz holk
    healk:
    movw $2,%dx
    call _cann
    jmp steel
    holk:
    movb $0x0a,%es:(%bx)
    movw $2,%dx
    call _cann
    pushw $0x0880
    pushw $0x0000
    movw %sp,%di
    pushw %ds
    movw $0x0080,%ax
    movw %ax,%ds
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
    jc test # TODO proper error handling
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
    # %ax - Read ASCII or 16-bit code (ax=0x00000: ASCII, ax!=0x0000: 16-bit code)
    # %es:(%bx) - Destination location start
    # %cx - Length in units
    # Cannot cross segment boundary of %es
    jcxz reand
    test %ax,%ax
    jz rheat
    coalm:
    movb $0x00,%ah
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
