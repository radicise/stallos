#ifndef __SYSCALLS_H__
#define __SYSCALLS_H__ 1
/*
 *
 * System call interface implementation as defined by '/man2' within the Linux man-pages project's source
 *
 */
#include "errno.h"
#include "types.h"
#include "capabilities.h"
#include "perThreadgroup.h"
#include "perThread.h"
#include "Map.h"
#include "kmemman.h"
#include "paging.h"
struct FileKey {
	pid_t tgpid;
	int fd;
};
int FileKeyComparator(uintptr a, uintptr b) {
	struct FileKey* aF = (struct FileKey*) (((struct Map_pair*) a)->key);
	struct FileKey* bF = (struct FileKey*) b;
	return (aF->tgpid != bF->tgpid) || (aF->fd != bF->fd);
}
struct Map* FileKeyKfdMap;
int getDesc(pid_t pIdent, int fd) {
	struct FileKey match;
	match.tgpid = pIdent;
	match.fd = fd;
	uintptr i = Map_findByCompare((uintptr) &match, FileKeyComparator, FileKeyKfdMap);
	if (i == (uintptr) (-1)) {
		return (-1);
	}
	i = Map_fetch(i, FileKeyKfdMap);
	if (i == (uintptr) (-1)) {
		return (-1);
	}
	return i;
}
#include "FileDriver.h"
#include "VGATerminal.h"
#include "ATA.h"
struct Map* kfdDriverMap;
struct FileDriver* resolveFileDriver(int kfd) {
	uintptr drvr = Map_fetch(kfd, kfdDriverMap);
	if (drvr == (uintptr) (-1)) {
		return NULL;
	}
	return (struct FileDriver*) drvr;
}
unsigned int getMemOffset(void) {// TODO Utilise virtual memory pages
	if (tid == ((pid_t) 1)) {// TODO Support for multiple userspace threads
		return 0x800000 - RELOC;
	}
	bugCheck();
	return 0;
}
void initSystemCallInterface(void) {
	kmem_init();
	kfdDriverMap = Map_create();// Map, int "kfd" -> struct FileDriver* "driver"
	Map_add(1, (uintptr) &FileDriver_VGATerminal, kfdDriverMap);// "/dev/console"
	Map_add(2, (uintptr) &FileDriver_ATA, kfdDriverMap);// "/dev/hda"
	Map_add(3, (uintptr) &FileDriver_ATA, kfdDriverMap);// "/dev/hdb"
	Map_add(4, (uintptr) &FileDriver_ATA, kfdDriverMap);// "/dev/hdc"
	Map_add(5, (uintptr) &FileDriver_ATA, kfdDriverMap);// "/dev/hdd"
	FileKeyKfdMap = Map_create();// Map, struct FileKey* "file key" -> int "kfd"
	struct FileKey* k = alloc(sizeof(struct FileKey));
	k->tgpid = 1;
	k->fd = 0;
	Map_add((uintptr) k, 1, FileKeyKfdMap);// "stdin" for `init'
	k = alloc(sizeof(struct FileKey));
	k->tgpid = 1;
	k->fd = 1;
	Map_add((uintptr) k, 1, FileKeyKfdMap);// "stdout" for `init'
	k = alloc(sizeof(struct FileKey));
	k->tgpid = 1;
	k->fd = 2;
	Map_add((uintptr) k, 1, FileKeyKfdMap);// "stderr" for `init'
	k = alloc(sizeof(struct FileKey));
	k->tgpid = 1;
	k->fd = 3;
	Map_add((uintptr) k, 2, FileKeyKfdMap);// "ATA Drive 1" for `init'
	k = alloc(sizeof(struct FileKey));
	k->tgpid = 1;
	k->fd = 4;
	Map_add((uintptr) k, 3, FileKeyKfdMap);// "ATA Drive 2" for `init'
	k = alloc(sizeof(struct FileKey));
	k->tgpid = 1;
	k->fd = 5;
	Map_add((uintptr) k, 4, FileKeyKfdMap);// "ATA Drive 3" for `init'
	k = alloc(sizeof(struct FileKey));
	k->tgpid = 1;
	k->fd = 6;
	Map_add((uintptr) k, 5, FileKeyKfdMap);// "ATA Drive 4" for `init'
	initPaging();
	return;
}
void endingCleanup(void) {
	Map_destroy(FileKeyKfdMap);
	Map_destroy(kfdDriverMap);
	// TODO Check whether the heap has been cleared
	return;
}
void processCleanup(pid_t pIdent) {// To be run at the end of the lifetime of a thread
	// TODO Remove associated entries from `FileKeyKfdMap'
	// TODO Remove entries from `kfdDriverMap' that are not held by other processes and are associated with the `kfd' values that were the values of key-value pairs removed from `FileKeyKfdMap'
	return;
}
int validateCap(int cap) {
	return 1;// TODO Implement
}
/*
 *
 * System call interface
 *
 */
ssize_t write(int fd, const void* buf, size_t count) {
	if (fd < 0) {
		errno = EBADF;
		return (-1);
	}
	int kfd = getDesc(tgid, fd);
	if (kfd == (-1)) {
		errno = EBADF;// File descriptor <fd> is not opened for the process
		return (-1);
	}
	struct FileDriver* driver = resolveFileDriver(kfd);
	if (driver == NULL) {
		bugCheck();
	}
	return driver->write(kfd, buf, count);
}
ssize_t read(int fd, void* buf, size_t count) {
	if (fd < 0) {
		errno = EBADF;
		return (-1);
	}
	int kfd = getDesc(tgid, fd);
	if (kfd == (-1)) {
		errno = EBADF;// File descriptor <fd> is not opened for the process
		return (-1);
	}
	struct FileDriver* driver = resolveFileDriver(kfd);
	if (driver == NULL) {
		bugCheck();
	}
	return driver->read(kfd, buf, count);
}
off_t lseek(int fd, off_t off, int how) {
	if (fd < 0) {
		errno = EBADF;
		return (-1);
	}
	int kfd = getDesc(tgid, fd);
	if (kfd == (-1)) {
		errno = EBADF;// File descriptor <fd> is not opened for the process
		return (-1);
	}
	struct FileDriver* driver = resolveFileDriver(kfd);
	if (driver == NULL) {
		bugCheck();
	}
	return driver->lseek(kfd, off, how);
}
int _llseek(int fd, off_t offHi, off_t offLo, loff_t* res, int how) {// Introduced in Linux 1.1.46
	if (fd < 0) {
		errno = EBADF;
		return (-1);
	}
	int kfd = getDesc(tgid, fd);
	if (kfd == (-1)) {
		errno = EBADF;// File descriptor <fd> is not opened for the process
		return (-1);
	}
	struct FileDriver* driver = resolveFileDriver(kfd);
	if (driver == NULL) {
		bugCheck();
	}
	return driver->_llseek(kfd, offHi, offLo, res, how);
}
time_t time(time_t* resAddr) {
	time_t n = timeFetch();
	if (resAddr != NULL) {
		(*resAddr) = n;
	}
	return n;
}
int stime(const time_t* valAddr) {
	if (!(validateCap(CAP_SYS_TIME))) {
		errno = EPERM;
		return (-1);
	}
	timeStore(*valAddr);
	return 0;
}
uidnatural_t getuid(void) {
	return uid32_to_uidnatural(ruid);
}
uid32_t getuid32(void) {// Introduced in Linux 2.3.39
	return ruid;
}
uidnatural_t geteuid(void) {
	return uid32_to_uidnatural(euid);
}
uid32_t geteuid32(void) {// Introduced in Linux 2.3.39
	return euid;
}
pid_t getpid(void) {
	return tgid;
}
pid_t gettid(void) {// Introduced in Linux 2.4.11
	return tid;
}






// TODO Implement all applicable system calls
/*
 * System call appearance history sources:
 * /_?sys_.+/-matching functions in Linux release source
 * Linux man-pages syscalls(2)
 * Linux '/include/linux/sys.h' until 0.99.13k
 * Linux '/kernel/sched.c' from 0.99.13k and onward until 1.1.0
 * Linux '/kernel/sys_call.S' from 1.1.0 and onward until 1.1.52
 * Linux '/arch/i386/entry.S' from 1.1.52 and onward until 1.1.77
 * Linux '/arch/i386/kernel/entry.S' from 1.1.77 and onward until 2.6.12
 * Linux '/arch/i386/kernel/syscall_table.S' from 2.6.12 and onward until 2.6.24
 * Linux '/arch/x86/kernel/syscall_table_32.S' from 2.6.24 and onward until 3.3
 */
unsigned long system_call(unsigned long arg1, unsigned long arg2, unsigned long arg3, unsigned long arg4, unsigned long arg5, unsigned long arg6, unsigned long arg7, unsigned long nr) {// "nr" values are as they are for x86_32 Linux system call numbers; other calls will have "nr" values allocated for them as needed
	errno = 0;
	Mutex_acquire(&(PerThread_context->dataLock));
	unsigned long retVal = 0;
	switch (nr) {// TODO Authenticate memory access
		case (3):
			retVal = (unsigned long) read((int) arg1, (void*) (arg2 + getMemOffset()), (size_t) arg3);
			break;
		case (4):
			retVal = (unsigned long) write((int) arg1, (const void*) (arg2 + getMemOffset()), (size_t) arg3);
			break;
		case (13):
			retVal = (unsigned long) time((time_t*) ((arg1 == 0) ? 0 : (arg1 + getMemOffset())));
			break;
		case (19):
			retVal = (unsigned long) lseek((int) arg1, (off_t) arg2, (int) arg3);
			break;
		case (20):
			retVal = (unsigned long) getpid();
			break;
		case (24):
			retVal = (unsigned long) getuid();
			break;
		case (25):
			retVal = (unsigned long) stime((const time_t*) (arg1 + getMemOffset()));
			break;
		case (49):
			retVal = (unsigned long) geteuid();
			break;
#if LINUX_COMPAT_VERSION >= 0x10104600
		case (140):// Prototype is sourced from Linux man-pages lseek64(3)
			retVal = (unsigned long) _llseek((int) arg1, (off_t) arg2, (off_t) arg3, (loff_t*) (arg4 + getMemOffset()), (int) arg5);
			break;
#endif
#if LINUX_COMPAT_VERSION >= 0x20303900
		case (199):
			retVal = (unsigned long) getuid32();
			break;
#endif
#if LINUX_COMPAT_VERSION >= 0x20303900
		case (201):
			retVal = (unsigned long) geteuid32();
			break;
#endif
#if LINUX_COMPAT_VERSION >= 0x20401100
		case (224):
			retVal = (unsigned long) gettid();
			break;
#endif
		default:
			errno = ENOSYS;
			retVal = (-1);
			break;
	}
	Mutex_release(&(PerThread_context->dataLock));
	return retVal;
}// TODO Allow returning of values wider than the `long' type
#endif
