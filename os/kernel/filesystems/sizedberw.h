#ifndef __SIZEDBERW_H__
#define __SIZEDBERW_H__ 1

#include "./fsdefs.h"



// #if TSFSROOTBLOCK_DSIZE - 53
// #error "ROOTBLOCK FUNCTIONS ARE OUTDATED"
// #endif

// void write_rootblock(FileSystem* fs, TSFSRootBlock* rb) {
//     unsigned char x[TSFSROOTBLOCK_DSIZE - 8];
//     unsigned char* xp = (unsigned char*)x;
//     awriteu16be(xp, rb->breakver);
//     awriteu64be(xp+2, rb->partition_size);
//     x[10] = rb->system_size;
//     awriteu64be(xp+11, rb->creation_time);
//     awrite_buf(xp+19, rb->version, 16);
//     awriteu16be(xp+35, rb->block_size);
//     awriteu64be(xp+37, rb->top_dir);
//     rb->checksum = hashsized(xp, TSFSROOTBLOCK_DSIZE - 8);
//     write_buf(fs, xp, TSFSROOTBLOCK_DSIZE - 8);
//     write_u64be(fs, rb->checksum);
// }

// void read_rootblock(FileSystem* fs, TSFSRootBlock* rb) {
//     rb->breakver = read_u16be(fs);
//     rb->partition_size = read_u64be(fs);
//     rb->system_size = read_u8(fs);
//     rb->creation_time = read_u64be(fs);
//     read_buf(fs, rb->version, 16);
//     rb->block_size = read_u16be(fs);
//     rb->top_dir = read_u64be(fs);
//     rb->checksum = read_u64be(fs);
// }

// #if TSFSDATABLOCK_DSIZE - 48
// #error "DATABLOCK FUNCTIONS ARE OUTDATED"
// #endif

// void write_datablock(FileSystem* fs, TSFSDataBlock* db) {
//     write_u8(fs, db->storage_flags);
//     write_u64be(fs, db->disk_loc);
//     write_u16be(fs, db->owner_id);
//     write_u64be(fs, db->next_block);
//     write_u64be(fs, db->prev_block);
//     write_u32be(fs, db->data_length);
//     write_u8(fs, db->blocks_to_terminus);
//     write_u64be(fs, db->metachecksum);
//     write_u64be(fs, db->datachecksum);
// }
// void read_datablock(FileSystem* fs, TSFSDataBlock* db) {
//     db->storage_flags = read_u8(fs);
//     db->disk_loc = read_u64be(fs);
//     db->owner_id = read_u16be(fs);
//     db->next_block = read_u64be(fs);
//     db->prev_block = read_u64be(fs);
//     db->data_length = read_u32be(fs);
//     db->blocks_to_terminus = read_u8(fs);
//     db->metachecksum = read_u64be(fs);
//     db->datachecksum = read_u64be(fs);
// }

// #if TSFSSTRUCTNODE_DSIZE - 85
// #error "STRUCTNODE FUNCTIONS ARE OUTDATED"
// #endif

// void write_structnode(FileSystem* fs, TSFSStructNode* sn) {
//     write_u8(fs, sn->storage_flags);
//     write_u16be(fs, sn->nodeid);
//     write_u64be(fs, sn->data_loc);
//     write_u16be(fs, sn->parent_id);
//     write_buf(fs, sn->name, NAME_LENGTH);
//     write_u64be(fs, sn->checksum);
// }

// void read_structnode(FileSystem* fs, TSFSStructNode* sn) {
//     sn->storage_flags = read_u8(fs);
//     sn->nodeid = read_u16be(fs);
//     sn->data_loc = read_u64be(fs);
//     sn->parent_id = read_u16be(fs);
//     read_buf(fs, sn->name, NAME_LENGTH);
//     sn->checksum = read_u64be(fs);
// }

#endif