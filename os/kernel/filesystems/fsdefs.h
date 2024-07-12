#ifndef __FSDEFS_H__
#define __FSDEFS_H__ 1

// #include "./fsmacros.h"
#ifndef __MOCKTEST
// #include "../types.h"
#include "./tsfsstd.h"
#define allocate alloc
#define deallocate dealloc
#else
typedef int pid_t;
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long u64;
typedef char s8;
typedef short s16;
typedef int s32;
typedef long s64;
typedef unsigned int uid32_t;
#ifdef __DARWIN_BYTE_ORDER
typedef unsigned long long loff_t;
#endif
#define kernelWarnMsg(msg) ;
#include <stdlib.h>
#include <stdio.h>
// #include <string.h>

// #error "reminder, use double indirection for inode tables"

void dalloc(void* p, size_t s) {
    free(p);
}
#define allocate malloc
#define deallocate dalloc
#endif
// size_t strlen(const char* s) {
//     size_t c = 0;
//     while (*s) {
//         c ++;
//         s ++;
//     }
//     return c;
// }
// #ifndef SEEK_CUR
// #define SEEK_SET 0
// #define SEEK_CUR 1
// #define SEEK_END 2
// #endif
// #include "./tsfsconst.h"
#include "../FileDriver.h"
#include "../fsiface.h"
// typedef struct FSReturn FSRet;
#include "./tsfserr.h"
#define u48 u64
// imagine there are '.'s between each digit
#define VERNOHI 001
#define VERNOLO 002
// only one that really counts, any change between this and what is on disk will result in failure, BN stand for breaking number (version of breaking changes)
#define VERNOBN 8

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
#define BLOCK_SIZE 1024


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
    pygen-over: [block_size] 2 {
        write: ((u16)`block_size`)
        block_size: ((u64)`$TMP`)
    }
    PYGENSTART
    comment: the size of root block data stored on disk
    name: TSFSROOTBLOCK_DSIZE
    pygen-mk-rw: rootblock
    */
    u16   breakver;
    u8    partition_size;
    u64   creation_time;
    char  version[16];
    // u64   block_size;
    u32   top_dir;
    // LH blocks used
    u32   usedleft;
    // RH blocks used
    u32   usedright;
    u32   usedhalfleft;
    u32   usedhalfright;
    u64   checksum;
} TSFSRootBlock;

// // is this block valid? (allows for fast delete by not overwriting data)
// #define TSFS_SF_VALID 0b1
// all data blocks have this set
// #define TSFS_SF_LIVE 0b1
#define TSFS_SF_LIVE 1
// what kind of thing is this? (file, directory, link, data)
// #define TSFS_SF_KIND  0b1110
#define TSFS_SF_KIND  12
// is this block the head of a run of blocks storing data for the same owner?
// #define TSFS_SF_HEAD_BLOCK 0b10
#define TSFS_SF_HEAD_BLOCK 2
// is this block the tail of a run of blocks storing data for the same owner?
// #define TSFS_SF_TAIL_BLOCK 0b0100
#define TSFS_SF_TAIL_BLOCK 4
// does this block contain a checksum for its data?
// #define TSFS_SF_CHECKSUM 0b1
#define TSFS_SF_CHECKSUM 1
// is this block the last one belonging to a file?
// #define TSFS_SF_FINAL_BLOCK 0b1000
#define TSFS_SF_FINAL_BLOCK 8
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
    c - create
    m - modify
    a - access
    s - secs
    n - nsec
    */
    s64 cs;
    s64 cn;
    s64 ms;
    s64 mn;
    s64 as;
    s64 an;
} TIMES;

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
    u32   disk_loc;
    u16   rc;
    u8    id;
    //
    // number of hard links to this data, max is 0x7fff, or 32767
    u16   refcount;
    // disk location of the first data block
    u32   head;
    // blocks used by data
    u32   blocks;
    // size of data in bytes
    u64   size;
    // permissions and other metadata
    // permissions
    u16   perms;
    // WARNING: DO NOT ACCESS THIS FIELD, USE THE [get_dhtimes] AND [set_dhtimes] HELPER FUNCTIONS
    u64   times[6];
    u64   checksum;
} TSFSDataHeader;

void get_dhtimes(TSFSDataHeader* dh, TIMES* times) {
    s64* timelist = (s64*)(dh->times);
    times->cs = timelist[0];
    times->cn = timelist[1];
    times->ms = timelist[2];
    times->mn = timelist[3];
    times->as = timelist[4];
    times->an = timelist[5];
}
void set_dhtimes(TSFSDataHeader* dh, TIMES* times) {
    s64* timelist = (s64*)(dh->times);
    timelist[0] = times->cs;
    timelist[1] = times->cn;
    timelist[2] = times->ms;
    timelist[3] = times->mn;
    timelist[4] = times->as;
    timelist[5] = times->an;
}

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
    u32   disk_loc;
    u16   rc;
    u8    id;
    //
    // flags on how the block is stored and its status
    u8    storage_flags;
    // // how many hard links refer to this
    // u8    refcount;
    u32   next_block;
    u32   prev_block;
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
    u32   disk_loc;
    u16   rc;
    u8    id;
    //
    u8    storage_flags;
    u32   data_loc;
    // location of the child table, name is shot because the child tables used to own the nodes
    u32   parent_loc;
    // parent node
    u32   pnode;
    // inode number
    u32   inum;
    // u32   blocks; // number of blocks forming the data of this node
    // u64   size;
    char  name[255];
    u64   checksum;
} TSFSStructNode;

/*
contains up to 72 child entries
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
    u32   disk_loc;
    u16   rc;
    u8    id;
    //
    u8    entrycount;
    u8    flags;
    u32   disk_ref;
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
    u32   dloc;
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

#include "./tsfsrw.h"

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

u32 tsfs_loc_to_block(u64 loc) {
    return (u32)(loc/BLOCK_SIZE);
}

/*
like seek, but goes in increments of the block size
if abs is zero, offset is absolute, otherwise relative
negative absolute seeking is invalid
*/
int block_seek(FileSystem* fs, s32 offset, char abs) {
    if (abs) {
        return fs->fdrive->lseek(fs->kfd, (off_t)(((s32)BLOCK_SIZE) * offset), SEEK_CUR);
    }
    if (offset < 0) {
        return -1;
    }
    loff_t ac = ((u64)BLOCK_SIZE) * (u64)(*((u32*)&offset));
    loff_t x = 0;
    return fs->fdrive->_llseek(fs->kfd, ac>>32, ac&0xffffffff, &x, SEEK_SET);
}

// #define loc_seek(fs, loc) block_seek(fs, (s32)((loc>>7)*fs->rootblock->block_size + (loc&7f)*128));

int loc_seek(FileSystem* fs, u48 loc) {
    return longseek(fs, loc, SEEK_SET);
}

void read_childentry(FileSystem* fs, TSFSSBChildEntry* ce) {
    unsigned char b[14];
    read_buf(fs, b, 14);
    awrite_buf(ce->name, b, 9);
    ce->flags = b[9];
    ce->dloc = areadu32be(b+10);
}
void write_childentry(FileSystem* fs, TSFSSBChildEntry* ce) {
    unsigned char b[14];
    unsigned char* bp = (unsigned char*)b;
    awrite_buf(bp, ce->name, 9);
    b[9] = ce->flags;
    awriteu32be(bp+10, ce->dloc);
    write_buf(fs, bp, 14);
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

void __DBG_print_block(TSFSStructBlock*, long, const char*, const char*);
void __DBG_print_node(TSFSStructNode*, long, const char*, const char*);
void __DBG_print_cename(char const*, long, const char*, const char*);
void __DBG_print_child(TSFSSBChildEntry*, long, const char*, const char*);
void __DBG_print_head(TSFSDataHeader*, long, const char*, const char*);
void __DBG_print_data(TSFSDataBlock*, long, const char*, const char*);
void __DBG_here(long, const char*, const char*);
#define _DBG_print_block(sb) __DBG_print_block(sb, __LINE__, __FILE__, __func__)
#define _DBG_print_node(sn) __DBG_print_node(sn, __LINE__, __FILE__, __func__)
#define _DBG_print_cename(name) __DBG_print_cename(name, __LINE__, __FILE__, __func__)
#define _DBG_print_child(ce) __DBG_print_child(ce, __LINE__, __FILE__, __func__)
#define _DBG_print_head(dh) __DBG_print_head(dh, __LINE__, __FILE__, __func__)
#define _DBG_print_data(db) __DBG_print_data(db, __LINE__, __FILE__, __func__)
#define _DBG_here() __DBG_here(__LINE__, __FILE__, __func__)

#include "./funcdefs.h"
#include "./diskmanip.h"
#include "./tsfs_magic.h"
#include "./tsfshelpers.h"
#include "./itable.h"

#endif
