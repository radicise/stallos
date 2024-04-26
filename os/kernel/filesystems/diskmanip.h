#ifndef __DISKMANIP_H__
#define __DISKMANIP_H__ 1
/*
this file defines a bunch of helper functions to manipulate the data on disk
*/

#include "./fsdefs.h"

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

/*
fills the specified range of blocks (start (inclusive) to start + count (exclusive))
*/
int dmanip_fill(FileSystem* fs, u32 start, u32 count, unsigned char value) {
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
    off_t inc = (off_t)(bsize * FS_DMANIP_MAX_RAM_BLKS);
    while (count > FS_DMANIP_MAX_RAM_BLKS) {
        write_buf(fs, bigbuf, bufsize);
        seek(fs, inc, SEEK_CUR);
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

int dmanip_null(FileSystem* fs, u32 start, u32 count) {
    return dmanip_fill(fs, start, count, 0);
}

/*
will shift the run of [count] blocks at [start] right by [delta] blocks
*/
int dmanip_shift_right(FileSystem* fs, u32 start, u32 count, u32 delta) {
    if (delta == 0) {
        return 0;
    }
    u32 bsize = BLOCK_SIZE;
    char f = dmanip_albuf(bsize);
    unsigned char* bigbuf = dman_buf;
    size_t bufsize = (bsize * FS_DMANIP_MAX_RAM_BLKS) / 8;
    while (count > FS_DMANIP_MAX_RAM_BLKS) {
        longseek(fs, bsize * (start + count - FS_DMANIP_MAX_RAM_BLKS), SEEK_SET);
        read_buf(fs, bigbuf, bufsize);
        seek(fs, ((s32)delta - FS_DMANIP_MAX_RAM_BLKS) * (s32)bsize, SEEK_CUR);
        write_buf(fs, bigbuf, bufsize);
        count -= FS_DMANIP_MAX_RAM_BLKS;
    }
    if (count) {
        longseek(fs, bsize * start, SEEK_SET);
        u32 amt = bsize * count;
        read_buf(fs, bigbuf, amt);
        seek(fs, ((s32)delta - (s32)count) * (s32)bsize, SEEK_CUR);
        write_buf(fs, bigbuf, amt);
    }
    if (f) {
        dmanip_debuf(bsize);
    }
    return 0;
}

/*
will shift the run of [count] blocks at [start] left by [delta] blocks
*/
int dmanip_shift_left(FileSystem* fs, u32 start, u32 count, u32 delta) {
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
        read_buf(fs, bigbuf, bufsize);
        seek(fs, seekoff, SEEK_CUR);
        write_buf(fs, bigbuf, bufsize);
        count -= FS_DMANIP_MAX_RAM_BLKS;
        start += FS_DMANIP_MAX_RAM_BLKS;
    }
    if (count) {
        longseek(fs, bsize * start, SEEK_SET);
        u32 amt = bsize * count;
        read_buf(fs, bigbuf, amt);
        seek(fs, -(s32)(delta * bsize + amt), SEEK_CUR);
        write_buf(fs, bigbuf, amt);
    }
    if (f) {
        dmanip_debuf(bsize);
    }
    return 0;
}

int dmanip_null_shift_right(FileSystem* fs, u32 start, u32 count, u32 delta) {
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

int dmanip_null_shift_left(FileSystem* fs, u32 start, u32 count, u32 delta) {
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
