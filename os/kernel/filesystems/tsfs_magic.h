#ifndef __TSFS_MAGIC_H__
#define __TSFS_MAGIC_H__ 1
/*
does all the logic for the absolute MAGIC BULL that I'm pulling with the system redordering the blocks
whenever it feels like it
*/

#include "./fsdefs.h"

// minimum magic size
#define TSFSIMAGIC 16
// maximum magic size
#define TSFSMAGICMAX 1024

#define TSFS_MAGIC_DATA 1
#define TSFS_MAGIC_NODE 2
#define TSFS_MAGIC_BLOC 3
#define TSFS_MAGIC_HEAD 4

#define SB TSFSStructBlock
#define SN TSFSStructNode
#define DB TSFSDataBlock
#define DH TSFSDataHeader

size_t _tsfs_magic_grabno(void* object) {
    return *((size_t*)object);
}
void _tsfs_magic_stabno(void* object, size_t no) {
    *((size_t*)object) = no;
}

/*
copies only non-zero entries from src to dst
dst MUST have enough space for ALL non-zero entries in src
*/
size_t _tsfs_magic_copy(void** dst, void** src, size_t ds, size_t ss) {
    size_t j = 0;
    void* cc;
    for (int i = 0; i < ss; i ++) {
        cc = src[i];
        if (cc) {
            if (j >= ds) {
                return j;
            }
            _tsfs_magic_stabno(cc, j);
            dst[j++] = cc;
        }
    }
    return j;
}

/*
multiplies the size of the magic by 2^mult
AUTOMATICALLY REORGANIZES
CANNOT GO ABOVE [TSFSMAGICMAX]
*/
void _tsmagic_upsize(Magic* m, int mult) {
    size_t ns = m->csize * (size_t)(1<<mult);
    if (ns > TSFSMAGICMAX) {
        magic_smoke(FEWAND | FEBIG | FEALLOC);
        return;
    }
    void** nptr = allocate(sizeof(void*)*ns);
    m->umax = _tsfs_magic_copy(nptr, m->ptr, ns, m->csize);
    m->hmin = m->umax;
    m->cused = m->umax;
    deallocate(m->ptr, sizeof(void*)*m->csize);
    m->ptr = nptr;
    m->csize = ns;
}
/*
divides the size of the magic by 2^mult
AUTOMATICALLY REORGANIZES
CANNOT GO BELOW [TSFSIMAGIC]
*/
void _tsmagic_desize(Magic* m, int mult) {
    size_t ns = m->csize / (size_t)(1<<mult);
    if (ns < TSFSIMAGIC) {
        magic_smoke(FEWAND | FESMALL | FEALLOC);
        return;
    }
    void** nptr = allocate(sizeof(void*)*ns);
    m->umax = _tsfs_magic_copy(nptr, m->ptr, ns, m->csize);
    m->hmin = m->umax;
    m->cused = m->umax;
    deallocate(m->ptr, sizeof(void*)*m->csize);
    m->ptr = nptr;
    m->csize = ns;
}

size_t _tsfs_magic_getsize(void* object) {
    u8 id = *((u8*)(object)+sizeof(size_t)+sizeof(u32)+sizeof(u16));
    return (id == 0) ? sizeof(TSFSStructNode) : ((id == 1) ? sizeof(TSFSStructBlock) : ((id == 2) ? sizeof(TSFSDataBlock) : ((id == 3) ? sizeof(TSFSDataHeader) : 0)));
}

void tsmagic_make(FileSystem* fs) {
    fs->magic = allocate(sizeof(Magic));
    if (fs->magic == 0) {
        magic_smoke(FEWAND | FEALLOC);
    }
    Magic* magic = fs->magic;
    magic->ptr = allocate(sizeof(void*)*TSFSIMAGIC);
    if (fs->magic == 0) {
        magic_smoke(FEWAND | FEALLOC);
    }
    magic->csize = TSFSIMAGIC;
    magic->umax = 0;
    magic->cused = 0;
    magic->hmin = 0;
}

void tsfs_magic_release(FileSystem* fs) {
    deallocate(fs->magic->ptr, sizeof(void*)*fs->magic->csize);
    deallocate(fs->magic, sizeof(Magic));
}

size_t _tsmagic_get_slot(FileSystem* fs, void* object) {
    Magic* m = fs->magic;
    if (m->cused == m->csize) {
        _tsmagic_upsize(m, 1);
    }
    m->cused ++;
    m->ptr[m->hmin] = object;
    size_t p = m->hmin;
    if (p >= m->umax) {
        m->umax = p + 1;
    }
    // update lowest hole
    while (m->hmin < m->csize) {
        m->hmin ++;
        if (m->ptr[m->hmin] == 0) break;
    }
    return p;
}

void _tsmagic_rel_slot(FileSystem* fs, size_t slot) {
    Magic* m = fs->magic;
    deallocate(m->ptr[slot], _tsfs_magic_getsize(m->ptr[slot]));
    m->ptr[slot] = 0;
    m->cused --;
    if (slot == (m->umax-1)) {
        while (m->umax > 0) {
            m->umax --;
            if (m->ptr[m->umax-1] != 0) {
                break;
            }
        }
    }
    if ((m->cused << 1) >= m->csize && m->csize > TSFSIMAGIC) {
        _tsmagic_desize(m, 1);
    } else if (m->hmin > slot) {
        m->hmin = slot;
    }
}

SB* _tsmagic_block(FileSystem* fs) {
    SB* blk = allocate(sizeof(SB));
    blk->id = 1;
    blk->magicno = _tsmagic_get_slot(fs, blk);
    return blk;
}
void _tsmagic_deblock(FileSystem* fs, SB* blk) {
    _tsmagic_rel_slot(fs, blk->magicno);
    // deallocate(blk, sizeof(SB));
}

SN* _tsmagic_node(FileSystem* fs) {
    SN* node = allocate(sizeof(SN));
    node->id = 0;
    node->magicno = _tsmagic_get_slot(fs, node);
    return node;
}
void _tsmagic_denode(FileSystem* fs, SN* node) {
    _tsmagic_rel_slot(fs, node->magicno);
    // deallocate(node, sizeof(SN));
}

DB* _tsmagic_data(FileSystem* fs) {
    DB* data = allocate(sizeof(DB));
    data->id = 2;
    data->magicno = _tsmagic_get_slot(fs, data);
    return data;
}
void _tsmagic_dedata(FileSystem* fs, DB* data) {
    _tsmagic_rel_slot(fs, data->magicno);
    // deallocate(data, sizeof(DB));
}

DH* _tsmagic_head(FileSystem* fs) {
    DH* head = allocate(sizeof(DH));
    head->id = 3;
    head->magicno = _tsmagic_get_slot(fs, head);
    return head;
}
void _tsmagic_dehead(FileSystem* fs, DH* head) {
    _tsmagic_rel_slot(fs, head->magicno);
    // deallocate(head, sizeof(DH));
}

void _tsm_dbp_void(void* ptr) {
    u8 id = *((u8*)(ptr)+sizeof(size_t)+sizeof(u32)+sizeof(u16));
    if (id == 0) {
        _DBG_print_node((TSFSStructNode*)ptr);
    } else if (id == 1) {
        _DBG_print_block((TSFSStructBlock*)ptr);
    } else if (id == 2) {
        _DBG_print_data((TSFSDataBlock*)ptr);
    } else if (id == 3) {
        _DBG_print_head((TSFSDataHeader*)ptr);
    }
}

/*
check for interned object
*/
void* _tsm_cintern(Magic* m, u32 pos) {
    printf("CHECK FOR INTERN\n");
    size_t i = 0;
    while (i < m->umax) {
        void* ptr = m->ptr[i++];
        printf("INTERN PTR: %p\n", ptr);
        if (ptr == 0) continue;
        // _tsm_dbp_void(ptr);
        u32 tp = *((u32*)(((char*)ptr)+sizeof(size_t)));
        printf("INTERNCHECK: t:{%x} == %x?\n", tp, pos);
        if (tp == pos) {
            return ptr;
        }
    }
    return 0;
}

void _tsm_rc_inc(void* object) {
    (*((u16*)(((char*)object)+sizeof(size_t)+sizeof(u32)))) ++;
}
int _tsm_rc_dec(void* object) {
    return --*((u16*)(((char*)object)+sizeof(size_t)+sizeof(u32))) == 0;
}

/*
forces the given object to be released from the magic table, regardless of its current reference count
WARNING
THIS FUNCTION SHOULD BE USED ONLY TO ENSURE INVALIDATION OF DESTROYED RESOURCES
*/
void _tsmagic_force_release(FileSystem* fs, void* object) {
    _tsmagic_rel_slot(fs, _tsfs_magic_grabno(object));
}

void tsfs_unload(FileSystem* fs, void* object) {
    if (_tsm_rc_dec(object)) {
        _tsmagic_rel_slot(fs, _tsfs_magic_grabno(object));
    }
}

/*
does actual loading logic, other functions wrap for type & calling convenience
*/
void* _tsfs_magic_loader(FileSystem* fs, u32 rpos, void*(*_a)(FileSystem*), void(*_r)(FileSystem*,void*), u64 backup) {
    u64 pos = ((u64)rpos) * (u64)BLOCK_SIZE;
    pos = pos ? pos : tsfs_tell(fs);
    void* ptr = _tsm_cintern(fs->magic, tsfs_loc_to_block(pos)); // when pos == 0, pos = tsfs_tell(fs)
    if (ptr == 0) {
        longseek(fs, pos, SEEK_SET);
        ptr = _a(fs);
        _r(fs, ptr);
    } else {
        longseek(fs, pos+backup, SEEK_SET);
    }
    _tsm_rc_inc(ptr);
    return ptr;
}

typedef void*(*TSFS_MAGIC_LOADER)(FileSystem*);
typedef void(*TSFS_MAGIC_READER)(FileSystem*,void*);

SB* tsfs_load_block(FileSystem* fs, u32 pos) {
    return _tsfs_magic_loader(fs, pos, (TSFS_MAGIC_LOADER)_tsmagic_block, (TSFS_MAGIC_READER)read_structblock, TSFSSTRUCTBLOCK_DSIZE);
}
SN* tsfs_load_node(FileSystem* fs, u32 pos) {
    return _tsfs_magic_loader(fs, pos, (TSFS_MAGIC_LOADER)_tsmagic_node, (TSFS_MAGIC_READER)read_structnode, TSFSSTRUCTNODE_DSIZE);
}
DB* tsfs_load_data(FileSystem* fs, u32 pos) {
    return _tsfs_magic_loader(fs, pos, (TSFS_MAGIC_LOADER)_tsmagic_data, (TSFS_MAGIC_READER)read_datablock, TSFSDATABLOCK_DSIZE);
}
DH* tsfs_load_head(FileSystem* fs, u32 pos) {
    return _tsfs_magic_loader(fs, pos, (TSFS_MAGIC_LOADER)_tsmagic_head, (TSFS_MAGIC_READER)read_dataheader, TSFSDATAHEADER_DSIZE);
}

/*
reads a disk structure that may be interned into the magic table
*/
void* tsfs_minterned_read(FileSystem* fs, int kind) {
    switch (kind) {
        case TSFS_MAGIC_BLOC:
            return tsfs_load_block(fs, 0);
        case TSFS_MAGIC_NODE:
            return tsfs_load_node(fs, 0);
        case TSFS_MAGIC_DATA:
            return tsfs_load_data(fs, 0);
        case TSFS_MAGIC_HEAD:
            return tsfs_load_head(fs, 0);
        default:
            break;
    }
    magic_smoke(FEARG | FEINVL);
    return 0;
}

#undef SB
#undef SN
#undef DB
#undef DH

#endif
