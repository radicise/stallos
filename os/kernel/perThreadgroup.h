#ifndef __PERPROCESS_H__
#define __PERPROCESS_H__ 1
#include "types.h"
int errno = 0;// TODO Make per-thread
// TODO Make the following fields per-process
pid_t pid = 1;
uid32_t ruid = 0;
uid32_t euid = 0;
uid32_t suid = 0;
uid32_t fsuid = 0;
#endif
