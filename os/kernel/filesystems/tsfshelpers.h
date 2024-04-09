#ifndef __TSFS_HELPERS_H__
#define __TSFS_HELPERS_H__ 1
#include "./fsdefs.h"
struct PosDat {
    TSFSDataBlock bloc;
    u32   poff; // how far the start of the data of the block is into the entity
};

TSFSDataBlock tsfs_traverse_blkno(FileSystem* fs, TSFSStructNode* sn, u32 blockno) {
    TSFSDataBlock databloc = {0};
    longseek(fs, sn->data_loc, SEEK_SET);
    TSFSDataHeader dh;
    read_dataheader(fs, &dh);
    if (dh.blocks < blockno) {
        return databloc;
    }
    longseek(fs, dh.head, SEEK_SET);
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

void write_datablock_at(FileSystem* fs, TSFSDataBlock* db, u64 dlock) {
    longseek(fs, dlock, SEEK_SET);
    write_datablock(fs, db);
}

/*
struct - node to block
THE RETURNED RESOURCE MUST BE RELEASED WITH EITHER [tsfs_unload] OR [_tsmagic_force_release]
*/
TSFSStructBlock* tsfs_get_ntob(FileSystem* fs, TSFSStructNode* sn) {
    return tsfs_load_block(fs, sn->parent_loc);
}

/*
[_do] must not result in a changed disk location before returning control flow
if [_do] returns a non-zero value, then the foreach will end early

returns the address of the block that caused an early return, or zero otherwise
*/
u64 tsfs_sbcs_foreach(FileSystem* fs, TSFSStructBlock* sb, int(_do)(FileSystem*, TSFSSBChildEntry*, void*), void* data) {
    loc_seek(fs, sb->disk_loc);
    seek(fs, 16, SEEK_CUR);
    TSFSSBChildEntry ce = {0};
    for (int i = 0; i < sb->entrycount; i ++) {
        read_childentry(fs, &ce);
        if (ce.flags == TSFS_CF_EXTT) {
            printf("FE EXTT\n");
            TSFSStructBlock ssb = {0};
            loc_seek(fs, ce.dloc);
            read_structblock(fs, &ssb);
            u64 retv = tsfs_sbcs_foreach(fs, &ssb, _do, data);
            if (retv) {
                return retv;
            }
        } else if (_do(fs, &ce, data)) return sb->disk_loc;
    }
    return 0;
}

#define TSFS_ANSI_NUN "\x1b[0m"
#define TSFS_ANSI_RED "\x1b[38;2;235;0;0m"
#define TSFS_ANSI_GRN "\x1b[38;2;0;200;0m"
#define TSFS_ANSI_YEL "\x1b[38;2;200;175;0m"

void __DBG_print_block(TSFSStructBlock* sb, long l, const char* f, const char* fid) {
    printf("%sSOURCE {%ld} of {%s} (%s)%s\n", TSFS_ANSI_YEL, l, f, fid, TSFS_ANSI_GRN);
    printf("STRUCT BLOCK {\nDISK_LOC: 0x%llx,\nDISK_REF: 0x%llx,\nENTRYCOUNT: %u,\nFLAGS: %u,\nMAGIC_NO: %lu\n}%s\n", sb->disk_loc, sb->disk_ref, sb->entrycount, sb->flags, sb->magicno, TSFS_ANSI_NUN);
}
void __DBG_print_node(TSFSStructNode* sn, long l, const char* f, const char* fid) {
    printf("%sSOURCE {%ld} of {%s} (%s)%s\n", TSFS_ANSI_YEL, l, f, fid, TSFS_ANSI_GRN);
    printf("STRUCT NODE {\nDISK_LOC: 0x%llx,\nB_LOCK: 0x%llx,\nPNODE: 0x%llx,\nDATA_LOC: 0x%llx,\nFLAGS: %u,\nNAME:\n\"%s\",\nMAGIC_NO: %lu\n}%s\n", sn->disk_loc, sn->parent_loc, sn->pnode, sn->data_loc, sn->storage_flags, sn->name, sn->magicno, TSFS_ANSI_NUN);
}
void __DBG_print_cename(char const* name, long l, const char* f, const char* fid) {
    if (l) {
        printf("%sSOURCE {%ld} of {%s} (%s)%s\n", TSFS_ANSI_YEL, l, f, fid, TSFS_ANSI_GRN);
    }
    printf("0x%x%x%x%x%x%x%x%x%x", name[0], name[1], name[2], name[3], name[4], name[5], name[6], name[7], name[8]);
    if (l) {
        printf("%s\n", TSFS_ANSI_NUN);
    }
}
void __DBG_print_child(TSFSSBChildEntry* ce, long l, const char* f, const char* fid) {
    printf("%sSOURCE {%ld} of {%s} (%s)%s\n", TSFS_ANSI_YEL, l, f, fid, TSFS_ANSI_GRN);
    printf("CHILD ENTRY {\nDLOC: 0x%llx,\nFLAGS: %u,\nNAME:", ce->dloc, ce->flags);
    __DBG_print_cename(ce->name, 0, 0, 0);
    printf("\n}%s\n", TSFS_ANSI_NUN);
}
void __DBG_here(long l, const char* f, const char* fid) {
    printf("%sHERE {%ld} of {%s} (%s)%s\n", TSFS_ANSI_YEL, l, f, fid, TSFS_ANSI_NUN);
}
#define _DBG_print_block(sb) __DBG_print_block(sb, __LINE__, __FILE__, __func__)
#define _DBG_print_node(sn) __DBG_print_node(sn, __LINE__, __FILE__, __func__)
#define _DBG_print_cename(name) __DBG_print_cename(name, __LINE__, __FILE__, __func__)
#define _DBG_print_child(ce) __DBG_print_child(ce, __LINE__, __FILE__, __func__)
#define _DBG_here() __DBG_here(__LINE__, __FILE__, __func__)
#endif