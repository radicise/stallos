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
struct _kernel_oldold_utsname {
	char sysname[9];
	char nodename[9];
	char release[9];
	char version[9];
	char machine[9];
};
struct _kernel_old_utsname {
	char sysname[65];
	char nodename[65];
	char release[65];
	char version[65];
	char machine[65];
};
struct _kernel_new_utsname {
	char sysname[65];
	char nodename[65];
	char release[65];
	char version[65];
	char machine[65];
	char domainname[65];
};
#endif
