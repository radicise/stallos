#ifndef __KTYPES_H__
#define __KTYPES_H__ 1
/*
 * `uintptr' must be of the same width as `void*'
 */
#ifndef TARGETNUM
#error "'TARGETNUM' is not set"
#endif
#if TARGETNUM == 1
#include "machine/x86_32/ktypes.h"
#elif TARGETNUM == 2
#include "machine/x86_64/ktypes.h"
#elif TARGETNUM == 3
#include "machine/arm64_LP64/ktypes.h"
#else
#error "Target is not supported"
#endif
typedef volatile struct {
	_kernel_SimpleMutex stateLock;
	_kernel_pid_t ownerThread;
	unsigned long acquires;
} _kernel_Mutex;
#endif
