#ifndef __PERPROCESS_H__
#define __PERPROCESS_H__ 1
#include "types.h"
#include "paging.h"
struct PerThreadgroup {
	pid_t tgid;
	struct MemSpace* mem;
};
#include "threads.h"
#define tgid (PerThreadgroup_context->tgid)
#endif
