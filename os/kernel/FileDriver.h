#ifndef __FILEDRIVER_H__
#define __FILEDRIVER_H__ 1
struct FileDriver {// "kfd" values instead of "fd" values are passed
	ssize_t (*write)(int, const void*, size_t);
	ssize_t (*read)(int, void*, size_t);
	off_t (*lseek)(int, off_t, int);
	int (*_llseek)(int, off_t, off_t, loff_t*, int);// TODO Can this fail with any error that lseek can fail with? (Currently assuming so)
	int (*fync)(int);
	int (*fdatasync)(int);
};// TODO Add the remaining file functions that are wrapped by the system calls
#ifdef __STALLOS__
/*
 *
 * Source: Linux '/include/uapi/linux/fs.h'
 *
 */
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2
#define SEEK_DATA 3
#define SEEK_HOLE 4
#endif
#endif
