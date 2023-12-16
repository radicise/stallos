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
int getDesc(int fd) {
	if ((fd < 0) || (fd > 2)) {
		return -1;
	}
	if (fd == 2) {
		return 1;
	}
	return fd;
}
#include "FileDriver.h"
// File drivers
#include "VGATerminal.h"
struct FileDriver* resolveFileDriver(int kfd) {
	if ((kfd < 0) || (kfd > 2)) {
		return NULL;
	}// TODO Implement "kfd"-driver mapping
	return &FileDriver_VGATerminal;
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
		case (25):
			return (unsigned long) stime((const time_t*) (arg1 + getMemOffset(pid)));
		default:
			bugCheck();// Unrecognised / unimplemented system call
			return 0;
	}
}// TODO Allow returning of values wider than the `int' type
#endif
