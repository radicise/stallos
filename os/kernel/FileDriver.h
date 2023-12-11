#ifndef __FILEDRIVER_H__
#define __FILEDRIVER_H__ 1
struct FileDriver {// "kfd" values instead of "fd" values are passed
	ssize_t (*write)(int, const void*, size_t);
	ssize_t (*read)(int, void*, size_t);
};// TODO Add the remaining file functions that are wrapped by the system calls
#endif
