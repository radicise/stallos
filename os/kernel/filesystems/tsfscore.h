#ifndef __TSFSCORE_H__
#define __TSFSCORE_H__ 1
#include "./fsdefs.h"
int tsfs_root_corruption_check(TSFSRootBlock*, u32*);
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
[p_size] is the size in bytes
DO NOT CALL OUTSIDE THE CASE THAT A NEW PARTITION IS BEING MADE
RETURNS FS WITH NULL ROOTBLOCK ON ERROR
*/
FSRet createFS(struct FileDriver* fdr, int kfd, loff_t p_size) {
    s64 curr_time = (s64)(fetch_time(NULL));
    FSRet rv = {.err=EINVAL,.retptr=0};
    if (p_size % BLOCK_SIZE != 0) {
        kernelWarnMsg("INVALID PARTITION SIZE");
        return rv;
    }
    p_size /= BLOCK_SIZE;
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
    tslocks_make(fs);
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
    printf("CENTER: %lx\n", center);
    rblock->usedhalfleft = center;
    rblock->usedhalfright = center;
    rblock->top_dir = 5;
    u64 checksum = hash_rootblock(rblock);
    rblock->checksum = checksum;
    write_rootblock(fs, rblock);
    dmanip_null(fs, 1, 4);
    // block_seek(fs, 1, BSEEK_SET);
    // write_u32be(fs, center);
    TSFSStructNode snode = {
        .storage_flags = TSFS_KIND_DIR,
        .parent_loc = rblock->top_dir + 1,
        .disk_loc = rblock->top_dir,
        .pnode = 0,
        .ikey = 0
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
    tslocks_release(fs);
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
FSRet loadFS(struct FileDriver* fdr, int kfd, loff_t gsize) {
    kernelWarnMsg("WARNING: itable reorganization not implemented yet");
    FSRet rv = {.err=EINVAL, .retptr=0, .retval=0};
    if (gsize % BLOCK_SIZE != 0) {
        kernelWarnMsg("INVALID PARTITION SIZE");
        return rv;
    }
    u32 size = (u32)(gsize / BLOCK_SIZE);
    if (size > TSFS_MAX_PSIZE) {
        kernelWarnMsg("ERROR: INVALID PARTITION SIZE");
        return rv;
    }
    FileSystem* fs = (FileSystem*) allocate(sizeof(FileSystem));
    fs -> fdrive = fdr;
    fs -> kfd = kfd;
    fs -> err = 0;
    tsmagic_make(fs);
    tslocks_make(fs);
    fdr->lseek(kfd, 0, SEEK_SET);
    TSFSRootBlock* rblock = (TSFSRootBlock*) allocate(sizeof(TSFSRootBlock));
    fs -> rootblock = rblock;
    read_rootblock(fs, rblock);
    if (rblock->breakver != VERNOBN) {
        kernelWarnMsg("VERSION INCOMPAT");
        goto err;
    }
    if (tsfs_root_corruption_check(rblock, &size)) {
        kernelWarnMsg("DATA CORRUPT");
        goto err;
    }
    // u64 comphash = hash_rootblock(rblock);
    // if (comphash != rblock->checksum) {
    //     printf("EH: %llx\nAH: %llx\n", comphash, rblock->checksum);
    //     kernelWarnMsg("DATA CORRUPT");
    //     goto err;
    // }
    // if (rblock->partition_size != size) {
    //     kernelWarnMsg("PARTITION SIZE MISMATCH");
    //     goto err;
    // }
    goto ok;
    err:
    releaseFS(fs);
    return rv;
    ok:
    kernelWarnMsg("FS LOADED OK");
    rv.err = 0;
    rv.retptr = fs;
    kernelMsgULong_hex((unsigned long)rv.retptr);
    kernelMsg("\n");
    kernelMsgULong_hex((unsigned long)rv.err);
    kernelMsg("\n");
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
    block_seek(fs, ret, BSEEK_SET);
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
        tsfs_mk_ce_name(buf, nodeptr->name, tsfs_strlen(nodeptr->name)+1);
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

static DynBField* mk_bfield(void) {
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
    ptr->size = nc*8;
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
    // if (quo >= bf->size) {
    //     bf->size ++;
    // }
    bfield_setbit(bf, quo, rem, val);
}

static void bitfield_print(FileSystem* fs, DynBField* ptr) {
    u32 startb = fs->rootblock->usedright+1;
    u32 w = 8;
    u32 h = ptr->size / w;
    printf("PRINTING\n");
    printf("%u, %u\n", w, h);
    for (u32 i = 0; i < h; i ++) {
        for (u32 j = 0; j < w; j ++) {
            u32 blk = (i*w+j);
            printf("%lu:%u ", startb+blk, get_bfield_block(ptr, blk));
        }
        printf("\n");
    }
    if ((ptr->size % w) > 0) {
        u32 siz = ptr->size;
        for (u32 i = 0; i < siz%w; i ++) {
            u32 blk = siz - i - 1;
            printf("%lu:%u ", startb+blk, get_bfield_block(ptr, blk));
        }
        printf("\n");
    }
}

static inline u32 minu32(u32 n1, u32 n2) {return n1 ? n1 < n2 : n2;}

static char test_dataheader(FileSystem* fs, u32 block_no) {
    block_seek(fs, block_no, BSEEK_SET);
    TSFSDataHeader dh = {0};
    read_dataheader(fs, &dh);
    if (dh.checksum == hash_dataheader(&dh)) {
        return 1;
    }
    return 0;
}

/*
frees all data blocks starting from the given dataheader or datablock
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
    // init the bitfield
    DynBField* bf = mk_bfield();
    u32 used = tblocks-ur-1;
    {
        u32 bfsize = (used)/8;
        printf("USED: %u\nBFSIZE: %u\n", used, bfsize);
        bfield_setsize(bf, bfsize + ((used%8>0)?1:0));
        printf("BSACK: %lu\n", bf->cap);
    }
    set_bfield_block(bf, oblk-ur, 1);
    int rcode = 0;
    // # of blocks removed (for updating the used right count)
    u32 removed = 1;
    u32 last_hole = 0;
    u32 first_live = used;
    TSFSDataBlock crdb;
    // check for data header deletion
    if (dhtest.checksum == comphash) {
        // printf("REMDH\n");
        set_bfield_block(bf, dhtest.head-ur, 1);
        block_seek(fs, dhtest.head, BSEEK_SET);
        read_datablock(fs, &crdb);
        removed ++;
    } else {
        block_seek(fs, block_no, BSEEK_SET);
        read_datablock(fs, &crdb);
    }
    // populate the bitfield
    while (crdb.next_block) {
        removed ++;
        set_bfield_block(bf, crdb.next_block-ur, 1);
        block_seek(fs, crdb.next_block, BSEEK_SET);
        read_datablock(fs, &crdb);
    }
    for (u32 i = 0; i < used; i ++) {
        if (get_bfield_block(bf, i)) {
            if (i > last_hole) {
                last_hole = i;
            }
        } else {
            if (i < first_live) {
                first_live = i;
            }
        }
    }
    printf("FL-LH DATA: %lu, %lu\n", first_live, last_hole);
    // DEBUG INFO
    bitfield_print(fs, bf);
    if (first_live > last_hole) { // simple deletion, no movement required
        dmanip_null(fs, ur, first_live);
    } else { // movement is required
        printf("NEEDS MOVE\n");
        unsigned char* bigbuf = (unsigned char*)alloc(1024);
        TSFSDataHeader workdh = {0};
        TSFSDataBlock workdb = {0};
        TSFSDataBlock dbadj = {0};
        int itercount = 0;
        while (!(first_live > last_hole)) {
            itercount ++;
            if (itercount > 25) {
                printf("ITERC EXCEEDED\n");
                break;
            }
            printf("ITER: %i\nFL: %lu\nLH: %lu\n", itercount, first_live, last_hole);
            bitfield_print(fs, bf);
            // number of holes in the run
            u32 hamt = 0;
            for (u32 i = last_hole; i > 0; i --) {
                if (get_bfield_block(bf, i)) {
                    hamt ++;
                } else {
                    break;
                }
            }
            printf("HAMT,LH: %lu, %lu\n", hamt, last_hole);
            // number of live blocks in the run
            u32 lamt = 0;
            for (u32 i = first_live; i < first_live+hamt; i ++) {
                if (!get_bfield_block(bf, i)) {
                    set_bfield_block(bf, i, 1);
                    lamt ++;
                } else {
                    break;
                }
            }
            printf("LAMT,FL: %lu, %lu\n", lamt, first_live);
            for (u32 i = first_live+lamt-1; i >= first_live; i --) {
                printf("I: %lu\n", i);
                u32 sblk = i+ur;
                u32 dblk = last_hole - ((first_live+lamt-1)-i);
                set_bfield_block(bf, dblk, 0);
                dblk += ur;
                dmanip_null(fs, dblk, 1);
                if (test_dataheader(fs, sblk)) { // move data header
                    block_seek(fs, sblk, BSEEK_SET);
                    read_dataheader(fs, &workdh);
                    dmanip_null(fs, sblk, 1);
                    update_itable_entry(fs, workdh.ikey, dblk);
                    block_seek(fs, dblk, BSEEK_SET);
                    write_dataheader(fs, &workdh);
                    block_seek(fs, workdh.head, BSEEK_SET);
                    read_datablock(fs, &workdb);
                    seek(fs, -TSFSDATABLOCK_DSIZE, SEEK_CUR);
                    workdb.prev_block = dblk;
                    write_datablock(fs, &workdb);
                } else { // move data block
                    block_seek(fs, sblk, BSEEK_SET);
                    read_datablock(fs, &workdb);
                    read_buf(fs, bigbuf, workdb.data_length);
                    dmanip_null(fs, sblk, 1);
                    block_seek(fs, dblk, BSEEK_SET);
                    write_datablock(fs, &workdb);
                    write_buf(fs, bigbuf, workdb.data_length);
                    if (test_dataheader(fs, workdb.prev_block)) {
                        block_seek(fs, workdb.prev_block, BSEEK_SET);
                        read_dataheader(fs, &workdh);
                        workdh.head = dblk;
                        seek(fs, -TSFSDATAHEADER_DSIZE, SEEK_CUR);
                        write_dataheader(fs, &workdh);
                    } else {
                        block_seek(fs, workdb.prev_block, BSEEK_SET);
                        read_datablock(fs, &dbadj);
                        dbadj.next_block = dblk;
                        seek(fs, -TSFSDATABLOCK_DSIZE, SEEK_CUR);
                        write_datablock(fs, &dbadj);
                    }
                    if (workdb.next_block) {
                        block_seek(fs, workdb.next_block, BSEEK_SET);
                        read_datablock(fs, &dbadj);
                        seek(fs, -TSFSDATABLOCK_DSIZE, SEEK_CUR);
                        dbadj.prev_block = dblk;
                        write_datablock(fs, &dbadj);
                    }
                }
            }
            last_hole = 0;
            first_live = used;
            for (u32 i = 0; i < used; i ++) {
                if (get_bfield_block(bf, i)) {
                    if (i > last_hole) {
                        last_hole = i;
                    }
                } else {
                    if (i < first_live) {
                        first_live = i;
                    }
                }
            }
            if (first_live > last_hole) {
                printf("NORMAL ESCAPE\n");
            }
        }
        dealloc(bigbuf, 1024);
        last_hole = 0;
        first_live = used;
        for (u32 i = 0; i < used; i ++) {
            if (get_bfield_block(bf, i)) {
                if (i > last_hole) {
                    last_hole = i;
                }
            } else {
                if (i < first_live) {
                    first_live = i;
                }
            }
        }
        dmanip_null(fs, ur, first_live);
    }
    cleanup:
    rel_bfield(bf);
    fs->rootblock->usedright += removed;
    block_seek(fs, 0, BSEEK_SET);
    write_rootblock(fs, fs->rootblock);
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
        // seek(fs, -TSFSDATABLOCK_DSIZE, SEEK_CUR);
        block_seek(fs, odb->disk_loc, BSEEK_SET);
        write_datablock(fs, odb);
        tsfs_unload(fs, odb);
        block_seek(fs, blk, BSEEK_SET);
        write_datablock(fs, db);
    } else {
        block_seek(fs, pos, BSEEK_SET);
        read_datablock(fs, db);
    }
}

/*
writes data to a file
if [position] is greater than the size of the file, the operation fails

FS INVARIANT CONSIDERATION:
it must never be the case that all of a file's data blocks are completely filled

*/
size_t data_write(FileSystem* fs, TSFSStructNode* sn, u64 position, const void* data, size_t size) {
    // constants
    size_t osize = size;
    u32 dbfullsize = BLOCK_SIZE - TSFSDATABLOCK_DSIZE;
    // validations
    u32 data_loc = resolve_itable_entry(fs, sn->ikey);
    if (data_loc == 0) {
        return 0;
    }
    TSFSDataHeader* header = tsfs_load_head(fs, data_loc);
    if (position > header->size) {
        goto err;
    }
    // traversal
    u64 cpos = 0;
    TSFSDataBlock dblock = {0};
    block_seek(fs, header->head, BSEEK_SET);
    read_datablock(fs, &dblock);
    while (1) {
        if ((cpos + dblock.data_length) >= position) {
            break;
        }
        if (dblock.next_block == 0) {
            goto err;
        }
        cpos += dblock.data_length;
        block_seek(fs, dblock.next_block, BSEEK_SET);
        read_datablock(fs, &dblock);
    }
    u32 first_offset = position - cpos;
    seek(fs, first_offset, SEEK_CUR);
    // at this point:
    // the position in the partition is at the first byte data should be written to
    u64 schange = 0;
    int blocks_added = 0;
    if (dbfullsize - first_offset >= size) { // only need to modify one block
        write_buf(fs, data, size);
        schange = (first_offset + size) - dblock.data_length;
        dblock.data_length = first_offset + size;
        block_seek(fs, dblock.disk_loc, BSEEK_SET);
        write_datablock(fs, &dblock);
        header->size += schange;
        block_seek(fs, header->disk_loc, BSEEK_SET);
        write_dataheader(fs, header);
        tsfs_unload(fs, header);
        return size;
    }
    size_t amount_left = size - (dbfullsize - first_offset);
    write_buf(fs, data, (dbfullsize - first_offset));
    schange = dbfullsize - dblock.data_length;
    dblock.data_length = dbfullsize;
    block_seek(fs, dblock.disk_loc, BSEEK_SET);
    write_datablock(fs, &dblock);
    while (amount_left) {
        getcreat_databloc(fs, dblock.disk_loc, dblock.next_block, &dblock, &blocks_added);
        // block_seek(fs, dblock.next_block, BSEEK_SET);
        // read_datablock(fs, &dblock);
        if (amount_left > dbfullsize) {
            schange += (dbfullsize - dblock.data_length);
            write_buf(fs, (const void*)(((char*)data) + (size - amount_left)), dbfullsize);
            dblock.data_length = dbfullsize;
            block_seek(fs, dblock.disk_loc, BSEEK_SET);
            write_datablock(fs, &dblock);
            amount_left -= dbfullsize;
            continue;
        }
        schange += amount_left >= dblock.data_length ? (amount_left - dblock.data_length) : 0;
        write_buf(fs, (const void*)(((char*)data)+(size-amount_left)), amount_left);
        dblock.data_length = amount_left >= dblock.data_length ? amount_left : dblock.data_length;
        block_seek(fs, dblock.disk_loc, BSEEK_SET);
        write_datablock(fs, &dblock);
        amount_left = 0;
    }
    header->size += schange;
    header->blocks += blocks_added;
    block_seek(fs, header->disk_loc, BSEEK_SET);
    write_dataheader(fs, header);
    tsfs_unload(fs, header);
    return size;
    err:
    tsfs_unload(fs, header);
    return 0;
}

// TODO: make work with more than one data block
size_t data_read(FileSystem* fs, TSFSStructNode* sn, u64 position, void* data, size_t size) {
    size_t osize = size;
    struct PosDat data_loc = tsfs_traverse(fs, sn, position, 0);
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

/*
tests a [TSFSRootBlock] for corrupted / invalid / malicious data
[psize_actual] is a pointer to the known partition size, if available
 if unavailable, [psize_actual] should be set to zero
returns `1` if corruption was detected
*/
int tsfs_root_corruption_check(TSFSRootBlock* rb, u32* psize_actual) {
    int corrupt = 0;
    if (psize_actual != 0) { // if psize_actual was provided
        if (rb->partition_size != *psize_actual) { // root does not match size known to be true
            corrupt = 1;
            kernelWarnMsg("partition size mismatch");
        }
    }
    u64 chek = hash_rootblock(rb);
    if (chek != rb->checksum) { // mismatched hashes
        corrupt = 1;
        kernelWarnMsg("checksum mismatch");
    }
    u32 maxb = rb->partition_size;
    if (rb->usedright >= maxb) { // invalid usedright
        corrupt = 1;
        kernelWarnMsg("invalid usedright");
    }
    if (rb->usedleft < TSFS_CORE_LPROTECT) { // usedleft is below minimum
        corrupt = 1;
        kernelWarnMsg("usedleft below minimum");
    }
    if (rb->usedleft >= rb->usedright) { // used left crossed used right
        corrupt = 1;
        kernelWarnMsg("usedleft crossed usedright");
    }
    if (rb->usedhalfleft >= rb->usedhalfright || rb->usedhalfleft <= rb->usedleft) { // invalid usedhalfleft
        corrupt = 1;
        kernelWarnMsg("invalid usedhalfleft");
    }
    if (rb->usedhalfright >= rb->usedright) { // invalid usedhalfright
        corrupt = 1;
        kernelWarnMsg("invalid usedhalfright");
    }
    return corrupt;
}

#endif
