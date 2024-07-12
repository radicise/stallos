#ifndef __TSFSCORE_H__
#define __TSFSCORE_H__ 1
#include "./fsdefs.h"
// extern int printf(const char*, ...);
// #include "./tsfsmanage.h"
// #include <string.h>
// #undef __MOCKTEST
// #ifndef __MOCKTEST
char* strcpy(char* dst, const char* src) {
    while (*src) {
        *dst = *src;
        src++;
        dst++;
    }
    return 0;
}
// #else
// #include <string.h>
// #endif

#define TSFS_CORE_LPROTECT 7

u32 partition_blocks(FileSystem* fs) {
    // return (u32)((((u64)1)<<(fs->rootblock->partition_size)) / BLOCK_SIZE);
    return fs->rootblock->partition_size;
}

#define TSFS_MAX_PSIZE 2147483648

/*
available for external use
[p_size] is the size in blocks
DO NOT CALL OUTSIDE THE CASE THAT A NEW PARTITION IS BEING MADE
RETURNS FS WITH NULL ROOTBLOCK ON ERROR
*/
FSRet createFS(struct FileDriver* fdr, int kfd, loff_t p_size) {
    s64 curr_time = (s64)(kernel_time(NULL));
    FSRet rv = {.err=EINVAL};
    if (p_size > TSFS_MAX_PSIZE) {
        kernelWarnMsg("PARTITION TOO BIG");
        return rv;
    }
    rv.err = 0;
    FileSystem* fs = (FileSystem*) allocate(sizeof(FileSystem));
    fs -> fdrive = fdr;
    fs -> kfd = kfd;
    fdr->lseek(kfd, 0, SEEK_SET);
    tsmagic_make(fs);
    TSFSRootBlock* rblock = (TSFSRootBlock*) allocate(sizeof(TSFSRootBlock));
    fs -> rootblock = rblock;
    rblock->breakver = VERNOBN;
    strcpy(rblock->version, TSFSVERSION);
    rblock->partition_size = (u32)p_size;
    rblock->creation_time = *((u64*)(&curr_time));
    rblock->usedleft = 7;
    // u32 tblocks = (u32)((((u64)1)<<(rblock->partition_size)) / BLOCK_SIZE);
    u32 tblocks = (u32)p_size;
    rblock->usedright = tblocks - 1;
    u32 center = tblocks/2;
    rblock->usedhalfleft = center-1;
    rblock->usedhalfright = center+1;
    rblock->top_dir = 5;
    u64 checksum = hash_rootblock(rblock);
    rblock->checksum = checksum;
    write_rootblock(fs, rblock);
    dmanip_null(fs, 1, 4);
    block_seek(fs, 1, BSEEK_SET);
    write_u32be(fs, center);
    TSFSStructNode snode = {
        .storage_flags = TSFS_KIND_DIR,
        .parent_loc = rblock->top_dir + 1,
        .disk_loc = rblock->top_dir,
        .pnode = 0,
        .data_loc = 0,
        .inum = 0
    };
    snode.name[0] = '/';
    block_seek(fs, rblock->top_dir, BSEEK_SET);
    write_structnode(fs, &snode);
    TSFSStructBlock sblock = {0};
    sblock.disk_ref = rblock->top_dir;
    sblock.flags = TSFS_CF_DIRE;
    sblock.entrycount = 0;
    block_seek(fs, snode.parent_loc, BSEEK_SET);
    write_structblock(fs, &sblock);
    block_seek(fs, center, BSEEK_SET);
    write_u32be(fs, 0xff010000UL);
    write_u32be(fs, 1);
    block_seek(fs, center+1, BSEEK_SET);
    write_u32be(fs, 0xff010100UL);
    write_u32be(fs, center);
    write_u32be(fs, rblock->top_dir);
    rv.retptr = fs;
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
FSRet loadFS(struct FileDriver* fdr, int kfd, loff_t size) {
    kernelWarnMsg("WARNING: itable reorganization not implemented yet");
    FSRet rv = {.err=EINVAL};
    if (size > TSFS_MAX_PSIZE) {
        kernelWarnMsg("ERROR: INVALID PARTITION SIZE");
        return rv;
    }
    FileSystem* fs = (FileSystem*) allocate(sizeof(FileSystem));
    fs -> fdrive = fdr;
    fs -> kfd = kfd;
    fs -> err = 0;
    tsmagic_make(fs);
    fdr->lseek(kfd, 0, SEEK_SET);
    TSFSRootBlock* rblock = (TSFSRootBlock*) allocate(sizeof(TSFSRootBlock));
    fs -> rootblock = rblock;
    read_rootblock(fs, rblock);
    if (rblock->breakver != VERNOBN) {
        kernelWarnMsg("VERSION INCOMPAT");
        goto err;
    }
    u64 comphash = hash_rootblock(rblock);
    if (comphash != rblock->checksum) {
        kernelWarnMsg("DATA CORRUPT");
        goto err;
    }
    if (rblock->partition_size != size) {
        kernelWarnMsg("PARTITION SIZE MISMATCH");
        goto err;
    }
    goto ok;
    err:
    releaseFS(fs);
    return rv;
    ok:
    rv.err = 0;
    rv.retptr = fs;
    return rv;
}

/*
allocates count contiguous blocks and returns the block number of the first one (left to right)
returns zero if unable to allocate the requested blocks

area is ternary and designates which way to search for available blocks, zero is start to end, one is end to start,
and 2 is balanced centered
NOTE: when area is 2, count is irrelevant, and exactly one block is allocated
*/
u32 allocate_blocks(FileSystem* fs, u8 area, u16 count) {
    u32 ul = fs->rootblock->usedleft;
    u32 ur = fs->rootblock->usedright;
    u32 uhl = fs->rootblock->usedhalfleft;
    u32 uhr = fs->rootblock->usedhalfright;
    u32 ret;
    if (area == 1) {
        if (count >= ur) return 0;
        ur -= count;
        if (ur < uhr) { // ensure no overwrite
            return 0;
        }
        fs->rootblock->usedright = ur;
        ret = ur;
        goto end;
    }
    if (area == 2) {
        // u32 center = ((u32)((((u64)2)<<(fs->rootblock->partition_size-1)) / BLOCK_SIZE))/2;
        u32 center = fs->rootblock->partition_size/2;
        if ((center-uhl) == (uhr-center)) {
            ret = uhl;
            uhl -= 1;
            if (uhl < ul) return 0;
            fs->rootblock->usedhalfleft = uhl;
            goto end;
        }
        uhr += 1;
        if (uhr >= ur) return 0;
        fs->rootblock->usedhalfright = uhr;
        ret = uhr;
        goto end;
    }
    ul += count;
    if (ul > uhl) { // ensure no overwrite
        return 0;
    }
    fs->rootblock->usedleft = ul;
    ret = ul - count;
    end:
    block_seek(fs, 0, BSEEK_SET);
    write_rootblock(fs, fs->rootblock);
    return ret;
}

/*
frees an itable block

if a hole is created, the moved block may be from the other side of the center in order to keep the middle
balanced
*/
int tsfs_free_centered(FileSystem* fs, u32 block_no) {
    u32 ul = fs->rootblock->usedleft;
    u32 ur = fs->rootblock->usedright;
    u32 uhl = fs->rootblock->usedhalfleft;
    u32 uhr = fs->rootblock->usedhalfright;
    // u32 center = ((u32)((((u64)2)<<(fs->rootblock->partition_size-1)) / BLOCK_SIZE))/2;
    u32 center = fs->rootblock->partition_size/2;
    u32 dl = (center-uhl); 
    u32 dr = (uhr-center);
    if (block_no <= uhl || block_no > uhr) return -1; // bounds check
    u32 move_no;
    if (block_no > center) {
        uhr --;
        if ((block_no-1) == uhr) {
            goto good_exit;
        }
        if (dl >= dr) {
            uhl ++;
            uhr ++;
            move_no = uhl;
        } else {
            move_no = uhr+1;
        }
    } else {
        uhl ++;
        if (block_no == uhl) {
            goto good_exit;
        }
        if (dr >= dl) {
            uhl --;
            uhr --;
            move_no = uhr+1;
        } else {
            move_no = uhl;
        }
    }
    block_seek(fs, move_no, BSEEK_SET);
    unsigned char* buffer = allocate(BLOCK_SIZE);
    read_buf(fs, buffer, BLOCK_SIZE);
    block_seek(fs, block_no, BSEEK_SET);
    write_buf(fs, buffer, BLOCK_SIZE);
    u32 meta = areadu32be(buffer);
    u32 ploc = areadu32be(buffer+4);
    deallocate(buffer, BLOCK_SIZE);
    block_seek(fs, ploc, BSEEK_SET);
    seek(fs, ((meta&0xff0000)?8:0) // offset by 8 bytes if the parent is an L1 table to account for metadata
        +(4*(meta&0xff)), // go to the entry that needs to be updated
        SEEK_CUR);
    write_u32be(fs, block_no);
    good_exit:
    fs->rootblock->usedhalfleft = uhl;
    fs->rootblock->usedhalfright = uhr;
    block_seek(fs, 0, BSEEK_SET);
    write_rootblock(fs, fs->rootblock);
    return 0;
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
    if (block_no >= ul || block_no < TSFS_CORE_LPROTECT) { // protect bounds
        printf("BLKNO: %u\n", block_no);
        magic_smoke(FEDRIVE | FEARG | FEINVL);
        return -1;
    }
    fs->rootblock->usedleft -= 1;
    ul --;
    longseek(fs, 0, SEEK_SET);
    write_rootblock(fs, fs->rootblock);
    // printf("AFTER ROOT UPDATE\n");
    // printf("NULLING BLOCK {%u}\n", block_no);
    // getchar();
    dmanip_null(fs, block_no, 1); // destroy invalid data
    // getchar();
    if (block_no == ul) { // freed the last used block, nothing else needs to be done
        return 0;
    }
    // printf("AFTER EARLY END CHECK\n");
    u64 np = ((u64)(block_no)) * ((u64)BLOCK_SIZE);
    // printf("AFTER CALC, NEW POS: %lx, NEW BLK: %u\n", np, tsfs_loc_to_block(np));
    TSFSStructBlock sblock = {0};
    // TSFSStructNode snode = {0};
    TSFSStructBlock* blockptr = 0;
    TSFSStructNode* nodeptr = 0;
    block_seek(fs, (s32)ul, BSEEK_SET);
    read_structblock(fs, &sblock);
    // blockptr = tsfs_load_block(fs, 0);
    // printf("AFTER BLOCK LOAD, PTR = %p\n", (void*)blockptr);
    // _DBG_print_block(blockptr);
    u64 comphash = hash_structblock(&sblock);
    // u64 comphash = hash_structblock(blockptr);
    // printf("AFTER HASH\n");
    // printf("COMP HASH = %lx\n", comphash);
    // printf("PTRHASH = %lx\n", blockptr->checksum);
    // check if the final block is a struct block by reading it as one and seeing if the hash is correct
    if (sblock.checksum == comphash) { // struct block
        // printf("FREE BLOCK\n");
        // _DBG_print_block(blockptr);
        // loc_seek(fs, blockptr->disk_ref);
        // read_structnode(fs, &snode);
        nodeptr = tsfs_load_node(fs, blockptr->disk_ref);
        // _DBG_print_node(nodeptr);
        // snode.parent_loc = np;
        nodeptr->parent_loc = block_no;
        // sblock.disk_loc = snode.parent_loc;
        blockptr->disk_loc = block_no;
        dmanip_null(fs, ul, 1); // ensure random invalid data isn't sticking around on the drive
        loc_seek(fs, nodeptr->parent_loc); // move the struct block to its new location
        write_structblock(fs, blockptr);
    } else { // struct node
        // printf("FREE NODE\n");
        seek(fs, -14, SEEK_CUR);
        // read_structnode(fs, &snode);
        nodeptr = tsfs_load_node(fs, 0);
        // _DBG_print_node(nodeptr);
        dmanip_null(fs, ul, 1);
        if (nodeptr->parent_loc) {
            // tsfs_unload(fs, blockptr);
            // loc_seek(fs, snode.parent_loc);
            // read_structblock(fs, &sblock);
            blockptr = tsfs_load_block(fs, nodeptr->parent_loc);
            // sblock.disk_ref = np;
            blockptr->disk_ref = block_no;
            seek(fs, -14, SEEK_CUR);
            write_structblock(fs, blockptr);
            tsfs_unload(fs, blockptr);
        }
        block_seek(fs, (s32)block_no, BSEEK_SET);
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
    // tsfs_unload(fs, blockptr);
    tsfs_unload(fs, nodeptr);
    return 0;
}

#define DYNFIELD_MAX_SIZE 2048

/*
dynamically sized bit field
*/
typedef struct DynBitField {
    u32    cap;
    u32    size;
    char*  field;
} DynBField;

static DynBField* mk_bfield() {
    DynBField* ptr = allocate(sizeof(DynBField));
    ptr->size = 0;
    ptr->cap = 4;
    ptr->field = allocate(4);
    ptr->field[0] = 0;
    ptr->field[1] = 0;
    ptr->field[2] = 0;
    ptr->field[3] = 0;
    return ptr;
}
static void rel_bfield(DynBField* ptr) {
    deallocate(ptr->field, ptr->cap);
    deallocate(ptr, sizeof(DynBField));
}
static void bfield_upsize(DynBField* ptr) {
    u32 nc = ptr->cap * 2;
    if (nc > DYNFIELD_MAX_SIZE) {
        magic_smoke(FEALLOC|FEBIG);
        return;
    }
    char* nb = allocate((size_t)nc);
    awrite_buf(nb, ptr->field, ptr->size);
    for (int i = ptr->size; i < nc; i ++) {
        nb[i] = 0;
    }
    deallocate(ptr->field, ptr->cap);
    ptr->field = nb;
    ptr->cap = nc;
}
static void bfield_downsize(DynBField* ptr) {
    u32 nc = ptr->cap / 2;
    if (nc < 4 || nc <= ptr->size) {
        magic_smoke(FEARG|FESMALL);
        return;
    }
    char* nb = allocate((size_t)nc);
    awrite_buf(nb, ptr->field, ptr->size);
    for (int i = ptr->size; i < nc; i ++) {
        nb[i] = 0;
    }
    deallocate(ptr->field, ptr->cap);
    ptr->field = nb;
    ptr->cap = nc;
}
static inline char bfield_get(DynBField* bf, u32 index) {
    return bf->field[index];
}
static inline void bfield_set(DynBField* bf, u32 index, char value) {
    bf->field[index] = value;
}
static inline char bfield_getbit(DynBField* bf, u32 index, char bit) {
    return (bfield_get(bf, index)>>bit)&1;
}
static void bfield_setbit(DynBField* bf, u32 index, char bit, char value) {
    value = value << bit;
    char item = bfield_get(bf, index);
    item = (item&(~(1<<bit)))|value;
    bfield_set(bf, index, item);
}
static void bfield_push(DynBField* bf, char item) {
    if ((bf->size+1) == bf->cap) {
        bfield_upsize(bf);
    }
    bf->field[(bf->size)++] = item;
}
static char bfield_pop(DynBField* bf) {
    char r = bf->field[--(bf->size)];
    if (bf->cap > 4 && bf->size <= (bf->cap/4)) {
        bfield_downsize(bf);
    }
    return r;
}
static void bfield_setsize(DynBField* ptr, u32 nc) {
    char* nb = (nc)?allocate((size_t)nc):0;
    for (int i = 0; i < nc; i ++) {
        nb[i] = 0;
    }
    if (ptr->cap > 0) {
        deallocate(ptr->field, ptr->cap);
    }
    ptr->cap = nc;
    ptr->size = 0;
    ptr->field = nb;
}

static inline char map_block_to_bit(u32 block) {return (char)(block%8);}
static inline char get_bfield_block(DynBField* bf, u32 block) {
    u32 quo = block / 8;
    char rem = (char)(block % 8);
    return bfield_getbit(bf, quo, rem);
}
static inline void set_bfield_block(DynBField* bf, u32 block, char val) {
    u32 quo = block / 8;
    char rem = (char)(block % 8);
    bfield_setbit(bf, quo, rem, val);
}

static void bitfield_print(FileSystem* fs, DynBField* ptr) {
    u32 startb = fs->rootblock->usedright+1;
    u32 w = 8;
    u32 h = ptr->size / w + ((ptr->size%w)>0?1:0);
    printf("PRINTING\n");
    printf("%u, %u\n", w, h);
    for (u32 i = 0; i < h; i ++) {
        for (u32 j = 0; j < w; j ++) {
            u32 blk = (i*w+j);
            printf("%lu:%u ", startb+blk, get_bfield_block(ptr, blk));
        }
        printf("\n");
    }
}

/*
frees all data blocks starting from the given dataheader
!!WARNING!!
cleanup MUST be complete BEFORE freeing data blocks
*/
int tsfs_free_data(FileSystem* fs, u32 block_no) {
    u32 ur = fs->rootblock->usedright;
    u32 tblocks = partition_blocks(fs);
    if (block_no < ur || block_no >= tblocks) { // protect bounds
        printf("BLKNO: %u\nMAXB: %u\nMINB: %u\n", block_no, tblocks, ur);
        magic_smoke(FEDRIVE | FEARG | FEINVL);
        return -1;
    }
    TSFSDataHeader dhtest;
    block_seek(fs, block_no, BSEEK_SET);
    read_dataheader(fs, &dhtest);
    u64 comphash = hash_dataheader(&dhtest);
    u32 oblk = block_no;
    // u32 tblocks = (u32)((((u64)1)<<(fs->rootblock->partition_size)) / BLOCK_SIZE);
    DynBField* bf = mk_bfield();
    {
        u32 used = tblocks-fs->rootblock->usedright-1;
        u32 bfsize = (used)/8;
        printf("USED: %u\nBFSIZE: %u\n", used, bfsize);
        bfield_setsize(bf, bfsize + ((used%8>0)?1:0));
    }
    set_bfield_block(bf, oblk, 1);
    int rcode = 0;
    u32 removed = 1;
    TSFSDataBlock crdb;
    if (dhtest.checksum == comphash) {
        block_seek(fs, dhtest.head, BSEEK_SET);
    } else {
        block_seek(fs, block_no, BSEEK_SET);
    }
    read_datablock(fs, &crdb);
    while (crdb.next_block) {
        set_bfield_block(bf, crdb.next_block, 1);
        block_seek(fs, crdb.next_block, BSEEK_SET);
        read_datablock(fs, &crdb);
    }
    bitfield_print(fs, bf);
    cleanup:
    rel_bfield(bf);
    return rcode;
}

#undef DYNFIELD_MAX_SIZE

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
        TSFSDataBlock* odb = tsfs_load_data(fs, loc);
        u32 blk = allocate_blocks(fs, 1, 1);
        db->disk_loc = blk;
        db->data_length = 0;
        db->blocks_to_terminus = 0;
        db->next_block = 0;
        db->prev_block = loc;
        db->storage_flags = TSFS_SF_FINAL_BLOCK|TSFS_SF_LIVE;
        odb->storage_flags &= ~TSFS_SF_FINAL_BLOCK;
        odb->next_block = blk;
        seek(fs, -TSFSDATABLOCK_DSIZE, SEEK_CUR);
        write_datablock(fs, odb);
        tsfs_unload(fs, odb);
        block_seek(fs, blk, BSEEK_SET);
        write_datablock(fs, db);
    } else {
        block_seek(fs, pos, BSEEK_SET);
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
