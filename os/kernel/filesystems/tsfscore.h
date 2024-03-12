#ifndef __TSFSCORE_H__
#define __TSFSCORE_H__ 1

#include "./fsdefs.h"
// #include "./tsfsmanage.h"
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
    printf("FS ALLOC, %p\n", fs);
    fs -> fdrive = fdr;
    fs -> kfd = kfd;
    fdr->lseek(kfd, 0, SEEK_SET);
    printf("BMAKE\n");
    tsmagic_make(fs);
    printf("AMAKE\n");
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
    TSFSStructNode snode = {
        .storage_flags = TSFS_KIND_DIR,
        .size = 0,
        .parent_loc = ((u64)(rblock->block_size)) + rblock->top_dir,
        .disk_loc = rblock->top_dir,
        .pnode = 0,
        .blocks = 0,
        .data_loc = 0
    };
    snode.name[0] = '/';
    // _DBG_print_node(&snode);
    longseek(fs, rblock->top_dir, SEEK_SET);
    write_structnode(fs, &snode);
    TSFSStructBlock sblock = {0};
    sblock.disk_ref = rblock->top_dir;
    sblock.flags = TSFS_CF_DIRE;
    sblock.entrycount = 0;
    // printf("NODE: %llx\nBLOC: %llx\n", sblock.disk_loc, snode.parent_loc);
    longseek(fs, snode.parent_loc, SEEK_SET);
    write_structblock(fs, &sblock);
    rv.retptr = fs;
    // write_u64be(fs, checksum);
    return rv;
}

void releaseFS(FileSystem* fs) {
    deallocate(fs->rootblock, sizeof(TSFSRootBlock));
    tsfs_magic_release(fs);
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
    tsmagic_make(fs);
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

/*
allocates count contiguous blocks and returns the block number of the first one (left to right)
returns zero if unable to allocate the requested blocks

area is boolean and designates which way to search for available blocks, zero is start to end, one is end to start
*/
u32 allocate_blocks(FileSystem* fs, u8 area, u16 count) {
    u32 ul = fs->rootblock->usedleft;
    u32 ur = fs->rootblock->usedright;
    if (area) {
        ur -= count;
        if (ur < ul) { // ensure no overwrite
            return 0;
        }
        fs->rootblock->usedright = ur;
        return ur;
    }
    ul += count;
    if (ul > ur) { // ensure no overwrite
        return 0;
    }
    fs->rootblock->usedleft = ul;
    return ul - count;
}

int _tsfs_free_struct_sbcsfe_do(FileSystem* fs, TSFSSBChildEntry* ce, void* data) {
    if (tsfs_cmp_cename(ce->name, (char*)data)) {
        seek(fs, -16, SEEK_CUR);
        ce->dloc = areadu64be(((unsigned char*)data)+9);
        write_childentry(fs, ce);
        return 1;
    }
    return 0;
}

/*
frees a structure block
!!WARNING!!
the block being freed MUST be inaccessible by other blocks
all cleanup MUST be done prior to freeing it
*/
int tsfs_free_structure(FileSystem* fs, u32 block_no) {
    u32 ul = fs->rootblock->usedleft;
    if (block_no >= ul || block_no < 3) { // protect bounds
        printf("BLKNO: %lu\n", block_no);
        magic_smoke(FEDRIVE | FEARG | FEBIG);
        return -1;
    }
    fs->rootblock->usedleft -= 1;
    ul --;
    longseek(fs, 0, SEEK_SET);
    write_rootblock(fs, fs->rootblock);
    printf("AFTER ROOT UPDATE\n");
    printf("NULLING BLOCK {%lu}\n", block_no);
    getchar();
    dmanip_null(fs, block_no, 1); // destroy invalid data
    getchar();
    if (block_no == ul) { // freed the last used block, nothing else needs to be done
        return 0;
    }
    printf("AFTER EARLY END CHECK\n");
    u64 np = ((u64)(block_no)) * ((u64)(fs->rootblock->block_size));
    printf("AFTER CALC, NEW POS: %llx, NEW BLK: %lu\n", np, tsfs_loc_to_block(fs, np));
    // TSFSStructBlock sblock = {0};
    // TSFSStructNode snode = {0};
    TSFSStructBlock* blockptr = 0;
    TSFSStructNode* nodeptr = 0;
    block_seek(fs, (s32)ul, 0);
    // read_structblock(fs, &sblock);
    blockptr = tsfs_load_block(fs, 0);
    printf("AFTER BLOCK LOAD, PTR = %p\n", blockptr);
    _DBG_print_block(blockptr);
    u64 comphash = hash_structblock(blockptr);
    printf("AFTER HASH\n");
    printf("COMP HASH = %llx\n", comphash);
    printf("PTRHASH = %llx\n", blockptr->checksum);
    // check if the final block is a struct block by reading it as one and seeing if the hash is correct
    if (blockptr->checksum == comphash) { // struct block
        printf("FREE BLOCK\n");
        _DBG_print_block(blockptr);
        // loc_seek(fs, blockptr->disk_ref);
        // read_structnode(fs, &snode);
        nodeptr = tsfs_load_node(fs, blockptr->disk_ref);
        _DBG_print_node(nodeptr);
        // snode.parent_loc = np;
        nodeptr->parent_loc = np;
        // sblock.disk_loc = snode.parent_loc;
        blockptr->disk_loc = np;
        dmanip_null(fs, ul, 1); // ensure random invalid data isn't sticking around on the drive
        loc_seek(fs, nodeptr->parent_loc); // move the struct block to its new location
        write_structblock(fs, blockptr);
    } else { // struct node
        printf("FREE NODE\n");
        seek(fs, -16, SEEK_CUR);
        // read_structnode(fs, &snode);
        nodeptr = tsfs_load_node(fs, 0);
        _DBG_print_node(nodeptr);
        dmanip_null(fs, ul, 1);
        if (nodeptr->parent_loc) {
            tsfs_unload(fs, blockptr);
            // loc_seek(fs, snode.parent_loc);
            // read_structblock(fs, &sblock);
            blockptr = tsfs_load_block(fs, nodeptr->parent_loc);
            // sblock.disk_ref = np;
            blockptr->disk_ref = np;
            seek(fs, -16, SEEK_CUR);
            write_structblock(fs, blockptr);
        }
        block_seek(fs, (s32)block_no, 0);
        write_structnode(fs, nodeptr);
        nodeptr->disk_loc = np;
        // TSFSStructNode pnode = {0};
        // TSFSStructBlock pblock = {0};
        TSFSStructNode* pnp = 0;
        TSFSStructBlock* pbp = 0;
        // loc_seek(fs, snode.pnode);
        // read_structnode(fs, &pnode);
        pnp = tsfs_load_node(fs, nodeptr->pnode);
        // loc_seek(fs, pnode.parent_loc);
        // read_structblock(fs, &pblock);
        pbp = tsfs_load_block(fs, pnp->parent_loc);
        char buf[17];
        tsfs_mk_ce_name(buf, nodeptr->name, strlen(nodeptr->name)+1);
        awriteu64be(((unsigned char*)buf)+9, np);
        tsfs_sbcs_foreach(fs, pbp, _tsfs_free_struct_sbcsfe_do, buf);
        tsfs_unload(fs, pnp);
        tsfs_unload(fs, pbp);
    }
    tsfs_unload(fs, blockptr);
    tsfs_unload(fs, nodeptr);
    return 0;
}

/*
frees all data blocks starting from the given one
!!WARNING!!
cleanup MUST be complete BEFORE freeing data blocks
*/
int tsfs_free_data(FileSystem* fs, u32 block_no) {
    u32 ur = fs->rootblock->usedright;
    if (block_no >= ur) { // protect bounds
        return -1;
    }
    return 0;
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