#ifndef __TSFS_ERRDATA_H__
#define __TSFS_ERRDATA_H__ 1
/*
this file pertains to tsfs error logging via data on disk
*/
#include "fsdefs.h"

#define _TSFS_EDC_EID 0 // 16-bit sequentially assigned id, may never be reused as it corresponds to source locations
                        // exception to this rule: co-located related blocks may share an EID as long as they remain so
#define _TSFS_EDC_8   1
#define _TSFS_EDC_16  2
#define _TSFS_EDC_32  3
#define _TSFS_EDC_64  4
#define _TSFS_EDC_KEY 5
#define _TSFS_EDC_LOC 6
#define _TSFS_EDC_STR 7

struct _TSFS_EDT_STR {
    u16 length;
    char* data;
    // char ntrunc;
};

struct _TSFS_EDATUM {
    unsigned char type  : 3;
    // only absolutely necessary info should be provided
    // in the case that more than 32 instances of the given type appear needed, the necessity of each item must be re-evaluated
    unsigned char count : 5; // count begins at one (eg. if datum.count == 0 then actual_count = 1)
    u8* data;
};

// number of blocks to reserve for error data
#define TSFS_EDATA_SIZE 4
#define _EDATA_START 7

static void _write_eid(u8* buffer, size_t maxlen, size_t* bpos, struct _TSFS_EDATUM* eid) {
    buffer[*bpos] = _TSFS_EDC_EID;
    (*bpos) ++;
    awrite_buf(buffer+(*bpos), eid->data, 2);
    (*bpos) += 2;
}
static void _write_8(u8* buffer, size_t maxlen, size_t* bpos, struct _TSFS_EDATUM* n) {
    buffer[*bpos] = _TSFS_EDC_8;
    (*bpos) ++;
    buffer[*bpos] = n->count;
    (*bpos) ++;
    for (int i = 0; i < (n->count+1); i ++) {
        buffer[*bpos] = *((n->data)+i);
        (*bpos) ++;
    }
}
static void _write_16(u8* buffer, size_t maxlen, size_t* bpos, struct _TSFS_EDATUM* n) {
    buffer[*bpos] = _TSFS_EDC_16;
    (*bpos) ++;
    buffer[*bpos] = n->count;
    (*bpos) ++;
    for (int i = 0; i < (n->count+1); i ++) {
        awrite_buf(buffer+(*bpos), (n->data)+(i<<1), 2);
        (*bpos) += 2;
    }
}
static void _write_32(u8* buffer, size_t maxlen, size_t* bpos, struct _TSFS_EDATUM* n) {
    buffer[*bpos] = _TSFS_EDC_32;
    (*bpos) ++;
    buffer[*bpos] = n->count;
    (*bpos) ++;
    for (int i = 0; i < (n->count+1); i ++) {
        awrite_buf(buffer+(*bpos), (n->data)+(i<<2), 4);
        (*bpos) += 4;
    }
}
static void _write_64(u8* buffer, size_t maxlen, size_t* bpos, struct _TSFS_EDATUM* n) {
    buffer[*bpos] = _TSFS_EDC_64;
    (*bpos) ++;
    buffer[*bpos] = n->count;
    (*bpos) ++;
    for (int i = 0; i < (n->count+1); i ++) {
        awrite_buf(buffer+(*bpos), (n->data)+(i<<3), 8);
        (*bpos) += 8;
    }
}
static void _write_key(u8* buffer, size_t maxlen, size_t* bpos, struct _TSFS_EDATUM* n) {
    buffer[*bpos] = _TSFS_EDC_KEY;
    (*bpos) ++;
    buffer[*bpos] = n->count;
    (*bpos) ++;
    for (int i = 0; i < (n->count+1); i ++) {
        awrite_buf(buffer+(*bpos), (n->data)+(i<<2), 4);
        (*bpos) += 4;
    }
}
static void _write_loc(u8* buffer, size_t maxlen, size_t* bpos, struct _TSFS_EDATUM* n) {
    buffer[*bpos] = _TSFS_EDC_LOC;
    (*bpos) ++;
    buffer[*bpos] = n->count;
    (*bpos) ++;
    for (int i = 0; i < (n->count+1); i ++) {
        awrite_buf(buffer+(*bpos), (n->data)+(i<<2), 4);
        (*bpos) += 4;
    }
}
static void _write_str(u8* buffer, size_t maxlen, size_t* bpos, struct _TSFS_EDATUM* n) {
    buffer[*bpos] = _TSFS_EDC_STR;
    (*bpos) ++;
    buffer[*bpos] = n->count;
    (*bpos) ++;
    struct _TSFS_EDT_STR* strs = (struct _TSFS_EDT_STR*)(n->data);
    size_t acmax = maxlen - ((n->count+1)<<1);
    // size_t avail = (maxlen - (*bpos)) - ((n->count+1)<<1);
    // size_t lpad = 0;
    for (int i = 0; i < (n->count+1); i ++) {
        struct _TSFS_EDT_STR* cstr = (strs+i);
        if (cstr->length > acmax-(*bpos)) {
            buffer[*bpos] = 0;
            buffer[(*bpos)+1] = 0;
            (*bpos) += 2;
        } else {
            awriteu16be(buffer+(*bpos), cstr->length);
            (*bpos) += 2;
            awrite_buf(buffer+(*bpos), cstr->data, cstr->length);
            (*bpos) += cstr->length;
        }
        // if (cstr->length > avail) {
        //     cstr->ntrunc = 1;
        // } else {
        //     lpad += cstr->length;
        // }
    }
}

static struct _TSFS_EDATUM SELFID = {.type=_TSFS_EDC_EID,.count=0,.data={0,0}};

/*
the first datum MUST be of type EID to properly handle secondary errors
if EID is not the first datum, the list will be scanned for EID if any, and a secondary error will be logged instead
if multiple datums of the same type appear, a secondary error will be logged instead

buffer is the size of maxlen
count is the number of edatum structs given in edata
*/
void write_error_data(u8* buffer, size_t maxlen, size_t count, struct _TSFS_EDATUM* edata) {
    char usedtypes = 0;
    size_t bpos = 0;
    struct _TSFS_EDATUM* hope_eid = edata;
    if (hope_eid->type != _TSFS_EDC_EID) {
        goto handle_bad_eid;
    }
    for (int i = 0; i < count; i ++) {
        struct _TSFS_EDATUM* datum = edata+i;
        switch (datum->type) {
            case _TSFS_EDC_EID:{_write_eid(buffer, maxlen, &bpos, datum);break;}
            case _TSFS_EDC_8:{_write_8(buffer, maxlen, &bpos, datum);break;}
            case _TSFS_EDC_16:{_write_16(buffer, maxlen, &bpos, datum);break;}
            case _TSFS_EDC_32:{_write_32(buffer, maxlen, &bpos, datum);break;}
            case _TSFS_EDC_64:{_write_64(buffer, maxlen, &bpos, datum);break;}
            case _TSFS_EDC_KEY:{_write_key(buffer, maxlen, &bpos, datum);break;}
            case _TSFS_EDC_LOC:{_write_loc(buffer, maxlen, &bpos, datum);break;}
            case _TSFS_EDC_STR:{_write_str(buffer, maxlen, &bpos, datum);break;}
        }
    }
    handle_bad_eid:
    _write_eid(buffer, maxlen, &bpos, &SELFID);
    for (int i = 1; i < count; i ++) {
        if ((edata+i)->type == _TSFS_EDC_EID) {
            _write_eid(buffer, maxlen, &bpos, edata+i);
            goto found_bad_eid;
        }
    }
    _write_eid(buffer, maxlen, &bpos, &SELFID);
    found_bad_eid:
    {
        struct _TSFS_EDT_STR _tmpstr = {.data="EID IMPROPER POS", .length=16};
        struct _TSFS_EDATUM _tmp = {.type=_TSFS_EDC_STR, .count=0, .data=&_tmpstr};
        _write_str(buffer, maxlen, &bpos, &_tmp);
    }
}

/*
EID table:
0x0000 - "*"(errdata.h) - secondary error
0x0001 - "_tsfs_full_respath_act"(tsfsmanage.h) - path normalization failure
*/

#endif
