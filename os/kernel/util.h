#ifndef __UTIL_H__
#define __UTIL_H__ 1
#include "types.h"
int strcmp(const char* d, const char* g) {
	if (!((*d) | (*g))) {
		return 0;
	}
	while (1) {
		if (!(*d)) {
			if (!(*g)) {
				if ((*d) == (*g)) {
					return 0;
				}
				if ((*d) > (*g)) {
					return 1;
				}
			}
			return (-1);
		}
		if (!(*g)) {
			return 1;
		}
		d++;
		g++;
	}
}
#ifndef TARGETNUM
#error "'TARGETNUM' is not set"
#endif
#if TARGETNUM == 1
#include "machine/x86_32/util.h"
#else
#error "Target is not supported"
#endif
extern unsigned long AtomicULong_get(AtomicULong*);// Performs a memory barrier after loading, before returning
extern void AtomicULong_set(AtomicULong*, unsigned long);// Performs a memory barrier after storing, before returning
extern void SimpleMutex_acquire(SimpleMutex*);
extern void SimpleMutex_release(SimpleMutex*);
extern int SimpleMutex_tryAcquire(SimpleMutex*);// Returns 1 if acquired, otherwise returns 0
extern void SimpleMutex_wait(void);// Wastes enough time to let at least one other thread acquire a SimpleMutex in that time span if it is already executing SimpleMutex_acquire, assuming that the thread attempting to acquire is not interrupted
extern void SimpleMutex_initUnlocked(SimpleMutex*);// Performs a memory barrier
typedef volatile struct {
	SimpleMutex stateLock;
	pid_t ownerThread;
	unsigned long acquires;
} Mutex;// Reentrant, acquisition even if it is already acquired by the thread also acts as a memory fence for the thread
void Mutex_acquire(Mutex* mutex) {
	pid_t id = handlingIRQ ? (~currentThread) : currentThread;
	while (1) {
		SimpleMutex_acquire(&(mutex->stateLock));
		if (mutex->acquires == 0) {
			(mutex->ownerThread) = id;
		}
		else if ((mutex->ownerThread) != id) {
			SimpleMutex_release(&(mutex->stateLock));
			SimpleMutex_wait();
			continue;
		}
		(mutex->acquires)++;
		if ((mutex->acquires) == 0) {
			bugCheckNum(0x0101 | FAILMASK_SYSTEM);
		}
		SimpleMutex_release(&(mutex->stateLock));
		return;
	}
}
void Mutex_release(Mutex* mutex) {// Acts as a memory barrier
	pid_t id = handlingIRQ ? (~currentThread) : currentThread;
	SimpleMutex_acquire(&(mutex->stateLock));
	if ((mutex->ownerThread) != id) {
		bugCheckNum(0x0102 | FAILMASK_SYSTEM);
	}
	if ((mutex->acquires) == 0) {
		bugCheckNum(0x0103 | FAILMASK_SYSTEM);
	}
	(mutex->acquires)--;
	SimpleMutex_release(&(mutex->stateLock));
}
int Mutex_tryAcquire(Mutex* mutex) {// Returns 1 if acquired, otherwise returns 0; acts as a memory barrier
	pid_t id = handlingIRQ ? (~currentThread) : currentThread;
	SimpleMutex_acquire(&(mutex->stateLock));
	if (mutex->acquires == 0) {
		(mutex->ownerThread) = id;
	}
	else if ((mutex->ownerThread) != id) {
		SimpleMutex_release(&(mutex->stateLock));
		return 0;
	}
	(mutex->acquires)++;
	if ((mutex->acquires) == 0) {
		bugCheckNum(0x0104 | FAILMASK_SYSTEM);
	}
	SimpleMutex_release(&(mutex->stateLock));
	return 1;
}
void Mutex_wait(void) {// Wastes enough time to let at least one other thread acquire a Mutex in that time span if it is already executing Mutex_acquire, assuming that the thread attempting to acquire is not interrupted
	SimpleMutex_wait();
	return;
}
void Mutex_initUnlocked(Mutex* mutex) {// Acts as a memory barrier
	(mutex->ownerThread) = (pid_t) 0;
	(mutex->acquires) = 0;
	SimpleMutex_initUnlocked(&(mutex->stateLock));
	return;
}
typedef volatile struct {
	long l;
	Mutex lock;
} AtomicLongF;
long AtomicLongF_adjust(AtomicLongF* alf, long adj) {// Performs a memory barrier after adjusting, before returning
	Mutex_acquire(&(alf->lock));
	long m = (alf->l);
	(alf->l) += adj;
	Mutex_release(&(alf->lock));
	return m;
}
long AtomicLongF_get(AtomicLongF* alf) {// Performs a memory barrier after loading, before returning
	Mutex_acquire(&(alf->lock));
	long m = (alf->l);
	Mutex_release(&(alf->lock));
	return m;
}
void AtomicLongF_set(AtomicLongF* alf, long val) {// Performs a memory barrier after storing, before returning
	Mutex_acquire(&(alf->lock));
	(alf->l) = val;
	Mutex_release(&(alf->lock));
	return;
}
void AtomicLongF_init(AtomicLongF* alf, long val) {// Performs a memory barrier before returning
	(alf->l) = val;
	Mutex_initUnlocked(&(alf->lock));
	return;
}
void* move(void* dst, const void* buf, size_t count) {
	void* m = dst;
	if (dst < buf) {
		while (count--) {
			*((char*) dst) = *((char*) buf);
			dst = ((char*) dst) + 1;
			buf = ((char*) buf) + 1;
		}
	}
	else if (dst > buf) {
		dst = ((char*) dst) + count;
		buf = ((char*) buf) + count;
		while (count--) {
			dst = ((char*) dst) - 1;
			buf = ((char*) buf) - 1;
			*((char*) dst) = *((char*) buf);
		}
	}
	return m;
}
void* memmove(void* dst, const void* buf, size_t count) {
	return move(dst, buf, count);
}
void* cpy(void* dst, const void* src, size_t count) {
	return move(dst, src, count);
}
void* memcpy(void* dst, const void* src, size_t count) {
	return cpy(dst, src, count);
}
void* set(void* ptr, int val, size_t count) {
	char* n = ptr;
	while (count--) {
		*(n++) = val;
	}
	return ptr;
}
extern void bus_out_u8(unsigned long, u8);
extern void bus_out_u16(unsigned long, u16);
extern void bus_out_u32(unsigned long, u32);
extern u8 bus_in_u8(unsigned long);
extern u16 bus_in_u16(unsigned long);
extern u32 bus_in_u32(unsigned long);
extern void bus_wait(void);
#endif
