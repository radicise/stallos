#ifndef __FSIFACE_H__
#define __FSIFACE_H__ 1
struct FSReturn {// `errno' == 0: Success; `errno' != 0: Failure
	void* retptr;// Pointer to object, if `errno' == 0 and return type is a pointer
	ssize_t retval;// Value, if `errno' == 0 and the return type is not a pointer
	int err;// Error number if not zero, uses the same error number values as the kernel-userspace API
};
struct FSInterface {// Concurrency support is guaranteed(see note); `errno' in the `struct FSReturn' is used instead of the thread's `errno'
	struct FSReturn (*FSInit)(struct FileDriver*, int, loff_t);// Initialises for the filesystem instance; Arguments: ("partition", "kfd", "size"); Return: "fs"
	void (*FSClose)(void*);// Closes the filesystem instance, deallocates the filesystem instance, flushes data to underlying partition file; Arguments: ("fs"); Does not fail
	struct FSReturn (*open)(const char*, int, mode_t, void*);// Arguments: ("fileName", "flags", "mode", "fs"); Returns: "return"; The path passed is guaranteed to be relative to the mount point; The non-error return value is `int' "kfd"
	struct FSReturn (*chmod)(const char*, mode_t);// Arguments: ("path", "mode"); Returns: "return"; The path passed is guaranteed to be relative to the mount point
	struct FSReturn (*link)(const char*, const char*);// Arguments: ("old", "new"); Returns: "return"; The paths passed are guaranteed to be relative to the mount point
};
/*
NOTE:
concurrency won't be real, instead each FS syscall that could break if called concurrently will
uses mutexes to ensure that does not happen
- Tristan
*/
#endif
