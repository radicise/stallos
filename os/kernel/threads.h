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
struct Thread {
	struct PerThread thread;
	struct Thread_state state;
	struct PerThreadgroup* group;
};
extern void Thread_restore(struct Thread_state*, long);// Does not return
extern void Thread_run(struct Thread_state*);// Does not return
Mutex Threads_threadManage;
#ifndef TARGETNUM
#error "`TARGETNUM' is not set"
#endif
#if TARGETNUM == 1
#include "machine/x86_32/threads.h"
#else
#error "Target is not supported"
#endif
#endif
