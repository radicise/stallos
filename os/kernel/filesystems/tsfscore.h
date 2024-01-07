#ifndef __TSFSCORE_H__
#define __TSFSCORE_H__ 1

#include "./fsdefs.h"
#include "./sizedberw.h"
// #include <string.h>
// #undef __MOCKTEST
#ifndef __MOCKTEST
#define printf(...)
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
available for external use
DO NOT CALL OUTSIDE THE CASE THAT A NEW PARTITION IS BEING MADE
RETURNS FS WITH NULL ROOTBLOCK ON ERROR
*/
FileSystem* createFS(struct FileDriver* fdr, int kfd, u64 p_start, u64 p_size, u8 blocksize, u64 curr_time) {
    if (blocksize < 10 || blocksize > BLK_SIZE_MAX) {
        return 0;
    }
    FileSystem* fs = (FileSystem*) allocate(sizeof(FileSystem));
    fs -> fdrive = fdr;
    fs -> kfd = kfd;
    fs -> partition_start = p_start;
    loff_t verifseek = 0;
    fdr->_llseek(kfd, (off_t)(p_start >> 32), (off_t)(p_start&0xffffffff), &verifseek, SEEK_SET);
    if (verifseek != (loff_t)p_start) {
        return fs;
    }
    TSFSRootBlock* rblock = (TSFSRootBlock*) allocate(sizeof(TSFSRootBlock));
    fs -> rootblock = rblock;
    rblock->breakver = VERNOBN;
    strcpy(rblock->version, TSFSVERSION);
    rblock->block_size = 2<<(blocksize-1);
    rblock->partition_size = p_size;
    rblock->creation_time = curr_time;
    rblock->top_dir = 0;
    // write_u16be(fs, VERNOBN);
    // write_u64be(fs, rblock->partition_size);
    rblock->system_size = 64;
    // fs->fdrive->write(fs->kfd, &rblock->system_size, 1);
    // write_u64be(fs, curr_time);
    // fs->fdrive->write(fs->kfd, rblock->version, 16);
    // write_u16be(fs, rblock->block_size);
    // write_u64be(fs, rblock->top_dir);
    u64 checksum = hash_rootblock(rblock);
    // printf("CHECKSUM: %llx\n", checksum);
    rblock->checksum = checksum;
    write_rootblock(fs, rblock);
    // write_u64be(fs, checksum);
    return fs;
}

// void dbp(FileSystem* fs, char* msg) {
//     loff_t r = 0;
//     fs->fdrive->_llseek(fs->kfd, 0, 0, &r, SEEK_CUR);
//     printf("CURRENT POSITION (%s): %llu\n", msg, r);
// }

/*
loads a file system from a disk partition
available for external use

ON ERROR:
returns a filesystem object with a null rootblock or a rootblock with zero breakver
*/
FileSystem* loadFS(struct FileDriver* fdr, int kfd, u64 p_start) {
    FileSystem* fs = (FileSystem*) allocate(sizeof(FileSystem));
    fs -> fdrive = fdr;
    fs -> kfd = kfd;
    fs -> partition_start = p_start;
    loff_t verifseek = 0;
    fdr->_llseek(kfd, (off_t)(p_start >> 32), (off_t)(p_start&0xffffffff), &verifseek, SEEK_SET);
    if (verifseek != (loff_t)p_start) {
        return fs;
    }
    // u16 breakvno = read_u16be(fs);
    // if (breakvno != VERNOBN) {
    //     return fs;
    // }
    TSFSRootBlock* rblock = (TSFSRootBlock*) allocate(sizeof(TSFSRootBlock));
    fs -> rootblock = rblock;
    // rblock->breakver = breakvno;
    // dbp(fs, "part size");
    // rblock->partition_size = read_u64be(fs);
    // dbp(fs, "sys size");
    // fs->fdrive->read(fs->kfd, &(rblock->system_size), 1);
    // dbp(fs, "creat time");
    // rblock->creation_time = read_u64be(fs);
    // dbp(fs, "version");
    // fs->fdrive->read(fs->kfd, &(rblock->version), 16);
    // dbp(fs, "blok size");
    // rblock->block_size = read_u16be(fs);
    // dbp(fs, "top dir");
    // rblock->top_dir = read_u64be(fs);
    // dbp(fs, "checksum");
    // rblock->checksum = read_u64be(fs);
    // printf("OCHECKSUM: %llx\n", rblock->checksum);
    read_rootblock(fs, rblock);
    if (rblock->breakver != VERNOBN) {
        rblock->breakver = 0;
        return fs;
    }
    u64 comphash = hash_rootblock(rblock);
    // printf("NCHECKSUM: %llx\n", comphash);
    if (comphash != rblock->checksum) {
        rblock->breakver = 0;
    }
    return fs;
}

int longseek(FileSystem* fs, loff_t offset, int whence) {
    loff_t x = 0;
    return fs->fdrive->_llseek(fs->kfd, offset>>32, offset&0xffffffff, &x, whence);
}
int seek(FileSystem* fs, off_t offset, int whence) {
    return fs->fdrive->lseek(fs->kfd, offset, whence);
}

struct PosDat {
    TSFSDataBlock bloc;
    u32   poff; // how far the start of the data of the block is into the entity
};

// traverses the data blocks until it reaches the specified data position, then returns the location on disk
// returns zero if OOB
// when this function returns, the current position on disk will be the start of the data block
struct PosDat traverse(FileSystem* fs, TSFSStructNode* sn, u32 position) {
    struct FileDriver* fdr = fs->fdrive;
    int kfd = fs->kfd;
    struct PosDat pd;
    TSFSDataBlock databloc;
    longseek(fs, sn->data_loc, SEEK_SET);
    read_datablock(fs, &databloc);
    u32 cpos = 0; // current position as seen by program, not absolute location on disk
    while (1) {
        // check if this block contains target position
        if (cpos + databloc.data_length >= position) {
            // write info to containing struct
            pd.poff = cpos;
            pd.bloc = databloc;
            break;
        }
        cpos += databloc.data_length;
        // tail of a chain, traverse as linked list
        if (databloc.storage_flags & TSFS_SF_TAIL_BLOCK) {
            longseek(fs, databloc.next_block, SEEK_SET);
            read_datablock(fs, &databloc);
            continue;
        }
        // within a chain, traverse as array list
        longseek(fs, fs->rootblock->block_size - TSFSDATABLOCK_DSIZE, SEEK_CUR);
        read_datablock(fs, &databloc);
    }
    longseek(fs, databloc.disk_loc, SEEK_SET);
    return pd;
}

int data_write(FileSystem* fs, TSFSStructNode* sn, u32 position, const void* data, size_t size) {
    struct PosDat data_loc = traverse(fs, sn, position);
    u32 realoffset = position-data_loc.poff;
    seek(fs, realoffset + TSFSDATABLOCK_DSIZE, SEEK_CUR);
    size_t cpos = 0;
    // while (data_loc.bloc.data_length - realoffset < size) {
    //     //
    // }
    if (size > 0) {
        write_buf(fs, data+cpos, size);
    }
    return 0;
}

int data_read(FileSystem* fs, TSFSStructNode* sn, u32 position, void* data, size_t size) {
    struct PosDat data_loc = traverse(fs, sn, position);
    u32 realoffset = position - data_loc.poff;
    seek(fs, realoffset + TSFSDATABLOCK_DSIZE, SEEK_CUR);
    size_t cpos = 0;
    if (size > 0) {
        read_buf(fs, data+cpos, size);
    }
    return 0;
}

#endif