#ifndef __TSFSINTERFACE_H__
#define __TSFSINTERFACE_H__ 1

#include "fsdefs.h"
#include "../perProcess.h"
#include "../errno.h"
#include "../kfds.h"
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