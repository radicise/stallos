#ifndef __FILEDRIVER_H__
#define __FILEDRIVER_H__ 1
struct FileDriver {// "kfd" values instead of "fd" values are passed and returned
#ifdef __STALLOS__
	ssize_t (*write)(int, const void*, size_t);
#else
	_kernel_ssize_t (*write)(int, const void*, _kernel_size_t);
#endif
#ifdef __STALLOS__
	ssize_t (*read)(int, void*, size_t);
#else
	_kernel_ssize_t (*read)(int, void*, _kernel_size_t);
#endif
#ifdef __STALLOS__
	off_t (*lseek)(int, off_t, int);
#else
	_kernel_off_t (*lseek)(int, _kernel_off_t, int);
#endif
#ifdef __STALLOS__
	int (*_llseek)(int, off_t, off_t, loff_t*, int);// TODO Can this fail with any error that lseek can fail with? (Currently assuming so)
#else
	int (*_llseek)(int, _kernel_off_t, _kernel_off_t, _kernel_loff_t*, int);// TODO Can this fail with any error that lseek can fail with? (Currently assuming so)
#endif
	int (*fsync)(int);
	int (*fdatasync)(int);
	int (*close)(int);
	void (*closeFinal)(int);// This is invoked after `close', when the last thread doing anything with the file descriptor has finished; this is not necessarily invoked from the same thread that the `close' call was invoked from but is necessarily invoked from the same thread group that the `close' call was invoked from; this is invoked before the "KFD" value is allowed to be reallocated, if the "KFD" value is allowed to be reallocated
	int (*dup)(int);
	int (*dup2)(int, int);
	// TODO add fcntl(), fstat(), fstatfs(), and fdatasync()
	// TODO plan for mmap()
};// TODO Add the remaining file functions that are wrapped by the system calls
/*
 *
 * Source: Linux '/include/uapi/linux/fs.h'
 *
 */
#define _kernel_SEEK_SET 0
#define _kernel_SEEK_CUR 1
#define _kernel_SEEK_END 2
#define _kernel_SEEK_DATA 3
#define _kernel_SEEK_HOLE 4
#ifdef __STALLOS__
#define SEEK_SET _kernel_SEEK_SET
#define SEEK_CUR _kernel_SEEK_CUR
#define SEEK_END _kernel_SEEK_END
#define SEEK_DATA _kernel_SEEK_DATA
#define SEEK_HOLE _kernel_SEEK_HOLE
#endif
/*
 *
 * Source: Linux '/include/uapi/asm-generic-fcntl.h'
 *
 */
#define _kernel_O_RDONLY 0x00000000
#define _kernel_O_WRONLY 0x00000001
#define _kernel_O_RDWR 0x00000002
#ifdef __STALLOS__
#define O_RDONLY _kernel_O_RDONLY
#define O_WRONLY _kernel_O_WRONLY
#define O_RDWR _kernel_O_RDWR
#endif
#endif
