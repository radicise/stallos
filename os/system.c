#define RELOC 0x00040000
unsigned long long PIT0Ticks = 0;
void bugMsg(void) {
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
}
void bugCheck(void) {// Fatal kernel errors
	bugMsg();
	while (1) {
	}
	return;
}
typedef unsigned char Mutex;
extern void Mutex_acquire(Mutex*);
extern void Mutex_release(Mutex*);
extern int Mutex_tryAcquire(Mutex*);// Returns 1 if acquired, otherwise returns 0
extern void Mutex_wait();// Wastes enough time to let at least one other thread acquire a Mutex in that time span if it is already executing Mutex_acquire, assuming that the waiting thread is not interrupted
extern void Mutex_initUnlocked(Mutex*);
#include "kernel/types.h"
void* move(void* dst, void* buf, size_t count) {
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
void* cpy(void* dst, void* src, size_t count) {
	return move(dst, src, count);
}
#include "kernel/VGATerminal.h"
unsigned int strlen(char* str) {
	char* n = str;
	while (*n++) {
	}
	return n - str - 1;
}
struct VGATerminal mainTerm;
int kernelMsg(char* msg) {
	unsigned int len = strlen(msg);
	if (len != VGATerminalWrite(&mainTerm, (unsigned char*) msg, len)) {
		return (-1);
	}
	return 0;
}
extern int runELF(void*, void*, int*);
void bugCheckNum_u32(u32 num) {
	bugMsg();
	(*((unsigned char*) (0xb80a0 - RELOC))) = 0x49;
	(*((unsigned char*) (0xb80a2 - RELOC))) = 0x4e;
	(*((unsigned char*) (0xb80a4 - RELOC))) = 0x46;
	(*((unsigned char*) (0xb80a6 - RELOC))) = 0x4f;
	(*((unsigned char*) (0xb80a8 - RELOC))) = 0x3a;
	(*((unsigned char*) (0xb80aa - RELOC))) = 0x20;
	(*((unsigned char*) (0xb80ac - RELOC))) = 0x30;
	(*((unsigned char*) (0xb80ae - RELOC))) = 0x78;
	unsigned char* place = (unsigned char*) (0xb80ae - RELOC);
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
void bugCheckNum(unsigned long num) {// Fatal kernel errors
	bugCheckNum_u32(num);
}
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
#include "kernel/kbd8042.h"
#define KBDBUF_SIZE 16
unsigned char kbdBuf[KBDBUF_SIZE];
struct Keyboard8042 kbdMain;
void irupt_handler_70h(void) {// IRQ 0
	PIT0Ticks++;
	return;
}
void irupt_handler_71h(void) {// IRQ 1
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
	bugCheckNum(0x007e | FAILMASK_SYSTEM);
	return;
}
void irupt_handler_7fh(void) {// IRQ 15
	bugCheckNum(0x007f | FAILMASK_SYSTEM);
	return;
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
extern void int_enable(void);
void substitute_irupt_address_vector(unsigned char iruptNum, void* addr, unsigned short segSel) {
	(*((unsigned short*) (0x7f800 - RELOC + (iruptNum * 8)))) = ((long) addr);
	(*((unsigned short*) (0x7f802 - RELOC + (iruptNum * 8)))) = segSel;
	(*((unsigned short*) (0x7f806 - RELOC + (iruptNum * 8)))) = (((long) addr) >> 16);
}
void systemEntry(void) {
	PICInit(0x70, 0x78);
	substitute_irupt_address_vector(0x70, (void*) irupt_70h, 0x18);
	substitute_irupt_address_vector(0x71, (void*) irupt_71h, 0x18);
	substitute_irupt_address_vector(0x72, (void*) irupt_72h, 0x18);
	substitute_irupt_address_vector(0x73, (void*) irupt_73h, 0x18);
	substitute_irupt_address_vector(0x74, (void*) irupt_74h, 0x18);
	substitute_irupt_address_vector(0x75, (void*) irupt_75h, 0x18);
	substitute_irupt_address_vector(0x76, (void*) irupt_76h, 0x18);
	substitute_irupt_address_vector(0x77, (void*) irupt_77h, 0x18);
	substitute_irupt_address_vector(0x78, (void*) irupt_78h, 0x18);
	substitute_irupt_address_vector(0x79, (void*) irupt_79h, 0x18);
	substitute_irupt_address_vector(0x7a, (void*) irupt_7ah, 0x18);
	substitute_irupt_address_vector(0x7b, (void*) irupt_7bh, 0x18);
	substitute_irupt_address_vector(0x7c, (void*) irupt_7ch, 0x18);
	substitute_irupt_address_vector(0x7d, (void*) irupt_7dh, 0x18);
	substitute_irupt_address_vector(0x7e, (void*) irupt_7eh, 0x18);
	substitute_irupt_address_vector(0x7f, (void*) irupt_7fh, 0x18);
	//bugCheck();
	initKeyboard8042(kbdBuf, KBDBUF_SIZE, 0, &kbdMain);
	//bugCheck();
	int_enable();
	//bugCheck();
	initializeVGATerminal(&mainTerm, 80, 25, (struct VGACell*) (0x000b8000 - RELOC), kbd8042_read);
	for (unsigned int i = (0x000b8000 - RELOC); i < (0x000b8fa0 - RELOC); i += 2) {
		((struct VGACell*) i)->format = mainTerm.format;
		((struct VGACell*) i)->text = 0x20;
	}
	/* Style */
	((struct VGACell*) (0x000b8000 - RELOC))->format ^= 0x77;
	mainTerm.onlcr = 1;
	mainTerm.cursor = 1;
	substitute_irupt_address_vector(0x80, (void*) irupt_80h, 0x0018);
	/* End-of-style */
	int retVal = 0;
	//bugCheck();
	int errVal = runELF((void*) 0x00020000, (void*) 0x00800000, &retVal);
	(*((unsigned char*) (0xb8000 - RELOC))) = errVal + 0x30;
	while (1) {
	}
}
