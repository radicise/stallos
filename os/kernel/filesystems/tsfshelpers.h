#ifndef __TSFS_HELPERS_H__
#define __TSFS_HELPERS_H__ 1
#include "./fsdefs.h"
#include "./perms.h"

static char locked_threadinfo = 0;
void aquire_infolock(void) {
    if (!locked_threadinfo) {
        Mutex_acquire(&PerThread_context->dataLock);
        Mutex_acquire(&PerThread_context->fsinfo->dataLock);
        locked_threadinfo = 1;
    }
}
void release_infolock(void) {
    if (locked_threadinfo) {
        Mutex_release(&PerThread_context->fsinfo->dataLock);
        Mutex_release(&PerThread_context->dataLock);
        locked_threadinfo = 0;
    }
}
char* get_cwd(void) {
    const char* volatile cwd = PerThread_context->fsinfo->cwd;
    char* buffer = allocate(tsfs_strlen(cwd));
    int i = 0;
    while (cwd[i]) {
        buffer[i] = cwd[i++];
    }
    buffer[i] = 0;
    return buffer;
}
char* get_root(void) {
    const char* volatile root = PerThread_context->fsinfo->root;
    char* buffer = allocate(tsfs_strlen(root));
    int i = 0;
    while (root[i]) {
        buffer[i] = root[i++];
    }
    buffer[i] = 0;
    return buffer;
}
kuid_t get_euid(void) {
    return PerThread_context->euid;
}
kuid_t get_egid(void) {
    return 0;
}
u64 get_capa(void) {
    return PerThread_context->cap_effective;
}
char test_dataheader(FileSystem* fs, u32 block_no) {
    block_seek(fs, block_no, BSEEK_SET);
    TSFSDataHeader obj = {0};
    read_dataheader(fs, &obj);
    if (obj.checksum == hash_dataheader(&obj)) {
        return 1;
    }
    return 0;
}
char test_datablock(FileSystem* fs, u32 block_no) {
    block_seek(fs, block_no, BSEEK_SET);
    TSFSDataBlock obj = {0};
    read_datablock(fs, &obj);
    if (obj.checksum == hash_datablock(&obj)) {
        return 1;
    }
    return 0;
}
char test_structnode(FileSystem* fs, u32 block_no) {
    block_seek(fs, block_no, BSEEK_SET);
    TSFSStructNode obj = {0};
    read_structnode(fs, &obj);
    if (obj.checksum == hash_structnode(&obj)) {
        return 1;
    }
    return 0;
}
char test_structblock(FileSystem* fs, u32 block_no) {
    block_seek(fs, block_no, BSEEK_SET);
    TSFSStructBlock obj = {0};
    read_structblock(fs, &obj);
    if (obj.checksum == hash_structblock(&obj)) {
        return 1;
    }
    return 0;
}

// seek to then write struct node
inline void update_structnode(FileSystem* fs, TSFSStructNode* sn) {
    block_seek(fs, sn->disk_loc, BSEEK_SET);
    write_structnode(fs, sn);
}
// update struct node, then return to original location
inline void update_structnode_stable(FileSystem* fs, TSFSStructNode* sn) {
    u64 rest = tsfs_tell(fs);
    update_structnode(fs, sn);
    longseek(fs, rest, SEEK_SET);
}
// seek to then write struct block
inline void update_structblock(FileSystem* fs, TSFSStructBlock* sb) {
    block_seek(fs, sb->disk_loc, BSEEK_SET);
    write_structblock(fs, sb);
}
// update struct block, then return to original location
inline void update_structblock_stable(FileSystem* fs, TSFSStructBlock* sb) {
    u64 rest = tsfs_tell(fs);
    update_structblock(fs, sb);
    longseek(fs, rest, SEEK_SET);
}
// seek to then write data block
inline void update_datablock(FileSystem* fs, TSFSDataBlock* sb) {
    block_seek(fs, sb->disk_loc, BSEEK_SET);
    write_datablock(fs, sb);
}
// update data block, then return to original location
inline void update_datablock_stable(FileSystem* fs, TSFSDataBlock* sb) {
    u64 rest = tsfs_tell(fs);
    update_datablock(fs, sb);
    longseek(fs, rest, SEEK_SET);
}
// seek to then write data header
inline void update_dataheader(FileSystem* fs, TSFSDataHeader* sb) {
    block_seek(fs, sb->disk_loc, BSEEK_SET);
    write_dataheader(fs, sb);
}
// update data header, then return to original location
inline void update_dataheader_stable(FileSystem* fs, TSFSDataHeader* sb) {
    u64 rest = tsfs_tell(fs);
    update_dataheader(fs, sb);
    longseek(fs, rest, SEEK_SET);
}

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
    block_seek(fs, resolve_itable_entry(fs, sn->ikey), BSEEK_SET);
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

char* strjoin(const char*,const char*);

/*
prepends s1 with s2, then deallocates s1, returning the new string
*/
char* strprepend(char* s1, const char* s2) {
    char* b = strjoin(s2, s1);
    deallocate(s1, tsfs_strlen(s1));
    return b;
}
/*
joins s1 and s2, deallocates s1, and returns the new string
*/
char* strappend(char* s1, const char* s2) {
    char* b = strjoin(s1, s2);
    deallocate(s1, tsfs_strlen(s1));
    return b;
}
char* strjoin(const char* s1, const char* s2) {
    char* b = allocate(tsfs_strlen(s1) + tsfs_strlen(s2) - 1);
    size_t i = 0;
    size_t j = 0;
    while (s1[i]) {b[i] = s1[i++];}
    while (s2[j]) {b[i+j] = s2[j++];}
    b[i+j] = 0;
    return b;
}
char* strmove(const char* s) {
    char* d = allocate(tsfs_strlen(s));
    size_t i = 0;
    while (s[i]) {d[i] = s[i++];}
    d[i] = 0;
    return d;
}
char* substrmove(const char* s, size_t l) {
    char* d = allocate(l+1);
    for (size_t i = 0; i < l; i ++) {
        d[i] = s[i];
    }
    d[l] = 0;
    return d;
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
    printf("STRUCT NODE {\nDISK_LOC: 0x%x,\nB_LOCK: 0x%x,\nPNODE: 0x%x,\nIKEY: 0x%x,\nFLAGS: %u,\nNAME:\n\"%s\",\nMAGIC_NO: %lu\n}%s\n", sn->disk_loc, sn->parent_loc, sn->pnode, sn->ikey, sn->storage_flags, sn->name, sn->magicno, TSFS_ANSI_NUN);
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
