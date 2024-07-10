#ifndef __INPUT_H__
#define __INPUT_H__ 1
#include <stdio.h>
#include <stdlib.h>
#include "./dynstr.h"
#include "strops.h"

char* readline(void) {
    size_t c = 32; // capacity
    size_t p = 0; // current position in buffer
    // buffer allocs use c+1 to account for the needed null termination
    char* x = (char*)malloc(c+1);
    if (x == NULL) { // check that malloc succeeded
        return NULL;
    }
    while (1) {
        int ch = getchar();
        if (ch == -1) { // check getchar is ok
            free(x);
            return NULL;
        }
        // ignore CR
        if (ch == 13) continue;
        if (ch == 10) { // LF was hit, end of line
            x[p++] = 0; // add null termination
            return x;
        }
        x[p++] = (char)ch;
        if (p == c) { // reallocate buffer if full
            c *= 2;
            char* n = (char*)malloc(c+1); // alloc new buffer
            if (n == NULL) { // check that new buffer is ok
                free(x);
                return NULL;
            }
            for (int i = 0; i < p; i ++) { // transfer data
                n[i] = x[i];
            }
            free(x); // free old buffer
            x = n;
        }
    }
}

typedef struct {
    int clicode;
    void* data;
} CLIData;

CLIData clihelper(void) {
    CLIData ret = {0};
    while (1) {
        ret.clicode = -1; // set code for error
        printf("> ");
        char* ptr = readline();
        if (ptr == NULL) { // check that readline went ok
            return ret;
        }
        printf("%s\n", ptr);
        int r = -1;
        // string is "exit"
        if (stringcmp(ptr, "exit")) { // 0
            r = 0;
        // string is "goto {path}"
        } else if (startswith(ptr, "goto ")) { // 1
            r = 1;
            size_t plen = stringlen("goto ");
            size_t flen = stringlen(ptr);
            if (flen <= plen) {
                r = -2;
                goto end;
            }
            char* dst = ptr + plen;
            size_t dlen = stringlen(dst);
            ret.data = malloc(dlen+1);
            if (ret.data == NULL) {
                free(ptr);
                return ret;
            }
            strcopy(dst, ret.data);
        // string is "make [d|f] {name}"
        } else if (startswith(ptr, "make ")) { // 2 & 3
            printf("MAKE\n");
            size_t plen = stringlen("make ");
            printf("PLEN: %zu\n", plen);
            size_t flen = stringlen(ptr);
            printf("FLEN: %zu\n", flen);
            if (flen-2 <= plen) {
                r = -2;
                goto end;
            }
            char* typ = ptr + plen;
            char* dst = typ + 2;
            printf("TYP: %s\nDST: %s\n", typ, dst);
            int t = 0;
            if (startswith(typ, "d ")) { // directory
                printf("directory\n");
            } else if (startswith(typ, "f ")) { // file
                printf("file\n");
                t = 1;
            } else {
                printf("invalid: %c\n", typ[0]);
                r = -3;
                goto end;
            }
            r = t + 2;
            size_t dlen = stringlen(dst);
            ret.data = malloc(dlen+1);
            if (ret.data == NULL) {
                free(ptr);
                return ret;
            }
            strcopy(dst, ret.data);
            printf("RB: %s\n", ret.data);
        }
        end:
        free(ptr);
        if (r < 0) { // no match, try again
            if (r == -1) {
                printf("unrecognized command, use \"help\" to see a list of commands\n");
            } else if (r == -2) {
                printf("incomplete command\n");
            } else if (r == -3) {
                printf("invalid command args\n");
            } else {
                printf("bad error code: %d\n", r);
            }
            continue;
        }
        ret.clicode = r;
        return ret;
    }
    return ret;
}

#endif
