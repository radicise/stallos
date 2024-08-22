#ifndef __TSFS_HELPERS_H__
#define __TSFS_HELPERS_H__ 1
#include "./fsdefs.h"
// void __DBG_print_block(TSFSStructBlock*, long, const char*, const char*);
// void __DBG_print_node(TSFSStructNode*, long, const char*, const char*);
// void __DBG_print_cename(char const*, long, const char*, const char*);
// void __DBG_print_child(TSFSSBChildEntry*, long, const char*, const char*);
// void __DBG_print_head(TSFSDataHeader*, long, const char*, const char*);
// void __DBG_print_data(TSFSDataBlock*, long, const char*, const char*);
// void __DBG_here(long, const char*, const char*);
// #define _DBG_print_block(sb) __DBG_print_block(sb, __LINE__, __FILE__, __func__)
// #define _DBG_print_node(sn) __DBG_print_node(sn, __LINE__, __FILE__, __func__)
// #define _DBG_print_cename(name) __DBG_print_cename(name, __LINE__, __FILE__, __func__)
// #define _DBG_print_child(ce) __DBG_print_child(ce, __LINE__, __FILE__, __func__)
// #define _DBG_print_head(dh) __DBG_print_head(dh, __LINE__, __FILE__, __func__)
// #define _DBG_print_data(db) __DBG_print_data(db, __LINE__, __FILE__, __func__)
// #define _DBG_here() __DBG_here(__LINE__, __FILE__, __func__)
struct PosDat {
    TSFSDataBlock bloc;
    u32   poff; // how far the start of the data of the block is into the entity
};

TSFSDataBlock tsfs_traverse_blkno(FileSystem* fs, TSFSStructNode* sn, u32 blockno, char create) {
    printf("blkno: %u\n", blockno);
    // _DBG_print_node(sn);
    TSFSDataBlock databloc = {0};
    block_seek(fs, sn->data_loc, BSEEK_SET);
    TSFSDataHeader dh;
    read_dataheader(fs, &dh);
    // _DBG_print_head(&dh);
    if (dh.blocks <= blockno) {
        if (create == 0) {
            _DBG_here();
            return databloc;
        } else {
            u16 cnt = (u16)(blockno - dh.blocks + 1);
            int r = append_datablocks(fs, sn, cnt);
            printf("APPENDING %zu BLOCKS, RESULT: %d\n", cnt, r);
        }
    }
    block_seek(fs, dh.head, BSEEK_SET);
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
            block_seek(fs, databloc.disk_loc, BSEEK_SET);
            // loc_seek(fs, (databloc.disk_loc + databloc.blocks_to_terminus<<7));
            cblock += databloc.blocks_to_terminus;
        } else {
            block_seek(fs, databloc.disk_loc, BSEEK_SET);
            // loc_seek(fs, databloc.next_block);
            cblock ++;
        }
        read_datablock(fs, &databloc);
    }
    block_seek(fs, databloc.disk_loc, BSEEK_SET);
    // loc_seek(fs, databloc.disk_loc);
    return databloc;
}

// traverses the data blocks until it reaches the specified data position, then returns the location on disk
// if OOB, the TSFSDataBlock will have storage_flags set to zero
// if [create] is set, the traversal will create new data blocks as needed until the specified position is reached
//  these data blocks will be zero initialized
// when this function returns, the current position on disk will be the start of the data block
struct PosDat tsfs_traverse(FileSystem* fs, TSFSStructNode* sn, u64 position, char create) {
    struct PosDat pd;
    u32 dblockdlen = BLOCK_SIZE - TSFSDATABLOCK_DSIZE;
    u32 blockno = (u32)(position / (u64)dblockdlen);
    pd.poff = ((u32)blockno) * dblockdlen;
    pd.bloc = tsfs_traverse_blkno(fs, sn, blockno, create);
    return pd;
}

void write_datablock_at(FileSystem* fs, TSFSDataBlock* db, u32 dlock) {
    block_seek(fs, dlock, BSEEK_SET);
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
    block_seek(fs, sb->disk_loc, BSEEK_SET);
    seek(fs, 14, SEEK_CUR);
    TSFSSBChildEntry ce = {0};
    for (int i = 0; i < sb->entrycount; i ++) {
        read_childentry(fs, &ce);
        // _DBG_print_child(&ce);
        if (ce.flags == TSFS_CF_EXTT) {
            printf("FE EXTT\n");
            TSFSStructBlock ssb = {0};
            u64 opos = tsfs_tell(fs);
            block_seek(fs, ce.dloc, BSEEK_SET);
            read_structblock(fs, &ssb);
            u64 retv = tsfs_sbcs_foreach(fs, &ssb, _do, data);
            if (retv) {
                return retv;
            }
            loc_seek(fs, opos);
        } else if (_do(fs, &ce, data)) return ((u64)(sb->disk_loc))*1024;
    }
    return 0;
}

size_t tsfs_strlen(const char* s) {
    size_t o = 0;
    while (s[o++]);
    return o;
}

void __DBG_print_root(TSFSRootBlock* rb, long l, const char* f, const char* fid) {
    printf("%sSOURCE {%ld} of {%s} (%s)%s\n", TSFS_ANSI_YEL, l, f, fid, TSFS_ANSI_GRN);
    printf("ROOT BLOCK {\nBVN: %hu,\nP_SIZE: %lu,\nCREAT_T: 0x%llx,\nVER: %s,\nTOP: 0x%lx,\nUL: 0x%lx,\nUR: 0x%lx,\nUHL: 0x%lx,\nUHR: 0x%lx,\nCHK: 0x%llx\n}%s\n", rb->breakver, rb->partition_size, rb->creation_time, rb->version, rb->top_dir, rb->usedleft, rb->usedright, rb->usedhalfleft, rb->usedhalfright, rb->checksum, TSFS_ANSI_NUN);
}
void __DBG_print_block(TSFSStructBlock* sb, long l, const char* f, const char* fid) {
    printf("%sSOURCE {%ld} of {%s} (%s)%s\n", TSFS_ANSI_YEL, l, f, fid, TSFS_ANSI_GRN);
    printf("STRUCT BLOCK {\nDISK_LOC: 0x%x,\nDISK_REF: 0x%x,\nENTRYCOUNT: %u,\nFLAGS: %u,\nMAGIC_NO: %lu\n}%s\n", sb->disk_loc, sb->disk_ref, sb->entrycount, sb->flags, sb->magicno, TSFS_ANSI_NUN);
}
void __DBG_print_node(TSFSStructNode* sn, long l, const char* f, const char* fid) {
    printf("%sSOURCE {%ld} of {%s} (%s)%s\n", TSFS_ANSI_YEL, l, f, fid, TSFS_ANSI_GRN);
    printf("STRUCT NODE {\nDISK_LOC: 0x%x,\nB_LOCK: 0x%x,\nPNODE: 0x%x,\nDATA_LOC: 0x%x,\nFLAGS: %u,\nNAME:\n\"%s\",\nMAGIC_NO: %lu\n}%s\n", sn->disk_loc, sn->parent_loc, sn->pnode, sn->data_loc, sn->storage_flags, sn->name, sn->magicno, TSFS_ANSI_NUN);
}
void __DBG_print_head(TSFSDataHeader* dh, long l, const char* f, const char* fid) {
    printf("%sSOURCE {%ld} of {%s} (%s)%s\n", TSFS_ANSI_YEL, l, f, fid, TSFS_ANSI_GRN);
    printf("DATA HEADER {\nDISK_LOC: 0x%x,\nREF_C: %hu,\nHEAD: 0x%x,\nBLOCKS: %u,\nSIZE: %lu,\nMAGIC_NO: %lu\n}%s\n", dh->disk_loc, dh->refcount, dh->head, dh->blocks, dh->size, dh->magicno, TSFS_ANSI_NUN);
}
void __DBG_print_data(TSFSDataBlock* db, long l, const char* f, const char* fid) {
    printf("%sSOURCE {%ld} of {%s} (%s)%s\n", TSFS_ANSI_YEL, l, f, fid, TSFS_ANSI_GRN);
    printf("DATA BLOCK {\nDISK_LOC: 0x%x,\nTERM_BLKS: %u,\nNEXT: %x,\nPREV: %x,\nLENGTH: %hu,\nFLAGS: %x\n}%s\n", db->disk_loc, db->blocks_to_terminus, db->next_block, db->prev_block, db->data_length, db->storage_flags, TSFS_ANSI_NUN);
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
    printf("CHILD ENTRY {\nDLOC: 0x%x,\nFLAGS: %u,\nNAME:", ce->dloc, ce->flags);
    __DBG_print_cename(ce->name, 0, 0, 0);
    printf("\n}%s\n", TSFS_ANSI_NUN);
}
void __DBG_here(long l, const char* f, const char* fid) {
    printf("%sHERE {%ld} of {%s} (%s)%s\n", TSFS_ANSI_YEL, l, f, fid, TSFS_ANSI_NUN);
}
#endif
