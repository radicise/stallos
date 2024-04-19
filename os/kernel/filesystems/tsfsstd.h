#ifndef __TSFSSTD_H__
#define __TSFSSTD_H__ 1
#include "types.h"
extern kuid_t fetch_euid(void);
extern int have_cap(int);
extern void associate_kfd(int, struct FileDriver*, int, int);
extern int reserve_kfd(void);
extern void Mutex_acquire(Mutex* mutex);
extern void Mutex_release(Mutex* mutex);
extern int Mutex_tryAcquire(Mutex* mutex);
void Mutex_initUnlocked(Mutex* mutex);
extern void* alloc(size_t);
extern void* dealloc(volatile void*, size_t);
#endif
