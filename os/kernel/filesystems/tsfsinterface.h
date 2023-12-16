#ifndef __TSFSINTERFACE_H__
#define __TSFSINTERFACE_H__ 1

#include "fsdefs.h"
#include "../errno.h"

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
    volatile FDENTRY* fde = &FD_TABLE[kfd]; // zeros the table entry
    fde->disk_loc = 0;
    fde->procid = -1;
    fde->flags = 0;
    fde->position = 0;
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
int acquire_fd(FileSystem* fs, u64 disk_loc, pid_t pid, u8 flags) {
    // full table :(
    if (FDT_HOLE_COUNT == 0 && LARGEST_KFD == KFD_RESERVED_HI) {
        return -1;
    }
    // no holes in the table, just put it at the end
    // the ratio of holes to the largest kfd is small enough that iterating probably isn't worth it
    // or the largest assigned kfd is small enough BUT the ratio isn't too big
    // above all, ensure that the kfd is NOT assigned within the hi reserved range
    double ratio = FDT_HOLE_COUNT > 0 ? ((double)LARGEST_KFD / (double)FDT_HOLE_COUNT) : 0.0;
    if (LARGEST_KFD < KFD_RESERVED_HI && ratio < FDT_ACQ_MITER_THRESH && (FDT_HOLE_COUNT == 0 || LARGEST_KFD <= FDT_ACQ_SMALL_LKFD || ratio <= FDT_ACQ_NITER_THRESH)) {
        LARGEST_KFD ++;
        return LARGEST_KFD;
    }
    // iterate through the table up to largest kfd to find an open kfd
    for (u16 check = KFD_RESERVED_LO; check < LARGEST_KFD; check ++) {
        if (!(FD_TABLE[check].flags & FDE_OPEN)) {
            FDT_HOLE_COUNT --; // a hole was just filled, so the count must reflect that
            // populate table entry
            FD_TABLE[check].flags = flags | FDE_OPEN | FDE_VALID;
            FD_TABLE[check].procid = pid;
            FD_TABLE[check].disk_loc = disk_loc;
            FD_TABLE[check].position = 0;
            return check;
        }
    }
    // the table is full :(
    return -1;
}
/*
flushes the associated file if buffered
non-zero return value indicates an error
*/
int flush_fd(FileSystem* fs, int kfd) {
    errno = EOPNOTSUPP;
    return -1;
}
/*
available for external use
this function guarantees that either the table entry will be zeroed, or program execution will have stopped
non-zero return value indicates an error
*/
int close_fd(FileSystem* fs, int kfd, char doflush, char update_lkfd) {
    int retc = 0;
    if (doflush) retc |= flush_fd(fs, kfd);
    free_fd(kfd, update_lkfd);
    return retc; // propagate possible error codes
}
/*
available for external use
closes all file handles owned by a process
non-zero return value indicates an error
*/
int close_all_procfds(FileSystem* fs, pid_t pid, char doflush) {
    for (u16 check = LARGEST_KFD; check >= KFD_RESERVED_LO; check --) {
        if (FD_TABLE[check].flags & FDE_OPEN && FD_TABLE[check].procid == pid) {
            if (close_fd(fs, check, doflush, 0)) return -1;
        }
        if (check == 0) break;
    }
    return 0;
}

#endif