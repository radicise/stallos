#ifndef __TSFS_H__
#define __TSFS_H__ 1
#include "FileDriver.h"
#include "types.h"
#include "fsdefs.h"
#include <string.h>
// TODO
extern void* allocate(size_t);
extern u64 compute_checksum(size_t*);
extern u8 validate_checksum(size_t*, u64);
// extern void strcopy(char*, char*);
/**
 * Custom File System, see the stallOS spec for more details - Tristan
 */
struct FileSystem {
    struct FileDriver* fdrive;
    int kfd;
};
struct TSFSRootBlock {
    u64   partition_start;
    u64   partition_size;
    u8    system_size;
    u64   creation_time;
    char  version[16];
    u64   checksum;
};
/**
 * DO NOT CALL OUTSIDE THE CASE THAT A NEW PARTITION IS BEING MADE
 */
struct FileSystem* createFS(struct FileDriver* fdr, int kfd) {
    struct FileSystem* fs = (struct FileSystem*) allocate(sizeof(FileSystem));
    fs -> fdrive = fdr;
    fs -> kfd = kfd;
    struct TSFSRootBlock* rblock = (struct TSFSRootBlock*) allocate(sizeof(TSFSRootBlock));
    strcpy(rblock->version, TSFSVERSION);
    return fs;
}
#include "fsundefs.h"
#endif