#ifndef __FSTYPES_H__
#define __FSTYPES_H__ 1
/*
typedefs for use externally
*/

#ifndef TARGETNUM
#define TARGETNUM 2
#endif
#include "../ktypes.h"
#include "../FileDriver.h"

struct FSReturn {// `errno' == 0: Success; `errno' != 0: Failure
	void* retptr;// Pointer to object, if `errno' == 0 and return type is a pointer
	_kernel_ssize_t retval;// Value, if `errno' == 0 and the return type is not a pointer
	int err;// Error number if not zero, uses the same error number values as the kernel-userspace API
};
struct FSInterface {// Concurrency support is guaranteed; `errno' in the `struct FSReturn' is used instead of the thread's `errno'
	struct FSReturn (*FSInit)(struct FileDriver*, int, _kernel_loff_t);// Initialises for the filesystem instance; Arguments: ("partition", "kfd", "size"); Return: "fs"
	void (*FSClose)(void*);// Closes the filesystem instance, deallocates the filesystem instance, flushes data to underlying partition file; Arguments: ("fs"); Does not fail
	struct FSReturn (*open)(const char*, int, _kernel_mode_t, void*);// Arguments: ("fileName", "flags", "mode", "fs"); Returns: "return"; The path passed is guaranteed to be relative to the mount point; The non-error return value is a "kfd"
	struct FSReturn (*chmod)(const char*, _kernel_mode_t);// Arguments: ("path", "mode"); Returns: "return"; The path passed is guaranteed to be relative to the mount point
	struct FSReturn (*link)(const char*, const char*);// Arguments: ("old", "new"); Returns: "return"; The paths passed are guaranteed to be relative to the mount point
};

typedef struct {
    _kernel_u16   breakver;
    _kernel_u8    partition_size;
    _kernel_u64   creation_time;
    char  version[16];
    _kernel_u32   top_dir;
    _kernel_u32   usedleft;
    _kernel_u32   usedright;
    _kernel_u32   usedhalfleft;
    _kernel_u32   usedhalfright;
    _kernel_u64   checksum;
} TSFSRootBlock;

typedef struct {
    /*
    c - create
    m - modify
    a - access
    s - secs
    n - nsec
    */
    _kernel_s64 cs;
    _kernel_s64 cn;
    _kernel_s64 ms;
    _kernel_s64 mn;
    _kernel_s64 as;
    _kernel_s64 an;
} TIMES;

typedef struct {
    size_t magicno;
    _kernel_u32   disk_loc;
    _kernel_u16   rc;
    _kernel_u8    id;
    _kernel_u16   refcount;
    _kernel_u32   head;
    _kernel_u32   blocks;
    _kernel_u32   ikey;
    _kernel_u64   size;
    _kernel_u16   perms;
    // WARNING: DO NOT ACCESS THIS FIELD, USE THE [get_dhtimes] AND [set_dhtimes] HELPER FUNCTIONS
    _kernel_u64   times[6];
    _kernel_u64   checksum;
} TSFSDataHeader;

typedef struct {
    _kernel_size_t magicno;
    _kernel_u32   disk_loc;
    _kernel_u16   rc;
    _kernel_u8    id;
    _kernel_u8    storage_flags;
    _kernel_u32   next_block;
    _kernel_u32   prev_block;
    _kernel_u16   data_length;
    _kernel_u8    blocks_to_terminus;
    _kernel_u64   checksum;
} TSFSDataBlock;

typedef struct {
    _kernel_size_t magicno;
    _kernel_u32   disk_loc;
    _kernel_u16   rc;
    _kernel_u8    id;
    _kernel_u8    storage_flags;
    _kernel_u32   parent_loc;
    _kernel_u32   pnode;
    _kernel_u32   ikey;
    char  name[255];
    _kernel_u64   checksum;
} TSFSStructNode;

typedef struct {
    _kernel_size_t magicno;
    _kernel_u32   disk_loc;
    _kernel_u16   rc;
    _kernel_u8    id;
    _kernel_u8    entrycount;
    _kernel_u8    flags;
    _kernel_u32   disk_ref;
    _kernel_u64   checksum;
} TSFSStructBlock;

typedef struct {
    _kernel_size_t csize;
    _kernel_size_t cused;
    _kernel_size_t umax;
    _kernel_size_t hmin;
    void** ptr;
} Magic;

typedef struct {
    struct FileDriver* fdrive;
    int kfd;
    int err;
    TSFSRootBlock* rootblock;
    Magic* magic;
} FileSystem;

typedef struct {
    char  name[9];
    _kernel_u32   dloc;
    _kernel_u8    flags;
} TSFSSBChildEntry;

#endif
