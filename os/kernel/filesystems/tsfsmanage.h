#ifndef __TSFSMANAGE_H__
#define __TSFSMAGAGE_H__ 1
/*
everything to do with managing the file system
*/

#include "./fsdefs.h"

struct PosDat {
    TSFSDataBlock bloc;
    u32   poff; // how far the start of the data of the block is into the entity
};

TSFSDataBlock tsfs_traverse_blkno(FileSystem* fs, TSFSStructNode* sn, u32 blockno) {
    TSFSDataBlock databloc;
    longseek(fs, sn->data_loc, SEEK_SET);
    // loc_seek(fs, sn->data_loc);
    read_datablock(fs, &databloc);
    u16 cblock = 0;
    while (1) {
        if (cblock == blockno) {
            break;
        }
        if (databloc.storage_flags & TSFS_SF_FINAL_BLOCK) {
            databloc.storage_flags = 0;
            break;
        }
        if (databloc.storage_flags & TSFS_SF_HEAD_BLOCK && databloc.blocks_to_terminus > 0) {
            longseek(fs, databloc.disk_loc, SEEK_SET);
            // loc_seek(fs, (databloc.disk_loc + databloc.blocks_to_terminus<<7));
            cblock += databloc.blocks_to_terminus;
        } else {
            longseek(fs, databloc.disk_loc, SEEK_SET);
            // loc_seek(fs, databloc.next_block);
            cblock ++;
        }
        read_datablock(fs, &databloc);
    }
    longseek(fs, databloc.disk_loc, SEEK_SET);
    // loc_seek(fs, databloc.disk_loc);
    return databloc;
}

// traverses the data blocks until it reaches the specified data position, then returns the location on disk
// if OOB, the TSFSDataBlock will have storage_flags set to zero
// when this function returns, the current position on disk will be the start of the data block
struct PosDat tsfs_traverse(FileSystem* fs, TSFSStructNode* sn, u32 position) {
    struct PosDat pd;
    u16 bsize = fs->rootblock->block_size;
    u16 dblockdlen = bsize - TSFSDATABLOCK_DSIZE;
    u16 blockno = (position / (u32)dblockdlen);
    pd.poff = ((u16)blockno) * dblockdlen;
    pd.bloc = tsfs_traverse_blkno(fs, sn, blockno);
    return pd;
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

void write_datablock_at(FileSystem* fs, TSFSDataBlock* db, u64 dlock) {
    longseek(fs, dlock, SEEK_SET);
    write_datablock(fs, db);
}

/*
adds count new data blocks to the end of a file structure, returns zero on success
*/
int append_datablocks(FileSystem* fs, TSFSStructNode* sn, u16 count) {
    u64 prev = sn->data_loc ? tsfs_traverse_blkno(fs, sn, sn->blocks).disk_loc : 0;
    sn->blocks += count;
    longseek(fs, sn->data_loc, SEEK_SET);
    write_structnode(fs, sn);
    u32 fblock = allocate_blocks(fs, 1, count);
    block_seek(fs, (s32)fblock, BSEEK_SET);
    u64 bsize = (u64)(fs->rootblock->block_size);
    TSFSDataBlock hdb = {.blocks_to_terminus=255, .storage_flags=TSFS_SF_HEAD_BLOCK};
    TSFSDataBlock tdb = {.storage_flags=TSFS_SF_TAIL_BLOCK};
    TSFSDataBlock mdb = {0};
    u64 curr = ((u64)fblock) * bsize;
    u64 next = curr + bsize;
    while (count > 256) {
        hdb.prev_block = prev;
        hdb.next_block = next;
        hdb.disk_loc = curr;
        prev = curr;
        curr = next;
        next += bsize;
        write_datablock_at(fs, &hdb, curr);
        for (int i = 254; i > 0; i --) {
            mdb.prev_block = prev;
            mdb.next_block = next;
            mdb.disk_loc = curr;
            prev = curr;
            curr = next;
            next += bsize;
            write_datablock_at(fs, &mdb, curr);
        }
        tdb.prev_block = prev;
        tdb.next_block = next;
        tdb.disk_loc = curr;
        prev = curr;
        curr = next;
        next += bsize;
        write_datablock_at(fs, &tdb, curr);
    }
    if (count) {
        hdb.prev_block = prev;
        hdb.next_block = next;
        hdb.disk_loc = curr;
        if (count == 1) {
            hdb.storage_flags = TSFS_SF_HEAD_BLOCK | TSFS_SF_TAIL_BLOCK | TSFS_SF_FINAL_BLOCK;
            hdb.next_block = 0;
            hdb.blocks_to_terminus = 0;
            write_datablock_at(fs, &hdb, curr);
        } else {
            count --;
            hdb.blocks_to_terminus = (u8)count;
            write_datablock_at(fs, &hdb, curr);
            prev = curr;
            curr = next;
            next = bsize;
            count --;
            for (int i = 0; i < count; i ++) {
                mdb.prev_block = prev;
                hdb.next_block = next;
                hdb.disk_loc = curr;
                prev = curr;
                curr = next;
                next = bsize;
                write_datablock_at(fs, &mdb, curr);
            }
            tdb.prev_block = prev;
            tdb.next_block = 0;
            tdb.disk_loc = curr;
            tdb.storage_flags = TSFS_SF_TAIL_BLOCK | TSFS_SF_FINAL_BLOCK;
            write_datablock_at(fs, &mdb, curr);
        }
    }
    fsflush(fs);
    return 0;
}

char tsfs_truncate_pfrag(char const* iname, char* oname) {
    int i = 0;
    int x = NAME_LENGTH - 1;
    loop_head:
        if (i >= x) {
            awriteu64be((unsigned char*)(oname+NAME_LENGTH-8), hashstr((unsigned char*)(iname+NAME_LENGTH-8)));
            return 1;
        }
        oname[i] = iname[i];
        if (iname[i] == 0) {
            return 0;
        }
        i ++;
        goto loop_head;
    return 0;
}

#endif