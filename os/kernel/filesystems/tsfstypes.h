#ifndef __TSFSTYPES_H__
#define __TSFSTYPES_H__ 1
#ifndef __MOCKTEST
#include "../types.h"
#else
#define u32 unsigned long
#define s32 long
#define u16 unsigned short
#define s16 short
#define kdev_t u32
#define mode_t unsigned short
#define uid32_t unsigned int
#define time_t long
#endif
typedef unsigned int __fsword_t;
typedef int fsid_t;
typedef unsigned long fsblkcnt_t;
typedef unsigned long fsfilcnt_t;
typedef s16 blksize_t;
typedef s32 blkcnt_t;
#define uid_t uid32_t
#define gid_t uid32_t
struct statfs {
    __fsword_t f_type;    /* Type of filesystem (see below) */
    __fsword_t f_bsize;   /* Optimal transfer block size */
    fsblkcnt_t f_blocks;  /* Total data blocks in filesystem */
    fsblkcnt_t f_bfree;   /* Free blocks in filesystem */
    fsblkcnt_t f_bavail;  /* Free blocks available to
                             unprivileged user */
    fsfilcnt_t f_files;   /* Total inodes in filesystem */
    fsfilcnt_t f_ffree;   /* Free inodes in filesystem */
    fsid_t     f_fsid;    /* Filesystem ID */
    __fsword_t f_namelen; /* Maximum length of filenames */
    __fsword_t f_frsize;  /* Fragment size (since Linux 2.6) */
    __fsword_t f_flags;   /* Mount flags of filesystem
                             (since Linux 2.6.36) */
    __fsword_t f_spare[0];
};
struct timespec {
    time_t tv_sec;
    time_t tv_nsec;
};
struct stat {
    kdev_t st_dev;
    int st_ino;
    mode_t st_mode;
    u16 st_nlink;
    uid_t st_uid;
    gid_t st_gid;
    kdev_t st_rdev;
    off_t st_size;
    blksize_t st_blksize;
    blkcnt_t st_blocks;
    struct timespec st_atim;
    struct timespec st_mtim;
    struct timespec st_ctim;
    #define st_atime st_atim.tv_sec
    #define st_mtime st_mtim.tv_sec
    #define st_ctime st_ctim.tv_sec
};
#ifdef __MOCKTEST
#undef kdev_t
#undef mode_t
#undef uid32_t
#undef time_t
#undef s16
#undef s32
#undef u16
#undef u32
#endif
#endif