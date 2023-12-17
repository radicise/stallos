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
#include "perProcess.h"
/*
 *
 * Reserved "kfd" values:
 * 1: /dev/console
 * 2: "/dev/hda" (this device file only exists if the device is present on the system)
 * 3: "/dev/hdb" (this device file only exists if the device is present on the system)
 * 5: "/dev/hdc" (this device file only exists if the device is present on the system)
 * 6: "/dev/hdd" (this device file only exists if the device is present on the system)
 *
 */
int getDesc(int fd) {
	switch (fd) {
		case (0):
		case (1):
		case (2):
			return 1;
		case (3):
		case (4):
		case (5):
		case (6):
			return fd - 1;
		default:
			return (-1);
	}
}
#include "FileDriver.h"
// File drivers
#include "VGATerminal.h"
struct FileDriver* resolveFileDriver(int kfd) {
	switch (kfd) {
		case (1):
			return &FileDriver_VGATerminal;
		case (2):
		case (3):
		case (4):
		case (5):
			return &FileDriver_ATA;
		default:
			return NULL;
	}
}
unsigned int getMemOffset(pid_t pid) {
	if (pid == ((pid_t) 1)) {
		return 0x800000 - RELOC;
	}
	bugCheck();
	return 0;
}
/*
 *
 * NOTES:
 *
 * On entry:
 * - Set `pid'
 *
 * "kfd" values:
 * 0 - Main terminal input
 * 1 - Main terminal output
 *
 */
int validateCap(int cap) {
	return 1;// TODO Implement
}
ssize_t write(int fd, const void* buf, size_t count) {
	if (fd < 0) {
		errno = EBADF;
		return -1;
	}
	int kfd = getDesc(fd);
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
	int kfd = getDesc(fd);
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
	int kfd = getDesc(fd);
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
	int kfd = getDesc(fd);
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
// TODO Implement all applicable syscalls
unsigned long system_call(unsigned long arg1, unsigned long arg2, unsigned long arg3, unsigned long arg4, unsigned long arg5, unsigned long arg6, unsigned long arg7, unsigned long nr) {// "nr" values are as they are for x86_32 Linux system call numbers; other calls will have "nr" values allocated for them as needed
	pid = (pid_t) 1;// TODO Allow multiple processes
	switch (nr) {// TODO Authenticate memory access
		case (3):
			return (unsigned long) read((int) arg1, (void*) (arg2 + getMemOffset(pid)), (size_t) arg3);
		case (4):
			return (unsigned long) write((int) arg1, (const void*) (arg2 + getMemOffset(pid)), (size_t) arg3);
		case (13):
			return (unsigned long) time((time_t*) (arg1 + getMemOffset(pid)));
		case (19):
			return (unsigned long) lseek((int) arg1, (off_t) arg2, (int) arg3);
		case (25):
			return (unsigned long) stime((const time_t*) (arg1 + getMemOffset(pid)));
		case (140):// Prototype is sourced from man-pages lseek64(3)
			return (unsigned long) _llseek((int) arg1, (off_t) arg2, (off_t) arg3, (loff_t*) (arg4 + getMemOffset(pid)), (int) arg5);
		default:
			bugCheck();// Unrecognised / unimplemented system call
			return 0;
	}
}// TODO Allow returning of values wider than the `long' type
#endif
