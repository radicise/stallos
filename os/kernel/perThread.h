#ifndef __PERTHREAD_H__
#define __PERTHREAD_H__ 1
#include "types.h"
#define FAILMASK_PERTHREAD 0x00160000
struct FSInfo {
	Mutex dataLock;// Must be acquired when any of the references `cwd' and `root' are held; must be acquired when any of the following fields are being accessed
	const char* volatile cwd;// Can be deallocated with dealloc(cwd, strlen(cwd) + 1)
	const char* volatile root;// Can be deallocated with dealloc(root, strlen(root) + 1)
	volatile mode_t umask;
};
struct PerThread {// TODO Make a system for threads to change the properties of other threads
	int errno;// This MUST be the first member (in order of member declaration) of `struct PerThread'; this may not be accessed in the kernel by any thread other than the one handling a system call of the thread specified by this structure
	pid_t tid;// Does not change
	Mutex dataLock;// TODO Must be acquired when any of the following fields are being accessed; this must be acquired when the reference `fsinfo' is had
	volatile kuid_t ruid;
	volatile kuid_t euid;
	volatile kuid_t suid;
	volatile kuid_t fsuid;
	volatile u64 cap_effective;
	volatile u64 cap_permitted;
	volatile u64 cap_inheritable;
	struct FSInfo* fsinfo;// Can be deallocated with dealloc(fsinfo, sizeof(struct FSInfo))
    volatile int __curr_userFD;
};
#include "threads.h"
#endif
