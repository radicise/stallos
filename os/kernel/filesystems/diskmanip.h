#ifndef __DISKMANIP_H__
#define __DISKMANIP_H__ 1
/*
this file defines a bunch of helper functions to manipulate the data on disk
*/

#include "./fsdefs.h"

char DMAN_TRACING = 0;

int _dmanip_fill(FileSystem*, u32, u32, unsigned char, unsigned long, const char*, const char*);
int _dmanip_null(FileSystem*, u32, u32, unsigned long, const char*, const char*);
int _dmanip_shift_right(FileSystem*, u32, u32, u32, unsigned long, const char*, const char*);
int _dmanip_null_shift_right(FileSystem*, u32, u32, u32, unsigned long, const char*, const char*);
int _dmanip_shift_left(FileSystem*, u32, u32, u32, unsigned long, const char*, const char*);
int _dmanip_null_shift_left(FileSystem*, u32, u32, u32, unsigned long, const char*, const char*);
int _dmanip_swap(FileSystem*, u32, u32, u32, unsigned long, const char*, const char*);
#define dmanip_fill(fs, start, count, value) _dmanip_fill(fs, start, count, value, __LINE__, __FILE__, __func__)
#define dmanip_null(fs, start, count) _dmanip_null(fs, start, count, __LINE__, __FILE__, __func__)
#define dmanip_shift_right(fs, start, count, delta) _dmanip_shift_right(fs, start, count, delta, __LINE__, __FILE__, __func__)
#define dmanip_shift_left(fs, start, count, delta) _dmanip_shift_left(fs, start, count, delta, __LINE__, __FILE__, __func__)
#define dmanip_null_shift_right(fs, start, count, delta) _dmanip_null_shift_right(fs, start, count, delta, __LINE__, __FILE__, __func__)
#define dmanip_null_shift_left(fs, start, count, delta) _dmanip_null_shift_left(fs, start, count, delta, __LINE__, __FILE__, __func__)
#define dmanip_swap(fs, a, b, count) _dmanip_swap(fs, a, b, count, __LINE__, __FILE__, __func__)

/*
maximum number of blocks that may be loaded into RAM at any point by any of the functions defined here
*/
#define FS_DMANIP_MAX_RAM_BLKS 5

u64 dmanip_rep1to8(unsigned char one) {
    u64 v = (u64)one;
    v |= (v << 8);
    v |= (v << 16);
    v |= (v << 32);
    return v;
}

static void* dman_buf = 0;

static char dmanip_albuf(u32 bsize) {
    if (dman_buf != 0) {
        return 0;
    }
    dman_buf = allocate(bsize * FS_DMANIP_MAX_RAM_BLKS);
    return 1;
}
static void dmanip_debuf(u32 bsize) {
    if (dman_buf == 0) {
        return;
    }
    deallocate(dman_buf, bsize * FS_DMANIP_MAX_RAM_BLKS);
    dman_buf = 0;
}

int _dmanip_swap(FileSystem* fs, u32 a, u32 b, u32 count, unsigned long line, const char* func, const char* file) {
    if (DMAN_TRACING) {
        __DBG_here(line, func, file);
        printf("%sDMAN TRACE <SWAP> {A: %lu, B: %lu, COUNT: %lu}%s\n", TSFS_ANSI_GRN, a, b, count, TSFS_ANSI_NUN);
    }
    u32 prog = 0;
    if (a > b) {
        if (b + count >= a) {
            goto overlap;
        }
        goto clear;
    }
    if (a < b) {
        if (a + count >= b) {
            goto overlap;
        }
        goto clear;
    }
    overlap:
    printf("%sDMAN ERROR: overlapping swap regions%s\n", TSFS_ANSI_RED, TSFS_ANSI_NUN);
    return -1;
    clear:
    b = b;
    u32 bsize = BLOCK_SIZE;
    u32 conc = (FS_DMANIP_MAX_RAM_BLKS == 1) ? 2 : (FS_DMANIP_MAX_RAM_BLKS/2);
    if (FS_DMANIP_MAX_RAM_BLKS == 1) {
        dmanip_albuf(bsize * 2);
    } else {
        dmanip_albuf(bsize);
    }
    unsigned char* buf = (unsigned char*)dman_buf;
    while (prog < count) {
        if (prog + conc >= count) {
            conc = count - prog;
        }
        size_t concsize = (size_t)(bsize * conc);
        longseek(fs, (loff_t)(bsize * (a+prog)), SEEK_SET);
        read_buf(fs, buf, concsize);
        longseek(fs, (loff_t)(bsize * (b+prog)), SEEK_SET);
        read_buf_stable(fs, buf+concsize, concsize);
        write_buf(fs, buf, concsize);
        longseek(fs, (loff_t)(bsize * (a+prog)), SEEK_SET);
        write_buf(fs, buf+concsize, concsize);
        prog += conc;
    }
    if (FS_DMANIP_MAX_RAM_BLKS == 1) {
        dmanip_debuf(bsize * 2);
    } else {
        dmanip_debuf(bsize);
    }
    return 0;
}

/*
fills the specified range of blocks (start (inclusive) to start + count (exclusive))
*/
int _dmanip_fill(FileSystem* fs, u32 start, u32 count, unsigned char value, unsigned long line, const char* func, const char* file) {
    if (DMAN_TRACING) {
        __DBG_here(line, func, file);
        printf("%sDMAN TRACE <FILL> {START: %lu, COUNT: %lu, VALUE: %u}%s\n", TSFS_ANSI_GRN, start, count, value, TSFS_ANSI_NUN);
    }
    u32 bsize = BLOCK_SIZE;
    char f = dmanip_albuf(bsize);
    unsigned char* bigbuf = dman_buf;
    u32 bufsize = FS_DMANIP_MAX_RAM_BLKS * bsize;
    int lend = bufsize / 8;
    u64 v = dmanip_rep1to8(value);
    for (int i = 0; i < lend; i += 1) {
        ((u64*)bigbuf)[i] = v;
    }
    longseek(fs, (loff_t)(bsize * start), SEEK_SET);
    while (count > FS_DMANIP_MAX_RAM_BLKS) {
        write_buf(fs, bigbuf, bufsize);
        count -= FS_DMANIP_MAX_RAM_BLKS;
    }
    if (count) {
        write_buf(fs, bigbuf, bsize * count);
    }
    if (f) {
        dmanip_debuf(bsize);
    }
    return 0;
}

int _dmanip_null(FileSystem* fs, u32 start, u32 count, unsigned long line, const char* func, const char* file) {
    if (DMAN_TRACING) {
        __DBG_here(line, func, file);
        printf("%sDMAN TRACE <NULL> {START: %lu, COUNT: %lu}%s\n", TSFS_ANSI_GRN, start, count, TSFS_ANSI_NUN);
    }
    return dmanip_fill(fs, start, count, 0);
}

/*
will shift the run of [count] blocks at [start] right by [delta] blocks
*/
int _dmanip_shift_right(FileSystem* fs, u32 start, u32 count, u32 delta, unsigned long line, const char* func, const char* file) {
    if (DMAN_TRACING) {
        __DBG_here(line, func, file);
        printf("%sDMAN TRACE <SHR> {START: %lu, COUNT: %lu, DELTA: %lu}%s\n", TSFS_ANSI_GRN, start, count, delta, TSFS_ANSI_NUN);
    }
    if (delta == 0) {
        return 0;
    }
    u32 bsize = BLOCK_SIZE;
    char f = dmanip_albuf(bsize);
    unsigned char* bigbuf = dman_buf;
    size_t bufsize = (bsize * FS_DMANIP_MAX_RAM_BLKS) / 8;
    while (count > FS_DMANIP_MAX_RAM_BLKS) {
        longseek(fs, bsize * (start + count - FS_DMANIP_MAX_RAM_BLKS), SEEK_SET);
        read_buf_stable(fs, bigbuf, bufsize);
        seek(fs, ((s32)delta - FS_DMANIP_MAX_RAM_BLKS) * (s32)bsize, SEEK_CUR);
        write_buf_stable(fs, bigbuf, bufsize);
        count -= FS_DMANIP_MAX_RAM_BLKS;
    }
    if (count) {
        longseek(fs, bsize * start, SEEK_SET);
        u32 amt = bsize * count;
        read_buf_stable(fs, bigbuf, amt);
        seek(fs, ((s32)delta - (s32)count) * (s32)bsize, SEEK_CUR);
        write_buf_stable(fs, bigbuf, amt);
    }
    if (f) {
        dmanip_debuf(bsize);
    }
    return 0;
}

/*
will shift the run of [count] blocks at [start] left by [delta] blocks
*/
int _dmanip_shift_left(FileSystem* fs, u32 start, u32 count, u32 delta, unsigned long line, const char* func, const char* file) {
    if (DMAN_TRACING) {
        __DBG_here(line, func, file);
        printf("%sDMAN TRACE <SHL> {START: %lu, COUNT: %lu, DELTA: %lu}%s\n", TSFS_ANSI_GRN, start, count, delta, TSFS_ANSI_NUN);
    }
    if (delta == 0) {
        return 0;
    }
    u32 bsize = BLOCK_SIZE;
    char f = dmanip_albuf(bsize);
    size_t bufsize = (bsize * FS_DMANIP_MAX_RAM_BLKS) / 8;
    unsigned char* bigbuf = dman_buf;
    s32 seekoff = -(s32)(delta * bsize + bufsize);
    while (count > FS_DMANIP_MAX_RAM_BLKS) {
        longseek(fs, bsize * start, SEEK_SET);
        read_buf_stable(fs, bigbuf, bufsize);
        seek(fs, seekoff, SEEK_CUR);
        write_buf_stable(fs, bigbuf, bufsize);
        count -= FS_DMANIP_MAX_RAM_BLKS;
        start += FS_DMANIP_MAX_RAM_BLKS;
    }
    if (count) {
        longseek(fs, bsize * start, SEEK_SET);
        u32 amt = bsize * count;
        read_buf_stable(fs, bigbuf, amt);
        seek(fs, -(s32)(delta * bsize + amt), SEEK_CUR);
        write_buf_stable(fs, bigbuf, amt);
    }
    if (f) {
        dmanip_debuf(bsize);
    }
    return 0;
}

int _dmanip_null_shift_right(FileSystem* fs, u32 start, u32 count, u32 delta, unsigned long line, const char* func, const char* file) {
    if (DMAN_TRACING) {
        __DBG_here(line, func, file);
        printf("%sDMAN TRACE <NULSHR> {START: %lu, COUNT: %lu, DELTA: %lu}%s\n", TSFS_ANSI_GRN, start, count, delta, TSFS_ANSI_NUN);
    }
    u32 bsize = BLOCK_SIZE;
    char f = dmanip_albuf(bsize);
    dmanip_shift_right(fs, start, count, delta);
    if (delta >= count) {
        dmanip_null(fs, start, count);
    } else {
        dmanip_null(fs, start, count - delta);
    }
    if (f) {
        dmanip_debuf(bsize);
    }
    return 0;
}

int _dmanip_null_shift_left(FileSystem* fs, u32 start, u32 count, u32 delta, unsigned long line, const char* func, const char* file) {
    if (DMAN_TRACING) {
        __DBG_here(line, func, file);
        printf("%sDMAN TRACE <NULSHL> {START: %lu, COUNT: %lu, DELTA: %lu}%s\n", TSFS_ANSI_GRN, start, count, delta, TSFS_ANSI_NUN);
    }
    u32 bsize = BLOCK_SIZE;
    char f = dmanip_albuf(bsize);
    dmanip_shift_left(fs, start, count, delta);
    if (delta >= count) {
        dmanip_null(fs, start, count);
    } else {
        dmanip_null(fs, start + delta, count - delta);
    }
    if (f) {
        dmanip_debuf(bsize);
    }
    return 0;
}

#endif
