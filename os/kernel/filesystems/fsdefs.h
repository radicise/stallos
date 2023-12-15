#ifndef __FSDEFS_H__
#define __FSDEFS_H__ 1
/**
 * types that make my life easier, designed to be used in combination with fsuntypes.h because no thoughts of compatability went into this - Tristan
 */


#define TSFSVERSION "NOSPEC"

typedef int pid_t;

/*maximum open filehandles*/
const u16 SYS_MAX_FDS = 0xffff;

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
} FDENTRY;

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