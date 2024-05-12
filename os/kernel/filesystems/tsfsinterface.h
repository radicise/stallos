#ifndef __TSFSINTERFACE_H__
#define __TSFSINTERFACE_H__ 1

#include "fsdefs.h"

typedef struct {
    int tsfs_node_id;
    unsigned long long cursor;
} TSFS_FILE;

// struct FSInterface {// Concurrency support is guaranteed; `errno' in the `struct FSReturn' is used instead of the thread's `errno'
// 	struct FSReturn (*FSInit)(struct FileDriver*, int, loff_t);// Initialises for the filesystem instance; Arguments: ("partition", "kfd", "size"); Return: "fs"
// 	void (*FSClose)(void*);// Closes the filesystem instance, deallocates the filesystem instance, flushes data to underlying partition file; Arguments: ("fs"); Does not fail
// 	struct FSReturn (*open)(const char*, int, mode_t, void*);// Arguments: ("fileName", "flags", "mode", "fs"); Returns: "return"; The path passed is guaranteed to be relative to the mount point; The non-error return value is a "kfd"
// 	struct FSReturn (*chmod)(const char*, mode_t);// Arguments: ("path", "mode"); Returns: "return"; The path passed is guaranteed to be relative to the mount point
// 	struct FSReturn (*link)(const char*, const char*);// Arguments: ("old", "new"); Returns: "return"; The paths passed are guaranteed to be relative to the mount point
// };

FSRet tsfs_fsinit(struct FileDriver* fdr, int kfd, u8 size) {
    FSRet r = loadFS(fdr, kfd);
    return r;
}
void tsfs_fsclose(void* fso) {
    FileSystem* fs = (FileSystem*)fso;
    fsflush(fs);
    releaseFS(fs);
}
FSRet tsfs_iopen(const char* fileName, int flags, mode_t mode, void* fs) {
    FSRet r = {.err=EOPNOTSUPP};
    return r;
}
FSRet tsfs_ichmod(const char* path, mode_t mode) {
    FSRet r = {.err=EOPNOTSUPP};
    return r;
}
FSRet tsfs_ilink(const char* old, const char* new) {
    FSRet r = {.err=EOPNOTSUPP};
    return r;
}

struct FSInterface FS_FUNCS = {
    .FSInit=tsfs_fsinit,
    .FSClose=tsfs_fsclose,
    .open=tsfs_iopen,
    .chmod=tsfs_ichmod,
    .link=tsfs_ilink
};

#endif
