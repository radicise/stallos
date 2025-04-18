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
	struct PerThreadgroup* group;// Does not change; can be deallocated with dealloc(group, sizeof(struct PerThreadgroup))
};
Mutex Threads_threadManage;
struct PerThreadgroup* volatile PerThreadgroup_context;
struct PerThread* volatile PerThread_context;
struct Thread* volatile Thread_context;
#ifndef TARGETNUM
#error "`TARGETNUM' is not set"
#endif
#if TARGETNUM == 1
#include "machine/x86_32/threads.h"
#else
#error "Target is not supported"
#endif
#include "sched.h"
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
struct Thread* Threads_forkData(struct Thread* orig) {
	Mutex_acquire(&(orig->thread.dataLock));
	Mutex_acquire(&(orig->thread.fsinfo->dataLock));
	struct Thread* nth = alloc(sizeof(struct Thread));
	moveExv(nth, orig, sizeof(struct Thread));
	struct FSInfo* nfsi = alloc(sizeof(struct FSInfo));
	moveExv(nfsi, orig->thread.fsinfo, sizeof(struct FSInfo));
	const char* ocwd = orig->thread.fsinfo->cwd;
	const char* oroot = orig->thread.fsinfo->root;
	nth->thread.fsinfo = nfsi;
	size_t clen;
	const char* cstr = ocwd;
	char* nstr = alloc((clen = strlen(cstr)) + 1);
	moveExv(nstr, cstr, clen);
	nstr[clen] = 0x00;
	nfsi->cwd = nstr;
	cstr = oroot;
	nstr = alloc((clen = strlen(cstr)) + 1);
	moveExv(nstr, cstr, clen);
	nstr[clen] = 0x00;
	nfsi->root = nstr;
	Mutex_release(&(orig->thread.fsinfo->dataLock));
	Mutex_release(&(orig->thread.dataLock));
	Mutex_initUnlocked(&(nth->thread.fsinfo->dataLock));
	Scheduler_clearRusage(&(nth->thread.usage));
	Mutex_initUnlocked(&(nth->thread.dataLock));
	struct PerThreadgroup* ngrp = alloc(sizeof(struct PerThreadgroup));
	Mutex_acquire(&(orig->group->breakLock));
	memcpy(ngrp, orig->group, sizeof(struct PerThreadgroup));
	ngrp->mem = (MemSpace_fork(ngrp->mem));// TODO Maybe allow reading but not writing of `userBreak' while the memory is being forked
	Mutex_release(&(orig->group->breakLock));
	Mutex_initUnlocked(&(ngrp->breakLock));
	ngrp->desctors = Map_copy(orig->group->desctors);
	Scheduler_clearRusage(&(ngrp->reaped));
	Mutex_initUnlocked(&(ngrp->reapedLock));
	nth->group = ngrp;
	return nth;
}
pid_t kernel_fork(void* kstack) {// When there is no "tgid"-"tid" combination available for the new thread, the operation fails, no new thread is created, and the value of `(pid_t) (-1)' is returned to the caller; when there is at least one "tgid"-"tid" combination available for the new thread, the operation succeeds, the new thread's "tid" is returned to the calling thread, and the new thread returns to its saved state as if resulting from being a successfully-created child from fork(2); locks are NOT given to the new thread
	struct Thread* nth = Threads_forkData(Thread_context);
	pid_t result = Threads_forkExec(nth, kstack);
	if (result == ((pid_t) (-1))) {// TODO Maybe prevent the possibility of needing to deallocate in the case of the operation failing
		if (!(Mutex_tryAcquire(&(nth->group->breakLock)))) {
			bugCheckNum(0x0001 | FAILMASK_PERTHREAD);
		}
		dealloc(nth->group, sizeof(struct PerThreadgroup));
		if (!(Mutex_tryAcquire(&(nth->thread.dataLock)))) {
			bugCheckNum(0x0002 | FAILMASK_PERTHREAD);
		}
		if (!(Mutex_tryAcquire(&(nth->thread.fsinfo->dataLock)))) {
			bugCheckNum(0x0003 | FAILMASK_PERTHREAD);
		}
		dealloc((void*) (nth->thread.fsinfo->cwd), strlen(nth->thread.fsinfo->cwd) + 1);
		dealloc((void*) (nth->thread.fsinfo->root), strlen(nth->thread.fsinfo->root) + 1);
		dealloc(nth->thread.fsinfo, sizeof(struct FSInfo));
		dealloc(nth, sizeof(struct Thread));
	}
	return result;
}
#define errno (PerThread_context->errno)
#define tid (PerThread_context->tid)
#define tgid (PerThreadgroup_context->tgid)
#define usermem (PerThreadgroup_context->mem)
#endif
