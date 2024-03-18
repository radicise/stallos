#ifndef __PERTHREAD_H__
#define __PERTHREAD_H__ 1
#include "types.h"
struct FSInfo {
	const char* cwd;// Can be deallocated with dealloc(strlen(cwd) + 1)
	const char* root;// Can be deallocated with dealloc(strlen(root) + 1)
	mode_t umask;
	Mutex dataLock;
};
struct PerThread {// TODO Make a system for threads to change the properties of other threads
	int errno;// This MUST be the first member (in order of member declaration) of `struct PerThread'; this may not be accessed in the kernel by any thread other than the one handling a system call of the thread specified by this structure
	pid_t tid;// Does not change
	Mutex dataLock;// TODO Must be acquired when any of the following fields are being modified or read; this must be acquired when the reference `fsinfo' is had
	uid32_t ruid;
	uid32_t euid;
	uid32_t suid;
	uid32_t fsuid;
	u64 cap_effective;// TODO Initialise
	u64 cap_permitted;// TODO Initialise
	u64 cap_inheritable;// TODO Initialise
	struct FSInfo* fsinfo;// TODO Initialise
};
void lockThreadInfo(void) {
	Mutex_acquire(&(PerThread_context->dataLock));
	return;
}
void unlockThreadInfo(void) {
	Mutex_release(&(PerThread_context->dataLock));
	return;
}
void lockFSInfo(void) {
	Mutex_acquire(&(PerThread_context->fsinfo->dataLock));
	return;
}
void unlockFSInfo(void) {
	Mutex_release(&(PerThread_context->fsinfo->dataLock));
	return;
}
#include "threads.h"
#define errno (PerThread_context->errno)
#define tid (PerThread_context->tid)
#define ruid (PerThread_context->ruid)
#define euid (PerThread_context->euid)
#define suid (PerThread_context->suid)
#define fsuid (PerThread_context->fsuid)
#define cwd (PerThread_context->fsinfo->cwd)
#define root (PerThread_context->fsinfo->root)
#define umask (PerThread_context->fsinfo->umask)
#endif
