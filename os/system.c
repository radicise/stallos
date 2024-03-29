#ifndef TARGET
#error "`TARGET' is not set"
#endif
#define __STALLOS__ 1
#define __TESTING__ 1
#define RELOC 0x00040000
/* `RELOC' MUST be an integer multiple of both `KMEM_BS' and `KMEM_LB_BS' */
#define LINUX_COMPAT_VERSION 0x20603904
/*
 *
 * LINUX_COMPAT_VERSION=0xABBCCCDD for compatibility with version A.BB.CCC.DD
 * Compatibility with Linux versions below 1.0 is not supported
 *
 */
#include "kernel/types.h"
unsigned long long PIT0Ticks = 0;
extern void int_enable(void);
extern void int_disable(void);
extern void bugCheck(void);
extern void bugCheckNum(unsigned long);
void bugMsg(uintptr addr) {
	int_disable();
	for (int i = 0xb8000; i < 0xb8fa0; i += 2) {
		(*((volatile unsigned short*) (i - RELOC))) = 0x4720;
	}
	(*((volatile unsigned short*) (0xb8000 - RELOC))) = 0x4746;
	(*((volatile unsigned short*) (0xb8002 - RELOC))) = 0x4741;
	(*((volatile unsigned short*) (0xb8004 - RELOC))) = 0x4754;
	(*((volatile unsigned short*) (0xb8006 - RELOC))) = 0x4741;
	(*((volatile unsigned short*) (0xb8008 - RELOC))) = 0x474c;
	(*((volatile unsigned short*) (0xb800a - RELOC))) = 0x4720;
	(*((volatile unsigned short*) (0xb800c - RELOC))) = 0x474b;
	(*((volatile unsigned short*) (0xb800e - RELOC))) = 0x4745;
	(*((volatile unsigned short*) (0xb8010 - RELOC))) = 0x4752;
	(*((volatile unsigned short*) (0xb8012 - RELOC))) = 0x474e;
	(*((volatile unsigned short*) (0xb8014 - RELOC))) = 0x4745;
	(*((volatile unsigned short*) (0xb8016 - RELOC))) = 0x474c;
	(*((volatile unsigned short*) (0xb8018 - RELOC))) = 0x4720;
	(*((volatile unsigned short*) (0xb801a - RELOC))) = 0x4745;
	(*((volatile unsigned short*) (0xb801c - RELOC))) = 0x4752;
	(*((volatile unsigned short*) (0xb801e - RELOC))) = 0x4752;
	(*((volatile unsigned short*) (0xb8020 - RELOC))) = 0x474f;
	(*((volatile unsigned short*) (0xb8022 - RELOC))) = 0x4752;
	(*((volatile unsigned short*) (0xb80a0 - RELOC))) = 0x4741;
	(*((volatile unsigned short*) (0xb80a2 - RELOC))) = 0x4774;
	(*((volatile unsigned short*) (0xb80a4 - RELOC))) = 0x4720;
	(*((volatile unsigned short*) (0xb80a6 - RELOC))) = 0x4761;
	(*((volatile unsigned short*) (0xb80a8 - RELOC))) = 0x4764;
	(*((volatile unsigned short*) (0xb80aa - RELOC))) = 0x4764;
	(*((volatile unsigned short*) (0xb80ac - RELOC))) = 0x4772;
	(*((volatile unsigned short*) (0xb80ae - RELOC))) = 0x4765;
	(*((volatile unsigned short*) (0xb80b0 - RELOC))) = 0x4773;
	(*((volatile unsigned short*) (0xb80b2 - RELOC))) = 0x4773;
	(*((volatile unsigned short*) (0xb80b4 - RELOC))) = 0x4720;
	(*((volatile unsigned short*) (0xb80b6 - RELOC))) = 0x4730;
	(*((volatile unsigned short*) (0xb80b8 - RELOC))) = 0x4778;
	for (volatile unsigned char* i = (void*) (0xb80b8 - RELOC + (sizeof(uintptr) * 4)); i != (void*) (0xb80b8 - RELOC); i -= 2) {
		if ((addr & 0x0f) < 10) {
			*i = ((addr & 0x0f) + 0x30);
		}
		else {
			*i = ((addr & 0x0f) + 0x37);
		}
		addr >>= 4;
	}	
}
void bugCheckWrapped(uintptr addr) {// Fatal kernel errors
	bugMsg(addr);
	while (1) {
	}
	return;
}
const char hex[] = {0x30,
	0x31,
	0x32,
	0x33,
	0x34,
	0x35,
	0x36,
	0x37,
	0x38,
	0x39,
	0x41,
	0x42,
	0x43,
	0x44,
	0x45,
	0x46};
#define FAILMASK_SYSTEM 0x00010000
int strcmp(const char* d, const char* g) {
	if (!((*d) | (*g))) {
		return 0;
	}
	while (1) {
		if (!(*d)) {
			if (!(*g)) {
				if ((*d) == (*g)) {
					return 0;
				}
				if ((*d) > (*g)) {
					return 1;
				}
			}
			return (-1);
		}
		if (!(*g)) {
			return 1;
		}
		d++;
		g++;
	}
}
typedef volatile unsigned char SimpleMutex;// Not reentrant
typedef volatile unsigned long AtomicULong;
extern unsigned long AtomicULong_get(AtomicULong*);
extern void AtomicULong_set(AtomicULong*, unsigned long);
long handlingIRQ = 0;
pid_t currentThread;// Only to be changed when it is either changed by kernel code through `Thread_run' or not during kernel-space operation
extern void SimpleMutex_acquire(SimpleMutex*);
extern void SimpleMutex_release(SimpleMutex*);
extern int SimpleMutex_tryAcquire(SimpleMutex*);// Returns 1 if acquired, otherwise returns 0
extern void SimpleMutex_wait(void);// Wastes enough time to let at least one other thread acquire a SimpleMutex in that time span if it is already executing SimpleMutex_acquire, assuming that the thread attempting to acquire is not interrupted
extern void SimpleMutex_initUnlocked(SimpleMutex*);
typedef volatile struct {
	SimpleMutex stateLock;
	pid_t ownerThread;
	unsigned long acquires;
} Mutex;// Reentrant
void Mutex_acquire(Mutex* mutex) {
	pid_t id = handlingIRQ ? (~currentThread) : currentThread;
	while (1) {
		SimpleMutex_acquire(&(mutex->stateLock));
		if (mutex->acquires == 0) {
			(mutex->ownerThread) = id;
		}
		else if ((mutex->ownerThread) != id) {
			SimpleMutex_release(&(mutex->stateLock));
			SimpleMutex_wait();
			continue;
		}
		(mutex->acquires)++;
		if ((mutex->acquires) == 0) {
			bugCheckNum(0x0101 | FAILMASK_SYSTEM);
		}
		SimpleMutex_release(&(mutex->stateLock));
		return;
	}
}
void Mutex_release(Mutex* mutex) {	
	pid_t id = handlingIRQ ? (~currentThread) : currentThread;
	SimpleMutex_acquire(&(mutex->stateLock));
	if ((mutex->ownerThread) != id) {
		bugCheckNum(0x0102 | FAILMASK_SYSTEM);
	}
	if ((mutex->acquires) == 0) {
		bugCheckNum(0x0103 | FAILMASK_SYSTEM);
	}
	(mutex->acquires)--;
	SimpleMutex_release(&(mutex->stateLock));
}
int Mutex_tryAcquire(Mutex* mutex) {// Returns 1 if acquired, otherwise returns 0
	pid_t id = handlingIRQ ? (~currentThread) : currentThread;
	SimpleMutex_acquire(&(mutex->stateLock));
	if (mutex->acquires == 0) {
		(mutex->ownerThread) = id;
	}
	else if ((mutex->ownerThread) != id) {
		SimpleMutex_release(&(mutex->stateLock));
		return 0;
	}
	(mutex->acquires)++;
	if ((mutex->acquires) == 0) {
		bugCheckNum(0x0104 | FAILMASK_SYSTEM);
	}
	SimpleMutex_release(&(mutex->stateLock));
	return 1;
}
void Mutex_wait(void) {// Wastes enough time to let at least one other thread acquire a Mutex in that time span if it is already executing Mutex_acquire, assuming that the thread attempting to acquire is not interrupted
	SimpleMutex_wait();
	return;
}
void Mutex_initUnlocked(Mutex* mutex) {
	(mutex->ownerThread) = (pid_t) 0;
	(mutex->acquires) = 0;
	SimpleMutex_initUnlocked(&(mutex->stateLock));
	return;
}
typedef volatile struct {
	long l;
	Mutex lock;
} AtomicLongF;
long AtomicLongF_adjust(AtomicLongF* alf, long adj) {
	Mutex_acquire(&(alf->lock));
	long m = (alf->l);
	(alf->l) += adj;
	Mutex_release(&(alf->lock));
	return m;
}
long AtomicLongF_get(AtomicLongF* alf) {
	Mutex_acquire(&(alf->lock));
	long m = (alf->l);
	Mutex_release(&(alf->lock));
	return m;
}
void AtomicLongF_set(AtomicLongF* alf, long val) {
	Mutex_acquire(&(alf->lock));
	(alf->l) = val;
	Mutex_release(&(alf->lock));
	return;
}
void AtomicLongF_init(AtomicLongF* alf, long val) {
	(alf->l) = val;
	Mutex_initUnlocked(&(alf->lock));
	return;
}
void* move(void* dst, const void* buf, size_t count) {
	void* m = dst;
	if (dst < buf) {
		while (count--) {
			*((char*) dst) = *((char*) buf);
			dst = ((char*) dst) + 1;
			buf = ((char*) buf) + 1;
		}
	}
	else if (dst > buf) {
		dst = ((char*) dst) + count;
		buf = ((char*) buf) + count;
		while (count--) {
			dst = ((char*) dst) - 1;
			buf = ((char*) buf) - 1;
			*((char*) dst) = *((char*) buf);
		}
	}
	return m;
}
void* memmove(void* dst, const void* buf, size_t count) {
	return move(dst, buf, count);
}
void* cpy(void* dst, const void* src, size_t count) {
	return move(dst, src, count);
}
void* memcpy(void* dst, const void* src, size_t count) {
	return cpy(dst, src, count);
}
void* set(void* ptr, int val, size_t count) {
	char* n = ptr;
	while (count--) {
		*(n++) = val;
	}
	return ptr;
}
void bugCheckNum_u32(u32 num, uintptr addr) {
	bugMsg(addr);
	(*((volatile unsigned char*) (0xb8140 - RELOC))) = 0x49;
	(*((volatile unsigned char*) (0xb8142 - RELOC))) = 0x4e;
	(*((volatile unsigned char*) (0xb8144 - RELOC))) = 0x46;
	(*((volatile unsigned char*) (0xb8146 - RELOC))) = 0x4f;
	(*((volatile unsigned char*) (0xb8148 - RELOC))) = 0x3a;
	(*((volatile unsigned char*) (0xb814a - RELOC))) = 0x20;
	(*((volatile unsigned char*) (0xb814c - RELOC))) = 0x30;
	(*((volatile unsigned char*) (0xb814e - RELOC))) = 0x78;
	volatile unsigned char* place = (volatile unsigned char*) (0xb814e - RELOC);
	for (volatile unsigned char* i = place + 16; i != place; i -= 2) {
		if ((num & 0x0f) < 10) {
			*i = ((num & 0x0f) + 0x30);
		}
		else {
			*i = ((num & 0x0f) + 0x37);
		}
		num >>= 4;
	}
	while (1) {
	}
}
void bugCheckNumWrapped(unsigned long num, uintptr addr) {// Fatal kernel errors
	bugCheckNum_u32(num, addr);
}
#include "kernel/driver/VGATerminal.h"
size_t strlen(const char* str) {
	const char* n = str;
	while (*(n++)) {
	}
	return n - str - 1;
}
const char* strct(const char** c) {// Does not check for string size overflow
	int i = 0;
	while (c[i] != NULL) {
		i++;
	}
	size_t l[i];
	size_t ll = 1;
	for (int j = 0; j < i; j++) {
		l[j] = strlen(c[j]);
		ll += l[j];
	}
	char* s = alloc(ll);
	char* m = s;
	for (int j = 0; j < i; j++) {
		cpy(m, c[j], l[j]);
		m += l[j];
	}
	(*m) = 0x00;
	return s;
}
extern const char* strconcat();
struct VGATerminal mainTerm;
Mutex kmsg;
int kernelMsg(const char* msg) {
	unsigned int len = strlen(msg);
	Mutex_acquire(&kmsg);// TODO initialise
	if (len != VGATerminal_write(1, msg, len)) {
		Mutex_release(&kmsg);
		return (-1);
	}
	Mutex_release(&kmsg);
	return 0;
}
int kernelWarnMsg(const char* msg) {
	int w = 0;
	Mutex_acquire(&kmsg);
	w |= kernelMsg("Warning: ");
	w |= kernelMsg(msg);
	w |= kernelMsg("\n");
	Mutex_release(&kmsg);
	return w ? (-1) : 0;
}
int kernelWarnMsgCode(const char* msg, unsigned long code) {
	int w = 0;
	Mutex_acquire(&kmsg);
	w |= kernelMsg("Warning: ");
	w |= kernelMsg(msg);
	w |= kernelMsg("0x");
	int n;
	char tx[(n = (sizeof(unsigned long) * CHAR_BIT / 4)) + 1];
	tx[n] = 0x00;
	while (n--) {
		tx[n] = hex[code & 0x0f];
		code >>= 4;
	}
	w |= kernelMsg(tx);
	w |= kernelMsg("\n");
	Mutex_release(&kmsg);
	return w ? (-1) : 0;
}
int kernelMsgULong(unsigned long n) {
	if (n == 0) {
		return kernelMsg("0");
	}
	int count = 0;
	unsigned long m = n;
	while (m) {
		m /= 10;
		count++;
	}
	char buf[count + 1];
	buf[count] = 0x00;
	int i = count - 1;
	while (1) {
		buf[i] = 0x30 + (n % 10);
		if (i == 0) {
			break;
		}
		n /= 10;
		i--;
	}
	return kernelMsg(buf);
}
#include "kernel/paging.h"
int printMem(unsigned long long n) {
	unsigned long b = n & 0x3ff;
	n >>= 10;
	unsigned long kib = n & 0x3ff;
	n >>= 10;
	unsigned long mib = n & 0x3ff;
	n >>= 10;
	unsigned long gib = n & 0x3ff;
	unsigned long tib = n >> 10;
	int g = tib ? 1 : (gib ? 2 : (mib ? 3 : (kib ? 4 : 5)));
	int h = b ? 1 : (kib ? 2 : (mib ? 3 : (gib ? 4 : 5)));
	int w = 0;
	Mutex_acquire(&kmsg);
	switch (g) {
		case (1):
			w |= kernelMsgULong(tib);
			w |= kernelMsg("TiB");
			if (h == 5) {
				break;
			}
			w |= kernelMsg("+");
		case (2):
			if (gib) {
				w |= kernelMsgULong(gib);
				w |= kernelMsg("GiB");
				if (h == 4) {
					break;
				}
				w |= kernelMsg("+");
			}
		case (3):
			if (mib) {
				w |= kernelMsgULong(mib);
				w |= kernelMsg("MiB");
				if (h == 3) {
					break;
				}
				w |= kernelMsg("+");
			}
		case (4):
			if (kib) {
				w |= kernelMsgULong(kib);
				w |= kernelMsg("KiB");
				if (h == 2) {
					break;
				}
				w |= kernelMsg("+");
			}
		case (5):
			w |= kernelMsgULong(b);
			w |= kernelMsg("B");
			break;
		default:
			bugCheck();
	}
	Mutex_release(&kmsg);
	return w;
}
int printMemUsage(void) {
	Mutex_acquire(&kmsg);
	printMem(getMemUsage());
	kernelMsg(" of ");
	printMem(amntMem);
	Mutex_release(&kmsg);
}
int kernelMsgULong_hex(unsigned long code) {
	int w = 0;
	Mutex_acquire(&kmsg);
	w |= kernelMsg("0x");
	int n;
	char tx[(n = (sizeof(unsigned long) * CHAR_BIT / 4)) + 1];
	tx[n] = 0x00;
	while (n--) {
		tx[n] = hex[code & 0x0f];
		code >>= 4;
	}
	w |= kernelMsg(tx);
	Mutex_release(&kmsg);
	return w ? (-1) : 0;
}
int kernelMsgCode(const char* msg, unsigned long code) {
	int w = 0;
	Mutex_acquire(&kmsg);
	w |= kernelMsg(msg);
	w |= kernelMsg("0x");
	int n;
	char tx[(n = (sizeof(unsigned long) * CHAR_BIT / 4)) + 1];
	tx[n] = 0x00;
	while (n--) {
		tx[n] = hex[code & 0x0f];
		code >>= 4;
	}
	w |= kernelMsg(tx);
	w |= kernelMsg("\n");
	Mutex_release(&kmsg);
	return w ? (-1) : 0;
}
int kernelInfoMsg(const char* msg) {
	int w = 0;
	Mutex_acquire(&kmsg);
	w |= kernelMsg("Info.: ");
	w |= kernelMsg(msg);
	w |= kernelMsg("\n");
	Mutex_release(&kmsg);
	return w ? (-1) : 0;
}
extern void bus_out_long(unsigned long, unsigned long);
extern void bus_out_u8(unsigned long, u8);
extern void bus_out_u16(unsigned long, u16);
extern void bus_out_u32(unsigned long, u32);
extern long bus_in_long(unsigned long);
extern u8 bus_in_u8(unsigned long);
extern u16 bus_in_u16(unsigned long);
extern u32 bus_in_u32(unsigned long);
extern void bus_wait(void);
extern void bus_outBlock_long(u16, const long*, unsigned long);
extern void bus_outBlock_u32(u16, const u32*, unsigned long);
extern void bus_outBlock_u16(u16, const u16*, unsigned long);
extern void bus_outBlock_u8(u16, const u8*, unsigned long);
extern void bus_inBlock_long(u16, long*, unsigned long);
extern void bus_inBlock_u32(u16, u32*, unsigned long);
extern void bus_inBlock_u16(u16, u16*, unsigned long);
extern void bus_inBlock_u8(u16, u8*, unsigned long);
extern unsigned long readLongPhysical(u32);
extern void writeLongPhysical(u32, unsigned long);
#include "kernel/kbd8042.h"
#define KBDBUF_SIZE 16
unsigned char kbdBuf[KBDBUF_SIZE];
#define KBDBUFTERM_SIZE 320
unsigned char kbdBufTerm[KBDBUFTERM_SIZE];
#define KBDBUFTERMCANON_SIZE 256
unsigned char kbdBufTermCanon[KBDBUFTERMCANON_SIZE];
struct Keyboard8042 kbdMain;
volatile const time_t ___currentTime___ = 0;// Do NOT access directly except for within the prescribed methods of access
extern void timeIncrement(void);// Atomic, increment system time by 1 second
extern time_t timeFetch(void);// Atomic, get system time (time in seconds)
extern void timeStore(time_t);// Atomic, set system time (time in seconds)
#include "kernel/threads.h"
#include "kernel/perThreadgroup.h"
#include "kernel/perThread.h"
extern int runELF(void*, void*, struct Thread_state*);
void irupt_handler_70h(struct Thread_state* state) {// IRQ 0, frequency (Hz) = (1193181 + (2/3)) / 11932 = 3579545 / 35796
	PIT0Ticks++;
	if (((PIT0Ticks * ((unsigned long long) 35796)) % ((unsigned long long) 3579545)) < ((unsigned long long) 35796)) {
		timeIncrement();
	}
	Thread_restore(state, 0x70);
	return;
}
void irupt_handler_71h(void) {// IRQ 1
	//bugCheck();
	kbd8042_onIRQ(&kbdMain);
	return;
}
void irupt_handler_72h(void) {// IRQ 2, this should not be triggered
	bugCheckNum(0x0072 | FAILMASK_SYSTEM);
	return;
}
void irupt_handler_73h(void) {// IRQ 3
	bugCheckNum(0x0073 | FAILMASK_SYSTEM);
	return;
}
void irupt_handler_74h(void) {// IRQ 4
	bugCheckNum(0x0074 | FAILMASK_SYSTEM);
	return;
}
void irupt_handler_75h(void) {// IRQ 5
	bugCheckNum(0x0075 | FAILMASK_SYSTEM);
	return;
}
void irupt_handler_76h(void) {// IRQ 6
	bugCheckNum(0x0076 | FAILMASK_SYSTEM);
	return;
}
void irupt_handler_77h(void) {// IRQ 7
	bugCheckNum(0x0077 | FAILMASK_SYSTEM);
	return;
}
void irupt_handler_78h(void) {// IRQ 8
	bugCheckNum(0x0078 | FAILMASK_SYSTEM);
	return;
}
void irupt_handler_79h(void) {// IRQ 9
	bugCheckNum(0x0079 | FAILMASK_SYSTEM);
	return;
}
void irupt_handler_7ah(void) {// IRQ 10
	bugCheckNum(0x007a | FAILMASK_SYSTEM);
	return;
}
void irupt_handler_7bh(void) {// IRQ 11
	bugCheckNum(0x007b | FAILMASK_SYSTEM);
	return;
}
void irupt_handler_7ch(void) {// IRQ 12
	bugCheckNum(0x007c | FAILMASK_SYSTEM);
	return;
}
void irupt_handler_7dh(void) {// IRQ 13
	bugCheckNum(0x007d | FAILMASK_SYSTEM);
	return;
}
void irupt_handler_7eh(void) {// IRQ 14
	return;// ATA IRQ
}
void irupt_handler_7fh(void) {// IRQ 15
	return;// ATA IRQ
}
void PICInit(unsigned char mOff, unsigned char sOff) {
	if ((mOff & 0x07) || (sOff & 0x07)) {
		bugCheck();
	}
	u8 mOr = bus_in_u8(0x0021);
	u8 sOr = bus_in_u8(0x00a1);
	bus_wait();
	bus_out_u8(0x0020, 0x11);
	bus_out_u8(0x00a0, 0x11);
	bus_wait();
	bus_out_u8(0x0021, mOff);
	bus_out_u8(0x00a1, sOff);
	bus_wait();
	bus_out_u8(0x0021, 0x04);
	bus_out_u8(0x00a1, 0x02);
	bus_wait();
	bus_out_u8(0x0021, 0x01);
	bus_out_u8(0x00a1, 0x01);
	bus_wait();
	bus_out_u8(0x0021, mOr);
	bus_out_u8(0x00a1, sOr);
	bus_wait();
}
#include "kernel/driver/ATA.h"
#include "kernel/blockdev.h"
#include "kernel/syscalls.h"
extern void irupt_70h(void);
extern void irupt_71h(void);
extern void irupt_72h(void);
extern void irupt_73h(void);
extern void irupt_74h(void);
extern void irupt_75h(void);
extern void irupt_76h(void);
extern void irupt_77h(void);
extern void irupt_78h(void);
extern void irupt_79h(void);
extern void irupt_7ah(void);
extern void irupt_7bh(void);
extern void irupt_7ch(void);
extern void irupt_7dh(void);
extern void irupt_7eh(void);
extern void irupt_7fh(void);
extern void irupt_80h(void);
void substitute_irupt_address_vector(unsigned char iruptNum, void (*addr)(void), unsigned short segSel) {
	(*((volatile unsigned short*) (0x7f800 - RELOC + (iruptNum * 8)))) = ((long) addr);
	(*((volatile unsigned short*) (0x7f802 - RELOC + (iruptNum * 8)))) = segSel;
	(*((volatile unsigned short*) (0x7f806 - RELOC + (iruptNum * 8)))) = (((long) addr) >> 16);
}
#define FAILMASK_ELFLOADER 0x000a0000
void systemEntry(void) {
	Mutex_initUnlocked(&kmsg);
	initializeVGATerminal(&mainTerm, 80, 25, (struct VGACell*) (0x000b8000 - RELOC), kbd8042_read);
	mainTerm.pos = readLongPhysical(0x00000506) & 0xffff;
	mainTerm.format = readLongPhysical(0x00000508) & 0x00ff;
	/*
	for (unsigned int i = (0x000b8000 - RELOC); i < (0x000b8fa0 - RELOC); i += 2) {
		((struct VGACell*) i)->format = mainTerm.format;
		((struct VGACell*) i)->text = 0x20;
	}
	*/
	/* Style */
	((struct VGACell*) (0x000b8000  + (mainTerm.pos << 1) - RELOC))->format ^= 0x77;
	mainTerm.onlcr = 1;
	AtomicULong_set(&(mainTerm.xon), 1);
	//AtomicULong_set(&(mainTerm.xctrl), 1);// Keep disabled because of possible deadlocking when echoing is enabled
	mainTerm.cursor = 1;
	/* End-of-style */
	kernelMsg("Stallos Kernel v0.0.1.1-dev\n");
	kernelMsg("Redefining Intel 8259 Programmable Interrupt Controller IRQ mappings . . . ");
	PICInit(0x70, 0x78);
	kernelMsg("done\n");
	kernelMsg("Setting interrupt handlers . . . ");
	substitute_irupt_address_vector(0x70, irupt_70h, 0x18);// Refer to the comment on the following line
	substitute_irupt_address_vector(0x71, irupt_71h, 0x18);// Refer to the comment on the following line
	substitute_irupt_address_vector(0x72, irupt_72h, 0x18);// Refer to the comment on the following line
	substitute_irupt_address_vector(0x73, irupt_73h, 0x18);// Refer to the comment on the following line
	substitute_irupt_address_vector(0x74, irupt_74h, 0x18);// Refer to the comment on the following line
	substitute_irupt_address_vector(0x75, irupt_75h, 0x18);// Refer to the comment on the following line
	substitute_irupt_address_vector(0x76, irupt_76h, 0x18);// Refer to the comment on the following line
	substitute_irupt_address_vector(0x77, irupt_77h, 0x18);// Refer to the comment on the following line
	substitute_irupt_address_vector(0x78, irupt_78h, 0x18);// Refer to the comment on the following line
	substitute_irupt_address_vector(0x79, irupt_79h, 0x18);// Refer to the comment on the following line
	substitute_irupt_address_vector(0x7a, irupt_7ah, 0x18);// Refer to the comment on the following line
	substitute_irupt_address_vector(0x7b, irupt_7bh, 0x18);// Refer to the comment on the following line
	substitute_irupt_address_vector(0x7c, irupt_7ch, 0x18);// Refer to the comment on the following line
	substitute_irupt_address_vector(0x7d, irupt_7dh, 0x18);// Refer to the comment on the following line
	substitute_irupt_address_vector(0x7e, irupt_7eh, 0x18);// Refer to the comment on the following line
	substitute_irupt_address_vector(0x7f, irupt_7fh, 0x18);// Refer to the comment on the following line
	substitute_irupt_address_vector(0x80, irupt_80h, 0x0018);// The system call interface is the only place where, during normal operation, the thread-specific and thread-determining data may be interacted with
	kernelMsg("done\n");
	kernelMsg("Redefining Intel 8253 / 8254 Programmable Interval Timer channel 0 interval . . . ");
	bus_out_u8(0x0043, 0x34);
	bus_out_u8(0x0040, 0x9c);// 11932 lobyte
	bus_out_u8(0x0040, 0x2e);// 11932 hibyte
	bus_wait();// TODO Remove this line when it is deemed unnecessary
	kernelMsg("done\n");
	kernelMsg("Performing Intel 8042 CHMOS 8-bit Slave Microcontroller driver and PS/2 keyboard driver initialization  . . . ");
	initKeyboard8042(kbdBuf, KBDBUF_SIZE, kbdBufTerm, KBDBUFTERM_SIZE, kbdBufTermCanon, KBDBUFTERMCANON_SIZE, 0, &mainTerm, &kbdMain);
	kernelMsg("done\n");
	kernelMsg("Initializing ATA driver . . . ");
	initATA();
	kernelMsg("done\n");
	kernelMsg("Re-enabling IRQ, non-maskable interrupts, and software interrupts . . . ");
	int_enable();
	kernelMsg("done\n");
	kernelMsg("Initializing system call interface . . . ");
	initSystemCallInterface();
	kernelMsg("done\n");
	kernelMsg("Initializing kernel thread management interface . . . ");
	Threads_init();
	kernelMsg("done\n");
	kernelMsg("Starting `init'\n");
	Mutex_acquire(&Threads_threadManage);
	volatile struct Thread* th = alloc(sizeof(struct Thread));
	PerThread_context = &(th->thread);
	(th->group) = (PerThreadgroup_context = alloc(sizeof(struct PerThreadgroup)));
	___nextTask___ = 1;// For consistency of the Linux behaviour of the "tid" / "tgid" (pid_t) 0 not being given to any userspace process
	Mutex_release(&Threads_threadManage);
	currentThread = Threads_addThread(th);
	Mutex_acquire(&Threads_threadManage);
	tgid = currentThread;
	tid = tgid;
	(PerThreadgroup_context->mem) = MemSpace_kernel;// TODO Set up a different memory space for the process
	ruid = 0;
	euid = 0;
	suid = 0;
	fsuid = 0;
	Mutex_initUnlocked(&(PerThread_context->dataLock));
	errno = 0;
	___nextTask___ = PID_USERSTART;// TODO Should this step be done?
	Mutex_release(&Threads_threadManage);
	int errVal = runELF((void*) 0x00010000, (void*) 0x00800000, (struct Thread_state*) (((char*) (&(th->state))) + RELOC));
	if (errVal != 0) {
		bugCheckNum(errVal | FAILMASK_ELFLOADER);
	}
	Thread_run(&(th->state));
	bugCheck();
	return;
}
