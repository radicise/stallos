#ifndef __PERPROCESS_H__
#define __PERPROCESS_H__ 1
#include "types.h"
#include "paging.h"
#include "obj/Map.h"
struct PerThreadgroup {
	pid_t tgid;// DOES NOT CHANGE
	struct MemSpace* mem;// DOES NOT CHANGE
	Mutex breakLock;
	volatile uintptr userBreak;
	struct Map* desctors;// DOES NOT CHANGE; Map, int "descriptor" -> int "description"
	Mutex reapedLock;
	volatile struct rusage reaped;
	Mutex tusageLock;
	volatile struct rusage tusage;
};
#include "threads.h"
#endif
