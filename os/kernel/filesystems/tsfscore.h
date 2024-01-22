#ifndef __TSFSCORE_H__
#define __TSFSCORE_H__ 1

#include "./fsdefs.h"
#include "./tsfsmanage.h"
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
FileSystem* createFS(struct FileDriver* fdr, int kfd, u64 p_start, u64 p_size, u8 blocksize, s64 curr_time) {
    if (blocksize < 10 || blocksize > BLK_SIZE_MAX) {
        return 0;
    }
    FileSystem* fs = (FileSystem*) allocate(sizeof(FileSystem));
    fs -> fdrive = fdr;
    fs -> kfd = kfd;
    fs -> partition_start = p_start;
    fdr->lseek(kfd, 0, SEEK_SET);
    TSFSRootBlock* rblock = (TSFSRootBlock*) allocate(sizeof(TSFSRootBlock));
    fs -> rootblock = rblock;
    rblock->breakver = VERNOBN;
    strcpy(rblock->version, TSFSVERSION);
    rblock->block_size = 2<<(blocksize-1);
    rblock->partition_size = p_size;
    rblock->creation_time = *((u64*)(&curr_time));
    rblock->top_dir = 0;
    // write_u16be(fs, VERNOBN);
    // write_u64be(fs, rblock->partition_size);
    rblock->usedleft = 1;
    rblock->usedright = (u32)(rblock->partition_size / rblock->block_size) - 1;
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

void releaseFS(FileSystem* fs) {
    deallocate(fs->rootblock, sizeof(TSFSRootBlock));
    deallocate(fs, sizeof(FileSystem));
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
    fdr->lseek(kfd, 0, SEEK_SET);
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
        releaseFS(fs);
        return 0;
    }
    u64 comphash = hash_rootblock(rblock);
    // printf("NCHECKSUM: %llx\n", comphash);
    if (comphash != rblock->checksum) {
        releaseFS(fs);
        return 0;
    }
    return fs;
}

int data_write(FileSystem* fs, TSFSStructNode* sn, u32 position, const void* data, size_t size) {
    struct PosDat data_loc = tsfs_traverse(fs, sn, position);
    if (data_loc.bloc.storage_flags == 0) {
        return -1;
    }
    u32 realoffset = position-data_loc.poff;
    seek(fs, realoffset + TSFSDATABLOCK_DSIZE, SEEK_CUR);
    size_t cpos = 0;
    // while (data_loc.bloc.data_length - realoffset < size) {
    //     //
    // }
    if (size > 0) {
        write_buf(fs, data+cpos, size);
        longseek(fs, data_loc.bloc.disk_loc, SEEK_SET);
        data_loc.bloc.data_length += (u32)size;
        write_datablock(fs, &data_loc.bloc);
    }
    return 0;
}

// TODO: make work with more than one data block
int data_read(FileSystem* fs, TSFSStructNode* sn, u32 position, void* data, size_t size) {
    struct PosDat data_loc = tsfs_traverse(fs, sn, position);
    if (data_loc.bloc.storage_flags == 0) {
        return -1;
    }
    u32 realoffset = position - data_loc.poff;
    seek(fs, realoffset + TSFSDATABLOCK_DSIZE, SEEK_CUR);
    size_t cpos = 0;
    if (size > 0) {
        read_buf(fs, data+cpos, size);
    }
    return 0;
}

#endif