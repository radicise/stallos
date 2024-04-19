#ifndef __TYPES_H__
#define __TYPES_H__ 1
#include "ktypes.h"
#ifndef TARGETNUM
#error "'TARGETNUM' is not set"
#endif
#if TARGETNUM == 1
#include "machine/x86_32/types.h"
#elif TARGETNUM == 2
#include "machine/x86_64/types.h"
#elif TARGETNUM == 3
#include "machine/arm64_LP64/types.h"
#else
#error "Target is not supported"
#endif
/*
 *
 * Definitions for Linux kernel-userspace interface types
 *
 */
typedef _kernel_size_t size_t;
typedef _kernel_ssize_t ssize_t;
typedef _kernel_pid_t pid_t;
typedef _kernel_time_t time_t;
typedef _kernel_time64_t time64_t;
typedef _kernel_off_t off_t;
typedef _kernel_loff_t loff_t;
typedef _kernel_uidnatural_t uidnatural_t;
typedef _kernel_kuid_t kuid_t;
typedef _kernel_mode_t mode_t;
typedef _kernel_kdev_t kdev_t;
typedef _kernel_udev_old_t udev_old_t;
typedef _kernel_udev_new_t udev_new_t;
/*
 *
 * Other
 *
 */
typedef _kernel_SimpleMutex SimpleMutex;
typedef _kernel_AtomicULong AtomicULong;
typedef volatile struct {
	SimpleMutex stateLock;
	pid_t ownerThread;
	unsigned long acquires;
} Mutex;// Reentrant, acquisition even if it is already acquired by the thread also acts as a memory fence for the thread
#endif
