#ifndef __TSFSCORE_H__
#define __TSFSCORE_H__ 1

#include "./fsdefs.h"
// #include <string.h>
#ifndef __MOCKTEST
void strcpy(char* dst, const char* src) {
    while (src && dst) {
        *dst = *src;
        src++;
        dst++;
    }
}
#else
#include <string.h>
#endif

/*
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
void write_u16be(FileSystem* fs, u16 n) {
    char ptr[2];
    ptr[0] = (char)((n>>8)&0xff);
    ptr[1] = (char)(n&0xff);
    (fs->fdrive->write)(fs->kfd,ptr,2);
}

u64 read_u64be(FileSystem* fs) {
    char ptr[8];
    (fs->fdrive->read)(fs->kfd,ptr,8);
    u64 n = 0;
    n |= ((u64)ptr[0]) << 56;
    n |= ((u64)ptr[1]) << 48;
    n |= ((u64)ptr[2]) << 40;
    n |= ((u64)ptr[3]) << 32;
    n |= ((u64)ptr[4]) << 24;
    n |= ((u64)ptr[5]) << 16;
    n |= ((u64)ptr[6]) << 8;
    n |= ((u64)ptr[7]);
    return n;
}
u16 read_u16be(FileSystem* fs) {
    char ptr[2];
    (fs->fdrive->read)(fs->kfd,ptr,2);
    return ((u16)ptr[0])<<8 | (u16)ptr[1];
}

/*
available for external use
DO NOT CALL OUTSIDE THE CASE THAT A NEW PARTITION IS BEING MADE
RETURNS ZERO ON ERROR
*/
FileSystem* createFS(struct FileDriver* fdr, int kfd, u64 p_start, u64 p_size, u8 blocksize, u64 curr_time) {
    if (blocksize < 10 || blocksize > 32) {
        return 0;
    }
    FileSystem* fs = (FileSystem*) allocate(sizeof(FileSystem));
    fs -> fdrive = fdr;
    fs -> kfd = kfd;
    fs -> partition_start = p_start;
    TSFSRootBlock* rblock = (TSFSRootBlock*) allocate(sizeof(TSFSRootBlock));
    fs -> rootblock = rblock;
    strcpy(rblock->version, TSFSVERSION);
    rblock->block_size = 2<<(blocksize-1);
    rblock->partition_size = p_size;
    rblock->creation_time = curr_time;
    write_u64be(fs, rblock->partition_size);
    u8 x = 64;
    fs->fdrive->write(fs->kfd, &x, 1);
    write_u64be(fs, 0);
    fs->fdrive->write(fs->kfd, rblock->version, 16);
    write_u16be(fs, rblock->block_size);
    write_u64be(fs, rblock->top_dir);
    write_u64be(fs, 0x00ff00ff00ff00ff);
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
    TSFSRootBlock* rblock = (TSFSRootBlock*) allocate(sizeof(TSFSRootBlock));
    rblock->partition_size = read_u64be(fs);
    fs->fdrive->read(fs->kfd, rblock->system_size, 1);
    rblock->creation_time = read_u64be(fs);
    fs->fdrive->read(fs->kfd, rblock->version, 16);
    rblock->block_size = read_u16be(fs);
    rblock->top_dir = read_u64be(fs);
    rblock->checksum = read_u64be(fs);
    fs -> rootblock = rblock;
    return fs;
}

#endif