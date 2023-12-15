#ifndef __FSDEFS_H__
#define __FSDEFS_H__ 1

#include "types.h"

extern void* allocate(size_t);
extern u64 compute_checksum(size_t*);
extern u8 validate_checksum(size_t*, u64);
extern void disk_seek(u64);
extern u64 disk_tell();


u64 hashstr(u8* str) {
    u64 hash = 0;
    int c;
    while (c = *str++) {
        hash = c + (hash << 6) + (hash << 16) - hash;
    }
    return hash;
}


#define TSFSVERSION "NOSPEC"

typedef int pid_t;

/*maximum open filehandles*/
const u16 SYS_MAX_FDS = 0xffff;
const double FDT_ACQ_NITER_THRESH = 0.2; // threshold under which kfd_acquire shouldn't iter because it is unlikely to be fast enough
const double FDT_ACQ_MITER_THRESH = 1.0; // threshold over which kfd_acquire MUST iter because holes are too prevalent in the table
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
stores data about the file system as a whole
*/
typedef struct {
    u64   partition_start;
    u64   partition_size;
    u8    system_size;
    u64   creation_time;
    char  version[16];
    u8    block_size;
    u64   top_dir;
    u64   checksum;
} TSFSRootBlock;

#endif