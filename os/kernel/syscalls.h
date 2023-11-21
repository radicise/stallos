#ifndef __SYSCALLS_H__
#define __SYSCALLS_H__ 1
/*
 *
 * System call interface implementation as defined by '/man2' within the Linux man-pages project's source
 *
 */
#include "errno.h"
#include "types.h"
pid_t pid;
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
//TODO Implement all applicable syscalls
unsigned int system_call(unsigned long arg1, unsigned long arg2, unsigned long arg3, unsigned long arg4, unsigned long arg5, unsigned long arg6, unsigned long arg7, unsigned long nr) {// "nr" values are as they are for x86_32 Linux system call numbers; other calls will have "nr" values allocated for them as needed
	pid = (pid_t) 1;//TODO Allow multiple processes
	switch (nr) {
		case (4):
			return write((int) arg1, (const void*) (arg2 + getMemOffset(pid)), (size_t) arg3);
		default:
			bugCheck();// Unrecognised / unimplemented system call
			return 0;
	}
}// TODO Allow returning of values wider than the `int' type
#endif
