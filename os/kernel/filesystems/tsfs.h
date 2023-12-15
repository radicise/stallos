#ifndef __TSFS_H__
#define __TSFS_H__ 1
#include "FileDriver.h"
#include "types.h"
#include "fsdefs.h"
#include <string.h>
/*
Anything not explicitly marked as something for use outside of the file system code should NOT be used
Doing so is undefined behavior
- Tristan
*/
// TODO
extern void* allocate(size_t);
extern u64 compute_checksum(size_t*);
extern u8 validate_checksum(size_t*, u64);
extern void disk_seek(u64);
extern u64 disk_tell();

// maps KFDs to disk locations, owners, and flags
volatile FDENTRY FD_TABLE[SYS_MAX_FDS] = {0};
// tracks the maximum used KFD, allows optimizations when closing based on process ids or other criteria
volatile u16 LARGEST_KFD = 0;
// minimum number that assigned kfds must start at
const u16 KFD_RESERVED_LO = 0;
// maximum number that a kfd can be assigned as
// the -1 component should NEVER be modified, it is there to prevent out-of-bounds accesses due to off-by-one errors
const u16 KFD_RESERVED_HI = SYS_MAX_FDS - 1 - 0;

u64 hashstr(u8* str) {
    u64 hash = 0;
    int c;
    while (c = *str++) {
        hash = c + (hash << 6) + (hash << 16) - hash;
    }
    return hash;
}
/*
Custom File System, see the stallOS spec for more details - Tristan
DO NOT modify any instance provided by the file system
*/
typedef struct {
    struct FileDriver* fdrive;
    int kfd;
    // in bytes
    u64 partition_start;
} FileSystem;

/*
available for external use
DO NOT CALL OUTSIDE THE CASE THAT A NEW PARTITION IS BEING MADE
RETURNS ZERO ON ERROR
*/
FileSystem* createFS(struct FileDriver* fdr, int kfd, u64 p_start, u8 blocksize) {
    if (blocksize < 10 || blocksize > 32) {
        return 0;
    }
    FileSystem* fs = (FileSystem*) allocate(sizeof(FileSystem));
    fs -> fdrive = fdr;
    fs -> kfd = kfd;
    fs -> partition_start = p_start;
    TSFSRootBlock* rblock = (TSFSRootBlock*) allocate(sizeof(TSFSRootBlock));
    strcpy(rblock->version, TSFSVERSION);
    rblock->block_size = 2<<blocksize;
    return fs;
}
/*
loads a file system from a disk partition
available for external use
*/
FileSystem* loadFS(struct FileDriver* fdr, int kfd, u64 p_start) {
    FileSystem* fs = (FileSystem*) allocate(sizeof(FileSystem));
    fs -> fdrive = fdr;
    fs -> kfd = kfd;
    fs -> partition_start = p_start;
    return fs;
}
/*
clears an entry from the FDE table
*/
int free_fd(FileSystem* fs, int kfd, char update_lkfd) {
    volatile FDENTRY* fde = &FD_TABLE[kfd]; // zeros the table entry
    fde->disk_loc = 0;
    fde->procid = -1;
    fde->flags = 0;
    // check that the kfd is not in either reserved block
    if (update_lkfd && kfd >= KFD_RESERVED_LO && kfd <= KFD_RESERVED_HI) {
        // update largest kfd
        if ((kfd-KFD_RESERVED_LO+1) == LARGEST_KFD) {
            LARGEST_KFD --;
        }
    }
    return 0;
}
/*
assigns a kfd in the table
returns -1 on error
*/
int acquire_fd(FileSystem* fs, u64 disk_loc, pid_t pid, u8 flags) {
    return -1;
}
/*
available for external use
non-zero return value indicates an error
*/
int close_fd(FileSystem* fs, int kfd, char update_lkfd) {
    free_fd(fs, kfd, update_lkfd);
    return 0;
}
/*
available for external use
closes all file handles owned by a process
non-zero return value indicates an error
*/
int close_all_procfds(FileSystem* fs, pid_t pid, char doflush) {
    int (*_close)(FileSystem*, int, char);
    if (doflush) {
        _close = *close_fd;
    } else {
        _close = *free_fd;
    }
    for (u16 check = LARGEST_KFD; check >= KFD_RESERVED_LO; check --) {
        if (FD_TABLE[check].flags & FDE_OPEN && FD_TABLE[check].procid == pid) {
            _close(fs, check, 0);
        }
        if (check == 0) break;
    }
    return 0;
}
/**
 * writes a `u64` to disk in big endian format
 */
void write_u64be(FileSystem* fs, u64 n) {
    char ptr[8];
    ptr[0] = (char)((n>>56)&0xff);
    ptr[1] = (char)((n>>48)&0xff);
    ptr[2] = (char)((n>>40)&0xff);
    ptr[3] = (char)((n>>32)&0xff);
    ptr[4] = (char)((n>>24)&0xff);
    ptr[5] = (char)((n>>16)&0xff);
    ptr[6] = (char)((n>>8)&0xff);
    ptr[7] = (char)(n&0xff);
    (fs->fdrive->write)(fs->kfd,ptr,8);
}
#include "fsundefs.h"
#endif