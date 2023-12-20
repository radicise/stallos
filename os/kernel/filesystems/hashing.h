#ifndef __HASHING_H__
#define __HASHING_H__ 1
#include "./fsdefs.h"
#include "./sizedberw.h"
// #include <stdio.h>
// #include <string.h>

#if TSFSROOTBLOCK_DSIZE - 53
#error "ROOTBLOCK HASH NO LONGER VALID"
#endif

u64 hash_rootblock(TSFSRootBlock* rb) {
    unsigned char hashbuf[TSFSROOTBLOCK_DSIZE-8];
    unsigned char* ptr = (unsigned char*) hashbuf;
    awriteu16be(hashbuf, rb->breakver);
    awriteu64be(ptr+2, rb->partition_size);
    hashbuf[10] = rb->system_size;
    awriteu64be(ptr+11, rb->creation_time);
    for (int i = 0; i < 16; i ++) {
        hashbuf[19+i] = rb->version[i];
    }
    awriteu16be(ptr+35, rb->block_size);
    awriteu64be(ptr+37, rb->top_dir);
    /// debugging
    // for (int i = 0; i < 45; i ++) {
    //     printf("%02x ", hashbuf[i]);
    // }
    // putchar('\n');
    return hashsized(ptr, TSFSROOTBLOCK_DSIZE-8);
}

#endif