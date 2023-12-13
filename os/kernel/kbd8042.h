#ifndef __KBD8042_H__
#define __KBD8042_H__ 1
#include "types.h"
#include "keysPC.h"
#include "errno.h"
#define FAILMASK_KBD8042 0x00020000
struct Keyboard8042 {
	size_t bufSize;
	unsigned char* buf;
	size_t avail;
	Mutex bufLock;// Lock over reading and writing of `avail' and the data at `buf'
	u8 ID;
	unsigned char set;// 1: set 1; 2: set 2; 3: set 3
	unsigned char capL;
	unsigned char scrlL;
	unsigned char numL;
	unsigned char shift;
	unsigned char ctrl;
	unsigned char alt;
};
extern struct Keyboard8042 kbdMain;
void initKeyboard8042(unsigned char* buf, size_t bufSize, u8 ID, struct Keyboard8042* kbd) {// Disables other PS/2 device connected to the controller
	if (ID != 0) {
		bugCheckNum(0x0101 | FAILMASK_KBD8042);
	}
	kbd->ID = ID;
	Mutex_initUnlocked(&(kbd->bufLock));
	kbd->buf = buf;
	kbd->bufSize = bufSize;
	kbd->avail = 0;
	kbd->capL = 0;
	kbd->scrlL = 0;
	kbd->numL = 0;
	kbd->shift = 0;
	kbd->ctrl = 0;
	kbd->alt = 0;
	//bugCheck();
	while (bus_in_u8(0x0064) & 0x02) {
		bus_wait();
	}
	//bugCheck();
	bus_out_u8(0x0064, 0xad);
	bus_wait();
	while (bus_in_u8(0x0064) & 0x02) {
		bus_wait();
	}
	bus_out_u8(0x0064, 0xa7);// TODO Is this ignored on chips that only support one PS/2 port?
	bus_wait();
	while (bus_in_u8(0x0064) & 0x01) {
		bus_in_u8(0x0060);
		bus_wait();
	}
	while (bus_in_u8(0x0064) & 0x02) {
		bus_wait();
	}
	bus_out_u8(0x0064, 0x20);
	while (1) {
		if (bus_in_u8(0x0064) & 0x01) {
			break;
		}
		bus_wait();
	}
	u8 setup = bus_in_u8(0x0060);
	setup &= 0x3f;// TODO Prevent keyboard interrupt from executing before controller initialisation is complete
	bus_wait();
	while (bus_in_u8(0x0064) & 0x02) {
		bus_wait();
	}
	bus_out_u8(0x0064, 0x60);
	bus_wait();
	while (bus_in_u8(0x0064) & 0x02) {
		bus_wait();
	}
	bus_out_u8(0x0060, setup);
	bus_wait();
	while (bus_in_u8(0x0064) & 0x02) {
		bus_wait();
	}
	bus_out_u8(0x0064, 0xaa);
	while (1) {
		if (bus_in_u8(0x0064) & 0x01) {
			break;
		}
		bus_wait();
	}
	u8 res = bus_in_u8(0x0060);
	if (res != 0x55) {
		bugCheckNum(0x0200 | res | FAILMASK_KBD8042);
	}
	bus_wait();// In case of chip's internal output buffer clearing time need
	while (bus_in_u8(0x0064) & 0x02) {
		bus_wait();
	}
	bus_out_u8(0x0064, 0xab);
	while (1) {
		if (bus_in_u8(0x0064) & 0x01) {
			break;
		}
		bus_wait();
	}
	res = bus_in_u8(0x0060);
	if (res != 0x00) {
		bugCheckNum(0x0300 | res | FAILMASK_KBD8042);
	}
	bus_wait();// In case of chip's internal output buffer clearing time need
	while (bus_in_u8(0x0064) & 0x02) {
		bus_wait();
	}
	bus_out_u8(0x0064, 0xae);
	bus_wait();
	while (bus_in_u8(0x0064) & 0x01) {
		bus_in_u8(0x0060);
		bus_wait();
	}
	initKeyboard8042__disable_sc:
	while (bus_in_u8(0x0064) & 0x02) {
		bus_wait();
	}
	bus_out_u8(0x0060, 0xf5);// TODO Remove possibility of data coming through between flushing of the PS/2 data and disabling of scanning
	bus_wait();
	while (1) {
		if (bus_in_u8(0x0064) & 0x01) {
			break;
		}
		bus_wait();
	}
	//bugCheck();
	res = bus_in_u8(0x0060);
	if (res == 0xfe) {
		goto initKeyboard8042__disable_sc;
	}
	if (res != 0xfa) {
		bugCheckNum(0x0400 | res | FAILMASK_KBD8042);
	}
	initKeyboard8042__reset:
	while (bus_in_u8(0x0064) & 0x02) {
		bus_wait();
	}
	bus_out_u8(0x0060, 0xff);
	bus_wait();
	while (1) {
		if (bus_in_u8(0x0064) & 0x01) {
			break;
		}
		bus_wait();
	}
	initKeyboard8042_resetIntake:
	res = bus_in_u8(0x0060);
	if (res == 0xfe) {
		goto initKeyboard8042__reset;
	}
	if ((res != 0xaa) && (res != 0xfa)) {
		bugCheckNum(0x0500 | res | FAILMASK_KBD8042);
	}
	if (bus_in_u8(0x0064) & 0x01) {
		goto initKeyboard8042_resetIntake;
	}
	initKeyboard8042__getSet:
	while (bus_in_u8(0x0064) & 0x02) {
		bus_wait();
	}
	bus_out_u8(0x0060, 0xf0);
	bus_wait();
	while (bus_in_u8(0x0064) & 0x02) {
		bus_wait();
	}
	bus_out_u8(0x0060, 0x00);
	bus_wait();
	while (1) {
		if (bus_in_u8(0x0064) & 0x01) {
			break;
		}
		bus_wait();
	}
	res = bus_in_u8(0x0060);
	if (res == 0xfe) {
		goto initKeyboard8042__getSet;
	}
	if (res != 0xfa) {
		bugCheckNum(0x0600 | res | FAILMASK_KBD8042);
	}
	while (1) {
		if (bus_in_u8(0x0064) & 0x01) {
			break;
		}
		bus_wait();
	}
	res = bus_in_u8(0x0060);
	switch (res) {
		case (1):
		case (0x43):
			kbd->set = 1;
			break;
		case (2):
		case (0x41):
			kbd->set = 2;
			break;
		case (3):
		case (0x3f):
			kbd->set = 3;
			break;
		default:
			bugCheckNum(0x0700 | res | FAILMASK_KBD8042);
	}
	initKeyboard8042__enable_sc:
	while (bus_in_u8(0x0064) & 0x02) {
		bus_wait();
	}
	bus_out_u8(0x0060, 0xf4);
	bus_wait();
	while (1) {
		if (bus_in_u8(0x0064) & 0x01) {
			break;
		}
		bus_wait();
	}
	res = bus_in_u8(0x0060);// TODO Can scan data come before the response of success?
	if (res == 0xfe) {
		goto initKeyboard8042__enable_sc;
	}
	if (res != 0xfa) {
		bugCheckNum(0x0800 | res | FAILMASK_KBD8042);
	}
	/*
	setup |= 0x01;
	while (bus_in_u8(0x0064) & 0x02) {// TODO Remove if unnecessary
		bus_wait();
	}
	bus_out_u8(0x0064, 0x60);
	bus_wait();
	while (bus_in_u8(0x0064) & 0x02) {
		bus_wait();
	}
	bus_out_u8(0x0060, setup);
	bus_wait();
	while (bus_in_u8(0x0064) & 0x02) {// Waiting for sureness of the IRQ having been enabled
		bus_wait();
	}
	*/
	return;
}
void kbd8042_oldMultithreadOnIRQ(struct Keyboard8042* kbd) {
	Mutex_acquire(&(kbd->bufLock));
	while (1) {
		if (kbd->avail == kbd->bufSize) {
			Mutex_release(&(kbd->bufLock));
			return;
		}
		if (kbd->ID != 0x00) {
			bugCheckNum(0x0102 | FAILMASK_KBD8042);
		}
		if (!(bus_in_u8(0x0064) & 0x01)) {
			Mutex_release(&(kbd->bufLock));
			return;
		}
		kbd->buf[kbd->avail] = bus_in_u8(0x0060);
		(kbd->avail)++;
	}
}
void kbd8042_serviceIRQ(struct Keyboard8042* kbd) {// `bufLock' must have been acquired and is NOT released
	while (1) {
		if (kbd->avail == kbd->bufSize) {
			return;
		}
		if (kbd->ID != 0x00) {
			bugCheckNum(0x0102 | FAILMASK_KBD8042);
		}
		if (!(bus_in_u8(0x0064) & 0x01)) {
			return;
		}
		kbd->buf[kbd->avail] = bus_in_u8(0x0060);
		(kbd->avail)++;
	}
}
void kbd8042_onIRQ(struct Keyboard8042* kbd) {
	//bugCheck();
	int service = Mutex_tryAcquire(&(kbd->bufLock));
	if (!service) {
		return;//TODO URGENT Set timer to service the IRQ when it can
	}
	//bugCheck();
	kbd8042_serviceIRQ(kbd);
	Mutex_release(&(kbd->bufLock));
	return;
}
ssize_t kbd8042_readGiven(void* dat, size_t count, struct Keyboard8042* kbd) {
	size_t oCount = count;
	unsigned char* dats = (unsigned char*) dat;
	Mutex_acquire(&(kbd->bufLock));
	while (1) {
		if (kbd->avail == 0) {
			Mutex_release(&(kbd->bufLock));
			Mutex_wait();
			//for (int i = 0; i < 100000; i++) {
			//	Mutex_wait();
			//}
			Mutex_acquire(&(kbd->bufLock));
			continue;
		}
		if (kbd->avail >= count) {
			cpy(dats, kbd->buf, count);
			kbd->avail -= count;
			move(kbd->buf, kbd->buf + count, kbd->avail);
			Mutex_release(&(kbd->bufLock));
			return oCount;
		}
		count -= kbd->avail;
		cpy(dats, kbd->buf, kbd->avail);
		kbd->avail = 0;
	}
}
ssize_t kbd8042_read(int kfd, void* dat, size_t len) {
	if (kfd != 0) {
		bugCheckNum(FAILMASK_KBD8042 | EBADF);
	}
	return kbd8042_readGiven(dat, len, &kbdMain);
}
unsigned char kbdPS2_set1_0[] = {0x00,// Array: Non-prefixed
	KEY_ESCAPE,
	KEY_1,
	KEY_2,
	KEY_3,
	KEY_4,
	KEY_5,
	KEY_6,
	KEY_7,
	KEY_8,
	KEY_9,
	KEY_0,
	KEY_DASH,
	KEY_EQUALS,
	KEY_BACKSPACE,
	KEY_TAB,
	KEY_Q,
	KEY_W,
	KEY_E,
	KEY_R,
	KEY_T,
	KEY_Y,
	KEY_U,
	KEY_I,
	KEY_O,
	KEY_P,
	KEY_LBRACK,
	KEY_RBRACK,
	KEY_ENTER,
	KEY_LCTRL,
	KEY_A,
	KEY_S,
	KEY_D,
	KEY_F,
	KEY_G,
	KEY_H,
	KEY_J,
	KEY_K,
	KEY_L,
	KEY_SEMICOLON,
	KEY_SINGLEQUOTE,
	KEY_BACKTICK,
	KEY_LSHIFT,
	KEY_BACKSLASH,
	KEY_Z,
	KEY_X,
	KEY_C,
	KEY_V,
	KEY_B,
	KEY_N,
	KEY_M,
	KEY_COMMA,
	KEY_DOT,
	KEY_SLASH,
	KEY_RSHIFT,
	KEY_KPASTERISK,
	KEY_LALT,
	KEY_SPACE,
	KEY_CALO,
	KEY_F1,
	KEY_F2,
	KEY_F3,
	KEY_F4,
	KEY_F5,
	KEY_F6,
	KEY_F7,
	KEY_F8,
	KEY_F9,
	KEY_F10,
	KEY_KPNULO,
	KEY_SCLO,
	KEY_KP7,
	KEY_KP8,
	KEY_KP9,
	KEY_KPDASH,
	KEY_KP4,
	KEY_KP5,
	KEY_KP6,
	KEY_KPPLUS,
	KEY_KP1,
	KEY_KP2,
	KEY_KP3,
	KEY_KP0,
	KEY_KPDOT,
	0x00,
	0x00,
	0x00,
	KEY_F11,
	KEY_F12,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00};
unsigned char kbdPS2_set1_1[] = {0x00,// Array: 0xe0 prefix
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	KEY_KPENTER,
	KEY_RCTRL,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	KEY_PRSC,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	KEY_KPSLASH,
	0x00,
	0x00,
	KEY_RALT,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	KEY_HOME,
	KEY_UP,
	KEY_PAUP,
	0x00,
	KEY_LEFT,
	0x00,
	KEY_RIGHT,
	0x00,
	KEY_END,
	KEY_DOWN,
	KEY_PADO,
	KEY_INSERT,
	KEY_DELETE};
#endif
