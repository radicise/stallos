#ifndef __ITABLE_H__
#define __ITABLE_H__ 1
/*
functions for dealing with the inode tables
*/

#include "./fsdefs.h"

/*
ITABLE specs:
first four bytes - flags and index
next four bytes - parent location
all other bytes - children
<8>bytes
<a,1><b,1><c,1><d,1><e,4>
<a> - constant - 0xff, identifies this block as an ITABLE
<b> - bool - singly or doubly indirected block (0=L1,1=L2)
<c> - uint8 - number of entries in this table, when zero, the table will be deallocated, excepting if the table is singly indirected to the first table and has index zero, or is doubly indirected through that table and has index zero
<d> - uint8 - index of this block in parent
<e> - uint32 - disk addr of parent

IMAP key specs:
<4>bytes
<a,6-bit><b,26-bit>
<a> - flags about the key (see key flags)
<b> - the path to the entry (see key path)

key flags:
<6>bits
<a,1><b,1><c,1><d,1><e,1><f,1>
<a> - failed - there was no available entry, has different meanings dependent on what level of search is performed
    when searching an l1 or l2 table, the next table should be searched, when searching the root table, it signals
    that the inode tables are all full, this is highly unlikely to be possible
<b> - created - this is not a valid return of an l2 search, signals that there was no space in any existing
    sub-table, but there was space for a new sub-table to be created, and this has been done
    this can only be returned when the search flags include both ITABLE_FCREATE and ITABLE_FWRITE, the
    returned entry is guaranteed to be filled by the provided value
<c> - found - an available entry was found, if ITABLE_FWRITE was included in flags, the available entry
    is guaranteed to be filled by the provided value
<d-f> - reserved for future use

key path:
<26>bits
<a,10><b,8><c,8>
*/

#define ITABLE_FAILURE 0x80000000UL
#define ITABLE_CREATED 0x40000000UL
#define ITABLE_FOUND 0x20000000UL
#define ITABLE_INDEX 0x03ffffffUL

#define IDATA_LEVL 0xff0000UL
#define IDATA_ENTS 0xff00UL
#define IDATA_INDX 0xffUL

#define ITABLE_FSEARCH 0
#define ITABLE_FWRITE 1
#define ITABLE_FCREATE 2

u32 _search_l2_table(FileSystem* fs, u32 dl, char flags, u32 val) {
    u64 rem = tsfs_tell(fs);
    block_seek(fs, dl, BSEEK_SET);
    u32 dat = read_u32be(fs);
    seek(fs, 4, SEEK_CUR);
    int ec = (int)((dat>>8)&0xff);
    if (ec == 254) {
        loc_seek(fs, rem);
        return ITABLE_FAILURE;
    }
    int l = -1;
    for (int i = 0; i < 254; i ++) {
        u32 dl2 = read_u32be(fs);
        if (dl2 == 0) {
            l = i;
            if (flags & ITABLE_FWRITE) {
                seek(fs, -4, SEEK_CUR);
                write_u32be(fs, val);
                block_seek(fs, dl, BSEEK_SET);
                dat += 256;
                write_u32be(fs, dat);
            }
            break;
        }
    }
    loc_seek(fs, rem);
    if (l >= 0) {
        return ITABLE_FOUND | ((u32)l);
    }
    return ITABLE_FAILURE;
}

u32 _create_l2_table(FileSystem* fs, u32 ploc, u8 pindex, u32 initval) {
    u64 opos = tsfs_tell(fs);
    u32 r = allocate_blocks(fs, 0, 1);
    block_seek(fs, r, BSEEK_SET);
    write_u32be(fs, 0xff010100 + (u32)pindex);
    write_u32be(fs, ploc);
    write_u32be(fs, initval);
    longseek(fs, opos, SEEK_SET);
    return r;
}

u32 _create_l1_table(FileSystem* fs, u32 ploc, u8 pindex, u32 initval) {
    u64 opos = tsfs_tell(fs);
    u32 r = allocate_blocks(fs, 0, 1);
    block_seek(fs, r, BSEEK_SET);
    write_u32be(fs, 0xff000100 + (u32)pindex);
    write_u32be(fs, ploc);
    write_u32be(fs, _create_l2_table(fs, r, 0, initval));
    longseek(fs, opos, SEEK_SET);
    return r;
}

u32 _search_l1_table(FileSystem* fs, u32 dl, char flags, u32 val) {
    u64 rem = tsfs_tell(fs);
    block_seek(fs, dl, BSEEK_SET);
    u32 dat = read_u32be(fs);
    seek(fs, 4, SEEK_CUR);
    // int ec = (int)((dat>>8)&0xff);
    char create = (flags & ITABLE_FWRITE) ? (flags & ITABLE_FCREATE) : 0;
    u32 r = ITABLE_FAILURE;
    for (int i = 0; i < 254; i ++) {
        u32 dl2 = read_u32be(fs);
        if (dl2 == 0) {
            if (!create) continue;
            // create new entry
            seek(fs, -4, SEEK_CUR);
            write_u32be(fs, _create_l2_table(fs, dl, (u8)i, val));
            block_seek(fs, dl, BSEEK_SET);
            dat += 256;
            write_u32be(fs, dat);
            r = ITABLE_CREATED | (((u32)i)<<8);
            break;
        } else {
            u32 res = _search_l2_table(fs, dl2, flags, val);
            if (res & ITABLE_FOUND) {
                r = ITABLE_FOUND | (((u32)i)<<8) | (res&0xff);
                break;
            }
        }
    }
    loc_seek(fs, rem);
    return r;
}

/*
see ITABLE key specs for details on the return value
*/
u32 aquire_itable_slot(FileSystem* fs, u32 value) {
    _DBG_here();
    printf("ITABLE AQU: VALUE=%lx\n", value);
    block_seek(fs, 1, BSEEK_SET);
    for (u32 j = 1; j < 5; j ++) {
        for (int i = 0; i < 256; i ++) {
            u32 daddr = read_u32be(fs);
            if (daddr == 0) {
                // create new entry
                seek(fs, -4, SEEK_CUR);
                write_u32be(fs, _create_l1_table(fs, j, (u8)i, value));
                return ITABLE_CREATED | (((u32)i)<<16);
            } else {
                u32 res = _search_l1_table(fs, daddr, ITABLE_FWRITE | ITABLE_FCREATE, value);
                if (res & (ITABLE_FOUND | ITABLE_CREATED)) {
                    return ITABLE_FOUND | (((u32)i)<<16) | (res&0xffff);
                }
            }
        }
    }
    return ITABLE_FAILURE;
}

int release_itable_slot(FileSystem* fs, u32 imapkey) {
    imapkey = imapkey&ITABLE_INDEX;
    u32 croot, cl1, cl2;
    croot = imapkey>>16;
    cl1 = (imapkey>>8)&0xff;
    cl2 = imapkey&0xff;
    u32 l1p, l2p;
    block_seek(fs, 1, BSEEK_SET);
    seek(fs, 4*croot, SEEK_CUR);
    l1p = read_u32be(fs);
    block_seek(fs, l1p, BSEEK_SET);
    u32 l1d = read_u32be(fs);
    seek(fs, 8+(4*cl1), SEEK_CUR);
    l2p = read_u32be(fs);
    block_seek(fs, l2p, BSEEK_SET);
    u32 l2d = read_u32be(fs);
    seek(fs, 8+(4*cl1), SEEK_CUR);
    write_u32be(fs, 0);
    l2d -= 256;
    block_seek(fs, l2p, BSEEK_SET);
    write_u32be(fs, l2d);
    if (croot == 0 && cl1 == 0) return 0;
    if (l2d & IDATA_ENTS) return 0;
    l1d -= 256;
    block_seek(fs, l1p, BSEEK_SET);
    write_u32be(fs, l1d);
    seek(fs, 8 + (4*cl1&IDATA_INDX), SEEK_CUR);
    write_u32be(fs, 0);
    // tsfs_free_centered(fs, l2p);
    tsfs_free_structure(fs, l2p);
    if (croot == 0 || (l1d&IDATA_ENTS)) return 0;
    block_seek(fs, 1, BSEEK_SET);
    seek(fs, 4 * croot, SEEK_CUR);
    l1p = read_u32be(fs);
    seek(fs, -4, SEEK_CUR);
    write_u32be(fs, 0);
    // tsfs_free_centered(fs, l1p);
    tsfs_free_structure(fs, l1p);
    return 0;
}

int update_itable_entry(FileSystem* fs, u32 ikey, u32 value) {
    ikey = ikey & ITABLE_INDEX;
    u32 iroot = ikey >> 16;
    u32 il1 = (ikey >> 8) & 0xff;
    u32 il2 = ikey & 0xff;
    block_seek(fs, 1, BSEEK_SET);
    seek(fs, 4 * iroot, SEEK_CUR);
    block_seek(fs, read_u32be(fs), BSEEK_SET);
    seek(fs, 8 + (4 * il1), SEEK_CUR);
    block_seek(fs, read_u32be(fs), BSEEK_SET);
    seek(fs, 8 + (4 * il2), SEEK_CUR);
    write_u32be(fs, value);
    return fs->err;
}

u32 resolve_itable_entry(FileSystem* fs, u32 ikey) {
    ikey = ikey & ITABLE_INDEX;
    u32 iroot = ikey >> 16;
    u32 il1 = (ikey >> 8) & 0xff;
    u32 il2 = ikey & 0xff;
    block_seek(fs, 1, BSEEK_SET);
    seek(fs, 4 * iroot, SEEK_CUR);
    block_seek(fs, read_u32be(fs), BSEEK_SET);
    seek(fs, 8 + (4 * il1), SEEK_CUR);
    block_seek(fs, read_u32be(fs), BSEEK_SET);
    seek(fs, 8 + (4 * il2), SEEK_CUR);
    return read_u32be(fs);
}

#endif
