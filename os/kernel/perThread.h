#ifndef __PERTHREAD_H__
#define __PERTHREAD_H__ 1
#include "types.h"
struct PerThread {
	int errno;// This MUST be the first member (in order of member declaration) of `struct PerThread'
	pid_t tid;
	uid32_t ruid;
	uid32_t euid;
	uid32_t suid;
	uid32_t fsuid;
};
#include "threads.h"
#define errno (PerThread_context->errno)
#define tid (PerThread_context->tid)
#define ruid (PerThread_context->ruid)
#define euid (PerThread_context->euid)
#define suid (PerThread_context->suid)
#define fsuid (PerThread_context->fsuid)
#endif
