#ifndef __FILEDRIVER_H__
#define __FILEDRIVER_H__ 1
struct FileDriver {// "kfd" values instead of "fd" values are passed
	int (*write)(int, const void*, size_t);
};// TODO Add the file functions which are wrapped by the syscalls
#endif
