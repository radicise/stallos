#ifndef __TSFSCORE_H__
#define __TSFSCORE_H__ 1

#include "./fsdefs.h"
// #include "./tsfsmanage.h"
// #include <string.h>
// #undef __MOCKTEST
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
available for external use
DO NOT CALL OUTSIDE THE CASE THAT A NEW PARTITION IS BEING MADE
RETURNS FS WITH NULL ROOTBLOCK ON ERROR
*/
FSRet createFS(struct FileDriver* fdr, int kfd, u8 p_size, s64 curr_time) {
    FSRet rv = {.err=EINVAL};
    if (p_size > 48) {
        return rv;
    }
    rv.err = 0;
    FileSystem* fs = (FileSystem*) allocate(sizeof(FileSystem));
    printf("FS ALLOC, %p\n", (void*)fs);
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
    rblock->partition_size = p_size;
    rblock->creation_time = *((u64*)(&curr_time));
    // write_u16be(fs, VERNOBN);
    // write_u64be(fs, rblock->partition_size);
    rblock->usedleft = 3;
    rblock->usedright = (u32)((((u64)2)<<(rblock->partition_size-1)) / BLOCK_SIZE) - 1;
    // fs->fdrive->write(fs->kfd, &rblock->system_size, 1);
    // write_u64be(fs, curr_time);
    // fs->fdrive->write(fs->kfd, rblock->version, 16);
    // write_u16be(fs, rblock->block_size);
    // write_u64be(fs, rblock->top_dir);
    rblock->top_dir = 1;
    u64 checksum = hash_rootblock(rblock);
    // printf("CHECKSUM: %llx\n", checksum);
    rblock->checksum = checksum;
    write_rootblock(fs, rblock);
    TSFSStructNode snode = {
        .storage_flags = TSFS_KIND_DIR,
        // .size = 0,
        .parent_loc = rblock->top_dir + 1,
        .disk_loc = rblock->top_dir,
        .pnode = 0,
        // .blocks = 0,
        .data_loc = 0
    };
    snode.name[0] = '/';
    // _DBG_print_node(&snode);
    block_seek(fs, rblock->top_dir, BSEEK_SET);
    write_structnode(fs, &snode);
    TSFSStructBlock sblock = {0};
    sblock.disk_ref = rblock->top_dir;
    sblock.flags = TSFS_CF_DIRE;
    sblock.entrycount = 0;
    // printf("NODE: %llx\nBLOC: %llx\n", sblock.disk_loc, snode.parent_loc);
    block_seek(fs, snode.parent_loc, BSEEK_SET);
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
    FSRet rv = {.err=EINVAL};
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
    rv.err = 0;
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
        seek(fs, -14, SEEK_CUR);
        ce->dloc = areadu32be(((unsigned char*)data)+9);
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
        printf("BLKNO: %u\n", block_no);
        magic_smoke(FEDRIVE | FEARG | FEBIG);
        return -1;
    }
    fs->rootblock->usedleft -= 1;
    ul --;
    longseek(fs, 0, SEEK_SET);
    write_rootblock(fs, fs->rootblock);
    printf("AFTER ROOT UPDATE\n");
    printf("NULLING BLOCK {%u}\n", block_no);
    getchar();
    dmanip_null(fs, block_no, 1); // destroy invalid data
    getchar();
    if (block_no == ul) { // freed the last used block, nothing else needs to be done
        return 0;
    }
    printf("AFTER EARLY END CHECK\n");
    u64 np = ((u64)(block_no)) * ((u64)BLOCK_SIZE);
    printf("AFTER CALC, NEW POS: %lx, NEW BLK: %u\n", np, tsfs_loc_to_block(np));
    // TSFSStructBlock sblock = {0};
    // TSFSStructNode snode = {0};
    TSFSStructBlock* blockptr = 0;
    TSFSStructNode* nodeptr = 0;
    block_seek(fs, (s32)ul, 0);
    // read_structblock(fs, &sblock);
    blockptr = tsfs_load_block(fs, 0);
    printf("AFTER BLOCK LOAD, PTR = %p\n", (void*)blockptr);
    _DBG_print_block(blockptr);
    u64 comphash = hash_structblock(blockptr);
    printf("AFTER HASH\n");
    printf("COMP HASH = %lx\n", comphash);
    printf("PTRHASH = %lx\n", blockptr->checksum);
    // check if the final block is a struct block by reading it as one and seeing if the hash is correct
    if (blockptr->checksum == comphash) { // struct block
        printf("FREE BLOCK\n");
        _DBG_print_block(blockptr);
        // loc_seek(fs, blockptr->disk_ref);
        // read_structnode(fs, &snode);
        nodeptr = tsfs_load_node(fs, blockptr->disk_ref);
        _DBG_print_node(nodeptr);
        // snode.parent_loc = np;
        nodeptr->parent_loc = block_no;
        // sblock.disk_loc = snode.parent_loc;
        blockptr->disk_loc = block_no;
        dmanip_null(fs, ul, 1); // ensure random invalid data isn't sticking around on the drive
        loc_seek(fs, nodeptr->parent_loc); // move the struct block to its new location
        write_structblock(fs, blockptr);
    } else { // struct node
        printf("FREE NODE\n");
        seek(fs, -14, SEEK_CUR);
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
            blockptr->disk_ref = block_no;
            seek(fs, -14, SEEK_CUR);
            write_structblock(fs, blockptr);
        }
        block_seek(fs, (s32)block_no, 0);
        write_structnode(fs, nodeptr);
        nodeptr->disk_loc = block_no;
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
        awriteu32be(((unsigned char*)buf)+9, block_no);
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
    if (block_no <= ur) { // protect bounds
        return -1;
    }
    return 0;
}

/*
gets the next data block, creating one if it does not exist
[loc] is the location of the current data block, [pos] is the value of the current data block's
next_block field
[count] is a number that will be incremented if a new data block was created
*/
void getcreat_databloc(FileSystem* fs, u32 loc, u32 pos, TSFSDataBlock* db, int* count) {
    // magic_smoke(FEIMPL | FEDRIVE | FEDATA | FEOP);
    if (pos == 0) {
        (*count) ++;
        TSFSDataBlock odb = {0};
        block_seek(fs, loc, 0);
        read_datablock(fs, &odb);
        u32 blk = allocate_blocks(fs, 1, 1);
        db->disk_loc = blk;
        db->data_length = 0;
        db->blocks_to_terminus = 0;
        db->next_block = 0;
        db->prev_block = loc;
        db->storage_flags = TSFS_SF_FINAL_BLOCK|TSFS_SF_LIVE;
        odb.storage_flags &= ~TSFS_SF_FINAL_BLOCK;
        odb.next_block = blk;
        seek(fs, -TSFSDATABLOCK_DSIZE, SEEK_CUR);
        write_datablock(fs, &odb);
        block_seek(fs, blk, 0);
        write_datablock(fs, db);
    } else {
        block_seek(fs, pos, 0);
        read_datablock(fs, db);
    }
}

size_t data_write(FileSystem* fs, TSFSStructNode* sn, u64 position, const void* data, size_t size) {
    size_t osize = size;
    struct PosDat data_loc = tsfs_traverse(fs, sn, position);
    if (data_loc.bloc.storage_flags == 0) {
        return 0;
    }
    u32 realoffset = (u32)(position-data_loc.poff);
    seek(fs, realoffset + TSFSDATABLOCK_DSIZE, SEEK_CUR);
    size_t cpos = 0;
    u32 bsize = (u32)BLOCK_SIZE;
    u32 fsize = bsize - TSFSDATABLOCK_DSIZE;
    u32 left = fsize - realoffset;
    u64 sinc = 0;
    int binc = 0;
    if (left >= size) {
        write_buf(fs, data, size);
        block_seek(fs, data_loc.bloc.disk_loc, BSEEK_SET);
        size_t ns = size + realoffset;
        data_loc.bloc.data_length = ns > data_loc.bloc.data_length ? ns : data_loc.bloc.data_length;
        write_datablock(fs, &data_loc.bloc);
        sinc += size;
        size = 0;
    } else {
        write_buf(fs, data, left);
        block_seek(fs, data_loc.bloc.disk_loc, BSEEK_SET);
        data_loc.bloc.data_length = fsize;
        write_datablock(fs, &data_loc.bloc);
        sinc += left;
        size -= left;
        cpos += left;
    }
    if (size > 0) {
        TSFSDataBlock db = {0};
        // loc_seek(fs, data_loc.bloc.next_block);
        // read_datablock(fs, &db);
        while (1) {
            if (fsize > size) {
                break;
            }
            getcreat_databloc(fs, db.disk_loc, db.next_block, &db, &binc);
            seek(fs, -TSFSDATABLOCK_DSIZE, SEEK_CUR);
            sinc += fsize - db.data_length;
            db.data_length = fsize;
            write_datablock(fs, &db);
            write_buf(fs, (const void*)(((const char*)data)+cpos), fsize);
            cpos += fsize;
            size -= fsize;
        }
        if (size > 0) {
            getcreat_databloc(fs, db.disk_loc, db.next_block, &db, &binc);
            seek(fs, -TSFSDATABLOCK_DSIZE, SEEK_CUR);
            sinc += fsize - db.data_length;
            db.data_length = size > db.data_length ? size : db.data_length;
            write_datablock(fs, &db);
            write_buf(fs, (const void*)(((const char*)data)+cpos), size);
            size = 0;
        }
    }
    TSFSDataHeader dh;
    block_seek(fs, sn->data_loc, BSEEK_SET);
    read_dataheader(fs, &dh);
    dh.size += sinc;
    dh.blocks += binc;
    seek(fs, -TSFSDATAHEADER_DSIZE, SEEK_CUR);
    write_dataheader(fs, &dh);
    return osize - size;
}

// TODO: make work with more than one data block
size_t data_read(FileSystem* fs, TSFSStructNode* sn, u64 position, void* data, size_t size) {
    size_t osize = size;
    struct PosDat data_loc = tsfs_traverse(fs, sn, position);
    if (data_loc.bloc.storage_flags == 0) {
        return 0;
    }
    u32 realoffset = (u32)(position - data_loc.poff);
    if (realoffset >= data_loc.bloc.data_length) {
        return 0;
    }
    seek(fs, realoffset + TSFSDATABLOCK_DSIZE, SEEK_CUR);
    size_t cpos = 0;
    u32 bsize = (u32)BLOCK_SIZE;
    u32 left = data_loc.bloc.data_length - realoffset;
    if (left >= size) {
        read_buf(fs, data, size);
        size = 0;
    } else {
        read_buf(fs, data, left);
        cpos += left;
        size -= left;
    }
    if (size > 0) {
        TSFSDataBlock db = {0};
        block_seek(fs, data_loc.bloc.next_block, BSEEK_SET);
        read_datablock(fs, &db);
        u32 rem = db.data_length;
        while (1) {
            if (size < rem || rem < size) {
                break;
            }
            read_buf(fs, (void*)(((char*)data)+cpos), rem);
            size -= rem;
            cpos += rem;
            block_seek(fs, db.next_block, BSEEK_SET);
            read_datablock(fs, &db);
            rem = db.data_length;
        }
        if (size > 0) {
            if (size < rem) {
                read_buf(fs, (void*)(((char*)data)+cpos), size);
                size = 0;
            } else {
                read_buf(fs, (void*)(((char*)data)+cpos), rem);
                size -= rem;
            }
        }
    }
    return osize - size;
}

#endif
