#ifndef __STD_H__
#define __STD_H__ 1
#include "perThread.h"
#include "util.h"
#define FAILMASK_STD 0x00100000
int have_cap(int cap) {
	return 1;// TODO Implement
}
int thread_has_cap(int cap, pid_t tgpid) {
	return 1;// TODO Implement
}
kuid_t fetch_ruid(void) {
	lockThreadInfo();
	kuid_t uid = ruid;
	unlockThreadInfo();
	return uid;
}
kuid_t fetch_euid(void) {
	lockThreadInfo();
	kuid_t uid = euid;
	unlockThreadInfo();
	return uid;
}
pid_t fetch_tgid(void) {
	return tgid;
}
int reserve_kfd(void) {
	return makeKfd();
}
time_t fetch_time(void) {
	return timeFetch();
}
#include "syscalls.h"
void associate_kfd(int kfd, struct FileDriver* driver, int statusflags, int userspacefd) {
	struct KFDInfo* fi = alloc(sizeof(struct KFDInfo));
	fi->status = statusflags;
	Mutex_initUnlocked(&(fi->dataLock));
	AtomicLongF_init(&(fi->refs), 1);
	if (Map_add(kfd, (uintptr) fi, KFDStatus)) {
		bugCheckNum(0x0001 | FAILMASK_STD);
	}
	if (Map_add(kfd, (uintptr) driver, kfdDriverMap)) {
		 bugCheckNum(0x0002 | FAILMASK_STD);
	}
	struct FileKey* fk = alloc(sizeof(struct FileKey));
	fk->tgpid = fetch_tgid();
	fk->fd = userspacefd;// TODO Ensure that `userspacefd' was not already used, for that process
	if (Map_add((uintptr) fk, kfd, FileKeyKfdMap)) {
		bugCheckNum(0x0003 | FAILMASK_STD);
	}
	return;
}
#endif
