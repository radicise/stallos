#ifndef __KFD_H__
#define __KFD_H__ 1
#define KFD_NONRESBASE 6
#include "types.h"
#include "FileDriver.h"
struct FDMap {
	pid_t pid;
	int fd;
	int kfd;
};
#define KFD_BOUND 32
#define FD_OPENS 32




#endif
