#ifndef __THREADS_H__
#define __THREADS_H__ 1
#define FAILMASK_THREADS 0x00090000
#ifndef TARGETNUM
#error "`TARGETNUM' is not set"
#endif
#if TARGETNUM == 1
#include "machine/x86_32/Threadstate.h"
#else
#error "Target is not supported"
#endif
#include "perThreadgroup.h"
#include "perThread.h"
struct Thread {
	struct PerThread thread;
	struct Thread_state state;
	struct PerThreadgroup* group;
};
Mutex Threads_threadManage;
#ifndef TARGETNUM
#error "`TARGETNUM' is not set"
#endif
#if TARGETNUM == 1
#include "machine/x86_32/threads.h"
#else
#error "Target is not supported"
#endif
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
#define errno (PerThread_context->errno)
#define tid (PerThread_context->tid)
#define ruid (PerThread_context->ruid)
#define euid (PerThread_context->euid)
#define suid (PerThread_context->suid)
#define fsuid (PerThread_context->fsuid)
#define cwd (PerThread_context->fsinfo->cwd)
#define root (PerThread_context->fsinfo->root)
#define umask (PerThread_context->fsinfo->umask)
#define tgid (PerThreadgroup_context->tgid)
#define usermem (PerThreadgroup_context->mem)
#endif
