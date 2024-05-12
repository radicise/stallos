#ifndef __FSMACROS_H__
#define __FSMACROS_H__ 1
/*
macro constants
*/

#define BLOCK_SIZE 1024

// // is this block valid? (allows for fast delete by not overwriting data)
// #define TSFS_SF_VALID 0b1
// all data blocks have this set
// #define TSFS_SF_LIVE 0b1
#define TSFS_SF_LIVE 1
// what kind of thing is this? (file, directory, link, data)
// #define TSFS_SF_KIND  0b1110
#define TSFS_SF_KIND  12
// is this block the head of a run of blocks storing data for the same owner?
// #define TSFS_SF_HEAD_BLOCK 0b10
#define TSFS_SF_HEAD_BLOCK 2
// is this block the tail of a run of blocks storing data for the same owner?
// #define TSFS_SF_TAIL_BLOCK 0b0100
#define TSFS_SF_TAIL_BLOCK 4
// does this block contain a checksum for its data?
// #define TSFS_SF_CHECKSUM 0b1
#define TSFS_SF_CHECKSUM 1
// is this block the last one belonging to a file?
// #define TSFS_SF_FINAL_BLOCK 0b1000
#define TSFS_SF_FINAL_BLOCK 8
// IF HEAD & FINAL & !TAIL THEN FIRST

#define TSFS_KIND_DATA 0
#define TSFS_KIND_DIR  1
#define TSFS_KIND_FILE 2
#define TSFS_KIND_LINK 3
#define TSFS_KIND_HARD 4

#define TSFS_CF_EXTT 1
#define TSFS_CF_DIRE 2
#define TSFS_CF_HARD 4
#define TSFS_CF_LINK 8

#define BSEEK_SET 0
#define BSEEK_CUR 1

#define FEWAND 0x1
#define FEALLOC 0x2
#define FEFULL 0x4
#define FEBIG 0x8
#define FESMALL 0x10
// coolaid
#define FEDRIVE 0xc001a1d0
#define FEINVL 0x20
#define FEARG 0x40
#define FETEST 0x80
#define FEIMPL 0x100
#define FEDATA 0x200
#define FEOP   0x400

#define TSFS_ANSI_NUN "\x1b[0m"
#define TSFS_ANSI_RED "\x1b[38;2;235;0;0m"
#define TSFS_ANSI_GRN "\x1b[38;2;0;200;0m"
#define TSFS_ANSI_YEL "\x1b[38;2;200;175;0m"

#endif
