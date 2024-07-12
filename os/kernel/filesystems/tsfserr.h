#ifndef __TSFSERR_H__
#define __TSFSERR_H__ 1
/*
various things to do with errors
*/
//10101010010100011100
//10100001000100001100
/*
x = c ^ e
c ^ x == e
*/
#ifndef __MOCKTEST
#define PUTS(...) printf(__VA_ARGS__)
#else
#include <stdio.h>
int PUTS(const char* c) {while (*c){putchar((int)*c);c++;}return 0;}
#endif
#include "../errno.h"
#include "./fsdefs.h"
#define FEWAND  0x1
#define FEALLOC 0x2
#define FEFULL  0x4
#define FEBIG   0x8
#define FESMALL 0x10
// // coolaid
// #define FEDRIVE 0xc001a1d0
#define FEDRIVE 0x800
#define FEINVL  0x20
#define FEARG   0x40
#define FETEST  0x80
#define FEIMPL  0x100
#define FEDATA  0x200
#define FEOP    0x400
int magic_check(unsigned long code, long err) {
    // long x = code ^ err;
    // return ((code ^ x) == err);
    return (code & err) == err;
}
void _magic_smoke(unsigned long ecode, long line, const char* fin, const char* fun) {
    printf("SOURCE {%ld} of {%s} {%s}\n", line, fin, fun);
    PUTS("THE FOLLOWING CAUSED THE MAGIC SMOKE TO ESCAPE:\n");
    printf("%lx -- ", ecode);
    if (magic_check(ecode, FETEST)) {
        PUTS("TESTING");
    }
    if (magic_check(ecode, FEWAND)) {
        PUTS("MAGIC");
    }
    if (magic_check(ecode, FEDRIVE)) {
        PUTS("DRIVE");
    }
    if (magic_check(ecode, FEDATA)) {
        PUTS("DATA");
    }
    if (magic_check(ecode, FEOP)) {
        PUTS("OPERATION");
    }
    if (magic_check(ecode, FEARG)) {
        PUTS("ARG");
    }
    if (magic_check(ecode, FEINVL)) {
        PUTS("INVAL");
    }
    if (magic_check(ecode, FEBIG)) {
        PUTS("BIG");
    }
    if (magic_check(ecode, FESMALL)) {
        PUTS("SMALL");
    }
    if (magic_check(ecode, FEALLOC)) {
        PUTS("ALLOC");
    }
    if (magic_check(ecode, FEFULL)) {
        PUTS("FULL");
    }
    if (magic_check(ecode, FEIMPL)) {
        PUTS("UNIMPL");
    }
    if (ecode < 0) {
        PUTS("WISP");
    }
    PUTS("\n");
    bugCheckNum(ecode);
}

#define magic_smoke(code) _magic_smoke(code, __LINE__, __FILE__, __func__)

#endif
