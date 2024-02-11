#define RELOC 0x00040000
#define __STALLOS__ 1
#include "kernel/types.h"
unsigned long long PIT0Ticks = 0;
extern void int_enable(void);
extern void int_disable(void);
extern void bugCheck(void);
extern void bugCheckNum(unsigned long);
void bugMsg(uintptr addr) {
	int_disable();
	for (int i = 0xb8000; i < 0xb8fa0; i += 2) {
		(*((unsigned short*) (i - RELOC))) = 0x4720;
	}
	(*((unsigned short*) (0xb8000 - RELOC))) = 0x4746;
	(*((unsigned short*) (0xb8002 - RELOC))) = 0x4741;
	(*((unsigned short*) (0xb8004 - RELOC))) = 0x4754;
	(*((unsigned short*) (0xb8006 - RELOC))) = 0x4741;
	(*((unsigned short*) (0xb8008 - RELOC))) = 0x474c;
	(*((unsigned short*) (0xb800a - RELOC))) = 0x4720;
	(*((unsigned short*) (0xb800c - RELOC))) = 0x474b;
	(*((unsigned short*) (0xb800e - RELOC))) = 0x4745;
	(*((unsigned short*) (0xb8010 - RELOC))) = 0x4752;
	(*((unsigned short*) (0xb8012 - RELOC))) = 0x474e;
	(*((unsigned short*) (0xb8014 - RELOC))) = 0x4745;
	(*((unsigned short*) (0xb8016 - RELOC))) = 0x474c;
	(*((unsigned short*) (0xb8018 - RELOC))) = 0x4720;
	(*((unsigned short*) (0xb801a - RELOC))) = 0x4745;
	(*((unsigned short*) (0xb801c - RELOC))) = 0x4752;
	(*((unsigned short*) (0xb801e - RELOC))) = 0x4752;
	(*((unsigned short*) (0xb8020 - RELOC))) = 0x474f;
	(*((unsigned short*) (0xb8022 - RELOC))) = 0x4752;
	(*((unsigned short*) (0xb80a0 - RELOC))) = 0x4741;
	(*((unsigned short*) (0xb80a2 - RELOC))) = 0x4774;
	(*((unsigned short*) (0xb80a4 - RELOC))) = 0x4720;
	(*((unsigned short*) (0xb80a6 - RELOC))) = 0x4761;
	(*((unsigned short*) (0xb80a8 - RELOC))) = 0x4764;
	(*((unsigned short*) (0xb80aa - RELOC))) = 0x4764;
	(*((unsigned short*) (0xb80ac - RELOC))) = 0x4772;
	(*((unsigned short*) (0xb80ae - RELOC))) = 0x4765;
	(*((unsigned short*) (0xb80b0 - RELOC))) = 0x4773;
	(*((unsigned short*) (0xb80b2 - RELOC))) = 0x4773;
	(*((unsigned short*) (0xb80b4 - RELOC))) = 0x4720;
	(*((unsigned short*) (0xb80b6 - RELOC))) = 0x4730;
	(*((unsigned short*) (0xb80b8 - RELOC))) = 0x4778;
	for (unsigned char* i = (void*) (0xb80b8 - RELOC + (sizeof(uintptr) * 4)); i != (void*) (0xb80b8 - RELOC); i -= 2) {
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
char hex[] = {0x30,
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
typedef unsigned char Mutex;
typedef unsigned long AtomicULong;
extern unsigned long AtomicULong_get(AtomicULong*);
extern void AtomicULong_set(AtomicULong*, unsigned long);
extern void Mutex_acquire(Mutex*);// Idempotent
extern void Mutex_release(Mutex*);// Idempotent
extern int Mutex_tryAcquire(Mutex*);// Returns 1 if acquired, otherwise returns 0
extern void Mutex_wait(void);// Wastes enough time to let at least one other thread acquire a Mutex in that time span if it is already executing Mutex_acquire, assuming that the thread attempting to acquire is not interrupted
extern void Mutex_initUnlocked(Mutex*);
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
void* cpy(void* dst, const void* src, size_t count) {
	return move(dst, src, count);
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
	(*((unsigned char*) (0xb8140 - RELOC))) = 0x49;
	(*((unsigned char*) (0xb8142 - RELOC))) = 0x4e;
	(*((unsigned char*) (0xb8144 - RELOC))) = 0x46;
	(*((unsigned char*) (0xb8146 - RELOC))) = 0x4f;
	(*((unsigned char*) (0xb8148 - RELOC))) = 0x3a;
	(*((unsigned char*) (0xb814a - RELOC))) = 0x20;
	(*((unsigned char*) (0xb814c - RELOC))) = 0x30;
	(*((unsigned char*) (0xb814e - RELOC))) = 0x78;
	unsigned char* place = (unsigned char*) (0xb814e - RELOC);
	for (unsigned char* i = place + 16; i != place; i -= 2) {
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
#include "kernel/VGATerminal.h"
size_t strlen(const char* str) {
	const char* n = str;
	while (*(n++)) {
	}
	return n - str - 1;
}
struct VGATerminal mainTerm;
int kernelMsg(const char* msg) {
	unsigned int len = strlen(msg);
	if (len != VGATerminal_write(1, msg, len)) {
		return (-1);
	}
	return 0;
}
int kernelWarnMsg(const char* msg) {
	kernelMsg("Warning: ");
	kernelMsg(msg);
	kernelMsg("\n");
}
int kernelWarnMsgCode(const char* msg, unsigned long code) {
	int w = 0;
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
	return w ? (-1) : 0;
}
int kernelMsgCode(const char* msg, unsigned long code) {
	int w = 0;
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
	return w ? (-1) : 0;
}
int kernelInfoMsg(const char* msg) {
	int w = 0;
	w |= kernelMsg("Info.: ");
	w |= kernelMsg(msg);
	w |= kernelMsg("\n");
	return w ? (-1) : 0;
}
extern int runELF(void*, void*, int*);
#define FAILMASK_SYSTEM 0x00010000
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
extern u32 readPhysical(u32);
extern void writePhysical(u32, u32);
#include "kernel/kbd8042.h"
#define KBDBUF_SIZE 16
unsigned char kbdBuf[KBDBUF_SIZE];
#define KBDBUFTERM_SIZE 320
unsigned char kbdBufTerm[KBDBUFTERM_SIZE];
#define KBDBUFTERMCANON_SIZE 256
unsigned char kbdBufTermCanon[KBDBUFTERMCANON_SIZE];
struct Keyboard8042 kbdMain;
time_t ___currentTime___ = 0;// Do NOT access directly except for within the prescribed methods of access
extern void timeIncrement(void);// Atomic, increment system time by 1 second
extern time_t timeFetch(void);// Atomic, get system time (time in seconds)
extern void timeStore(time_t);// Atomic, set system time (time in seconds)
#include "kernel/threads.h"
void irupt_handler_70h(struct Thread_state* state) {// IRQ 0, frequency (Hz) = (1193181 + (2/3)) / 11932 = 3579545 / 35796
	PIT0Ticks++;
	if (((PIT0Ticks * ((unsigned long long) 35796)) % ((unsigned long long) 3579545)) < ((unsigned long long) 35796)) {
		timeIncrement();
		kernelMsgCode("Time: ", timeFetch());
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
#include "kernel/ATA.h"
#include "kernel/blockCompat.h"
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
	(*((unsigned short*) (0x7f800 - RELOC + (iruptNum * 8)))) = ((long) addr);
	(*((unsigned short*) (0x7f802 - RELOC + (iruptNum * 8)))) = segSel;
	(*((unsigned short*) (0x7f806 - RELOC + (iruptNum * 8)))) = (((long) addr) >> 16);
}
void systemEntry(void) {
	initializeVGATerminal(&mainTerm, 80, 25, (struct VGACell*) (0x000b8000 - RELOC), kbd8042_read);
	mainTerm.pos = readPhysical(0x00000506) & 0xffff;
	mainTerm.format = readPhysical(0x00000508) & 0x00ff;
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
	kernelMsg("Stall Kernel v0.0.1.0-dev\n");
	kernelMsg("Redefining Intel 8259 Programmable Interrupt Controller IRQ mappings . . . ");
	PICInit(0x70, 0x78);
	kernelMsg("done\n");
	kernelMsg("Setting interrupt handlers . . . ");
	substitute_irupt_address_vector(0x70, irupt_70h, 0x18);
	substitute_irupt_address_vector(0x71, irupt_71h, 0x18);
	substitute_irupt_address_vector(0x72, irupt_72h, 0x18);
	substitute_irupt_address_vector(0x73, irupt_73h, 0x18);
	substitute_irupt_address_vector(0x74, irupt_74h, 0x18);
	substitute_irupt_address_vector(0x75, irupt_75h, 0x18);
	substitute_irupt_address_vector(0x76, irupt_76h, 0x18);
	substitute_irupt_address_vector(0x77, irupt_77h, 0x18);
	substitute_irupt_address_vector(0x78, irupt_78h, 0x18);
	substitute_irupt_address_vector(0x79, irupt_79h, 0x18);
	substitute_irupt_address_vector(0x7a, irupt_7ah, 0x18);
	substitute_irupt_address_vector(0x7b, irupt_7bh, 0x18);
	substitute_irupt_address_vector(0x7c, irupt_7ch, 0x18);
	substitute_irupt_address_vector(0x7d, irupt_7dh, 0x18);
	substitute_irupt_address_vector(0x7e, irupt_7eh, 0x18);
	substitute_irupt_address_vector(0x7f, irupt_7fh, 0x18);
	substitute_irupt_address_vector(0x80, irupt_80h, 0x0018);
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
	int retVal = 0;
	int errVal = runELF((void*) 0x00020000, (void*) 0x00800000, (int*) (((char*) &retVal) + RELOC));
	if (errVal) {
		bugCheckNum(FAILMASK_SYSTEM | 0x0100 | (errVal & 0xff));
	}
	kernelMsgCode("Program exited with code ", retVal);
	while (1) {
	}
}
