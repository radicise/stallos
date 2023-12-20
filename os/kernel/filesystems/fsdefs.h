#ifndef __FSDEFS_H__
#define __FSDEFS_H__ 1

#ifndef __MOCKTEST
#include "../types.h"
extern void* allocate(size_t);
extern u64 compute_checksum(size_t*);
extern u8 validate_checksum(size_t*, u64);
#else
typedef int pid_t;
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned long u32;
typedef unsigned long long u64;
typedef unsigned int uid32_t;
typedef unsigned long long loff_t;
#include <stdlib.h>

#define allocate malloc
#endif
#include "../FileDriver.h"
#include "./gensizes.h"
// imagine there are '.'s between each digit
#define VERNOHI 001
#define VERNOLO 002
// only one that really counts, any change between this and what is on disk will result in failure, BN stand for breaking number (version of breaking changes)
#define VERNOBN 2



u64 hashstr(u8* str) {
    u64 hash = 0;
    int c;
    while ((c = *str++) != 0) {
        hash = c + (hash << 6) + (hash << 16) - hash;
    }
    return hash;
}

u64 hashsized(u8* data, off_t size) {
    // printf("data size: %li\n", size);
    u64 hash = 0;
    int c;
    while (size) {
        c = (*data++);
        // printf("%02x ", c);
        hash = c + (hash << 6) + (hash << 16) - hash;
        size --;
    }
    // putchar('\n');
    return hash;
}

typedef struct {
    u16 length;
    u8* buffer;
} WBuffer;

#define TSFSVERSION "NOSPEC"

/*
stores data about the file system as a whole
u16   breakver;
u64   partition_size;
u8    system_size;
u64   creation_time;
char  version[16];
u16   block_size;
u64   top_dir;
u64   checksum;
*/
typedef struct {
    /*
    PYGENSTART
    comment: the size of rootblock data stored on disk
    name: TSFSROOTBLOCK_DSIZE
    */
    u16   breakver;
    u64   partition_size;
    u8    system_size;
    u64   creation_time;
    char  version[16];
    u16   block_size;
    u64   top_dir;
    u64   checksum;
} TSFSRootBlock;

/*
Custom File System, see the stallOS spec for more details - Tristan
DO NOT modify any instance provided by the file system
*/
typedef struct {
    struct FileDriver* fdrive;
    int kfd;
    // in bytes
    u64 partition_start;
    TSFSRootBlock* rootblock;
} FileSystem;

#endif