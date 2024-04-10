#ifndef __TSFSMANAGE_H__
#define __TSFSMAGAGE_H__ 1
/*
everything to do with managing the file system
*/

#include "./fsdefs.h"
#include "./tsfscore.h"

/*
writes a valid dataheader to disk for the given structnode
fails if the structnode already has data
*/
int _tsfs_make_data(FileSystem* fs, TSFSStructNode* sn) {
    if (sn->data_loc != 0) {
        return -1;
    }
    // magic_smoke(FEIMPL | FEDRIVE | FEDATA | FEOP);
    u32 blkno = allocate_blocks(fs, 1, 2);
    u64 absloc = ((u64)blkno) * ((u64)(fs->rootblock->block_size));
    TSFSDataHeader dh = {.blocks=0,.head=absloc+((u64)(fs->rootblock->block_size)),.refcount=1,.size=0};
    loc_seek(fs, absloc);
    write_dataheader(fs, &dh);
    seek(fs, -TSFSDATAHEADER_DSIZE, SEEK_CUR);
    block_seek(fs, 1, 1);
    TSFSDataBlock db = {.data_length=0,.next_block=0,.prev_block=absloc,.blocks_to_terminus=0,.storage_flags=TSFS_SF_FINAL_BLOCK};
    write_datablock(fs, &db);
    loc_seek(fs, sn->disk_loc);
    sn->data_loc = absloc;
    write_structnode(fs, sn);
    return 0;
}

/*
adds count new data blocks to the end of a file structure, returns zero on success
*/
int append_datablocks(FileSystem* fs, TSFSStructNode* sn, u16 count) {
    // u64 prev = sn->data_loc ? tsfs_traverse_blkno(fs, sn, sn->blocks).disk_loc : 0;
    // sn->blocks += count;
    u64 prev = sn->data_loc ? tsfs_traverse_blkno(fs, sn, 0).disk_loc : 0;
    // sn->blocks += count;
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

int _tsfs_exists_fe_internaldo(FileSystem* fs, TSFSSBChildEntry* ce, void* data) {
    if (tsfs_cmp_cename(data, ce->name)) {
        *(((char*)data)-1) = 1;
        return 1;
    }
    return 0;
}

int tsfs_exists(FileSystem* fs, TSFSStructBlock* sb, char const* frag) {
    char f[10];
    awrite_buf(f+1, frag, 9);
    tsfs_sbcs_foreach(fs, sb, _tsfs_exists_fe_internaldo, f+1);
    return f[0];
}

int _tsfs_sbcs_fe_delnode_do(FileSystem* fs, TSFSSBChildEntry* ce, void* data) {
    // _DBG_print_child(ce);
    if (tsfs_cmp_cename(ce->name, data)) {
        u64 p = tsfs_tell(fs);
        *((u64*)(((char*)data)+9)) = p-16;
        // printf("\nLOC FOUND: 0x%llx\n\n", p);
        return 1;
    }
    return 0;
}

/*
WARNING: THIS FUNCTION CAN RELEASE THE [opb] RESOURCE
RETURNS ZERO IF [opb] WAS RELEASED
*/
int _tsfs_delce(FileSystem* fs, TSFSStructBlock* opb, u8 no) {
    if (opb->entrycount > 1) {
        _DBG_print_block(opb);
        opb->entrycount --;
        printf("DELETING ENTRY {%u} FROM BLOCK\n", no);
        loc_seek(fs, opb->disk_loc);
        seek(fs, 16 + 16 * no, SEEK_CUR);
        u8 b[16] = {0};
        write_buf(fs, b, 16);
        if (no < opb->entrycount) {
            unsigned char buffer[1024] = {0};
            size_t amt = (size_t)(opb->entrycount - no);
            // seek(fs, ((off_t)no)*16+16, SEEK_CUR);
            read_buf(fs, buffer, amt);
            seek(fs, -((off_t)amt)-16, SEEK_CUR);
            write_buf(fs, buffer, amt);
        }
        _DBG_print_block(opb);
        loc_seek(fs, opb->disk_loc);
        write_structblock(fs, opb);
        return 1;
    } else {
        TSFSStructBlock* par = tsfs_load_block(fs, opb->disk_ref);
        TSFSSBChildEntry opc = {0};
        u8 cno = 0;
        for (int i = 0; i < par->entrycount; i ++) {
            read_childentry(fs, &opc);
            if (opc.flags & 1) {
                if (opc.dloc == (opb->disk_loc)) {
                    cno = i;
                    break;
                }
            }
        }
        _tsfs_delce(fs, par, cno);
        tsfs_unload(fs, par);
        tsfs_free_structure(fs, tsfs_loc_to_block(fs, opb->disk_loc));
        _tsmagic_force_release(fs, opb);
    }
    return 0;
}

/*
WARNING: THIS FUNCTION RELEASES THE [sn] RESOURCE
*/
int _tsfs_delnode(FileSystem* fs, TSFSStructNode* sn) {
    char buf[17];
    tsfs_mk_ce_name(buf, sn->name, strlen(sn->name)+1);
    TSFSStructNode* pn = tsfs_load_node(fs, sn->pnode);
    TSFSStructBlock* pb = tsfs_load_block(fs, pn->parent_loc);
    // _DBG_print_node(sn);
    // _DBG_print_node(pn);
    // _DBG_print_block(pb);
    // printf("TARGET NAME:\n");
    // _DBG_print_cename(buf);
    printf("\n");
    // magic_smoke(FETEST);
    u64 b_loc = tsfs_sbcs_foreach(fs, pb, _tsfs_sbcs_fe_delnode_do, buf);
    tsfs_unload(fs, pn);
    tsfs_unload(fs, pb);
    if (b_loc == 0) {
        magic_smoke(FEDRIVE | FEINVL);
        return -1;
    }
    u64 c_loc = *((u64*)(buf+9));
    u8 no = (u8)((c_loc - b_loc) / 16);
    // printf("DELNODE\n");
    // _DBG_print_node(sn);
    printf("B_LOCK = %llx\nC_LOC = %llx\nNO = %u\n", b_loc, c_loc, no);
    TSFSStructBlock* opb = tsfs_load_block(fs, b_loc);
    if (_tsfs_delce(fs, opb, no)) {tsfs_unload(fs, opb);}
    sn->pnode = 0;
    tsfs_free_structure(fs, tsfs_loc_to_block(fs, sn->disk_loc));
    _tsmagic_force_release(fs, sn);
    return 0;
}

/*
deletes a "name" from the file system
*/
int tsfs_unlink(FileSystem* fs, TSFSStructNode* sn) {
    return 0;
}

/*
deletes a directory, which must be empty
*/
int tsfs_rmdir(FileSystem* fs, TSFSStructNode* sn) {
    if (sn->storage_flags != TSFS_KIND_DIR) {
        return ENOTDIR;
    }
    TSFSStructBlock* sb = tsfs_get_ntob(fs, sn); // get the struct block
    if (sb->entrycount) { // check that the count is empty
        tsfs_unload(fs, sb);
        return ENOTEMPTY;
    }
    _tsmagic_force_release(fs, sb);
    u32 blk = tsfs_loc_to_block(fs, sn->parent_loc);
    sn->parent_loc = 0; // ensure the free function does not try to update the block's reference to its parent
    tsfs_free_structure(fs, blk); // free the struct block
    _tsfs_delnode(fs, sn); // get rid of the node
    return 0;
}

/*
wrapper around [tsfs_exists] that accepts a name
*/
int tsfs_name_exists(FileSystem* fs, TSFSStructBlock* sb, char const* name) {
    char tn[9];
    tsfs_mk_ce_name(tn, name, strlen(name)+1);
    return tsfs_exists(fs, sb, tn);
}

int tsfs_add_sbce(FileSystem* fs, TSFSStructBlock* sb, TSFSSBChildEntry* ce) {
    u8 ec = sb->entrycount;
    if (ec < 62) { // block isn't full, just add at the end
        longseek(fs, sb->disk_loc + 16llu + (16llu*(u64)sb->entrycount), SEEK_SET);
        write_childentry(fs, ce);
    } else if (ec == 62) { // block needs and extended table, create it, then add the entry
        longseek(fs, sb->disk_loc + 16llu + (16llu*(u64)sb->entrycount), SEEK_SET);
        u64 achnk = ((u64)allocate_blocks(fs, 0, 1)) * ((u64)fs->rootblock->block_size);
        TSFSSBChildEntry nce = {.flags=TSFS_CF_EXTT,.dloc=achnk};
        write_childentry(fs, &nce);
        TSFSStructBlock nsb = {.flags=TSFS_CF_EXTT,.disk_ref=sb->disk_loc,.disk_loc=achnk,.entrycount=0};
        longseek(fs, achnk, SEEK_SET);
        write_structblock(fs, &nsb);
        tsfs_add_sbce(fs, &nsb, ce);
    } else { // block already has an extended table, go to it, then add the entry there
        longseek(fs, sb->disk_loc + 16*62 + 1, SEEK_SET);
        loc_seek(fs, read_u48be(fs));
        TSFSStructBlock csb = {0};
        read_structblock(fs, &csb);
        tsfs_add_sbce(fs, &csb, ce);
        return 0;
    }
    sb->entrycount ++;
    longseek(fs, sb->disk_loc, SEEK_SET); // update entry count
    write_structblock(fs, sb);
    return 0;
}

/*
converts a storage flag to equivalent child-entry flag
*/
unsigned char tsfs_sf_to_cf(unsigned char sf) {
    unsigned char k = (sf & TSFS_SF_KIND) >> 1;
    switch (k) {
        case (TSFS_KIND_DIR):
            return TSFS_CF_DIRE;
        case (TSFS_KIND_HARD):
            return TSFS_CF_HARD;
        case (TSFS_KIND_LINK):
            return TSFS_CF_LINK;
        default:
            return 0;
    }
}

int tsfs_add_sn(FileSystem* fs, TSFSStructBlock* parent, TSFSStructNode* sn) {
    TSFSSBChildEntry ce = {.flags=tsfs_sf_to_cf(sn->storage_flags),.dloc=sn->disk_loc};
    tsfs_mk_ce_name(ce.name, sn->name, ((sn->name)[254]) ? 256 : strlen(sn->name)+1);
    return tsfs_add_sbce(fs, parent, &ce);
}

int tsfs_mk_dir(FileSystem* fs, TSFSStructNode* parent, char const* name, TSFSStructBlock* nsb) {
    TSFSStructBlock* sb = tsfs_load_block(fs, parent->parent_loc); // parent block
    char tname[9];
    tsfs_mk_ce_name(tname, name, strlen(name)+1);
    if (tsfs_exists(fs, sb, tname)) {
        tsfs_unload(fs, sb);
        return EEXIST;
    }
    // addr of first allocated block
    u64 ac1 = ((u64)allocate_blocks(fs, 0, 2)) * ((u64)fs->rootblock->block_size);
    TSFSStructNode sn = {.storage_flags=TSFS_KIND_DIR,.parent_loc=ac1,.disk_loc=ac1+fs->rootblock->block_size,.pnode=parent->disk_loc};
    awrite_buf(sn.name, name, strlen(name));
    nsb->flags = TSFS_CF_DIRE;
    nsb->disk_ref = sn.disk_loc;
    nsb->disk_loc = ac1;
    loc_seek(fs, ac1);
    write_structblock(fs, nsb);
    loc_seek(fs, nsb->disk_ref);
    write_structnode(fs, &sn);
    tsfs_add_sn(fs, sb, &sn);
    tsfs_unload(fs, sb);
    return 0;
}

int tsfs_mk_file(FileSystem* fs, TSFSStructNode* parent, const char* name) {
    TSFSStructBlock* sb = tsfs_load_block(fs, parent->parent_loc); // parent block
    char tname[9];
    tsfs_mk_ce_name(tname, name, strlen(name)+1);
    if (tsfs_exists(fs, sb, tname)) {
        tsfs_unload(fs, sb);
        return EEXIST;
    }
    // addr of first allocated block
    u64 ac1 = ((u64)allocate_blocks(fs, 0, 1)) * ((u64)fs->rootblock->block_size);
    TSFSStructNode sn = {.storage_flags=TSFS_KIND_FILE,.disk_loc=ac1,.pnode=parent->disk_loc,.data_loc=0};
    awrite_buf(sn.name, name, strlen(name));
    loc_seek(fs, ac1);
    write_structnode(fs, &sn);
    tsfs_add_sn(fs, sb, &sn);
    tsfs_unload(fs, sb);
    _tsfs_make_data(fs, &sn);
    return 0;
}

int _tsfs_find_sbcsfe_do(FileSystem* fs, TSFSSBChildEntry* ce, void* data) {
    if (tsfs_cmp_cename(ce->name, data)) {
        u64 cl = tsfs_tell(fs);
        loc_seek(fs, ce->dloc);
        TSFSStructNode sn;
        read_structnode(fs, &sn);
        loc_seek(fs, cl);
        if (!tsfs_cmp_name(*((void**)(data+17)), sn.name)) {
            return 0;
        }
        *((u64*)(data+9)) = ce->dloc;
        return 1;
    }
    return 0;
}

/*
resolves the location of [par]'s child with [name] on disk
returns zero on failure
*/
u64 tsfs_find(FileSystem* fs, TSFSStructNode* par, const char* name) {
    if (par->storage_flags & ~TSFS_KIND_DIR) {
        return 0;
    }
    void* p = allocate(9+sizeof(u64)+sizeof(void*));
    *((u64*)(p+9)) = 0;
    tsfs_mk_ce_name(p, name, strlen(name)+1);
    *((const void**)(p+17)) = name;
    TSFSStructBlock sb;
    loc_seek(fs, par->parent_loc);
    read_structblock(fs, &sb);
    tsfs_sbcs_foreach(fs, &sb, _tsfs_find_sbcsfe_do, p);
    u64 r = *((u64*)(p+9));
    deallocate(p, 9+sizeof(u64)+sizeof(void*));
    return r;
}

/*
sets [curr->disk_loc] to zero on failure
*/
void _tsfs_respath_step(FileSystem* fs, TSFSStructNode* curr, const char* path, int cfs, int cfe) {
    int cfl = cfe - cfs;
    if (cfl < 3) { // handle special cases of: empty, '.', and '..'
        if (cfl == 0) {
            return;
        }
        if (path[cfs] == '.') {
            if (cfl == 1) {
                return;
            }
            if (path[cfs+1] == '.') {
                if (curr->pnode == 0) {
                    curr->disk_loc = 0;
                    return;
                }
                loc_seek(fs, curr->pnode);
                read_structnode(fs, curr);
                return;
            }
        }
    }
    char* frag = allocate(cfl);
    awrite_buf(frag, path+cfs, cfl);
    u64 nl = tsfs_find(fs, curr, frag);
    deallocate(frag, cfl);
    if (nl == 0) {
        curr->disk_loc = 0;
        return;
    }
    loc_seek(fs, nl);
    read_structnode(fs, curr);
}

/*
resolves [path] to a disk location, returns zero on failure
accepts only absolute paths
returns zero on failure
*/
u64 tsfs_resolve_path(FileSystem* fs, const char* path) {
    if (path[0] != '/') {
        return 0;
    }
    TSFSStructNode curr;
    loc_seek(fs, fs->rootblock->top_dir);
    read_structnode(fs, &curr);
    int i = 1;
    int cfs = 1;
    while (1) {
        if (path[i] == '/' || path[i] == 0) {
            _tsfs_respath_step(fs, &curr, path, cfs, i);
            if (curr.disk_loc == 0) {
                return 0;
            }
            cfs = i + 1;
        }
        if (path[i] == 0) break;
        i ++;
    }
    return curr.disk_loc;
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