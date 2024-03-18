#ifndef __FSIFACE_H__
#define __FSIFACE_H__ 1
struct FSReturn {// `errno' == 0: Success; `errno' != 0: Failure
	void* retptr;// Pointer to object, if `errno' == 0 and return type is a pointer
	ssize_t retval;// Value, if `errno' == 0 and the return type is not a pointer
	int err;// Error number if not zero, uses the same error number values as the kernel-userspace API
};
struct FSInterface {// Concurrency support is guaranteed
	struct FSReturn (*FSInit)(struct FileDriver*, int, loff_t);// Initialises for the filesystem instance; Arguments: ("partition", "kfd", "size"); Return: "fs"
	void (*FSClose)(void*);// Closes the filesystem instance, deallocates the filesystem instance, flushes data to underlying partition file; Arguments: ("fs"); Does not fail
	struct FSReturn (*open)(const char*, int, mode_t, void*);// Opens the file at "fileName"; Arguments: ("fileName", "flags", "mode", "fs"); Returns: "return"
};
#endif
