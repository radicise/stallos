#ifndef __FILEDRIVER_H__
#define __FILEDRIVER_H__ 1
struct FileDriver {// "kfd" values instead of "fd" values are passed
	ssize_t (*write)(int, const void*, size_t);
	ssize_t (*read)(int, void*, size_t);
	off_t (*lseek)(int, off_t, int);
	int (*_llseek)(int, off_t, off_t, loff_t*, int);// TODO Can this fail with any error that lseek can fail with? (Currently assuming so)
	int (*fync)(int);
	int (*fdatasync)(int);
	int (*close)(int);
	void (*closeFinal)(int);// This is invoked after `close', when the last thread doing anything with the file descriptor has finished; this is not necessarily invoked from the same thread that the `close' call was invoked from but is necessarily invoked from the same thread group that the `close' call was invoked from; this is invoked before the "KFD" value is allowed to be reallocated
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
/*
 *
 * Source: Linux '/include/uapi/asm-generic-fcntl.h'
 *
 */
#define O_RDONLY 0x00000000
#define O_WRONLY 0x00000001
#define O_RDWR 0x00000002
#endif
#endif
