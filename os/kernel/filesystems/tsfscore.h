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

#include "../errno.h"
/*
available for external use
DO NOT CALL OUTSIDE THE CASE THAT A NEW PARTITION IS BEING MADE
RETURNS FS WITH NULL ROOTBLOCK ON ERROR
*/
FSRet createFS(struct FileDriver* fdr, int kfd, u8 p_size, u8 blocksize, s64 curr_time) {
    FSRet rv = {.errno=EINVAL};
    if (blocksize < 10 || blocksize > BLK_SIZE_MAX) {
        return rv;
    }
    if (p_size > 48) {
        return rv;
    }
    rv.errno = 0;
    FileSystem* fs = (FileSystem*) allocate(sizeof(FileSystem));
    fs -> fdrive = fdr;
    fs -> kfd = kfd;
    fdr->lseek(kfd, 0, SEEK_SET);
    TSFSRootBlock* rblock = (TSFSRootBlock*) allocate(sizeof(TSFSRootBlock));
    fs -> rootblock = rblock;
    rblock->breakver = VERNOBN;
    strcpy(rblock->version, TSFSVERSION);
    rblock->block_size = 2<<(blocksize-1);
    rblock->partition_size = p_size;
    rblock->creation_time = *((u64*)(&curr_time));
    // write_u16be(fs, VERNOBN);
    // write_u64be(fs, rblock->partition_size);
    rblock->usedleft = 3;
    rblock->usedright = (u32)((((u64)2)<<(rblock->partition_size-1)) / rblock->block_size) - 1;
    // fs->fdrive->write(fs->kfd, &rblock->system_size, 1);
    // write_u64be(fs, curr_time);
    // fs->fdrive->write(fs->kfd, rblock->version, 16);
    // write_u16be(fs, rblock->block_size);
    // write_u64be(fs, rblock->top_dir);
    rblock->top_dir = (u64)(rblock->block_size);
    u64 checksum = hash_rootblock(rblock);
    // printf("CHECKSUM: %llx\n", checksum);
    rblock->checksum = checksum;
    write_rootblock(fs, rblock);
    TSFSStructNode snode = {0};
    snode.storage_flags = TSFS_KIND_DIR;
    snode.name[0] = '/';
    snode.parent_loc = (u64)(rblock->block_size * 2);
    longseek(fs, rblock->top_dir, SEEK_SET);
    write_structnode(fs, &snode);
    TSFSStructBlock sblock = {0};
    sblock.disk_loc = rblock->top_dir;
    longseek(fs, snode.parent_loc, SEEK_SET);
    write_structblock(fs, &sblock);
    rv.retptr = fs;
    // write_u64be(fs, checksum);
    return rv;
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
FSRet loadFS(struct FileDriver* fdr, int kfd) {
    FileSystem* fs = (FileSystem*) allocate(sizeof(FileSystem));
    fs -> fdrive = fdr;
    fs -> kfd = kfd;
    fs -> err = 0;
    fdr->lseek(kfd, 0, SEEK_SET);
    TSFSRootBlock* rblock = (TSFSRootBlock*) allocate(sizeof(TSFSRootBlock));
    fs -> rootblock = rblock;
    FSRet rv = {.errno=EINVAL};
    read_rootblock(fs, rblock);
    if (rblock->breakver != VERNOBN) {
        kernelWarnMsg("VERSION INCOMPAT");
        releaseFS(fs);
        return rv;
    }
    u64 comphash = hash_rootblock(rblock);
    if (comphash != rblock->checksum) {
        kernelWarnMsg("DATA CORRUPT");
        releaseFS(fs);
        return rv;
    }
    rv.errno = 0;
    rv.retptr = fs;
    return rv;
}

int data_write(FileSystem* fs, TSFSStructNode* sn, u32 position, const void* data, size_t size) {
    struct PosDat data_loc = tsfs_traverse(fs, sn, position);
    if (data_loc.bloc.storage_flags == 0) {
        return -1;
    }
    u32 realoffset = position-data_loc.poff;
    seek(fs, realoffset + TSFSDATABLOCK_DSIZE, SEEK_CUR);
    size_t cpos = 0;
    if (size > 0) {
        write_buf(fs, data+cpos, size);
        longseek(fs, data_loc.bloc.disk_loc, SEEK_SET);
        // loc_seek(fs, data_loc.bloc.disk_loc);
        // block_seek(fs, (s32)data_loc.bloc.disk_loc, BSEEK_SET);
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