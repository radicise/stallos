#ifndef __TSFSSTD_H__
#define __TSFSSTD_H__ 1
#define __THREADS_H__ 1
#ifndef TARGETNUM
#define TARGETNUM 2
#endif
#include "../types.h"
#include "../perThread.h"
extern struct PerThread* volatile PerThread_context;
struct FileDriver;
extern kuid_t fetch_euid(void);
extern int have_cap(int);
extern void associate_kfd(int, struct FileDriver*, int, int);
extern int reserve_kfd(void);
extern int reserve_userFD(void);
extern void Mutex_acquire(Mutex* mutex);
extern void Mutex_release(Mutex* mutex);
extern int Mutex_tryAcquire(Mutex* mutex);
void Mutex_initUnlocked(Mutex* mutex);
extern void* alloc(size_t);
extern void dealloc(volatile void*, size_t);
extern int strcmp(const char*, const char*);
extern void* memmove(void*, const void*, size_t);
extern void* memcpy(void*, const void*, size_t);
extern void* set(void*, int, size_t);
extern void bus_out_u8(unsigned long, u8);
extern void bus_out_u16(unsigned long, u16);
extern void bus_out_u32(unsigned long, u32);
extern u8 bus_in_u8(unsigned long);
extern u16 bus_in_u16(unsigned long);
extern u32 bus_in_u32(unsigned long);
extern void bugCheckNum(unsigned long);
extern int kernelWarnMsg(const char*);
extern int kernelMsg(const char*);
extern int kernelMsgULong_hex(unsigned long);
extern time_t fetch_time(time_t*);
#endif
