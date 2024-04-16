#ifndef __TSFSCONST_H__
#define __TSFSCONST_H__ 1
/*
defines constants such as O_RDWR
*/
#ifndef S_IRWXU
    // user (file owner) has read, write, and execute permission
    #define S_IRWXU 00700
    // user has read permission
    #define S_IRUSR 00400
    // user has write permission
    #define S_IWUSR 00200
    // user has execute permission
    #define S_IXUSR 00100
    // group has read, write, and execute permission
    #define S_IRWXG 00070
    // group has read permission
    #define S_IRGRP 00040
    // group has write permission
    #define S_IWGRP 00020
    // group has execute permission
    #define S_IXGRP 00010
    // others have read, write, and execute permission
    #define S_IRWXO 00007
    // others have read permission
    #define S_IROTH 00004
    // others have write permission
    #define S_IWOTH 00002
    // others have execute permission
    #define S_IXOTH 00001
#endif
#ifndef O_RDONLY
    // open in read only mode
    #define O_RDONLY 0b00000001
    // open in write only mode
    #define O_RDONLY 0b00000010
    // open in read+write mode
    #define O_RDWR   0b00000011
    // create file if it does not already exist
    #define O_CREAT  0b00000100
    // open in append mode
    #define O_APPEND 0b00001000
#endif
// magic number for f_type in fstatfs
#define TSFS_MAGIC 0x75f52024
#ifndef ST_RDONLY
    //Mandatory locking is permitted on the filesystem (see fcntl(2)).
    #define ST_MANDLOCK    0b0000000001U
    //Do not update access times; see mount(2).
    #define ST_NOATIME     0b0000000010U
    //Disallow access to device special files on this filesystem.
    #define ST_NODEV       0b0000000100U
    //Do not update directory access times; see mount(2).
    #define ST_NODIRATIME  0b0000001000U
    //Execution of programs is disallowed on this filesystem.
    #define ST_NOEXEC      0b0000010000U
    //The set-user-ID and set-group-ID bits are ignored by exec(3) for executable files on this filesystem
    #define ST_NOSUID      0b0000100000U
    //This filesystem is mounted read-only.
    #define ST_RDONLY      0b0001000000U
    //Update atime relative to mtime/ctime; see mount(2).
    #define ST_RELATIME    0b0010000000U
    //Writes are synched to the filesystem immediately (see the description of O_SYNC in open(2)).
    #define ST_SYNCHRONOUS 0b0100000000U
    //Symbolic links are not followed when resolving paths; see mount(2).
    #define ST_NOSYMFOLLOW 0b1000000000U
#endif
#endif
