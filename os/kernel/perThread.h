#ifndef __PERTHREAD_H__
#define __PERTHREAD_H__ 1
#include "types.h"
struct PerThread {// TODO Make a system for threads to change the properties of other threads
	int errno;// This MUST be the first member (in order of member declaration) of `struct PerThread'; this may not be accessed in the kernel by any thread other than the one handling a system call of the thread specified by this structure
	pid_t tid;// Does not change
	Mutex dataLock;// Must be acquired when any of the following fields are being modified or read
	uid32_t ruid;
	uid32_t euid;
	uid32_t suid;
	uid32_t fsuid;
	u64 cap_effective;// TODO Initialise
	u64 cap_permitted;// TODO Initialise
	u64 cap_inheritable;// TODO Initialise
};
#include "threads.h"
#define errno (PerThread_context->errno)
#define tid (PerThread_context->tid)
#define ruid (PerThread_context->ruid)
#define euid (PerThread_context->euid)
#define suid (PerThread_context->suid)
#define fsuid (PerThread_context->fsuid)
#endif
