#ifndef __TSFSCORE_H__
#define __TSFSCORE_H__ 1

#include "FileDriver.h"
#include "types.h"
#include "fsdefs.h"
#include <string.h>

/*
available for external use
DO NOT CALL OUTSIDE THE CASE THAT A NEW PARTITION IS BEING MADE
RETURNS ZERO ON ERROR
*/
FileSystem* createFS(struct FileDriver* fdr, int kfd, u64 p_start, u8 blocksize) {
    if (blocksize < 10 || blocksize > 32) {
        return 0;
    }
    FileSystem* fs = (FileSystem*) allocate(sizeof(FileSystem));
    fs -> fdrive = fdr;
    fs -> kfd = kfd;
    fs -> partition_start = p_start;
    TSFSRootBlock* rblock = (TSFSRootBlock*) allocate(sizeof(TSFSRootBlock));
    strcpy(rblock->version, TSFSVERSION);
    rblock->block_size = 2<<blocksize;
    return fs;
}

/*
loads a file system from a disk partition
available for external use
*/
FileSystem* loadFS(struct FileDriver* fdr, int kfd, u64 p_start) {
    FileSystem* fs = (FileSystem*) allocate(sizeof(FileSystem));
    fs -> fdrive = fdr;
    fs -> kfd = kfd;
    fs -> partition_start = p_start;
    return fs;
}

/**
 * writes a `u64` to disk in big endian format
 */
void write_u64be(FileSystem* fs, u64 n) {
    char ptr[8];
    ptr[0] = (char)((n>>56)&0xff);
    ptr[1] = (char)((n>>48)&0xff);
    ptr[2] = (char)((n>>40)&0xff);
    ptr[3] = (char)((n>>32)&0xff);
    ptr[4] = (char)((n>>24)&0xff);
    ptr[5] = (char)((n>>16)&0xff);
    ptr[6] = (char)((n>>8)&0xff);
    ptr[7] = (char)(n&0xff);
    (fs->fdrive->write)(fs->kfd,ptr,8);
}

#endif