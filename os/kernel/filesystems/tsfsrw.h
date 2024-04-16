#ifndef __TSFSRW_H__
#define __TSFSRW_H__ 1
#include "./fsdefs.h"
void awriteu64be(unsigned char* buf, u64 n) {
    buf[0] = (unsigned char)((n>>56)&0xff);
    buf[1] = (unsigned char)((n>>48)&0xff);
    buf[2] = (unsigned char)((n>>40)&0xff);
    buf[3] = (unsigned char)((n>>32)&0xff);
    buf[4] = (unsigned char)((n>>24)&0xff);
    buf[5] = (unsigned char)((n>>16)&0xff);
    buf[6] = (unsigned char)((n>>8)&0xff);
    buf[7] = (unsigned char)(n&0xff);
}
void awriteu48be(unsigned char* buf, u64 n) {
    buf[0] = (unsigned char)((n>>40)&0xff);
    buf[1] = (unsigned char)((n>>32)&0xff);
    buf[2] = (unsigned char)((n>>24)&0xff);
    buf[3] = (unsigned char)((n>>16)&0xff);
    buf[4] = (unsigned char)((n>>8)&0xff);
    buf[5] = (unsigned char)(n&0xff);
}
void awriteu32be(unsigned char* buf, u32 n) {
    buf[0] = (unsigned char)((n>>24)&0xff);
    buf[1] = (unsigned char)((n>>16)&0xff);
    buf[2] = (unsigned char)((n>>8)&0xff);
    buf[3] = (unsigned char)(n&0xff);
}
void awriteu16be(unsigned char* buf, u16 n) {
    buf[0] = (unsigned char)((n>>8)&0xff);
    buf[1] = (unsigned char)(n&0xff);
}

u64 areadu64be(unsigned char* buf) {
    return
    (((u64)(buf[0])) << 56) |
    (((u64)(buf[1])) << 48) |
    (((u64)(buf[2])) << 40) |
    (((u64)(buf[3])) << 32) |
    (((u64)(buf[4])) << 24) |
    (((u64)(buf[5])) << 16) |
    (((u64)(buf[6])) << 8) |
    ((u64)(buf[7]));
}
u64 areadu48be(unsigned char* buf) {
    return
    (((u64)(buf[0])) << 40) |
    (((u64)(buf[1])) << 32) |
    (((u64)(buf[2])) << 24) |
    (((u64)(buf[3])) << 16) |
    (((u64)(buf[4])) << 8) |
    ((u64)(buf[5]));
}
u32 areadu32be(unsigned char* buf) {
    return
    (((u32)buf[0]) << 24) |
    (((u32)buf[1]) << 16) |
    (((u32)buf[2]) << 8) |
    ((u32)buf[3]);
}
u16 areadu16be(unsigned char* buf) {
    return ((u16)buf[0])<<8 | (u16)buf[1];
}

void awrite_buf(void* dstp, void const* srcp, size_t size) {
    u8* dst = (u8*)dstp;
    u8* src = (u8*)srcp;
    for (int i = 0; i < size; i ++) {
        *(dst+i) = *(src+i);
    }
}

void write_u64be(FileSystem* fs, u64 n) {
    unsigned char ptr[8];
    awriteu64be(ptr, n);
    if ((fs->fdrive->write)(fs->kfd,ptr,8)) fs->err = 1;
}
void write_u48be(FileSystem* fs, u64 n) {
    unsigned char ptr[6];
    awriteu48be(ptr, n);
    if ((fs->fdrive->write)(fs->kfd,ptr,6)) fs->err = 1;
}
void write_u32be(FileSystem* fs, u32 n) {
    unsigned char ptr[4];
    awriteu32be(ptr, n);
    if ((fs->fdrive->write)(fs->kfd,ptr,4)) fs->err = 1;
}
void write_u16be(FileSystem* fs, u16 n) {
    unsigned char ptr[2];
    awriteu16be(ptr, n);
    if ((fs->fdrive->write)(fs->kfd,ptr,2)) fs->err = 1;
}
void write_u8(FileSystem* fs, u8 n) {
    if ((fs->fdrive->write)(fs->kfd,&n,1)) fs->err = 1;
}
void write_buf(FileSystem* fs, const void* buf, size_t size) {
    if ((fs->fdrive->write)(fs->kfd,buf,size)) fs->err = 1;
}

u64 read_u64be(FileSystem* fs) {
    unsigned char ptr[8];
    if ((fs->fdrive->read)(fs->kfd,ptr,8)) fs->err = 1;
    return areadu64be(ptr);
}
u64 read_u48be(FileSystem* fs) {
    unsigned char ptr[6];
    if ((fs->fdrive->read)(fs->kfd,ptr,6)) fs->err = 1;
    return areadu48be(ptr);
}
u32 read_u32be(FileSystem* fs) {
    unsigned char ptr[4];
    if ((fs->fdrive->read)(fs->kfd,ptr,4)) fs->err = 1;
    return areadu32be(ptr);
}
u16 read_u16be(FileSystem* fs) {
    unsigned char ptr[2];
    if ((fs->fdrive->read)(fs->kfd,ptr,2)) fs->err = 1;
    return areadu16be(ptr);
}
u8 read_u8(FileSystem* fs) {
    u8 n = 0;
    if ((fs->fdrive->read)(fs->kfd,&n,1)) fs->err = 1;
    return n;
}

void read_buf(FileSystem* fs, void* buf, size_t size) {
    if ((fs->fdrive->read)(fs->kfd,buf,size)) fs->err = 1;
}
#endif
