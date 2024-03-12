#ifndef __FSIFACE_H__
#define __FSIFACE_H__ 1
struct FSReturn {// `errno' == 0: Success; `errno' != 0: Failure
	void* retptr;// Pointer to object, if `errno' == 0 and return type is not a pointer
	ssize_t retval;// Value, if `errno' == 0 and the return type is not a pointer
	int errno;// Error number if not zero, uses standard error number definition
};
struct FSInterface {// Concurrency support not guaranteed
	struct FSReturn (*FSInit)(struct FileDriver*, int, loff_t);// Initialises for the filesystem instance; Arguments: ("partition", "kfd", "size"); Return: "fs"
	void (*FSClose)(void*);// Closes the filesystem instance, deallocates the filesystem instance, flushes data to underlying partition file; Arguments: ("fs"); Does not fail
	struct FSReturn (*open)(const char*, int, mode_t, void*);// Opens the file at "fileName"; Arguments: ("fileName", "flags", "mode", "fs"); Returns: "return"
	struct FSReturn (*read)(void*, void*, void*, size_t);// As read(2) except that on error the "errno" changed is the `errno' in the `struct FSReturn' and not the `errno' accessed by the main kernel; Arguments: ("fs", "file", "buf", "amnt");
	struct FSReturn (*write)(void*, void*, const void*, size_t);// As write(2) except that on error the "errno" changed is the `errno' in the `struct FSReturn' and not the `errno' accessed by the main kernel; Arguments: ("fs", "file", "buf", "amnt");
};
#endif
