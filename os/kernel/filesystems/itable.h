#ifndef __ITABLE_H__
#define __ITABLE_H__ 1
/*
functions for dealing with the inode tables
*/

#include "./fsdefs.h"

/*
ITABLE specs
first four bytes - flags and index
next four bytes - parent location
all other bytes - children
<8>bytes
<a,1><b,1><c,1><d,1><3,4>
<a> - constant - 0xff, identifies this block as an ITABLE
<b> - bool - singly or doubly indirected block
<c> - uint8 - number of entries in this table, when zero, the table will be deallocated, excepting if the table is singly indirected to the first table and has index zero, or is doubly indirected through that table and has index zero
<d> - uint8 - index of this block in parent
<e> - uint32 - disk addr of parent
*/

u64 aquire_itable_slot(FileSystem* fs) {
    block_seek(fs, 1, BSEEK_SET);
    for (int i = 0; i < 256; i ++) {
        //
    }
    return 0;
}

#endif
