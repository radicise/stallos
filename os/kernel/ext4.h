#ifndef __EXT4_H__
#define __EXT4_H__ 1
#include "FileDriver.h"
#include "types.h"
// many defs, hard to read stuff if they are in this file
#include "defhelp/e4sbdef.h"
// actual code, yippee!
struct FileSystem {
    struct FileDriver* fdrive;
    int kfd;
};
const int TOTAL_SB_DATA_SIZE = 0x400;
struct Ext4SuperBlock {
    unsigned char data[TOTAL_SB_DATA_SIZE];
};

// access the mess that is the super block's data
void* sbf_access(struct Ext4SuperBlock* sb, int field) {
    return (void*) (sb->data+field);
};

// un-pollute the macros
#include "defhelp/e4sbundef.h"
#endif