#ifndef __SYSCALLS_H__
#define __SYSCALLS_H__ 1
/*
 *
 * System call interface implementation as defined by '/man2' within the Linux man-pages project's source
 *
 */
#include "errno.h"
#include "types.h"
#include "capabilities.h"
#include "perThreadgroup.h"
#include "perThread.h"
#include "obj/Map.h"
#include "kmemman.h"
#include "paging.h"
#define FAILMASK_SYSCALLS 0x000e0000
struct kernel_utsname_oldold {
	char os[9];
	char node[9];
	char osrelease[9];
	char machine[9];
};
struct kernel_utsname_old {
	char os[65];
	char node[65];
	char osrelease[65];
	char machine[65];
};
struct kernel_utsname {
	char os[65];
	char node[65];
	char osrelease[65];
	char machine[65];
	char domain[65];
};
#if __TESTING__ == 1
/*
 *
 * System call names as on Linux, with the corresponding x86_32 Linux system call numbers as indices (system calls not present on x86_32 Linux are allocated the same as in `system_call')
 * Sources: '/usr/i686-linux-gnu/include/sys/syscall.h', '/usr/i686-linux-gnu/include/asm/unistd.h'
 *
 */
#if LINUX_COMPAT_VERSION < 0x20112100
const char* callname[] = {"setup" /* 0 */,
#elif LINUX_COMPAT_VERSION < 0x20505100
const char* callname[] = {"NOCALL" /* 0 */,// TODO Make sure that `setup' disappeared at the same time from all machines
#else
const char* callname[] = {"restart_syscall" /* 0 */,
#endif
	"_exit",
	"fork",
	"read",
	"write",
	"open",
	"close",
	"waitpid",
	"creat",
	"link",
	"unlink" /* 10 */,
	"execve",
	"chdir",
	"time",
	"mknod",
	"chmod",
	"lchown",
	"break",
	"oldstat",
	"lseek",
	"getpid" /* 20 */,
	"mount",
	"umount",
	"setuid",
	"getuid",
	"stime",
	"ptrace",
	"alarm",
	"oldfstat",
	"pause",
	"utime" /* 30 */,
	"stty",
	"gtty",
	"access",
	"nice",
	"ftime",
	"sync",
	"kill",
	"rename",
	"mkdir",
	"rmdir" /* 40 */,
	"dup",
	"pipe",
	"times",
	"prof",
	"brk",
	"setgid",
	"getgid",
	"signal",
	"geteuid",
	"getegid" /* 50 */,
	"acct",
	"umount2",
	"lock",
	"ioctl",
	"fcntl",
	"mpx",
	"setpgid",
	"ulimit",
	"oldolduname",
	"umask" /* 60 */,
	"chroot",
	"ustat",
	"dup2",
	"getppid",
	"getpgrp",
	"setsid",
	"sigaction",
	"sgetmask",
	"ssetmask",
	"setreuid" /* 70 */,
	"setregid",
	"sigsuspend",
	"sigpending",
	"sethostname",
	"setrlimit",
	"getrlimit",
	"getrusage",
	"gettimeofday",
	"settimeofday",
	"getgroups" /* 80 */,
	"setgroups",
	"select",
	"symlink",
	"oldlstat",
	"readlink",
	"uselib",
	"swapon",
	"reboot",
	"readdir",
	"mmap" /* 90 */,
	"munmap",
	"truncate",
	"ftruncate",
	"fchmod",
	"fchown",
	"getpriority",
	"setpriority",
	"profil",
	"statfs",
	"fstatfs" /* 100 */,
	"ioperm",
	"socketcall",
	"syslog",
	"setitimer",
	"getitimer",
	"stat",
	"lstat",
	"fstat",
	"olduname",
	"iopl" /* 110 */,
	"vhangup",
	"idle",
	"vm86old",
	"wait4",
	"swapoff",
	"sysinfo",
	"ipc",
	"fsync",
	"sigreturn",
	"clone" /* 120 */,
	"setdomainname",
	"uname",
	"modify_ldt",
	"adjtimex",
	"mprotect",
	"sigprocmask",
	"create_module",
	"init_module",
	"delete_module",
	"get_kernel_syms" /* 130 */,
	"quotactl",
	"getpgid",
	"fchdir",
	"bdflush",
	"sysfs",
	"personality",
	"afs_syscall",
	"setfsuid",
	"setfsgid",
	"_llseek" /* 140 */,
	"getdents",
	"_newselect",
	"flock",
	"msync",
	"readv",
	"writev",
	"getsid",
	"fdatasync",
	"_sysctl",
	"mlock" /* 150 */,
	"munlock",
	"mlockall",
	"munlockall",
	"sched_setparam",
	"sched_getparam",
	"sched_setscheduler",
	"sched_getscheduler",
	"sched_yield",
	"sched_get_priority_max",
	"sched_get_priority_min" /* 160 */,
	"sched_rr_get_interval",
	"nanosleep",
	"mremap",
	"setresuid",
	"getresuid",
	"vm86",
	"query_module",
	"poll",
	"nfsservctl",
	"setresgid" /* 170 */,
	"getresgid",
	"prctl",
	"rt_sigreturn",
	"rt_sigaction",
	"rt_sigprocmask",
	"rt_sigpending",
	"rt_sigtimedwait",
	"rt_sigqueueinfo",
	"rt_sigsuspend",
	"pread64" /* 180 */,
	"pwrite64",
	"chown",
	"getcwd",
	"capget",
	"capset",
	"sigaltstack",
	"sendfile",
	"getpmsg",
	"putpmsg",
	"vfork" /* 190 */,
	"ugetrlimit",
	"mmap2",
	"truncate64",
	"ftruncate64",
	"stat64",
	"lstat64",
	"fstat64",
	"lchown32",
	"getuid32",
	"getgid32" /* 200 */,
	"geteuid32",
	"getegid32",
	"setreuid32",
	"setregid32",
	"getgroups32",
	"setgroups32",
	"fchown32",
	"setresuid32",
	"getresuid32",
	"setresgid32" /* 210 */,
	"getresgid32",
	"chown32",
	"setuid32",
	"setgid32",
	"setfsuid32",
	"setfsgid32",
	"pivot_root",
	"mincore",
	"madvise",
	"getdents64" /* 220 */,
	"fcntl64",
	"NOCALL",
	"NOCALL",
	"gettid",
	"readahead",
	"setxattr",
	"lsetxattr",
	"fsetxattr",
	"getxattr",
	"lgetxattr" /* 230 */,
	"fgetxattr",
	"listxattr",
	"llistxattr",
	"flistxattr",
	"removexattr",
	"lremovexattr",
	"fremovexattr",
	"tkill",
	"sendfile64",
	"futex" /* 240 */,
	"sched_setaffinity",
	"sched_getaffinity",
	"set_thread_area",
	"get_thread_area",
	"io_setup",
	"io_destroy",
	"io_getevents",
	"io_submit",
	"io_cancel",
	"fadvise64" /* 250 */,
	"NOCALL",
	"exit_group",
	"lookup_dcookie",
	"epoll_create",
	"epoll_ctl",
	"epoll_wait",
	"remap_file_pages",
	"set_tid_address",
	"timer_create",
	"timer_settime" /* 260 */,
	"timer_gettime",
	"timer_getoverrun",
	"timer_delete",
	"clock_settime",
	"clock_gettime",
	"clock_getres",
	"clock_nanosleep",
	"statfs64",
	"fstatfs64",
	"tgkill" /* 270 */,
	"utimes",
	"fadvise64_64",
	"vserver",
	"mbind",
	"get_mempolicy",
	"set_mempolicy",
	"mq_open",
	"mq_unlink",
	"mq_timedsend",
	"mq_timedreceive" /* 280 */,
	"mq_notify",
	"mq_getsetattr",
	"kexec_load",
	"waitid",
	"NOCALL",
	"add_key",
	"request_key",
	"keyctl",
	"ioprio_set",
	"ioprio_get" /* 290 */,
	"inotify_init",
	"inotify_add_watch",
	"inotify_rm_watch",
	"migrate_pages",
	"openat",
	"mkdirat",
	"mknodat",
	"fchownat",
	"futimesat",
	"fstatat64" /* 300 */,
	"unlinkat",
	"renameat",
	"linkat",
	"symlinkat",
	"readlinkat",
	"fchmodat",
	"faccessat",
	"pselect6",
	"ppoll",
	"unshare" /* 310 */,
	"set_robust_list",
	"get_robust_list",
	"splice",
	"sync_file_range",
	"tee",
	"vmsplice",
	"move_pages",
	"getcpu",
	"epoll_pwait",
	"utimensat" /* 320 */,
	"signalfd",
	"timerfd_create",
	"eventfd",
	"fallocate",
	"timerfd_settime",
	"timerfd_gettime",
	"signalfd4",
	"eventfd2",
	"epoll_create1",
	"dup3" /* 330 */,
	"pipe2",
	"inotify_init1",
	"preadv",
	"pwritev",
	"rt_tgsigqueueinfo",
	"perf_event_open",
	"recvmmsg",
	"fanotify_init",
	"fanotify_mark",
	"prlimit64" /* 340 */,
	"name_to_handle_at",
	"open_by_handle_at",
	"clock_adjtime",
	"syncfs" /* 344 */};
#define SYSCALL_HIGH 344
#endif
Mutex kfdgenLock;
int kfdNext;
int makeKfd(void) {// TODO Allocate and deallocate `kfd' values properly
	Mutex_acquire(&kfdgenLock);
	int k = kfdNext;
	kfdNext++;
	if (kfdNext < 1) {
		bugCheckNum(0x0001 | FAILMASK_SYSCALLS);
	}
	Mutex_release(&kfdgenLock);
	return k;
}
void removeKfd(int kfd) {// TODO Allocate and deallocate `kfd' values properly
	return;
}
struct FileKey {
	pid_t tgpid;
	int fd;
};
int FileKeyComparator(uintptr a, uintptr b) {
	struct FileKey* aF = (struct FileKey*) (((struct Map_pair*) a)->key);
	struct FileKey* bF = (struct FileKey*) b;
	return (aF->tgpid != bF->tgpid) || (aF->fd != bF->fd);
}
int FileValueComparator(uintptr a, uintptr b) {
	return !((((struct Map_pair*) a)->value) == b);
}
struct Map* FileKeyKfdMap;
struct KFDInfo {
	Mutex dataLock;
	int status;
	AtomicLongF refs;
};
struct Map* KFDStatus;
int getDesc(pid_t pIdent, int fd) {
	struct FileKey match;
	match.tgpid = pIdent;
	match.fd = fd;
	Mutex_acquire(&(FileKeyKfdMap->set->lock));
	uintptr i = Map_findByCompare((uintptr) &match, FileKeyComparator, FileKeyKfdMap);
	if (i == (uintptr) (-1)) {
		return (-1);
	}
	i = Map_fetch(i, FileKeyKfdMap);
	if (i == (uintptr) (-1)) {
		bugCheckNum(0x0002 | FAILMASK_SYSCALLS);
	}
	uintptr st = Map_fetch(i, KFDStatus);
	if (st == (uintptr) (-1)) {
		bugCheckNum(0x0003 | FAILMASK_SYSCALLS);
	}
	AtomicLongF_adjust(&(((struct KFDInfo*) st)->refs), 1);
	Mutex_release(&(FileKeyKfdMap->set->lock));
	return i;
}
#include "FileDriver.h"
#include "fsiface.h"
#include "driver/VGATerminal.h"
#include "driver/ATA.h"
struct Map* kfdDriverMap;
struct {
	struct FSInterface* fs;
	void* obj;
} FSDesc;
struct Map* FSDriverMap;
struct FileDriver* resolveFileDriver(int kfd) {
	uintptr drvr = Map_fetch(kfd, kfdDriverMap);
	if (drvr == (uintptr) (-1)) {
		return NULL;
	}
	return (struct FileDriver*) drvr;
}
void retDesc(int kfd) {
	uintptr st = Map_fetch(kfd, KFDStatus);
	if (st == (uintptr) (-1)) {
		bugCheckNum(0x0004 | FAILMASK_SYSCALLS);
	}
	Mutex_acquire(&(FileKeyKfdMap->set->lock));
	long r = AtomicLongF_adjust(&(((struct KFDInfo*) st)->refs), (-1));
	if (!r) {
		uintptr i = Map_findByCompare(kfd, FileValueComparator, FileKeyKfdMap);
		if (i == (uintptr) (-1)) {
			bugCheckNum(0x0008 | FAILMASK_SYSCALLS);
		}
		if (Map_remove(i, FileKeyKfdMap)) {
			bugCheckNum(0x0009 | FAILMASK_SYSCALLS);
		}
		dealloc((void*) i, sizeof(struct FileKey));
		Mutex_release(&(FileKeyKfdMap->set->lock));
		struct FileDriver* fdrvr = resolveFileDriver(kfd);
		if (fdrvr == NULL) {
			bugCheckNum(0x0005 | FAILMASK_SYSCALLS);
		}
		(fdrvr->closeFinal)(kfd);
		if (Map_remove(kfd, kfdDriverMap)) {
			bugCheckNum(0x0006 | FAILMASK_SYSCALLS);
		}
		dealloc((void*) (struct KFDInfo*) st, sizeof(struct KFDInfo));
		if (Map_remove(kfd, KFDStatus)) {
			bugCheckNum(0x0007 | FAILMASK_SYSCALLS);
		}
		removeKfd(kfd);
	}
	else {
		Mutex_release(&(FileKeyKfdMap->set->lock));
	}
	return;
}
void* getUserMem(unsigned long upm, uintptr len, int implyReadable, int implyWritable) {
	if (!len) {
		return getUserMem(upm, 1, implyReadable, implyWritable);// TODO Is this correct? Moreover, is this correct specifically in the cases of calls to `read' and `write'?
	}
	uintptr uptr = (uintptr) upm;
	if (!(implyReadable | implyWritable)) {
		bugCheckNum(0x000f | FAILMASK_SYSCALLS);// TODO Are there any system calls that accept any memory pointers but do not require the userspace program to have either read or write rights for associated memory?
	}
	uintptr poff = uptr - PAGEOF(uptr);
	uptr -= poff;
	len += poff;
	uintptr lm = len & PAGE_LOMASK;
	if (lm) {
		len ^= lm;
		len += PAGE_SIZE;
	}
	lm = uptr;
	uintptr ln = len;
	while (ln) {
		if (pinPage(lm, usermem, implyReadable, implyWritable)) {
			while (lm != uptr) {
				lm -= PAGE_SIZE;
				unpinPage(lm, usermem);
			}
			errno = EFAULT;
			return NULL;
		}
		lm += PAGE_SIZE;
		ln -= PAGE_SIZE;
	}
	void* memArea = dedic_argblock(len);
	uintptr mA = (uintptr) memArea;
	ln = len;
	while (ln) {
		if (mapPage(mA, implyReadable ? (alloc_lb()) : (alloc_lb_wiped()), 0, 0, MemSpace_kernel)) {// TODO Decide whether driver implementations of system calls should be entrusted with ensuring that no kernel memory remnants are accessible to the userspace program and thus not need "wiping" of the allocated page backing memory when the page is implied to be readable
			bugCheckNum(0x000a | FAILMASK_SYSCALLS);
		}
		ln -= PAGE_SIZE;
		mA += PAGE_SIZE;
	}
	if (implyReadable) {
		ln = len;
		lm = uptr;
		void* mea = memArea;
		while (ln) {
			if (!(pageExists(lm, usermem))) {
				bugCheckNum(0x000b | FAILMASK_SYSCALLS);
			}
			void* backing = pageMapping(lm, usermem);
			moveExv(mea, backing, PAGE_SIZE);
			ln -= PAGE_SIZE;
			lm += PAGE_SIZE;
			mea = (void*) (((char*) mA) + PAGE_SIZE);
		}
	}
	return (void*) (((char*) memArea) + poff);
}
void retUserMem(void* mem, unsigned long upm, uintptr olen, uintptr rlen) {
	if (!olen) {
		retUserMem(mem, upm, 1, rlen);
		return;
	}
	uintptr uptr = upm;
	uintptr poff = uptr - PAGEOF(uptr);
	uintptr pmem = ((uintptr) mem) - poff;
	uintptr plen = olen + poff;
	uintptr lm = plen & PAGE_LOMASK;
	if (lm) {
		plen ^= lm;
		plen += PAGE_SIZE;
	}
	if (rlen) {
		void* mt = mem;
		uintptr upt = uptr;
		while (1) {
			if (!(pageExists(PAGEOF(upt), usermem))) {
				bugCheckNum(0x000e | FAILMASK_SYSCALLS);
			}
			void* upm = pageMapping(PAGEOF(upt), usermem);
			uintptr lfp = upt & PAGE_LOMASK;
			if ((rlen + lfp) <= ((uintptr) PAGE_SIZE)) {
				moveExv((void*) (((char*) upm) + lfp), mt, rlen);
				break;
			}
			else {
				moveExv((void*) (((char*) upm) + lfp), mt, PAGE_SIZE - lfp);
				mt = (void*) (((char*) mt) + (PAGE_SIZE - lfp));
				upt += PAGE_SIZE - lfp;
				rlen -= PAGE_SIZE - lfp;
			}
		}
	}
	uintptr amem = pmem;
	uptr -= poff;
	lm = plen;
	while (lm) {
		unpinPage(uptr, usermem);
		if (!(pageExists(amem, MemSpace_kernel))) {
			bugCheckNum(0x000c | FAILMASK_SYSCALLS);
		}
		void* mm = pageMapping(amem, MemSpace_kernel);
		if (unmapPage(amem, MemSpace_kernel)) {
			bugCheckNum(0x000d | FAILMASK_SYSCALLS);
		}
		dealloc_lb(mm);
		lm -= PAGE_SIZE;
		amem += PAGE_SIZE;
		uptr += PAGE_SIZE;
	}
	undedic_argblock((void*) (((char*) mem) - poff), plen);
	return;
}
void initSystemCallInterface(void) {
	kmem_init();
	kfdDriverMap = Map_create();// Map, int "kfd" -> struct FileDriver* "driver"
	FSDriverMap = Map_create();// Map, const char* "Filesystem name" -> struct FSDesc* "Filesystem driver"; "Filesystem" only ends with '/' if it is "/"
	KFDStatus = Map_create();// Map, int "kfd" -> struct KFDInfo* "status"
	Map_add(1, (uintptr) &FileDriver_VGATerminal, kfdDriverMap);// "/dev/tty1"
	Map_add(2, (uintptr) &FileDriver_ATA, kfdDriverMap);// "/dev/hda"
	Map_add(3, (uintptr) &FileDriver_ATA, kfdDriverMap);// "/dev/hdb"
	Map_add(4, (uintptr) &FileDriver_ATA, kfdDriverMap);// "/dev/hdc"
	Map_add(5, (uintptr) &FileDriver_ATA, kfdDriverMap);// "/dev/hdd"
	FileKeyKfdMap = Map_create();// Map, struct FileKey* "file key" -> int "kfd"
	struct KFDInfo* l = alloc(sizeof(struct KFDInfo));
	l->status = O_RDWR;
	Mutex_initUnlocked(&(l->dataLock));
	AtomicLongF_init(&(l->refs), 0);
	Map_add(1, (uintptr) l, KFDStatus);
	struct FileKey* k = alloc(sizeof(struct FileKey));
	k->tgpid = 1;
	k->fd = 0;
	Map_add((uintptr) k, 1, FileKeyKfdMap);// "stdin" for `init'
	k = alloc(sizeof(struct FileKey));
	k->tgpid = 1;
	k->fd = 1;
	Map_add((uintptr) k, 1, FileKeyKfdMap);// "stdout" for `init'
	k = alloc(sizeof(struct FileKey));
	k->tgpid = 1;
	k->fd = 2;
	Map_add((uintptr) k, 1, FileKeyKfdMap);// "stderr" for `init'
	l = alloc(sizeof(struct KFDInfo));
	l->status = O_RDWR;
	Mutex_initUnlocked(&(l->dataLock));
	AtomicLongF_init(&(l->refs), 0);
	Map_add(2, (uintptr) l, KFDStatus);
	k = alloc(sizeof(struct FileKey));
	k->tgpid = 1;
	k->fd = 3;
	Map_add((uintptr) k, 2, FileKeyKfdMap);// "ATA Drive 1" for `init'
	l = alloc(sizeof(struct KFDInfo));
	l->status = O_RDWR;
	Mutex_initUnlocked(&(l->dataLock));
	AtomicLongF_init(&(l->refs), 0);
	Map_add(3, (uintptr) l, KFDStatus);
	k = alloc(sizeof(struct FileKey));
	k->tgpid = 1;
	k->fd = 4;
	Map_add((uintptr) k, 3, FileKeyKfdMap);// "ATA Drive 2" for `init'
	l = alloc(sizeof(struct KFDInfo));
	l->status = O_RDWR;
	Mutex_initUnlocked(&(l->dataLock));
	AtomicLongF_init(&(l->refs), 0);
	Map_add(4, (uintptr) l, KFDStatus);
	k = alloc(sizeof(struct FileKey));
	k->tgpid = 1;
	k->fd = 5;
	Map_add((uintptr) k, 4, FileKeyKfdMap);// "ATA Drive 3" for `init'
	l = alloc(sizeof(struct KFDInfo));
	l->status = O_RDWR;
	Mutex_initUnlocked(&(l->dataLock));
	AtomicLongF_init(&(l->refs), 0);
	Map_add(5, (uintptr) l, KFDStatus);
	k = alloc(sizeof(struct FileKey));
	k->tgpid = 1;
	k->fd = 6;
	Map_add((uintptr) k, 5, FileKeyKfdMap);// "ATA Drive 4" for `init'
	kfdNext = 6;
	Mutex_initUnlocked(&kfdgenLock);
	initPaging();
	initSegmentation();
	return;
}
void endingCleanup(void) {
	Map_destroy(FileKeyKfdMap);
	Map_destroy(kfdDriverMap);
	// TODO Check whether the heap has been cleared
	return;
}
void processCleanup(pid_t pIdent) {// To be run at the end of the lifetime of a thread
	// TODO Remove associated entries from `FileKeyKfdMap'
	// TODO Remove entries from `kfdDriverMap' that are not held by other processes and are associated with the `kfd' values that were the values of key-value pairs removed from `FileKeyKfdMap'
	return;
}



#define FAILMASK_MOUNTINGTSFS 0x00ff0000
extern struct FSInterface FS_TSFS;
extern struct FSReturn tsfs_fsinit(struct FileDriver*, int, loff_t);
void testing_mount_tsfs(void) {
	struct FileDriver* drvr = resolveFileDriver(2);
	if (drvr == NULL) {
		bugCheckNum(0x1111);
	}
	struct FSReturn fsr = tsfs_fsinit(drvr, 2, 4 * 1024 * 1024);
	if (fsr.err != 0) {
		kernelMsg("Filesystem initialisation failed with err=");
		kernelMsgULong_hex(fsr.err);
		kernelMsg("\n");
		while (1) {}
	}
	kernelMsg("Filesystem initialisation succeeded\n");
	// add to filedriver mapping
	return;
}// NRW



#include "std.h"
/*
 *
 * System call interface
 *
 * NOTE: `oldstat' deals with `udev_old_t', while `newstat' deals with `udev_new_t' (and `udev_old_t' under certain conditions of Linux version and Linux 'BITS_PER_LONG')
 *
 */
void _exit(int code) {// TODO Actually implement
	kMsg_lock();
	kernelMsg("Thread ");
	kernelMsgULong_hex(tid);
	kernelMsg(" terminated with exit status ");
	kernelMsgULong_hex((unsigned long) code);
	kernelMsg(" & 0xff\n");
	kMsg_unlock();
	while (1) {}
}
ssize_t write(int fd, const void* buf, size_t count) {
	if (fd < 0) {
		errno = EBADF;
		return (-1);
	}
	int kfd = getDesc(fetch_tgid(), fd);
	if (kfd == (-1)) {
		errno = EBADF;// File descriptor <fd> is not opened for the process
		return (-1);
	}
	struct FileDriver* driver = resolveFileDriver(kfd);
	if (driver == NULL) {
		bugCheck();
	}
	errno = 0;
	ssize_t res = driver->write(kfd, buf, count);
	retDesc(kfd);
	return res;
}
ssize_t read(int fd, void* buf, size_t count) {
	if (fd < 0) {
		errno = EBADF;
		return (-1);
	}
	int kfd = getDesc(fetch_tgid(), fd);
	if (kfd == (-1)) {
		errno = EBADF;// File descriptor <fd> is not opened for the process
		return (-1);
	}
	struct FileDriver* driver = resolveFileDriver(kfd);
	if (driver == NULL) {
		bugCheck();
	}
	errno = 0;
	ssize_t res = driver->read(kfd, buf, count);
	retDesc(kfd);
	return res;
}
off_t lseek(int fd, off_t off, int how) {
	if (fd < 0) {
		errno = EBADF;
		return (-1);
	}
	int kfd = getDesc(fetch_tgid(), fd);
	if (kfd == (-1)) {
		errno = EBADF;// File descriptor <fd> is not opened for the process
		return (-1);
	}
	struct FileDriver* driver = resolveFileDriver(kfd);
	if (driver == NULL) {
		bugCheck();
	}
	errno = 0;
	off_t res = driver->lseek(kfd, off, how);
	retDesc(kfd);
	return res;
}
int _llseek(int fd, off_t offHi, off_t offLo, loff_t* res, int how) {// Introduced in Linux 1.1.46
	if (fd < 0) {
		errno = EBADF;
		return (-1);
	}
	int kfd = getDesc(fetch_tgid(), fd);
	if (kfd == (-1)) {
		errno = EBADF;// File descriptor <fd> is not opened for the process
		return (-1);
	}
	struct FileDriver* driver = resolveFileDriver(kfd);
	if (driver == NULL) {
		bugCheck();
	}
	errno = 0;
	int result = driver->_llseek(kfd, offHi, offLo, res, how);
	retDesc(kfd);
	return result;
}
time_t time(time_t* resAddr) {
	time_t n = timeFetch();
	if (resAddr != NULL) {
		(*resAddr) = n;
	}
	return n;
}
int stime(const time_t* valAddr) {
	if (!(have_cap(CAP_SYS_TIME))) {
		errno = EPERM;
		return (-1);
	}
	timeStore(*valAddr);
	return 0;
}
uidnatural_t getuid(void) {
	return kuid_to_uidnatural(fetch_ruid());
}
kuid_t getuid32(void) {// Introduced in Linux 2.3.39
	return fetch_ruid();
}
uidnatural_t geteuid(void) {
	return kuid_to_uidnatural(fetch_euid());
}
kuid_t geteuid32(void) {// Introduced in Linux 2.3.39
	return fetch_euid();
}
pid_t getpid(void) {
	return fetch_tgid();
}
pid_t gettid(void) {// Introduced in Linux 2.4.11
	return tid;
}
uintptr brk(uintptr rb) {
	Mutex_acquire(&(PerThreadgroup_context->breakLock));
	uintptr pb = PerThreadgroup_context->userBreak;
	if (!rb) {
		Mutex_release(&(PerThreadgroup_context->breakLock));
		return pb;
	}
	if (PAGEOF(rb - (uintptr) 1) == PAGEOF(pb - (uintptr) 1)) {// TODO URGENT What exactly does it mean for the requested break to be "reasonable"? Correctly determine whether the requested break is "reasonable"
		if (pb != rb) {
			PerThreadgroup_context->userBreak = rb;
			Mutex_release(&(PerThreadgroup_context->breakLock));
			return rb;
		}
		Mutex_release(&(PerThreadgroup_context->breakLock));
		return pb;
	}
	if (rb < pb) {
		uintptr lle = PAGEOF(rb - (uintptr) 1);
		for (uintptr ple = PAGEOF(pb - (uintptr) 1); ple != lle; ple -= PAGE_SIZE) {
			Mutex_acquire(&(usermem->lock));
			if (pageExists(ple, usermem)) {
				freeUserPage(ple, pageMapping(ple, usermem), NULL);
			}
			Mutex_release(&(usermem->lock));
		}
		PerThreadgroup_context->userBreak = rb;
		Mutex_release(&(PerThreadgroup_context->breakLock));
		return rb;
	}
	Mutex_acquire(&(usermem->lock));
	uintptr nla = PAGEOF(rb - (uintptr) 1);
	uintptr ola = PAGEOF(pb - (uintptr) 1);
	for (uintptr chk = nla; chk != ola; chk -= PAGE_SIZE) {
		if (pageExists(chk, usermem)) {
			Mutex_release(&(usermem->lock));
			Mutex_release(&(PerThreadgroup_context->breakLock));
			return pb;
		}
	}
	nla += PAGE_SIZE;
	for (uintptr pmem = ola + PAGE_SIZE; pmem != nla; pmem += PAGE_SIZE) {
		void* pmp = alloc_lb_wiped();
		initPageRef(1, userPageDealloc, pmp);
		if (mapPage(pmem, pmp, 1, 1, usermem)) {// TODO Set heap execution, reading, and writing permissions appropriately
			bugCheckNum(0x0010 | FAILMASK_SYSCALLS);
		}
	}
	Mutex_release(&(usermem->lock));
	PerThreadgroup_context->userBreak = rb;
	Mutex_release(&(PerThreadgroup_context->breakLock));
	return rb;
}
int close(int fd) {
	// TODO Implement
	bugCheck();
	return 0;
}
/*
int open(char* path, int flg, mode_t mode) {// `path' is not of type `const char*' because the calling program is not to be trusted
	// TODO URGENT Ensure that the area from `path' to (and including) the NUL terminator is readable by the userspace program and let `n' be the sum of the string length (not including the NUL terminator) and 1 and let `m' represent a copy of the string of the type `const char*' such that `m' can be deallocated with sizeof
	// TODO URGENT Validate string length
	size_t n = strlen(path) + 1;
	const char* m = alloc(n + 1);
	cpy(m, path, n + 1);
	if (!(*m)) {
		errno = ENOENT;
		return (-1);// Linux seems to give `ENOENT' upon attempting to open "" with the raw system call, even if `O_CREAT' is specifid with `O_RDWR', at least in one instance with an "ext4" filesystem; TODO Check whether this should always happen
	}
	char* fullpath = NULL;
	if ((*path) == 0x2f) {
		lockThreadInfo();
		lockFSInfo();
		fullpath = strconcat(root, m, NULL);
		unlockFSInfo();
		unlockThreadInfo();
	}
	else {
		lockThreadInfo();
		lockFSInfo();
		fullpath = strconcat(root, cwd, m, NULL);
		unlockFSInfo();
		unlockThreadInfo();
	}
	dealloc(m, n + 1);
	size_t fpl = strlen(fullpath) + 1;
	size_t ft = fpl;
	int i = 0;
	while (1) {
		if (!(fullpath[i])) {
			break;
		}
		if ((fullpath[i] == 0x2f) && (fullpath[i + 1] == 0x2f)) {
			move(fullpath + i + 1, fullpath + i + 2, ft - 2);
		}
		else {
			i++;
		}
		ft--;
	}
	// TODO [FINISH]
	bugCheck();
	return 0;
	// TODO retDesc the kfd after it is opened

}
*/

/*
int mount(const char* source, const char* target, const char* filesystemtype, unsigned long mountflags, const void* data) {
	bugCheck();
	return 0;// TODO Implement
}
*/
/*
void* mmap(void* addr, size_t len, int p, int flg, int fd, off_t off) {
	bugCheck();
	return NULL;//TODO Implement


	if (addr == NULL) {
		
	}
	else {
		// validate memory access
	}

}
*/

#if __TESTING__ == 1
#define TESTCALL_NUMBER 1025
unsigned long testcall(unsigned long val) {
	kMsg_lock();
	kernelMsg("Memory usage: ");
	printMemUsage();
	kernelMsg("\n");
	kMsg_unlock();
	return 0;
}
#endif
// TODO Implement all applicable system calls
/*
 * System call appearance history sources:
 * /_?sys_.+/-matching functions in Linux release source
 * Linux man-pages syscalls(2)
 * Linux '/include/linux/sys.h' until Linux 0.99.13k
 * Linux '/kernel/sched.c' from Linux 0.99.13k and onward until Linux 1.1.0
 * Linux '/kernel/sys_call.S' from Linux 1.1.0 and onward until Linux 1.1.52
 * Linux '/arch/i386/entry.S' from Linux 1.1.52 and onward until Linux 1.1.77
 * Linux '/arch/i386/kernel/entry.S' from Linux 1.1.77 and onward until Linux 2.6.12
 * Linux '/arch/i386/kernel/syscall_table.S' from Linux 2.6.12 and onward until Linux 2.6.24
 * Linux '/arch/x86/kernel/syscall_table_32.S' from Linux 2.6.24 and onward until Linux 3.3
 */
unsigned long system_call(unsigned long arg1, unsigned long arg2, unsigned long arg3, unsigned long arg4, unsigned long arg5, unsigned long arg6, unsigned long arg7, unsigned long nr) {// "nr" values are as they are for x86_32 Linux system call numbers; other calls have "nr" values allocated for them as needed
	errno = 0;
	Mutex_acquire(&(PerThread_context->dataLock));
	// kernelMsg("en\n");// NRW
	unsigned long retVal = 0;
#if __TESTING__ == 1
	if (nr > SYSCALL_HIGH) {
		kernelMsg("NOCALL");
	}
	else {
		kernelMsg(callname[nr]);
	}// TODO Maybe note the name of calls to the "testcall"
	// TODO Note the call number if the call is a "NOCALL"
	kernelMsg("(");
	kernelMsgULong_hex(arg1);
	kernelMsg(", ");
	kernelMsgULong_hex(arg2);
	kernelMsg(", ");
	kernelMsgULong_hex(arg3);
	kernelMsg(", ");
	kernelMsgULong_hex(arg4);
	kernelMsg(", ");
	kernelMsgULong_hex(arg5);
	kernelMsg(", ");
	kernelMsgULong_hex(arg6);
	kernelMsg(", ");
	kernelMsgULong_hex(arg7);
	kernelMsg(")");
#endif
	switch (nr) {
		case (1):
			_exit(arg1);
			bugCheckNum(0x0011 | FAILMASK_SYSCALLS);
		case (3):
			{
				void* mem1 = getUserMem(arg2, (size_t) arg3, 0, 1);
				if (errno) {
					retVal = (-1);
					break;
				}
				ssize_t res = read((int) arg1, (void*) mem1, (size_t) arg3);// TODO Ensure that the `ssize_t' is always wide enough to accomodate for all resulting transfer sizes appropriately
				if (res == ((ssize_t) (-1))) {
					retUserMem(mem1, arg2, (size_t) arg3, 0);
				}
				else {
					retUserMem(mem1, arg2, (size_t) arg3, res);
				}
				retVal = (unsigned long) res;
			}
			break;
		case (4):
			{
				void* mem1 = getUserMem(arg2, (size_t) arg3, 1, 0);
				if (errno) {
					retVal = (-1);
					break;
				}
				retVal = (unsigned long) write((int) arg1, (const void*) mem1, (size_t) arg3);
				retUserMem(mem1, arg2, (size_t) arg3, 0);
			}
			break;
		case (13):
			{
				void* uptr = (void*) ((uintptr) arg1);
				void* mem1;
				if (uptr != NULL) {
					mem1 = getUserMem(arg1, sizeof(time_t), 0, 1);
					if (errno) {
						retVal = (-1);
						break;
					}
				}
				else {
					mem1 = NULL;
				}
				retVal = (unsigned long) time((time_t*) mem1);
				if (uptr != NULL) {
					retUserMem(mem1, arg1, sizeof(time_t), sizeof(time_t));
				}
			}
			break;
		case (19):
			retVal = (unsigned long) lseek((int) arg1, (off_t) arg2, (int) arg3);
			break;
		case (20):
			retVal = (unsigned long) getpid();
			break;
		case (24):
			retVal = (unsigned long) getuid();
			break;
		case (25):
			{
				void* mem1 = getUserMem(arg1, sizeof(time_t), 1, 0);
				if (errno) {
					retVal = (-1);
					break;
				}
				retVal = (unsigned long) stime((const time_t*) mem1);
				retUserMem(mem1, arg1, sizeof(time_t), 0);
			}
			break;
		case (45):
			retVal = (unsigned long) brk((uintptr) arg1);
			break;
		case (49):
			retVal = (unsigned long) geteuid();
			break;
#if LINUX_COMPAT_VERSION >= 0x10104600
		case (140):// Prototype is sourced from Linux man-pages lseek64(3)
			{
				void* mem1 = getUserMem(arg4, sizeof(loff_t), 0, 1);
				if (errno) {
					retVal = (-1);// TODO Is the actual repositioning supposed to happen if the passed memory address is not properly-accessible to the user?
					break;
				}
				int res = _llseek((int) arg1, (off_t) arg2, (off_t) arg3, (loff_t*) mem1, (int) arg5);
				if (res == (-1)) {
					retUserMem(mem1, arg4, sizeof(loff_t), 0);
				}
				else {
					retUserMem(mem1, arg4, sizeof(loff_t), sizeof(loff_t));
				}
				retVal = (unsigned long) res;
			}
			break;
#endif
#if LINUX_COMPAT_VERSION >= 0x20303900
		case (199):
			retVal = (unsigned long) getuid32();
			break;
#endif
#if LINUX_COMPAT_VERSION >= 0x20303900
		case (201):
			retVal = (unsigned long) geteuid32();
			break;
#endif
#if LINUX_COMPAT_VERSION >= 0x20401100
		case (224):
			retVal = (unsigned long) gettid();
			break;
#endif
#if __TESTING__ == 1
		case (TESTCALL_NUMBER):
			retVal = (unsigned long) testcall(arg1);
#endif
		default:
			errno = ENOSYS;
			retVal = (-1);
			break;
	}
#if __TESTING__ == 1
	kernelMsg(" = ");
	kernelMsgULong_hex(retVal);
	kernelMsg("; `errno': ");
	kernelMsgULong_hex(errno);
	kernelMsg("\n");
#endif
	// kernelMsg("ex\n");// NRW
	Mutex_release(&(PerThread_context->dataLock));// THIS ADDITIONALLY SERVES AS A MEMORY BARRIER
	return retVal;
}// TODO Allow returning of values wider than the `long' type
#endif
