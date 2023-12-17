#ifndef __PERPROCESS_H__
#define __PERPROCESS_H__ 1
#include "types.h"
int errno = 0;// TODO Make per-thread
pid_t pid = 0;
uid32_t ruid = 0;
uid32_t euid = 0;
uid32_t suid = 0;
uid32_t fsuid = 0;
#endif
