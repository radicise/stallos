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
struct PerThreadgroup* volatile PerThreadgroup_context;
struct PerThread* volatile PerThread_context;
void Threads_nextThread(void) {// MAY ONLY BE CALLED WHEN THE INTERRUPT FLAG IS CLEARED
	if (!handlingIRQ) {
		bugCheckNum(0x0005 | FAILMASK_THREADS);
	}
	Mutex_acquire(&Threads_threadManage);// TODO URGENT Prevent dealocking due to the scheduler trying to switch tasks while a thread is being added from happening
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
	currentThread = (pid_t) uu[0];
	moveExv(&(((struct Thread*) st)->state.ktss), ((TSS*) (((volatile char*) physicalZero) + 0xb00)), sizeof(TSS));
	moveExv(&(((struct Thread*) st)->state.tss), ((TSS*) (((volatile char*) physicalZero) + 0xb00)) + 5, sizeof(TSS));
	((struct Thread*) st)->state.kernelExecution = (TS_isBusy(((SegDesc*) (((volatile char*) physicalZero) + 0x800)) + 7)) ? 1 : 0;
	uintptr thn = Map_fetch(uu[0], ___taskMap___);
	if (thn == ((uintptr) (-1))) {
		bugCheckNum(0x0003 | FAILMASK_THREADS);// The planned thread seemingly does not exist
	}
	struct Thread* thns = (struct Thread*) thn;
	PerThreadgroup_context = thns->group;
	PerThread_context = &(thns->thread);
	struct Thread_state* sttr = &(thns->state);
	moveExv(((TSS*) (((volatile char*) physicalZero) + 0xb00)), &(sttr->ktss), sizeof(TSS));
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
	mem_barrier();
	return;
}
#endif
