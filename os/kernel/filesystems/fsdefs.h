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




u64 hashstr(u8* str) {
    u64 hash = 0;
    int c;
    while ((c = *str++) != 0) {
        hash = c + (hash << 6) + (hash << 16) - hash;
    }
    return hash;
}


#define TSFSVERSION "NOSPEC"

/*
stores data about the file system as a whole
*/
typedef struct {
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