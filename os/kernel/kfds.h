#ifndef __KFDS_H__
#define __KFDS_H__ 1
#include "./types.h"

#ifndef __MOCKTEST
extern void move(void*, void*, size_t);
extern void memset(void*, u8, size_t);
#else
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define move memcpy
#endif

/*maximum open filehandles*/
const u16 SYS_MAX_FDS = 0xff;
const double FDT_ACQ_NITER_THRESH = 0.2; // threshold under which kfd_acquire shouldn't iter because it is unlikely to be fast enough
const double FDT_ACQ_MITER_THRESH = 0.5; // threshold over which kfd_acquire MUST iter because holes are too prevalent in the table
const u16 FDT_ACQ_SMALL_LKFD = 200; // threshold under which kfd_acquire shouldn't iter because there aren't enough entries to care

// is the file descriptor valid? e.g. no other process has deleted the file it refers to
#define FDE_VALID  0b1
// is the file descriptor entry still in use?
#define FDE_OPEN   0b10
// does the FD have read access?
#define FDE_RACC   0b100
// does the FD have write access?
#define FDE_WACC   0b1000
// does the FD have execute access?
#define FDE_XACC   0b10000

typedef struct {
    u64    disk_loc;  // location of the file's data on disk
    pid_t  procid;    // process id that owns this handle
    u8     flags;     // status and permission flags
    u64    position;  // corresponds to what `ftell()` returns
} FDENTRY;

// maps KFDs to disk locations, owners, and flags
volatile FDENTRY FD_TABLE[SYS_MAX_FDS];
// tracks the maximum used KFD, allows optimizations when closing based on process ids or other criteria
volatile u16 LARGEST_KFD = 0;
// minimum number that assigned kfds must start at
const u16 KFD_RESERVED_LO = 0;
// maximum number that a kfd can be assigned as
// the -1 component should NEVER be modified, it is there to prevent out-of-bounds accesses due to off-by-one errors
const u16 KFD_RESERVED_HI = SYS_MAX_FDS - 1 - 0;
volatile u16 FDT_HOLE_COUNT = 0;

/*
available for external use
exposes the process that owns the given kfd
USE THIS TO ENSURE THAT A PROCESS DOES NOT CLOSE A KFD THAT IT DOES NOT OWN
*/
pid_t gkfd_owner(int kfd) {
    return FD_TABLE[kfd].procid;
}
/*
clears an entry from the FDE table
*/
void free_fd(int kfd, char update_lkfd) {
    memset((void*)&FD_TABLE[kfd], 0, sizeof(FDENTRY));
    // FD_TABLE[kfd] = {};
    // volatile FDENTRY* fde = &FD_TABLE[kfd]; // zeros the table entry
    // fde->disk_loc = 0;
    // fde->flags = 0;
    // fde->position = 0;
    FD_TABLE[kfd].procid = -1;
    // check that the kfd is not in either reserved block
    if (update_lkfd && kfd >= KFD_RESERVED_LO && kfd <= KFD_RESERVED_HI) {
        // update largest kfd
        if ((kfd-KFD_RESERVED_LO+1) == LARGEST_KFD) {
            LARGEST_KFD --;
            // go through to next assigned kfd, stop if the beginning of the table is reached
            while (LARGEST_KFD-KFD_RESERVED_LO > 0 && FD_TABLE[LARGEST_KFD].flags & FDE_OPEN) {
                LARGEST_KFD --;
                FDT_HOLE_COUNT --;
            }
        } else {
            FDT_HOLE_COUNT ++;
        }
    }
}
/*
assigns a kfd in the table and returns it
returns -1 on error
*/
int acquire_fd(FDENTRY* fde) {
    // full table :(
    if (FDT_HOLE_COUNT == 0 && LARGEST_KFD == KFD_RESERVED_HI) {
        return -1;
    }
    // no holes in the table, just put it at the end
    // the ratio of holes to the largest kfd is small enough that iterating probably isn't worth it
    // or the largest assigned kfd is small enough BUT the ratio isn't too big
    // above all, ensure that the kfd is NOT assigned within the hi reserved range
    double ratio = FDT_HOLE_COUNT > 0 ? ((double)FDT_HOLE_COUNT / (double)LARGEST_KFD) : 0.0;
    if (LARGEST_KFD < KFD_RESERVED_HI && ratio < FDT_ACQ_MITER_THRESH && (FDT_HOLE_COUNT == 0 || LARGEST_KFD <= FDT_ACQ_SMALL_LKFD || ratio <= FDT_ACQ_NITER_THRESH)) {
        LARGEST_KFD ++;
        move((FDENTRY*) &(FD_TABLE[LARGEST_KFD]), fde, sizeof(FDENTRY));
        // FD_TABLE[LARGEST_KFD].flags = fde.flags | FDE_OPEN | FDE_VALID;
        // FD_TABLE[LARGEST_KFD].procid = fde.procid;
        // FD_TABLE[LARGEST_KFD].disk_loc = fde.disk_loc;
        // FD_TABLE[LARGEST_KFD].position = fde.position;
        return LARGEST_KFD;
    }
    // iterate through the table up to largest kfd to find an open kfd
    for (u16 check = KFD_RESERVED_LO; check < LARGEST_KFD; check ++) {
        if (!(FD_TABLE[check].flags & FDE_OPEN)) {
            FDT_HOLE_COUNT --; // a hole was just filled, so the count must reflect that
            // populate table entry
            move((FDENTRY*) &(FD_TABLE[LARGEST_KFD]), fde, sizeof(FDENTRY));
            // FD_TABLE[check].flags = fde.flags | FDE_OPEN | FDE_VALID;
            // FD_TABLE[check].procid = pid;
            // FD_TABLE[check].disk_loc = disk_loc;
            // FD_TABLE[check].position = 0;
            return check;
        }
    }
    // the table is full :(
    return -1;
}
#endif
