#ifndef __SCHED_H__
#define __SCHED_H__ 1
#include "threads.h"
#include "types.h"
void Scheduler_clearRusage(volatile struct rusage* usage) {
	setExv(usage, 0x00, sizeof(struct rusage));
	usage->ru_utime.tv_sec = 0;// TODO Track
	usage->ru_utime.tv_usec = 0;// TODO Track
	usage->ru_stime.tv_sec = 0;// TODO Track
	usage->ru_stime.tv_usec = 0;// TODO Track
	usage->ru_maxrss = 0;// TODO Track when Linux compat gteq 2.6.32
	usage->ru_ixrss = 0;
	usage->ru_idrss = 0;
	usage->ru_isrss = 0;
	usage->ru_minflt = 0;// TODO Track
	usage->ru_majflt = 0;// TODO Track
	usage->ru_nswap = 0;
	usage->ru_inblock = 0;// TODO Track when Linux compat gteq 2.6.22
	usage->ru_oublock = 0;// TODO Track when Linux compat gteq 2.6.22
	usage->ru_msgsnd = 0;
	usage->ru_msgrcv = 0;
	usage->ru_nsignals = 0;
	usage->ru_nvcsw = 0;// TODO Track when Linux compat gteq 2.6
	usage->ru_nivcsw = 0;// TODO Track when Linux compat gteq 2.6
	return;
}
void Scheduler_yield(int voluntary) {// To only be called from interrupts, when IRQ are disabled
	if (!handlingIRQ) {
		bugCheckNum(0x0101 | FAILMASK_THREADS);
	}
	Threads_nextThread();
	return;
}
void Scheduler_update(void) {
	Scheduler_yield(0);// TODO Maybe make thread yielding timing better
	return;
}
#endif
