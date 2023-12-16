#ifndef __FILEDRIVER_H__
#define __FILEDRIVER_H__ 1
struct FileDriver {// "kfd" values instead of "fd" values are passed
	ssize_t (*write)(int, const void*, size_t);
	ssize_t (*read)(int, void*, size_t);
	off_t (*lseek)(int, off_t, int);
	int (*_llseek)(unsigned int, unsigned long, unsigned long, loff_t*, unsigned int);
};// TODO Add the remaining file functions that are wrapped by the system calls
#endif
