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
    if (sn->ikey != 0) {
        return -1;
    }
    // magic_smoke(FEIMPL | FEDRIVE | FEDATA | FEOP);
    u32 blkno = allocate_blocks(fs, 1, 2);
    // u64 absloc = ((u64)blkno) * ((u64)(fs->rootblock->block_size));
    TSFSDataHeader dh = {.blocks=1,.head=blkno,.refcount=1,.size=0,.ikey=aquire_itable_slot(fs, blkno+1)};
    block_seek(fs, blkno, BSEEK_SET);
    TSFSDataBlock db = {.data_length=0,.next_block=0,.prev_block=blkno+1,.blocks_to_terminus=0,.storage_flags=TSFS_SF_FINAL_BLOCK};
    write_datablock(fs, &db);
    seek(fs, -TSFSDATABLOCK_DSIZE, SEEK_CUR);
    block_seek(fs, 1, 1);
    write_dataheader(fs, &dh);
    block_seek(fs, sn->disk_loc, BSEEK_SET);
    sn->ikey = dh.ikey;
    write_structnode(fs, sn);
    return 0;
}

/*
adds count new data blocks to the end of a file structure, returns zero on success
*/
int append_datablocks(FileSystem* fs, TSFSStructNode* sn, u16 count) {
    // u64 prev = sn->data_loc ? tsfs_traverse_blkno(fs, sn, sn->blocks).disk_loc : 0;
    // sn->blocks += count;
    TSFSDataHeader dh = {0};
    block_seek(fs, resolve_itable_entry(fs, sn->ikey), BSEEK_SET);
    read_dataheader(fs, &dh);
    u32 prev = tsfs_traverse_blkno(fs, sn, dh.blocks-1, 0).disk_loc;
    // sn->blocks += count;
    // write_structnode(fs, sn);
    u32 fblock = allocate_blocks(fs, 1, count);
    dmanip_null(fs, fblock, (u32)count);
    block_seek(fs, fblock, BSEEK_SET);
    // u64 bsize = (u64)(fs->rootblock->block_size);
    TSFSDataBlock hdb = {.blocks_to_terminus=255, .storage_flags=TSFS_SF_HEAD_BLOCK};
    TSFSDataBlock tdb = {.storage_flags=TSFS_SF_TAIL_BLOCK};
    TSFSDataBlock mdb = {0};
    u32 curr = fblock;
    u32 next = curr + 1;
    // u64 curr = ((u64)fblock) * bsize;
    // u64 next = curr + bsize;
    while (count > 256) {
        hdb.prev_block = prev;
        hdb.next_block = next;
        hdb.disk_loc = curr;
        prev = curr;
        curr = next;
        next += 1;
        write_datablock_at(fs, &hdb, curr);
        for (int i = 254; i > 0; i --) {
            mdb.prev_block = prev;
            mdb.next_block = next;
            mdb.disk_loc = curr;
            prev = curr;
            curr = next;
            next += 1;
            write_datablock_at(fs, &mdb, curr);
        }
        tdb.prev_block = prev;
        tdb.next_block = next;
        tdb.disk_loc = curr;
        prev = curr;
        curr = next;
        next += 1;
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
            next += 1;
            count --;
            for (int i = 0; i < count; i ++) {
                mdb.prev_block = prev;
                hdb.next_block = next;
                hdb.disk_loc = curr;
                prev = curr;
                curr = next;
                next += 1;
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
        *((u64*)(((char*)data)+9)) = p-14;
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
        block_seek(fs, opb->disk_loc, BSEEK_SET);
        seek(fs, 14 + 14 * no, SEEK_CUR);
        u8 b[16] = {0};
        write_buf(fs, b, 14);
        if (no < opb->entrycount) {
            unsigned char buffer[1024] = {0};
            size_t amt = (size_t)(opb->entrycount - no)*14;
            // seek(fs, ((off_t)no)*16+16, SEEK_CUR);
            read_buf(fs, buffer, amt);
            seek(fs, -((off_t)amt)-14, SEEK_CUR);
            write_buf(fs, buffer, amt);
            write_buf(fs, b, 14);
        }
        _DBG_print_block(opb);
        block_seek(fs, opb->disk_loc, BSEEK_SET);
        write_structblock(fs, opb);
        return 1;
    } else if (opb->flags == TSFS_CF_DIRE) {
        opb->entrycount = 0;
        block_seek(fs, opb->disk_loc, BSEEK_SET);
        write_structblock(fs, opb);
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
        tsfs_free_structure(fs, opb->disk_loc);
        _tsmagic_force_release(fs, opb);
    }
    return 0;
}

/*
WARNING: THIS FUNCTION RELEASES THE [sn] RESOURCE
*/
int _tsfs_delnode(FileSystem* fs, TSFSStructNode* sn) {
    char buf[17] = {0};
    tsfs_mk_ce_name(buf, sn->name, tsfs_strlen(sn->name)+1);
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
    u8 no = (u8)((c_loc - b_loc) / 14)-1;
    // printf("DELNODE\n");
    // _DBG_print_node(sn);
    printf("B_LOCK = %lx\nC_LOC = %lx\nNO = %u\n", b_loc, c_loc, no);
    TSFSStructBlock* opb = tsfs_load_block(fs, b_loc/BLOCK_SIZE);
    if (_tsfs_delce(fs, opb, no)) {tsfs_unload(fs, opb);}
    sn->pnode = 0;
    tsfs_free_structure(fs, sn->disk_loc);
    _tsmagic_force_release(fs, sn);
    return 0;
}

/*
deletes a "name" from the file system, the object MUST be a hard link
*/
int tsfs_unlink(FileSystem* fs, TSFSStructNode* sn) {
    if (sn->storage_flags&TSFS_KIND_FILE) {
        TSFSDataHeader* dh = tsfs_load_head(fs, resolve_itable_entry(fs, sn->ikey));
        dh->refcount --;
        block_seek(fs, dh->disk_loc, BSEEK_SET);
        write_dataheader(fs, dh);
        if (dh->refcount == 0) {
            u32 pos = dh->disk_loc;
            release_itable_slot(fs, dh->ikey);
            _tsmagic_force_release(fs, dh);
            // DELETE THE DATA
            tsfs_free_data(fs, pos);
            goto done;
        } else {
            tsfs_unload(fs, dh);
        }
        goto done;
    } else {
        goto err;
    }
    done:
    printf("DELNODE\n");
    _tsfs_delnode(fs, sn);
    printf("FULL DONE\n");
    return 0;
    err:
    return -1;
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
    u32 blk = sn->parent_loc;
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
    tsfs_mk_ce_name(tn, name, tsfs_strlen(name)+1);
    return tsfs_exists(fs, sb, tn);
}

int tsfs_add_sbce(FileSystem* fs, TSFSStructBlock* sb, TSFSSBChildEntry* ce) {
    u8 ec = sb->entrycount;
    if (ec < 71) { // block isn't full, just add at the end
        longseek(fs, (((u64)sb->disk_loc)*BLOCK_SIZE) + 14llu + (14llu*(u64)sb->entrycount), SEEK_SET);
        write_childentry(fs, ce);
        // sb->entrycount ++;
        // block_seek(fs, sb->disk_loc, BSEEK_SET);
        // write_structblock(fs, sb);
    } else if (ec == 71) { // block needs and extended table, create it, then add the entry
        longseek(fs, (((u64)sb->disk_loc)*BLOCK_SIZE) + 14llu + (14llu*(u64)sb->entrycount), SEEK_SET);
        u32 achnk = allocate_blocks(fs, 0, 1);
        TSFSSBChildEntry nce = {.flags=TSFS_CF_EXTT,.dloc=achnk};
        write_childentry(fs, &nce);
        // sb->entrycount ++;
        // block_seek(fs, sb->disk_loc, BSEEK_SET);
        // write_structblock(fs, sb);
        TSFSStructBlock nsb = {.flags=TSFS_CF_EXTT,.disk_ref=sb->disk_loc,.disk_loc=achnk,.entrycount=0};
        block_seek(fs, achnk, BSEEK_SET);
        write_structblock(fs, &nsb);
        tsfs_add_sbce(fs, &nsb, ce);
    } else { // block already has an extended table, go to it, then add the entry there
        longseek(fs, (((u64)sb->disk_loc)*BLOCK_SIZE) + 14*71 + 1, SEEK_SET);
        block_seek(fs, read_u32be(fs), BSEEK_SET);
        TSFSStructBlock csb = {0};
        read_structblock(fs, &csb);
        tsfs_add_sbce(fs, &csb, ce);
        return 0;
    }
    sb->entrycount ++;
    block_seek(fs, sb->disk_loc, BSEEK_SET); // update entry count
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
    tsfs_mk_ce_name(ce.name, sn->name, ((sn->name)[254]) ? 256 : tsfs_strlen(sn->name)+1);
    return tsfs_add_sbce(fs, parent, &ce);
}

int tsfs_mk_dir(FileSystem* fs, TSFSStructNode* parent, char const* name, TSFSStructBlock* nsb) {
    TSFSStructBlock* sb = tsfs_load_block(fs, parent->parent_loc); // parent block
    char tname[9];
    tsfs_mk_ce_name(tname, name, tsfs_strlen(name)+1);
    if (tsfs_exists(fs, sb, tname)) {
        tsfs_unload(fs, sb);
        return EEXIST;
    }
    // addr of first allocated block
    u32 ac1 = allocate_blocks(fs, 0, 2);
    TSFSStructNode sn = {.storage_flags=TSFS_KIND_DIR,.parent_loc=ac1,.disk_loc=ac1+1,.pnode=parent->disk_loc,.ikey=0};
    awrite_buf(sn.name, name, tsfs_strlen(name));
    nsb->flags = TSFS_CF_DIRE;
    nsb->disk_ref = sn.disk_loc;
    nsb->disk_loc = ac1;
    block_seek(fs, ac1, BSEEK_SET);
    write_structblock(fs, nsb);
    block_seek(fs, nsb->disk_ref, BSEEK_SET);
    write_structnode(fs, &sn);
    tsfs_add_sn(fs, sb, &sn);
    tsfs_unload(fs, sb);
    return 0;
}

int tsfs_mk_file(FileSystem* fs, TSFSStructNode* parent, const char* name) {
    TSFSStructBlock* sb = tsfs_load_block(fs, parent->parent_loc); // parent block
    char tname[9];
    tsfs_mk_ce_name(tname, name, tsfs_strlen(name)+1);
    if (tsfs_exists(fs, sb, tname)) {
        tsfs_unload(fs, sb);
        return EEXIST;
    }
    // addr of first allocated block
    u32 ac1 = allocate_blocks(fs, 0, 1);
    TSFSStructNode sn = {.storage_flags=TSFS_KIND_FILE,.disk_loc=ac1,.pnode=parent->disk_loc,.ikey=0};
    awrite_buf(sn.name, name, tsfs_strlen(name));
    block_seek(fs, ac1, BSEEK_SET);
    write_structnode(fs, &sn);
    tsfs_add_sn(fs, sb, &sn);
    tsfs_unload(fs, sb);
    _tsfs_make_data(fs, &sn);
    return 0;
}

int tsfs_truncate(FileSystem* fs, TSFSStructNode* file, u64 length) {
    if (file->storage_flags != TSFS_KIND_FILE) {
        return EBADF;
    }
    TSFSDataHeader* dh = tsfs_load_head(fs, resolve_itable_entry(fs, file->ikey));
    if (dh->size == length) { // truncate does a no-op
        tsfs_unload(fs, dh);
        return 0;
    }
    int retval = 0;
    TSFSDataBlock* db1 = tsfs_load_data(fs, dh->head);
    char dhbigger = dh->size > length;
    u64 diff = dhbigger ? dh->size - length : length - dh->size;
    dh->size = length;
    if (length == 0) { // needs special handling to ensure format isn't messed up
        u32 db2l = db1->next_block;
        db1->next_block = 0;
        if (db2l != 0) {
            tsfs_free_data(fs, db2l);
        }
        db1->data_length = 0;
        dh->blocks = 1;
        block_seek(fs, db1->disk_loc, BSEEK_SET);
        write_datablock(fs, db1);
        block_seek(fs, dh->disk_loc, BSEEK_SET);
        write_dataheader(fs, dh);
    } else if (dhbigger) { // truncation is making the file smaller
        u64 sreached = 0;
        u32 kept_blocks = 1;
        while (sreached + db1->data_length < length) { // find the last data block that will be kept
            kept_blocks ++;
            sreached += db1->data_length;
            if (db1->next_block == 0) break;
            u32 nb = db1->next_block;
            tsfs_unload(fs, db1);
            tsfs_load_data(fs, nb);
        }
        u32 nblk = db1->next_block;
        db1->next_block = 0;
        if (sreached + db1->data_length > length) {
            db1->data_length = (u16)(length - sreached);
        }
        seek(fs, -TSFSDATABLOCK_DSIZE, SEEK_CUR);
        write_datablock(fs, db1);
        tsfs_unload(fs, db1);
        dh->blocks = kept_blocks;
        block_seek(fs, dh->disk_loc, BSEEK_SET);
        write_dataheader(fs, dh);
        if (nblk) {
            tsfs_free_data(fs, nblk);
        }
    } else { // truncation is making the file larger
        u64 sreached = 0;
        while (db1->next_block) { // find the last data block
            sreached += db1->data_length;
            u32 nb = db1->next_block;
            tsfs_unload(fs, db1);
            tsfs_load_data(fs, nb);
        }
        sreached -= db1->data_length;
        u16 fsize = 1024 - TSFSDATABLOCK_DSIZE;
        sreached += fsize;
        if (sreached > length) {
            sreached -= fsize;
            sreached += db1->data_length;
            u16 nlen = length - sreached;
            size_t siz = TSFSDATABLOCK_DSIZE + db1->data_length;
            seek(fs, siz, SEEK_CUR);
            size_t len = nlen-db1->data_length;
            unsigned char zbuf[1024] = {0};
            write_buf(fs, zbuf, len);
            db1->data_length = nlen;
            block_seek(fs, db1->disk_loc, BSEEK_SET);
            write_datablock(fs, db1);
        } else {
            u64 needed = length - sreached;
            u16 blocks_needed = (u16)((needed / ((u64)fsize)) + ((needed % fsize) != 0));
            // printf("LEN: %llu\nREA: %llu\nNEE: %llu\nBNE: %u\n", length, sreached, needed, blocks_needed);
            db1->data_length = fsize;
            write_datablock(fs, db1);
            append_datablocks(fs, file, blocks_needed);
            dh->blocks += (u32)blocks_needed;
        }
        block_seek(fs, dh->disk_loc, BSEEK_SET);
        write_dataheader(fs, dh);
    }
    end:
    tsfs_unload(fs, db1);
    tsfs_unload(fs, dh);
    return retval;
}

int _tsfs_find_sbcsfe_do(FileSystem* fs, TSFSSBChildEntry* ce, void* data) {
    // _DBG_print_child(ce);
    if (tsfs_cmp_cename(ce->name, data)) {
        // _DBG_here();
        u64 cl = tsfs_tell(fs);
        block_seek(fs, ce->dloc, BSEEK_SET);
        TSFSStructNode sn;
        read_structnode(fs, &sn);
        loc_seek(fs, cl);
        if (!tsfs_cmp_name(*((void**)(((char*)data)+13)), sn.name)) {
            return 0;
        }
        *((u32*)(((char*)data)+9)) = ce->dloc;
        return 1;
    }
    return 0;
}

/*
resolves the location of [par]'s child with [name] on disk
returns zero on failure
*/
u32 tsfs_find(FileSystem* fs, TSFSStructNode* par, const char* name) {
    if (par->storage_flags & ~TSFS_KIND_DIR) {
        return 0;
    }
    void* p = allocate(9+sizeof(u32)+sizeof(void*));
    *((u32*)(((char*)p)+9)) = 0;
    printf("NAME: %s\n", name);
    tsfs_mk_ce_name(p, name, tsfs_strlen(name)+1);
    *((const void**)(((char*)p)+13)) = name;
    TSFSStructBlock sb;
    block_seek(fs, par->parent_loc, BSEEK_SET);
    read_structblock(fs, &sb);
    tsfs_sbcs_foreach(fs, &sb, _tsfs_find_sbcsfe_do, p);
    u32 r = *((u32*)(((char*)p)+9));
    deallocate(p, 9+sizeof(u32)+sizeof(void*));
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
                block_seek(fs, curr->pnode, BSEEK_SET);
                read_structnode(fs, curr);
                return;
            }
        }
    }
    char* frag = allocate(cfl+1);
    awrite_buf(frag, path+cfs, cfl);
    frag[cfl] = 0;
    u32 nl = tsfs_find(fs, curr, frag);
    deallocate(frag, cfl+1);
    if (nl == 0) {
        curr->disk_loc = 0;
        return;
    }
    block_seek(fs, nl, BSEEK_SET);
    read_structnode(fs, curr);
}

/*
resolves [path] to a disk location, returns zero on failure
accepts only absolute paths
returns zero on failure
*/
u32 tsfs_resolve_path(FileSystem* fs, const char* path) {
    if (path[0] != '/') {
        return 0;
    }
    TSFSStructNode curr;
    block_seek(fs, fs->rootblock->top_dir, BSEEK_SET);
    read_structnode(fs, &curr);
    int i = 1;
    int cfs = 1;
    while (1) {
        if (path[i] == '/' || path[i] == 0) {
            // _DBG_print_node(&curr);
            _tsfs_respath_step(fs, &curr, path, cfs, i);
            if (curr.disk_loc == 0) {
                return 0;
            }
            cfs = i + 1;
        }
        if (path[i] == 0) break;
        i ++;
    }
    // _DBG_print_node(&curr);
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
