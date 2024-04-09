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
#define kernelWarnMsg(msg) ;
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void dalloc(void* p, size_t s) {
    free(p);
}
#define allocate malloc
#define deallocate dalloc
#endif
#include "../FileDriver.h"
#include "../fsiface.h"
typedef struct FSReturn FSRet;
#include "./tsfserr.h"
typedef struct FSReturn FSRet;
#define u48 u64
// imagine there are '.'s between each digit
#define VERNOHI 001
#define VERNOLO 002
// only one that really counts, any change between this and what is on disk will result in failure, BN stand for breaking number (version of breaking changes)
#define VERNOBN 6

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

// no blocks greater than 4094 bytes
#define BLK_SIZE_MAX 12



u64 hashstr(void const* strp) {
    u8* str = (u8*)strp;
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
        c = *(data++);
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
    u8    partition_size;
    u64   creation_time;
    char  version[16];
    u16   block_size;
    u48   top_dir;
    // LH blocks used
    u32   usedleft;
    // RH blocks used
    u32   usedright;
    u64   checksum;
} TSFSRootBlock;

// // is this block valid? (allows for fast delete by not overwriting data)
// #define TSFS_SF_VALID 0b1
// all data blocks have this set
#define TSFS_SF_LIVE 0b1
// what kind of thing is this? (file, directory, link, data)
#define TSFS_SF_KIND  0b1110
// is this block the head of a run of blocks storing data for the same owner?
#define TSFS_SF_HEAD_BLOCK 0b10
// is this block the tail of a run of blocks storing data for the same owner?
#define TSFS_SF_TAIL_BLOCK 0b0100
// does this block contain a checksum for its data?
#define TSFS_SF_CHECKSUM 0b1
// is this block the last one belonging to a file?
#define TSFS_SF_FINAL_BLOCK 0b1000
// IF HEAD & FINAL & !TAIL THEN FIRST

#define TSFS_KIND_DATA 0
#define TSFS_KIND_DIR  1
#define TSFS_KIND_FILE 2
#define TSFS_KIND_LINK 3
#define TSFS_KIND_HARD 4

#define TSFS_CF_EXTT 1
#define TSFS_CF_DIRE 2
#define TSFS_CF_HARD 4
#define TSFS_CF_LINK 8

typedef struct {
    /*
    PYGENSTART
    comment: size of data header stored on disk
    name: TSFSDATAHEADER_DSIZE
    pygen-over: [-pyrepos, magicno, disk_loc, rc, id] 0 {
    write: ()
    disk_loc: (_pyrepos)
    id: (3)
    }
    pygen-mk-rw: dataheader
    */
    size_t magicno;
    u48   disk_loc;
    u16   rc;
    u8    id;
    //
    // number of hard links to this data
    u16   refcount;
    // disk location of the first data block
    u48   head;
    // blocks used by data
    u32   blocks;
    // size of data in bytes
    u64   size;
    u64   checksum;
} TSFSDataHeader;

typedef struct {
    /*
    PYGENSTART
    comment: size of data block data stored on disk
    name: TSFSDATABLOCK_DSIZE
    pygen-over: [-pyrepos, storage_flags, data_length, magicno, disk_loc, rc, id] 2 {
    write: (`data_length` | ((u16)`storage_flags`|1)<<12)
    data_length: (`$TMP`&0x0fff)
    storage_flags: (u8)(`$TMP`>>12)
    disk_loc: (_pyrepos)
    id: (2)
    }
    pygen-mk-rw: datablock
    */
    size_t magicno;
    u48   disk_loc;
    u16   rc;
    u8    id;
    //
    // flags on how the block is stored and its status
    u8    storage_flags;
    // // how many hard links refer to this
    // u8    refcount;
    u48   next_block;
    u48   prev_block;
    // how much of this block actually contains data
    u16   data_length;
    // CAN ONLY BE NON-ZERO FOR THE FIRST BLOCK OF A CONTIGUOUS GROUP
    u8    blocks_to_terminus;
    u64   checksum;
} TSFSDataBlock;

/*
stores an individual node, does not occupy a full block, see TSFSStructBlock for the block form
*/
typedef struct {
    /*
    PYGENSTART
    comment: size of structnode
    name: TSFSSTRUCTNODE_DSIZE
    pygen-over: [-pyrepos, disk_loc, magicno, rc, id] 0 {
    write: ()
    disk_loc: (_pyrepos)
    id: (0)
    }
    pygen-mk-rw: structnode
    */
    size_t magicno;
    u48   disk_loc;
    u16   rc;
    u8    id;
    //
    u8    storage_flags;
    u48   data_loc;
    // location of the child table, name is shot because the child tables used to own the nodes
    u48   parent_loc;
    // parent node
    u48   pnode;
    // u32   blocks; // number of blocks forming the data of this node
    // u64   size;
    char  name[255];
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
    pygen-over: [-pyrepos, disk_loc, magicno, rc, id] 0 {
    write: ()
    disk_loc: (_pyrepos)
    id: (1)
    }
    pygen-mk-rw: structblock
    */
    size_t magicno;
    u48   disk_loc;
    u16   rc;
    u8    id;
    //
    u8    entrycount;
    u8    flags;
    u48   disk_ref;
    u64   checksum;
} TSFSStructBlock;

typedef struct {
    size_t csize;
    size_t cused;
    size_t umax;
    size_t hmin;
    void** ptr;
} Magic;

/*
Custom File System, see the stallOS spec for more details - Tristan
DO NOT modify any instance provided by the file system
*/
typedef struct {
    struct FileDriver* fdrive;
    int kfd;
    int err;
    TSFSRootBlock* rootblock;
    // table that handles the magical BS required to do reordering on-the-fly
    Magic* magic;
} FileSystem;

/*
handy way to represent a struct block child entry
*/
typedef struct {
    char  name[9];
    u48   dloc;
    u8    flags;
} TSFSSBChildEntry;


int bufcmp(void const* b1, void const* b2, int size) {
    u8* a = (u8*)b1;
    u8* b = (u8*)b2;
    for (int i = 0; i < size; i ++) {
        if (*(a+i) != *(b+i)) {
            return 0;
        }
    }
    return 1;
}

int tsfs_cmp_name(void const* n1, void const* n2) {
    u8* a = (u8*)n1;
    u8* b = (u8*)n2;
    for (int i = 0; i < 255; i ++) {
        if (*(a+i) != *(b+i)) {
            return 0;
        }
        if (*(a+i) == 0) break;
    }
    return 1;
}

int tsfs_cmp_cename(void const* n1, void const* n2) {
    return bufcmp(n1, n2, 9);
}

int tsfs_cmp_ce(TSFSSBChildEntry* ce1, TSFSSBChildEntry* ce2) {
    if (ce1->flags != ce2->flags) {
        return 0;
    }
    return bufcmp(ce1->name, ce2->name, 9);
}

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
void awriteu48be(unsigned char* buf, u48 n) {
    buf[0] = (unsigned char)((n>>40)&0xff);
    buf[1] = (unsigned char)((n>>32)&0xff);
    buf[2] = (unsigned char)((n>>24)&0xff);
    buf[3] = (unsigned char)((n>>16)&0xff);
    buf[4] = (unsigned char)((n>>8)&0xff);
    buf[5] = (unsigned char)(n&0xff);
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
u48 areadu48be(unsigned char* buf) {
    return
    (((u64)(buf[0])) << 40) |
    (((u64)(buf[1])) << 32) |
    (((u64)(buf[2])) << 24) |
    (((u64)(buf[3])) << 16) |
    (((u64)(buf[4])) << 8) |
    ((u64)(buf[5]));
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

void awrite_buf(void* dstp, void const* srcp, size_t size) {
    u8* dst = (u8*)dstp;
    u8* src = (u8*)srcp;
    for (int i = 0; i < size; i ++) {
        *(dst+i) = *(src+i);
    }
}

void write_u64be(FileSystem* fs, u64 n) {
    unsigned char ptr[8];
    awriteu64be(ptr, n);
    if ((fs->fdrive->write)(fs->kfd,ptr,8)) fs->err = 1;
}
void write_u48be(FileSystem* fs, u48 n) {
    unsigned char ptr[6];
    awriteu48be(ptr, n);
    if ((fs->fdrive->write)(fs->kfd,ptr,6)) fs->err = 1;
}
void write_u32be(FileSystem* fs, u32 n) {
    unsigned char ptr[4];
    awriteu32be(ptr, n);
    if ((fs->fdrive->write)(fs->kfd,ptr,4)) fs->err = 1;
}
void write_u16be(FileSystem* fs, u16 n) {
    unsigned char ptr[2];
    awriteu16be(ptr, n);
    if ((fs->fdrive->write)(fs->kfd,ptr,2)) fs->err = 1;
}
void write_u8(FileSystem* fs, u8 n) {
    if ((fs->fdrive->write)(fs->kfd,&n,1)) fs->err = 1;
}
void write_buf(FileSystem* fs, const void* buf, size_t size) {
    if ((fs->fdrive->write)(fs->kfd,buf,size)) fs->err = 1;
}

u64 read_u64be(FileSystem* fs) {
    unsigned char ptr[8];
    if ((fs->fdrive->read)(fs->kfd,ptr,8)) fs->err = 1;
    return areadu64be(ptr);
}
u48 read_u48be(FileSystem* fs) {
    unsigned char ptr[6];
    if ((fs->fdrive->read)(fs->kfd,ptr,6)) fs->err = 1;
    return areadu48be(ptr);
}
u32 read_u32be(FileSystem* fs) {
    unsigned char ptr[4];
    if ((fs->fdrive->read)(fs->kfd,ptr,4)) fs->err = 1;
    return areadu32be(ptr);
}
u16 read_u16be(FileSystem* fs) {
    unsigned char ptr[2];
    if ((fs->fdrive->read)(fs->kfd,ptr,2)) fs->err = 1;
    return areadu16be(ptr);
}
u8 read_u8(FileSystem* fs) {
    u8 n = 0;
    if ((fs->fdrive->read)(fs->kfd,&n,1)) fs->err = 1;
    return n;
}

void read_buf(FileSystem* fs, void* buf, size_t size) {
    if ((fs->fdrive->read)(fs->kfd,buf,size)) fs->err = 1;
}
u64 tsfs_tell(FileSystem* fs) {
    loff_t x = 0;
    fs->fdrive->_llseek(fs->kfd, 0, 0, &x, SEEK_CUR);
    return (u64)x;
}
#include "./gensizes.h"


int longseek(FileSystem* fs, loff_t offset, int whence) {
    loff_t x = 0;
    // if (whence == SEEK_SET) {
    //     offset += fs->partition_start;
    // }
    if (whence == SEEK_END) {
        offset = fs->rootblock->partition_size - offset;
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
        if (longseek(fs, 0, SEEK_END)) return -1;
        offset *= -1;
        whence = SEEK_CUR;
    }
    return fs->fdrive->lseek(fs->kfd, offset, whence);
}

u32 tsfs_loc_to_block(FileSystem* fs, u64 loc) {
    return (u32)(loc/((u64)(fs->rootblock->block_size)));
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

// #define loc_seek(fs, loc) block_seek(fs, (s32)((loc>>7)*fs->rootblock->block_size + (loc&7f)*128));

int loc_seek(FileSystem* fs, u48 loc) {
    return longseek(fs, loc, SEEK_SET);
}

void read_childentry(FileSystem* fs, TSFSSBChildEntry* ce) {
    unsigned char b[16];
    read_buf(fs, b, 16);
    awrite_buf(ce->name, b, 9);
    ce->flags = b[9];
    ce->dloc = areadu48be(b+10);
}
void write_childentry(FileSystem* fs, TSFSSBChildEntry* ce) {
    unsigned char b[16];
    unsigned char* bp = (unsigned char*)b;
    awrite_buf(bp, ce->name, 9);
    b[9] = ce->flags;
    awriteu48be(bp+10, ce->dloc);
    write_buf(fs, bp, 16);
}

/*
nlen - length of n, including terminating null byte
*/
void tsfs_mk_ce_name(void* vb, char const* n, size_t nlen) {
    unsigned char* b = (unsigned char*)vb;
    char* sb = (char*)vb;
    awriteu32be(b, (u32)(hashstr(n)&0xffffffffllu));
    b[4] = nlen;
    sb[5] = n[0];
    sb[6] = n[1];
    u8 o = nlen == 1 ? 1 : 0;
    sb[7] = n[nlen-3+o];
    sb[8] = n[nlen-2+o];
}

#undef u48

#define BSEEK_SET 0
#define BSEEK_CUR 1

void tsfs_dummy_flush(FileSystem* fs) {}

#ifndef fsflush
#define fsflush(fs) fs->fdrive->fsync(fs->kfd);
#endif

#include "./diskmanip.h"
#include "./tsfs_magic.h"
#include "./tsfshelpers.h"

#endif