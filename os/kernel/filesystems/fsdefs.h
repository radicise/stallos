#ifndef __FSDEFS_H__
#define __FSDEFS_H__ 1

#ifndef __MOCKTEST
#include "../types.h"
extern void* alloc(size_t);
extern void dealloc(void*, size_t);
extern u64 compute_checksum(size_t*);
extern u8 validate_checksum(size_t*, u64);
#define allocate alloc
#define deallocate dealloc
#else
typedef int pid_t;
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned long u32;
typedef unsigned long long u64;
typedef char s8;
typedef short s16;
typedef long s32;
typedef long long s64;
typedef unsigned int uid32_t;
typedef unsigned long long loff_t;
#include <stdlib.h>
#include <stdio.h>

void dalloc(void* p, size_t s) {
    free(p);
}

#define allocate malloc
#define deallocate dalloc
#endif
#include "../FileDriver.h"
// imagine there are '.'s between each digit
#define VERNOHI 001
#define VERNOLO 002
// only one that really counts, any change between this and what is on disk will result in failure, BN stand for breaking number (version of breaking changes)
#define VERNOBN 4

#define TSFS_MAX_HELD_NODES 100

/*
PYGENSTART
pygen-include
"./fsdefs.h"
pygen-end
*/

/*
defines the format for generating helper read/write functions for TSFS structs
PYGENSTART
pygen-hf-fmt
name: tsfs
name: $default
writerpre: $none
readerpre: $none
objname: FileSystem
sbufread: areadu{size}be
sbufwrite: awriteu{size}be
bufcpy: awrite_buf
hashf: hashsized
pygen-end
*/

/*
PYGENSTART
pygen-macro
*/
#define NAME_LENGTH 64

// no blocks greater than 4 megabytes
#define BLK_SIZE_MAX 22



u64 hashstr(u8* str) {
    u64 hash = 0;
    int c;
    while ((c = *str++) != 0) {
        hash = c + (hash << 6) + (hash << 16) - hash;
    }
    return hash;
}

u64 hashsized(void* vdata, off_t size) {
    u8* data = (u8*) vdata;
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
    comment: the size of root block data stored on disk
    name: TSFSROOTBLOCK_DSIZE
    pygen-mk-rw: rootblock
    */
    u16   breakver;
    u32   partition_size;
    u64   creation_time;
    char  version[16];
    u16   block_size;
    u32   top_dir;
    // LH blocks used
    u32   usedleft;
    // RH blocks used
    u32   usedright;
    u64   checksum;
} TSFSRootBlock;

// is this block valid? (allows for fast delete by not overwriting data)
#define TSFS_SF_VALID 0b1
// what kind of thing is this? (file, directory, link, data)
#define TSFS_SF_KIND  0b1110
// is this block the head of a run of blocks storing data for the same owner?
#define TSFS_SF_HEAD_BLOCK 0b100000
// is this block the tail of a run of blocks storing data for the same owner?
#define TSFS_SF_TAIL_BLOCK 0b010000
// does this block contain a checksum for its data?
#define TSFS_SF_CHECKSUM 0b1000000
// is this block the last one belonging to a file?
#define TSFS_SF_FINAL_BLOCK 0b10000000
// IF HEAD & FINAL & !TAIL THEN FIRST

#define TSFS_KIND_DATA 0
#define TSFS_KIND_DIR  1
#define TSFS_KIND_FILE 2
#define TSFS_KIND_LINK 3
#define TSFS_KIND_HARD 4

typedef struct {
    /*
    PYGENSTART
    comment: size of data block data stored on disk
    name: TSFSDATABLOCK_DSIZE
    pygen-mk-rw: datablock
    */
    // flags on how the block is stored and its status
    u8    storage_flags;
    // how many hard links refer to this
    u8    refcount;
    // location on disk of this block
    u32   disk_loc;
    u32   next_block;
    u32   prev_block;
    // how much of this block actually contains data
    u16   data_length;
    // CAN ONLY BE NON-ZERO FOR THE FIRST BLOCK OF A CONTIGUOUS GROUP
    u8    blocks_to_terminus;
    u64   metachecksum;
    u64   datachecksum;
} TSFSDataBlock;

/*
stores an individual node, does not occupy a full block, see TSFSStructBlock for the block form
*/
typedef struct {
    /*
    PYGENSTART
    comment: size of structnode
    name: TSFSSTRUCTNODE_DSIZE
    pygen-mk-rw: structnode
    */
    u8    storage_flags;
    u16   nameid;
    u32   data_loc;
    u32   disk_loc;
    u32   parent_loc;
    u32   blocks; // number of blocks forming the data of this node
    char  name[NAME_LENGTH];
    u64   checksum;
} TSFSStructNode;

/*
contains up to 63 child entries
*/
typedef struct {
    /*
    PYGENSTART
    comment: size of structblock
    name: TSFSSTRUCTBLOCK_DSIZE
    */
    u8    entrycount;
    u8    flags;
    u16   nameid;
    u32   disk_loc;
    u64   checksum;
} TSFSStructBlock;

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
    TSFSStructNode TSFS_NODE_TABLE[TSFS_MAX_HELD_NODES];
} FileSystem;

void awriteu64be(unsigned char* buf, u64 n) {
    buf[0] = (unsigned char)((n>>56)&0xff);
    buf[1] = (unsigned char)((n>>48)&0xff);
    buf[2] = (unsigned char)((n>>40)&0xff);
    buf[3] = (unsigned char)((n>>32)&0xff);
    buf[4] = (unsigned char)((n>>24)&0xff);
    buf[5] = (unsigned char)((n>>16)&0xff);
    buf[6] = (unsigned char)((n>>8)&0xff);
    buf[7] = (unsigned char)(n&0xff);
}
void awriteu32be(unsigned char* buf, u32 n) {
    buf[0] = (unsigned char)((n>>24)&0xff);
    buf[1] = (unsigned char)((n>>16)&0xff);
    buf[2] = (unsigned char)((n>>8)&0xff);
    buf[3] = (unsigned char)(n&0xff);
}
void awriteu16be(unsigned char* buf, u16 n) {
    buf[0] = (unsigned char)((n>>8)&0xff);
    buf[1] = (unsigned char)(n&0xff);
}

u64 areadu64be(unsigned char* buf) {
    return
    (((u64)(buf[0])) << 56) |
    (((u64)(buf[1])) << 48) |
    (((u64)(buf[2])) << 40) |
    (((u64)(buf[3])) << 32) |
    (((u64)(buf[4])) << 24) |
    (((u64)(buf[5])) << 16) |
    (((u64)(buf[6])) << 8) |
    ((u64)(buf[7]));
}
u32 areadu32be(unsigned char* buf) {
    return
    (((u32)buf[0]) << 24) |
    (((u32)buf[1]) << 16) |
    (((u32)buf[2]) << 8) |
    ((u32)buf[3]);
}
u16 areadu16be(unsigned char* buf) {
    return ((u16)buf[0])<<8 | (u16)buf[1];
}

void awrite_buf(void* dstp, void* srcp, size_t size) {
    u8* dst = (u8*)dstp;
    u8* src = (u8*)srcp;
    for (int i = 0; i < size; i ++) {
        *(dst+i) = *(src+i);
    }
}

void write_u64be(FileSystem* fs, u64 n) {
    unsigned char ptr[8];
    awriteu64be(ptr, n);
    (fs->fdrive->write)(fs->kfd,ptr,8);
}
void write_u32be(FileSystem* fs, u32 n) {
    unsigned char ptr[4];
    awriteu32be(ptr, n);
    (fs->fdrive->write)(fs->kfd,ptr,4);
}
void write_u16be(FileSystem* fs, u16 n) {
    unsigned char ptr[2];
    awriteu16be(ptr, n);
    (fs->fdrive->write)(fs->kfd,ptr,2);
}
void write_u8(FileSystem* fs, u8 n) {
    (fs->fdrive->write)(fs->kfd,&n,1);
}
void write_buf(FileSystem* fs, const void* buf, size_t size) {
    (fs->fdrive->write)(fs->kfd,buf,size);
}

u64 read_u64be(FileSystem* fs) {
    unsigned char ptr[8];
    (fs->fdrive->read)(fs->kfd,ptr,8);
    return areadu64be(ptr);
}
u32 read_u32be(FileSystem* fs) {
    unsigned char ptr[4];
    (fs->fdrive->read)(fs->kfd,ptr,4);
    return areadu32be(ptr);
}
u16 read_u16be(FileSystem* fs) {
    unsigned char ptr[2];
    (fs->fdrive->read)(fs->kfd,ptr,2);
    return areadu16be(ptr);
}
u8 read_u8(FileSystem* fs) {
    u8 n = 0;
    (fs->fdrive->read)(fs->kfd,&n,1);
    return n;
}

void read_buf(FileSystem* fs, void* buf, size_t size) {
    (fs->fdrive->read)(fs->kfd,buf,size);
}
#include "./gensizes.h"

int longseek(FileSystem* fs, loff_t offset, int whence) {
    loff_t x = 0;
    // if (whence == SEEK_SET) {
    //     offset += fs->partition_start;
    // }
    if (whence == SEEK_END) {
        offset = fs->partition_start + fs->rootblock->partition_size - offset;
        whence = SEEK_SET;
    }
    return fs->fdrive->_llseek(fs->kfd, offset>>32, offset&0xffffffff, &x, whence);
}
int seek(FileSystem* fs, off_t offset, int whence) {
    // if (whence == SEEK_SET) {
    //     longseek(fs, 0, SEEK_SET);
    //     whence = SEEK_CUR;
    // }
    if (whence == SEEK_END) {
        longseek(fs, 0, SEEK_END);
        offset *= -1;
        whence = SEEK_CUR;
    }
    return fs->fdrive->lseek(fs->kfd, offset, whence);
}

/*
like seek, but goes in increments of the block size
if abs is zero, offset is absolute, otherwise relative
negative absolute seeking is invalid
*/
int block_seek(FileSystem* fs, s32 offset, char abs) {
    u16 bs = fs->rootblock->block_size;
    if (abs) {
        return fs->fdrive->lseek(fs->kfd, (off_t)(((s32)bs) * offset), SEEK_CUR);
    }
    if (offset < 0) {
        return -1;
    }
    loff_t ac = ((u64)bs) * ((u64)offset);
    loff_t x = 0;
    return fs->fdrive->_llseek(fs->kfd, ac>>32, ac&0xffffffff, &x, SEEK_SET);
}

void tsfs_dummy_flush(FileSystem* fs) {}

#ifndef fsflush
#define fsflush(fs) fs->fdrive->fsync(fs->kfd);
#endif

#include "./diskmanip.h"

#endif