#ifndef __PERPROCESS_H__
#define __PERPROCESS_H__ 1
#include "types.h"
#include "paging.h"
struct PerThreadgroup {
	pid_t tgid;// DOES NOT CHANGE
	struct MemSpace* mem;// DOES NOT CHANGE
	Mutex breakLock;
	uintptr userBreak;
};
#include "threads.h"
#endif
