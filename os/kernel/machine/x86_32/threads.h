#ifndef __MACHINE_X86_32_THREADS_H__
#define __MACHINE_X86_32_THREADS_H__ 1
#include "../../types.h"
#include "Threadstate.h"
#include "../../perThread.h"
#include "../../perThreadgroup.h"
#include "../../obj/Map.h"
#include "segments.h"
struct Map* ___taskMap___;// Do NOT access directly except for within the prescribed interaction functions; pid_t -> struct Thread*
int ___amntTasks___;// Do NOT access directly except for within the prescribed interaction functions
pid_t ___nextTask___;// Do NOT access directly except for within the prescribed interaction functions
volatile pid_t pid_max = 32768;// Acquire Threads_threadManage while accessing
#define PID_USERSTART 300
void Threads_init(void) {
	Mutex_initUnlocked(&Threads_threadManage);
	___amntTasks___ = 0;
	___taskMap___ = Map_create();
	___nextTask___ = 0;
	return;
}
pid_t Threads_addThread(struct Thread* thread) {// `thread' and `thread->group' must have been allocated with `alloc' with arguments of `sizeof(struct Thread)' and `sizeof(struct PerThreadgroup)', respectively; memory management of these objects is relinquished by the caller
	Mutex_acquire(&Threads_threadManage);
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
			bugCheckNum(0x0001 | FAILMASK_THREADS);// No "tid" / "tgid" could be allocated
		}
	}
	___nextTask___ = ((n + ((pid_t) 1)) == pid_max) ? ((pid_t) PID_USERSTART) : (n + ((pid_t) 1));
	if (Map_add((uintptr) n, (uintptr) thread, ___taskMap___)) {
		bugCheck();
	}
	___amntTasks___++;
	Mutex_release(&Threads_threadManage);
	return n;
}
int Threads_findNext(uintptr suspect, uintptr u) {// Threads_threadManage must have already been acquired and is NOT released
	struct Map_pair* pair = (struct Map_pair*) suspect;
	uintptr key = pair->key;
	if (key > ((uintptr) currentThread)) {
		if (key < (*((uintptr*) u))) {
			(*((uintptr*) u)) = key;
		}
	}
	return 1;
}
#include "../../perThreadgroup.h"
#include "../../perThread.h"
struct PerThreadgroup* volatile PerThreadgroup_context;
struct PerThread* volatile PerThread_context;
void Threads_nextThread(struct Thread_state* state, long irupt) {
	Mutex_acquire(&Threads_threadManage);
	uintptr st = Map_fetch((uintptr) currentThread, ___taskMap___);
	if (st == ((uintptr) (-1))) {
		bugCheckNum(0x0002 | FAILMASK_THREADS);// The current thread seemingly does not exist
	}
	cpy(&(((struct Thread*) st)->state), state, sizeof(struct Thread_state));
	uintptr u = pid_max;
	Map_findByCompare((uintptr) &u, Threads_findNext, ___taskMap___);
	if (u == (pid_max)) {
		if (Map_fetch((uintptr) ((pid_t) 0), ___taskMap___) == ((uintptr) (-1))) {
			currentThread = (pid_t) 0;
			Map_findByCompare((uintptr) &u, Threads_findNext, ___taskMap___);
			if (u == pid_max) {
				bugCheckNum(0x0004 | FAILMASK_THREADS);// There are no threads running, seemingly
			}
		}
		else {
			u = (pid_t) 0;
		}
	}
	currentThread = u;
	uintptr thn = Map_fetch((uintptr) u, ___taskMap___);
	if (thn == ((uintptr) (-1))) {
		bugCheckNum(0x0003 | FAILMASK_THREADS);// The planned thread seemingly does not exist
	}
	struct Thread* thns = (struct Thread*) thn;
	PerThreadgroup_context = thns->group;
	PerThread_context = &(thns->thread);
	struct Thread_state stt;
	cpy(&stt, &(thns->state), sizeof(struct Thread_state));
	Mutex_release(&Threads_threadManage);
	moveExv(((TSS*) (((volatile char*) physicalZero) + 0xb00)) + 5, &(stt.tss), sizeof(TSS));
	TS_setDesc(0x0d80, 127, 0, 0, ((SegDesc*) (((volatile char*) physicalZero) + 0x800)) + 13);
	Seg_enable(((SegDesc*) (((volatile char*) physicalZero) + 0x800)) + 13);
	return;
}
#endif
