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
#include "Map.h"
#include "kmemman.h"
#include "paging.h"
#define FAILMASK_SYSCALLS 0x000e0000
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
const char* callname[] = {"NOCALL" /* 0 */,
#else
const char* callname[] = {"restart_syscall" /* 0 */,
#endif
	"exit",
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
#include "driver/VGATerminal.h"
#include "driver/ATA.h"
struct Map* kfdDriverMap;
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
	long r = AtomicLongF_adjust(&(((struct KFDInfo*) st)->refs), (-1));
	if (!r) {
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
	}
	return;
}
unsigned int getMemOffset(void) {// TODO Utilise virtual memory pages properly
	if (tid == ((pid_t) 1)) {// TODO Support for multiple userspace threads
		return 0x800000 - RELOC;
	}
	bugCheck();
	return 0;
}
void initSystemCallInterface(void) {
	kmem_init();
	kfdDriverMap = Map_create();// Map, int "kfd" -> struct FileDriver* "driver"
	FSDriverMap = Map_create();// Map, const char* "Filesystem name" -> struct FSInterface* "Filesystem driver"; "Filesystem name" only ends with '/' if it is "/"
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
int validateCap(int cap) {
	return 1;// TODO Implement
}
/*
 *
 * System call interface
 *
 * NOTE: `oldstat' deals with `udev_old_t', while `newstat' deals with `udev_new_t'
 *
 */
ssize_t write(int fd, const void* buf, size_t count) {
	if (fd < 0) {
		errno = EBADF;
		return (-1);
	}
	int kfd = getDesc(tgid, fd);
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
	int kfd = getDesc(tgid, fd);
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
	int kfd = getDesc(tgid, fd);
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
	int kfd = getDesc(tgid, fd);
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
	if (!(validateCap(CAP_SYS_TIME))) {
		errno = EPERM;
		return (-1);
	}
	timeStore(*valAddr);
	return 0;
}
uidnatural_t getuid(void) {
	lockThreadInfo();
	uid32_t uid = ruid;
	unlockThreadInfo();
	return uid32_to_uidnatural(uid);
}
uid32_t getuid32(void) {// Introduced in Linux 2.3.39
	lockThreadInfo();
	uid32_t uid = ruid;
	unlockThreadInfo();
	return uid;
}
uidnatural_t geteuid(void) {
	lockThreadInfo();
	uid32_t uid = euid;
	unlockThreadInfo();
	return uid32_to_uidnatural(uid);
}
uid32_t geteuid32(void) {// Introduced in Linux 2.3.39
	lockThreadInfo();
	uid32_t uid = euid;
	unlockThreadInfo();
	return uid;
}
pid_t getpid(void) {
	return tgid;
}
pid_t gettid(void) {// Introduced in Linux 2.4.11
	return tid;
}
int close(int fd) {
	// TODO Implement
	bugCheck();
	return 0;
}
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


}



int mount(const char* source, const char* target, const char* filesystemtype, unsigned long mountflags, const void* data) {
	bugCheck();
	return 0;// TODO Implement
}
void* mmap(void* addr, size_t len, int p, int flg, int fd, off_t off) {
	bugCheck();
	return NULL;//TODO Implement


	if (addr == NULL) {
		
	}
	else {
		// validate memory access
	}

}


#if __TESTING__ == 1
#define TESTCALL_NUMBER 1025
unsigned long testcall(unsigned long val) {
	Mutex_acquire(&kmsg);
	kernelMsg("Memory usage: ");
	printMemUsage();
	kernelMsg("\n");
	Mutex_release(&kmsg);
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
	unsigned long retVal = 0;
#if __TESTING__ == 1
	if (nr != 13) {
		if (nr > SYSCALL_HIGH) {
			kernelMsg("NOCALL");
		}
		else {
			kernelMsg(callname[nr]);
		}
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
	}
#endif
	switch (nr) {// TODO Authenticate memory access
		case (3):
			retVal = (unsigned long) read((int) arg1, (void*) (arg2 + getMemOffset()), (size_t) arg3);
			break;
		case (4):
			retVal = (unsigned long) write((int) arg1, (const void*) (arg2 + getMemOffset()), (size_t) arg3);
			break;
		case (13):
			retVal = (unsigned long) time((time_t*) ((arg1 == 0) ? 0 : (arg1 + getMemOffset())));
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
			retVal = (unsigned long) stime((const time_t*) (arg1 + getMemOffset()));
			break;
		case (49):
			retVal = (unsigned long) geteuid();
			break;
#if LINUX_COMPAT_VERSION >= 0x10104600
		case (140):// Prototype is sourced from Linux man-pages lseek64(3)
			retVal = (unsigned long) _llseek((int) arg1, (off_t) arg2, (off_t) arg3, (loff_t*) (arg4 + getMemOffset()), (int) arg5);
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
	if (nr != 13) {
		kernelMsg(" = ");
		kernelMsgULong_hex(retVal);
		kernelMsg("; `errno': ");
		kernelMsgULong_hex(errno);
		kernelMsg("\n");
	}
#endif
	Mutex_release(&(PerThread_context->dataLock));
	return retVal;
}// TODO Allow returning of values wider than the `long' type
#endif
