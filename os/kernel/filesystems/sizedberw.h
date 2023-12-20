#ifndef __SIZEDBERW_H__
#define __SIZEDBERW_H__ 1

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

void write_u64be(FileSystem* fs, u64 n) {
    unsigned char ptr[8];
    awriteu64be(ptr, n);
    (fs->fdrive->write)(fs->kfd,ptr,8);
}
void write_u32be(FileSystem* fs, u32 n) {
    unsigned char ptr[4];
    awriteu32be(ptr, n);
    (fs->fdrive->write)(fs->kfd,ptr,4);
}
void write_u16be(FileSystem* fs, u16 n) {
    unsigned char ptr[2];
    awriteu16be(ptr, n);
    (fs->fdrive->write)(fs->kfd,ptr,2);
}
void write_u8(FileSystem* fs, u8 n) {
    (fs->fdrive->write)(fs->kfd,&n,1);
}
void write_buf(FileSystem* fs, const void* buf, size_t size) {
    (fs->fdrive->write)(fs->kfd,buf,size);
}

u64 read_u64be(FileSystem* fs) {
    unsigned char ptr[8];
    (fs->fdrive->read)(fs->kfd,ptr,8);
    return areadu64be(ptr);
}
u32 read_u32be(FileSystem* fs) {
    unsigned char ptr[4];
    (fs->fdrive->read)(fs->kfd,ptr,4);
    return areadu32be(ptr);
}
u16 read_u16be(FileSystem* fs) {
    unsigned char ptr[2];
    (fs->fdrive->read)(fs->kfd,ptr,2);
    return areadu16be(ptr);
}
u8 read_u8(FileSystem* fs) {
    u8 n = 0;
    (fs->fdrive->read)(fs->kfd,&n,1);
    return n;
}

void read_buf(FileSystem* fs, void* buf, size_t size) {
    (fs->fdrive->read)(fs->kfd,buf,size);
}

#if TSFSROOTBLOCK_DSIZE - 53
#error "ROOTBLOCK FUNCTIONS ARE OUTDATED"
#endif

void write_rootblock(FileSystem* fs, TSFSRootBlock* rb) {
    write_u16be(fs, rb->breakver);
    write_u64be(fs, rb->partition_size);
    write_u8(fs, rb->system_size);
    write_u64be(fs, rb->creation_time);
    write_buf(fs, rb->version, 16);
    write_u16be(fs, rb->block_size);
    write_u64be(fs, rb->top_dir);
    write_u64be(fs, rb->checksum);
}

void read_rootblock(FileSystem* fs, TSFSRootBlock* rb) {
    rb->breakver = read_u16be(fs);
    rb->partition_size = read_u64be(fs);
    rb->system_size = read_u8(fs);
    rb->creation_time = read_u64be(fs);
    read_buf(fs, rb->version, 16);
    rb->block_size = read_u16be(fs);
    rb->top_dir = read_u64be(fs);
    rb->checksum = read_u64be(fs);
}

#endif