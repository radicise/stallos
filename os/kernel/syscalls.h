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
	Map_add((uintptr) k, 1, FileKeyKfdMap);// "stdin" for the userspace boot process
	k = alloc(sizeof(struct FileKey));
	k->tgpid = 1;
	k->fd = 1;
	Map_add((uintptr) k, 1, FileKeyKfdMap);// "stdout" for the userspace boot process
	k = alloc(sizeof(struct FileKey));
	k->tgpid = 1;
	k->fd = 2;
	Map_add((uintptr) k, 1, FileKeyKfdMap);// "stderr" for the userspace boot process
	k = alloc(sizeof(struct FileKey));
	k->tgpid = 1;
	k->fd = 3;
	Map_add((uintptr) k, 2, FileKeyKfdMap);// "ATA Drive 1" for the userspace boot process
	k = alloc(sizeof(struct FileKey));
	k->tgpid = 1;
	k->fd = 4;
	Map_add((uintptr) k, 3, FileKeyKfdMap);// "ATA Drive 2" for the userspace boot process
	k = alloc(sizeof(struct FileKey));
	k->tgpid = 1;
	k->fd = 5;
	Map_add((uintptr) k, 4, FileKeyKfdMap);// "ATA Drive 3" for the userspace boot process
	k = alloc(sizeof(struct FileKey));
	k->tgpid = 1;
	k->fd = 6;
	Map_add((uintptr) k, 5, FileKeyKfdMap);// "ATA Drive 4" for the userspace boot process
	return;
}
void endingCleanup(void) {
	Map_destroy(FileKeyKfdMap);
	Map_destroy(kfdDriverMap);
	// TODO Check whether the heap has been cleared
	return;
}
void processCleanup(pid_t pIdent) {// To be run at the end of the lifetime of a process
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
void _exit(int status) {
	bugCheck();// TODO Implement
	// if this is the last thread of the thread group,
		// reparent children
		// send SIGCHLD to parent
		// uhh
	// uhh



}
ssize_t write(int fd, const void* buf, size_t count) {
	if (fd < 0) {
		errno = EBADF;
		return -1;
	}
	int kfd = getDesc(tgid, fd);
	if (kfd == (-1)) {
		errno = EBADF;// File descriptor <fd> is not opened for the process
		return -1;
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
		return -1;
	}
	int kfd = getDesc(tgid, fd);
	if (kfd == (-1)) {
		errno = EBADF;// File descriptor <fd> is not opened for the process
		return -1;
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
		return -1;
	}
	int kfd = getDesc(tgid, fd);
	if (kfd == (-1)) {
		errno = EBADF;// File descriptor <fd> is not opened for the process
		return -1;
	}
	struct FileDriver* driver = resolveFileDriver(kfd);
	if (driver == NULL) {
		bugCheck();
	}
	return driver->lseek(kfd, off, how);
}
int _llseek(int fd, off_t offHi, off_t offLo, loff_t* res, int how) {
	if (fd < 0) {
		errno = EBADF;
		return -1;
	}
	int kfd = getDesc(tgid, fd);
	if (kfd == (-1)) {
		errno = EBADF;// File descriptor <fd> is not opened for the process
		return -1;
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
		return -1;
	}
	timeStore(*valAddr);
	return 0;
}
uidnatural_t getuid(void) {
	return uid32_to_uidnatural(ruid);
}
uid32_t getuid32(void) {
	return ruid;
}
uidnatural_t geteuid(void) {
	return uid32_to_uidnatural(euid);
}
uid32_t geteuid32(void) {
	return euid;
}
pid_t getpid(void) {
	return tgid;
}
pid_t gettid(void) {
	return tid;
}
// TODO Implement all applicable system calls
unsigned long system_call(unsigned long arg1, unsigned long arg2, unsigned long arg3, unsigned long arg4, unsigned long arg5, unsigned long arg6, unsigned long arg7, unsigned long nr) {// "nr" values are as they are for x86_32 Linux system call numbers; other calls will have "nr" values allocated for them as needed
	errno = 0;
	switch (nr) {// TODO Authenticate memory access
		case (1):
			_exit((int) arg1);
			bugCheck();
		case (3):
			return (unsigned long) read((int) arg1, (void*) (arg2 + getMemOffset()), (size_t) arg3);
		case (4):
			return (unsigned long) write((int) arg1, (const void*) (arg2 + getMemOffset()), (size_t) arg3);
		case (13):
			return (unsigned long) time((time_t*) ((arg1 == 0) ? 0 : (arg1 + getMemOffset())));
		case (19):
			return (unsigned long) lseek((int) arg1, (off_t) arg2, (int) arg3);
		case (20):
			return (unsigned long) getpid();
		case (24):
			return (unsigned long) getuid();
		case (25):
			return (unsigned long) stime((const time_t*) (arg1 + getMemOffset()));
		case (49):
			return (unsigned long) geteuid();
		case (140):// Prototype is sourced from man-pages lseek64(3)
			return (unsigned long) _llseek((int) arg1, (off_t) arg2, (off_t) arg3, (loff_t*) (arg4 + getMemOffset()), (int) arg5);
		case (199):
			return (unsigned long) getuid32();
		case (201):
			return (unsigned long) geteuid32();
		case (224):
			return (unsigned long) gettid();
		default:// TODO Do not allow the system to crash upon the provision of non-existent system call numbers
			bugCheckNum(0xABADCA11);// Unrecognised / unimplemented system call ("A BAD CALL")
			return 0;
	}
}// TODO Allow returning of values wider than the `long' type
#endif
