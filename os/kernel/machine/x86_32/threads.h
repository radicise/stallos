#ifndef __MACHINE_X86_32_THREADS_H__
#define __MACHINE_X86_32_THREADS_H__ 1
#include "../../types.h"
#include "Threadstate.h"
#include "../../perThread.h"
#include "../../perThreadgroup.h"
#include "../../obj/Map.h"
#include "segments.h"
struct Map* ___taskMap___;// Do NOT access directly except for within the prescribed interaction functions; pid_t -> struct Thread*; the `struct Thread' aggregates associated with the values mapped to by the mapping can be deallocated with dealloc(value, sizeof(struct Thread))
int ___amntTasks___;// Do NOT access directly except for within the prescribed interaction functions
pid_t ___nextTask___;// Do NOT access directly except for within the prescribed interaction functions
volatile pid_t pid_max = 32768;// TODO URGENT Make an interface to allow for reading and writing of this value
#define PID_USERSTART 300
void Threads_init(void) {
	Mutex_initUnlocked(&Threads_threadManage);// TODO URGENT REMOVE Threads_threadManage
	___amntTasks___ = 0;
	___taskMap___ = Map_create();
	___nextTask___ = 0;
	return;
}
pid_t Threads_insertThread(struct Thread* thread, int newThreadgroup) {// `thread' and `thread->group' must have been allocated with `alloc' with arguments of `sizeof(struct Thread)' and `sizeof(struct PerThreadgroup)', respectively; the references of these objects and the memory management of these objects are relinquished by the caller in the case of non-failure; this function may be called ONLY when `handlingIRQ' is nonzero; this function sets the value of the `tid' member of `thread->thread' and, if and only if `newThreadgroup' is nonzero, the `tgid' member of `*(thread->group)'; `(pid_t) 0' is returned if and only if no pid could be allocated
	if (!(Mutex_tryAcquire(&Threads_threadManage))) {
		bugCheckNum(0x0006 | FAILMASK_THREADS);
	}
	pid_t n = ___nextTask___;
	while (1) {
		uintptr a = Map_fetch((uintptr) n, ___taskMap___);
		if (a == ((uintptr) (-1))) {
			break;
		}
		n++;
		if (n == pid_max) {
			n = (pid_t) PID_USERSTART;// PID_USERSTART is used and kept at 300 for consistency with Linux behaviours
		}
		if (n == ___nextTask___) {
			Mutex_release(&Threads_threadManage);
			return (pid_t) 0;// TODO URGENT Correctly implement behaviour for allocating tgid and tid when any threads with tid == tgid have ended
		}
	}
	___nextTask___ = ((n + ((pid_t) 1)) == pid_max) ? ((pid_t) PID_USERSTART) : (n + ((pid_t) 1));
	thread->thread.tid = n;
	if (newThreadgroup) {
		thread->group->tgid = n;
	}
	if (Map_add((uintptr) n, (uintptr) thread, ___taskMap___)) {
		bugCheckNum(0x0008 | FAILMASK_THREADS);
	}
	___amntTasks___++;
	Mutex_release(&Threads_threadManage);
	return n;
}
unsigned long Threads_addThread_kfunc(unsigned long arg0, unsigned long arg1, unsigned long arg2, unsigned long arg3, unsigned long arg4, unsigned long arg5, unsigned long arg6, unsigned long arg7) {// This function can ONLY be called when `handlingIRQ' is nonzero
	return (unsigned long) Threads_insertThread((struct Thread*) ((uintptr) arg0), (int) arg1);
}
pid_t Threads_addThread(struct Thread* thread, int newGroup) {
	return (pid_t) (kfunc((unsigned long) ((uintptr) thread), (unsigned long) newGroup, 0, 0, 0, 0, 0, 0, Threads_addThread_kfunc));
}
int Threads_findNext(uintptr suspect, uintptr u) {
	struct Map_pair* pair = (struct Map_pair*) suspect;
	uintptr key = pair->key;
	if (key > (((uintptr*) u)[1])) {
		if (key < (((uintptr*) u)[0])) {
			((uintptr*) u)[0] = key;
		}
	}
	return 1;
}
#include "../../perThreadgroup.h"
#include "../../perThread.h"
void Threads_nextThread(void) {// MAY ONLY BE CALLED WHEN THE INTERRUPT FLAG IS CLEARED
	if (!handlingIRQ) {
		bugCheckNum(0x0005 | FAILMASK_THREADS);
	}
	if (!(Mutex_tryAcquire(&Threads_threadManage))) {
		bugCheckNum(0x0007 | FAILMASK_THREADS);
	}
	uintptr st = Map_fetch((uintptr) currentThread, ___taskMap___);
	if (st == ((uintptr) (-1))) {
		bugCheckNum(0x0002 | FAILMASK_THREADS);// The current thread seemingly does not exist
	}
	uintptr uu[2];
	uu[0] = (uintptr) pid_max;// TODO Ensure that the `uintptr' type is at least as wide as the `pid_t' type and that the conversions both ways between the types are reversible by doing the inverse conversion
	uu[1] = (uintptr) currentThread;
	Map_findByCompare((uintptr) uu, Threads_findNext, ___taskMap___);
	if (uu[0] == (pid_max)) {
		if (Map_fetch((uintptr) ((pid_t) 0), ___taskMap___) == ((uintptr) (-1))) {
			uu[1] = (uintptr) ((pid_t) 0);
			Map_findByCompare((uintptr) uu, Threads_findNext, ___taskMap___);
			if (uu[0] == pid_max) {
				bugCheckNum(0x0004 | FAILMASK_THREADS);// TODO UGRENT There would be no threads running, seemingly. Is this allowed under Linux?
			}
		}
		else {
			uu[0] = (pid_t) 0;
		}
	}
	if (currentThread == ((pid_t) (uu[0]))) {
		Mutex_release(&Threads_threadManage);
		return;
	}
	flushThreadState(&(((struct Thread*) st)->state));
	uintptr thn = Map_fetch(uu[0], ___taskMap___);
	if (thn == ((uintptr) (-1))) {
		bugCheckNum(0x0003 | FAILMASK_THREADS);// The planned thread seemingly does not exist
	}
	struct Thread* thns = (struct Thread*) thn;
	struct Thread_state* sttr = &(thns->state);
	moveExv((TSS*) (((volatile char*) physicalZero) + 0xb00), &(sttr->ktss), sizeof(TSS));
	moveExv(((TSS*) (((volatile char*) physicalZero) + 0xb00)) + 5, &(sttr->tss), sizeof(TSS));
	TS_setDesc(0x0b00, 127, 0, 0, sttr->kernelExecution ? 1 : 0, ((SegDesc*) (((volatile char*) physicalZero) + 0x800)) + 7);
	Seg_enable(((SegDesc*) (((volatile char*) physicalZero) + 0x800)) + 7);
	TS_setDesc(0x0d80, 127, 0, 0, 1, ((SegDesc*) (((volatile char*) physicalZero) + 0x800)) + 13);// TODO Should re-writing the TSS descriptor for every thread change be avoided?
	Seg_enable(((SegDesc*) (((volatile char*) physicalZero) + 0x800)) + 13);
	unsigned long tsi = (storeTS() >> 3) - ((unsigned long) 7);
	if (tsi) {
		tsi--;
	}
	(((TSS*) (((volatile char*) physicalZero) + 0xb00)) + tsi)->prev = (sttr->kernelExecution ? (7 * 8) : (13 * 8));
	Mutex_release(&Threads_threadManage);
	Seg_enable(((SegDesc*) (((volatile char*) physicalZero) + 0x800)) + 7);
	Seg_enable(((SegDesc*) (((volatile char*) physicalZero) + 0x800)) + 13);
	currentThread = (pid_t) uu[0];
	PerThreadgroup_context = thns->group;
	PerThread_context = &(thns->thread);
	Thread_context = thns;
	mem_barrier();
	return;
}
pid_t Threads_executionFork(struct Thread* newThread, void* stack) {// THIS FUNCTION MAY BE CALLED ONLY THROUGH THE `kfunc' INTERFACE; the argument `stack' is the first byte not in the stack if 'MACHINE_KSTACK_GROWSDOWN' is nonzero and otherwise is the first byte in the stack
	flushThreadState(&(newThread->state));
	prepare_fork_child(&(newThread->state), stack);
	pid_t n = Threads_insertThread(newThread, 1);
	if (n == ((pid_t) 0)) {
		return (pid_t) (-1);
	}
	newThread->state.invocData.data[0] = 0;
	return n;// TODO URGENT Ensure that locks are correct upon returning of the new kernel thread to user space
}
unsigned long Threads_forkExec_kfunc(unsigned long arg0, unsigned long arg1, unsigned long arg2, unsigned long arg3, unsigned long arg4, unsigned long arg5, unsigned long arg6, unsigned long arg7) {// THIS FUNCTION MAY BE CALLED ONLY THROUGH THE `kfunc' INTERFACE
	return (unsigned long) (Threads_executionFork((struct Thread*) ((uintptr) arg0), (void*) ((uintptr) arg1)));
}
pid_t Threads_forkExec(struct Thread* newThread, void* stack) {// `newThread' and `newThread->group' must have been allocated with `alloc' with arguments of `sizeof(struct Thread)' and `sizeof(struct PerThreadgroup)', respectively; the references `newThread' and `newThread->group', as well as memory management of each of these references, are relinquished
	return (pid_t) (kfunc((unsigned long) ((uintptr) newThread), (unsigned long) ((uintptr) stack), 0, 0, 0, 0, 0, 0, Threads_forkExec_kfunc));
}
void Threads_threadstateDump(struct Thread_state* dump) {// THIS FUNCTION MAY BE CALLED ONLY THROUGH THE `kfunc' INTERFACE
	flushThreadState(dump);
	return;
}
unsigned long Threads_dumpThreadstate_kfunc(unsigned long arg0, unsigned long arg1, unsigned long arg2, unsigned long arg3, unsigned long arg4, unsigned long arg5, unsigned long arg6, unsigned long arg7) {// THIS FUNCTION MAY BE CALLED ONLY THROUGH THE `kfunc' INTERFACE
	Threads_threadstateDump((struct Thread_state*) ((uintptr) arg0));
	return 0;
}
void Threads_dumpThreadState(struct Thread_state* dump) {
	kfunc((unsigned long) ((uintptr) dump), 0, 0, 0, 0, 0, 0, 0, Threads_dumpThreadstate_kfunc);
	return;
}
#endif
