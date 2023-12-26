#ifndef __KBD8042_H__
#define __KBD8042_H__ 1
#include "types.h"
#include "keysPC.h"
#include "errno.h"
#define FAILMASK_KBD8042 0x00020000
struct Keyboard8042 {
	size_t bufSize;
	size_t bufTermSize;
	unsigned char* buf;
	size_t avail;
	unsigned char* bufTerm;
	size_t availTerm;
	Mutex bufLock;// Lock over reading and writing of `avail' and `availTerm' and the data at `buf' and the data at `bufTerm'
	u8 ID;
	unsigned char set;// 1: set 1; 2: set 2; 3: set 3
	unsigned char capL;
	unsigned char scrlL;
	unsigned char numL;
	unsigned char shift;
	unsigned char ctrl;
	unsigned char alt;
	unsigned char shiftL;
	unsigned char shiftR;
	unsigned char altL;
	unsigned char altR;
	unsigned char ctrlL;
	unsigned char ctrlR;
	unsigned char modeApp;
	u8 state;
};
extern struct Keyboard8042 kbdMain;
void initKeyboard8042(unsigned char* buf, size_t bufSize, unsigned char* bufTerm, size_t bufTermSize, u8 ID, struct Keyboard8042* kbd) {// Disables other PS/2 device connected to the controller
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
	kbd->shiftL = 0;
	kbd->shiftR = 0;
	kbd->altL = 0;
	kbd->altR = 0;
	kbd->ctrlL = 0;
	kbd->ctrlR = 0;
	kbd->modeApp = 0;
	kbd->bufTerm = bufTerm;
	kbd->bufTermSize = bufTermSize;
	kbd->availTerm = 0;
	while (bus_in_u8(0x0064) & 0x02) {
		bus_wait();
	}
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
int kbd8042_outChar(unsigned char val, struct Keyboard8042* kbd) {// Mutex `bufLock' must have been acquired and is NOT released
	if (kbd->ctrl) {
		val &= 0x1f;
	}
	if (kbd->alt) {
		if (!((kbd->bufTermSize - kbd->availTerm) & (~((size_t) 1)))) {
			return (-1);
		}
		kbd->bufTerm[kbd->availTerm] = 0x1b;
		kbd->bufTerm[kbd->availTerm + 1] = val;
		kbd->availTerm += 2;
		return 0;
	}
	if (!(kbd->bufTermSize - kbd->availTerm)) {
		return (-1);
	}
	kbd->bufTerm[kbd->availTerm] = val;
	kbd->availTerm++;
	return 0;
}
int kbd8042_outSeq(const void* dat, size_t siz, struct Keyboard8042* kbd) {// Mutex `bufLock' must have been acquired and is NOT released
	if (kbd->alt) {
		if ((kbd->availTerm + siz + 1) > kbd->bufTermSize) {
			return (-1);
		}
		kbd->bufTerm[kbd->availTerm] = 0x1b;
		kbd->availTerm++;
	}
	else {
		if ((kbd->availTerm + siz) > kbd->bufTermSize) {
			return (-1);
		}
	}
	cpy(kbd->bufTerm + kbd->availTerm, dat, siz);
	kbd->availTerm += siz;
	return 0;
}
int kbd8042_process(unsigned char dat, struct Keyboard8042* kbd) {// Mutex `bufLock' must have been acquired and is NOT released
	if (kbd->set != 2) {
		return (-1);// TODO Implement
	}
	u8 state = kbd->state;
	u16 code = ((u16) dat) + (((u16) state) << 8);// State when set 2: 0: N; 1: 0xe0 N; 2: 0xf0 N; 3: 0xe0 0xf0 N; 4: 0xe1 N; 5: 0xe1 0x14 N; 6: 0xe1 0xf0 N; 7: 0xe1 0xf0 0x14 N; 8: 0xe1 0xf0 0x14 0xf0 N
	unsigned char caps = (kbd->shift ? 1 : 0) ^ (kbd->capL ? 1 : 0);
	unsigned char shift = kbd->shift;
	unsigned char ctrl = kbd->ctrl;
	unsigned char numL = kbd->numL;
	unsigned char modeApp = kbd->modeApp;
	int i = 0;
	switch (code) {
		case (0xe0):
			kbd->state = 1;
			break;
		case (0xf0):
			kbd->state = 2;
			break;
		case (0x01f0):
			kbd->state = 3;
			break;
		case (0xe1):
			kbd->state = 4;
			break;
		case (0x0414):
			kbd->state = 5;
			break;
		case (0x04f0):
			kbd->state = 6;
			break;
		case (0x0614):
			kbd->state = 7;
			break;
		case (0x07f0):
			kbd->state = 8;
			break;
		case (0x0577):
			i = kbd8042_outChar(0x1a, kbd);
			break;
		case (0x1c):
			i = kbd8042_outChar(caps ? 0x41 : 0x61, kbd);
			break;
		case (0x32):
			i = kbd8042_outChar(caps ? 0x42 : 0x62, kbd);
			break;
		case (0x21):
			i = kbd8042_outChar(caps ? 0x43 : 0x63, kbd);
			break;
		case (0x23):
			i = kbd8042_outChar(caps ? 0x44 : 0x64, kbd);
			break;
		case (0x24):
			i = kbd8042_outChar(caps ? 0x45 : 0x65, kbd);
			break;
		case (0x2b):
			i = kbd8042_outChar(caps ? 0x46 : 0x66, kbd);
			break;
		case (0x34):
			i = kbd8042_outChar(caps ? 0x47 : 0x67, kbd);
			break;
		case (0x33):
			i = kbd8042_outChar(caps ? 0x48 : 0x68, kbd);
			break;
		case (0x43):
			i = kbd8042_outChar(caps ? 0x49 : 0x69, kbd);
			break;
		case (0x3b):
			i = kbd8042_outChar(caps ? 0x4a : 0x6a, kbd);
			break;
		case (0x42):
			i = kbd8042_outChar(caps ? 0x4b : 0x6b, kbd);
			break;
		case (0x4b):
			i = kbd8042_outChar(caps ? 0x4c : 0x6c, kbd);
			break;
		case (0x3a):
			i = kbd8042_outChar(caps ? 0x4d : 0x6d, kbd);
			break;
		case (0x31):
			i = kbd8042_outChar(caps ? 0x4e : 0x6e, kbd);
			break;
		case (0x44):
			i = kbd8042_outChar(caps ? 0x4f : 0x6f, kbd);
			break;
		case (0x4d):
			i = kbd8042_outChar(caps ? 0x50 : 0x70, kbd);
			break;
		case (0x15):
			i = kbd8042_outChar(caps ? 0x51 : 0x71, kbd);
			break;
		case (0x2d):
			i = kbd8042_outChar(caps ? 0x52 : 0x72, kbd);
			break;
		case (0x1b):
			i = kbd8042_outChar(caps ? 0x53 : 0x73, kbd);
			break;
		case (0x2c):
			i = kbd8042_outChar(caps ? 0x54 : 0x74, kbd);
			break;
		case (0x3c):
			i = kbd8042_outChar(caps ? 0x55 : 0x75, kbd);
			break;
		case (0x2a):
			i = kbd8042_outChar(caps ? 0x56 : 0x76, kbd);
			break;
		case (0x1d):
			i = kbd8042_outChar(caps ? 0x57 : 0x77, kbd);
			break;
		case (0x22):
			i = kbd8042_outChar(caps ? 0x58 : 0x78, kbd);
			break;
		case (0x35):
			i = kbd8042_outChar(caps ? 0x59 : 0x79, kbd);
			break;
		case (0x1a):
			i = kbd8042_outChar(caps ? 0x5a : 0x7a, kbd);
			break;
		case (0x45):
			i = kbd8042_outChar(shift ? 0x29 : 0x30, kbd);
			break;
		case (0x16):
			i = kbd8042_outChar(shift ? 0x21 : 0x31, kbd);
			break;
		case (0x1e):
			i = kbd8042_outChar(shift ? 0x40 : 0x32, kbd);
			break;
		case (0x26):
			i = kbd8042_outChar(shift ? 0x23 : 0x33, kbd);
			break;
		case (0x25):
			i = kbd8042_outChar(shift ? 0x24 : 0x34, kbd);
			break;
		case (0x2e):
			i = kbd8042_outChar(shift ? 0x25 : 0x35, kbd);
			break;
		case (0x36):
			i = kbd8042_outChar(shift ? 0x5e : 0x36, kbd);
			break;
		case (0x3d):
			i = kbd8042_outChar(shift ? 0x26 : 0x37, kbd);
			break;
		case (0x3e):
			i = kbd8042_outChar(shift ? 0x2a : 0x38, kbd);
			break;
		case (0x46):
			i = kbd8042_outChar(shift ? 0x28 : 0x39, kbd);
			break;
		case (0x0e):
			i = kbd8042_outChar(shift ? 0x7e : 0x60, kbd);
			break;
		case (0x4e):
			i = kbd8042_outChar(shift ? 0x5f : 0x2d, kbd);
			break;
		case (0x55):
			i = kbd8042_outChar(shift ? 0x2b : 0x3d, kbd);
			break;
		case (0x5d):
			i = kbd8042_outChar(shift ? 0x7c : 0x5c, kbd);
			break;
		case (0x66):
			i = kbd8042_outChar(0x7f, kbd);
			break;
		case (0x29):
			i = kbd8042_outChar(ctrl ? 0x00 : 0x20, kbd);
			break;
		case (0x0d):
			i = kbd8042_outChar(0x09, kbd);
			break;
		case (0x58):
			kbd->capL = !kbd->capL;
			// TODO Lock lights
			break;
		case (0x12):
			kbd->shiftL = 1;
			kbd->shift = 1;
			break;
		case (0x0212):
			kbd->shiftL = 0;
			if (!kbd->shiftR) {
				kbd->shift = 0;
			}
			break;
		case (0x14):
			kbd->ctrlL = 1;
			kbd->ctrl = 1;
			break;
		case (0x0214):
			kbd->ctrlL = 0;
			if (!kbd->ctrlR) {
				kbd->ctrl = 0;
			}
			break;
		case (0x11):
			kbd->altL = 1;
			kbd->alt = 1;
			break;
		case (0x0211):
			kbd->altL = 0;
			if (!kbd->altR) {
				kbd->alt = 0;
			}
			break;
		case (0x59):
			kbd->shiftR = 1;
			kbd->shift = 1;
			break;
		case (0x0259):
			kbd->shiftR = 0;
			if (!kbd->shiftL) {
				kbd->shift = 0;
			}
			break;
		case (0x5a):
		case (0x015a):
			i = kbd8042_outChar(0x0d, kbd);
			break;
		case (0x76):
			i = kbd8042_outChar(0x1b, kbd);
			break;
		case (0x05):
			i = kbd8042_outSeq("\033OP", 3, kbd);
			break;
		case (0x06):
			i = kbd8042_outSeq("\033OQ", 3, kbd);
			break;
		case (0x04):
			i = kbd8042_outSeq("\033OR", 3, kbd);
			break;
		case (0x0c):
			i = kbd8042_outSeq("\033OS", 3, kbd);
			break;
		case (0x03):
			i = kbd8042_outSeq("\033[15~", 5, kbd);
			break;
		case (0x0b):
			i = kbd8042_outSeq("\033[17~", 5, kbd);
			break;
		case (0x83):
			i = kbd8042_outSeq("\033[18~", 5, kbd);
			break;
		case (0x0a):
			i = kbd8042_outSeq("\033[19~", 5, kbd);
			break;
		case (0x01):
			i = kbd8042_outSeq("\033[20~", 5, kbd);
			break;
		case (0x09):
			i = kbd8042_outSeq("\033[21~", 5, kbd);
			break;
		case (0x78):
			i = kbd8042_outSeq("\033[23~", 5, kbd);
			break;
		case (0x07):
			i = kbd8042_outSeq("\033[24~", 5, kbd);
			break;
		case (0x7e):
			kbd->scrlL = 1;
			// TODO Lock lights
			break;
		case (0x54):
			i = kbd8042_outChar(shift ? 0x7b : 0x5b, kbd);
			break;
		case (0x77):
			kbd->numL = 1;
			// TODO Lock lights
			break;
		case (0x7c):
			i = kbd8042_outChar(0x2a, kbd);
			break;
		case (0x7b):
			i = kbd8042_outChar(0x2d, kbd);
			break;
		case (0x79):
			i = kbd8042_outChar(0x2b, kbd);
			break;
		case (0x71):
			if (numL) {
				i = kbd8042_outChar(0x2e, kbd);
			}
			else {
				i = kbd8042_outSeq("\033[3~", 4, kbd);
			}
			break;
		case (0x70):
			if (numL) {
				i = kbd8042_outChar(0x30, kbd);
			}
			else {
				i = kbd8042_outSeq("\033[2~", 4, kbd);
			}
			break;
		case (0x69):
			if (numL) {
				i = kbd8042_outChar(0x31, kbd);
			}
			else {
				i = kbd8042_outSeq(modeApp ? "\033OF" : "\033[F", 3, kbd);
			}
			break;
		case (0x72):
			if (numL) {
				i = kbd8042_outChar(0x32, kbd);
				break;
			}
		case (0x0172):
			if (ctrl) {
				i = kbd8042_outSeq("\033[1;5B", 6, kbd);
			}
			else {
				i = kbd8042_outSeq(modeApp ? "\033OB" : "\033[B", 3, kbd);
			}
			break;
		case (0x7a):
			if (numL) {
				i = kbd8042_outChar(0x33, kbd);
			}
			else {
				i = kbd8042_outSeq("\033[6~", 4, kbd);
			}
			break;
		case (0x6b):
			if (numL) {
				i = kbd8042_outChar(0x34, kbd);
				break;
			}
		case (0x016b):
			if (ctrl) {
				i = kbd8042_outSeq("\033[1;5D", 6, kbd);
			}
			else {
				i = kbd8042_outSeq(modeApp ? "\033OD" : "\033[D", 3, kbd);
			}
			break;
		case (0x73):
			if (numL) {
				i = kbd8042_outChar(0x35, kbd);
			}
			break;
		case (0x74):
			if (numL) {
				i = kbd8042_outChar(0x36, kbd);
				break;
			}
		case (0x0174):
			if (ctrl) {
				i = kbd8042_outSeq("\033[1;5C", 6, kbd);
			}
			else {
				i = kbd8042_outSeq(modeApp ? "\033OC" : "\033[C", 3, kbd);
			}
			break;
		case (0x6c):
			if (numL) {
				i = kbd8042_outChar(0x37, kbd);
			}
			else {
				i = kbd8042_outSeq(modeApp ? "\033OH" : "\033[H", 3, kbd);
			}
			break;
		case (0x75):
			if (numL) {
				i = kbd8042_outChar(0x38, kbd);
				break;
			}
		case (0x0175):
			if (ctrl) {
				i = kbd8042_outSeq("\033[1;5A", 6, kbd);
			}
			else {
				i = kbd8042_outSeq(modeApp ? "\033OA" : "\033[A", 3, kbd);
			}
			break;
		case (0x7d):
			if (numL) {
				i = kbd8042_outChar(0x39, kbd);
			}
			else {
				i = kbd8042_outSeq("\033[5~", 4, kbd);
			}
			break;
		case (0x5b):
			i = kbd8042_outChar(shift ? 0x7d : 0x5d, kbd);
			break;
		case (0x4c):
			i = kbd8042_outChar(shift ? 0x3a : 0x3b, kbd);
			break;
		case (0x52):
			i = kbd8042_outChar(shift ? 0x22 : 0x27, kbd);
			break;
		case (0x41):
			i = kbd8042_outChar(shift ? 0x3c : 0x2c, kbd);
			break;
		case (0x49):
			i = kbd8042_outChar(shift ? 0x3e : 0x2e, kbd);
			break;
		case (0x4a):
			i = kbd8042_outChar(shift ? 0x3f : 0x2f, kbd);
			break;
		case (0x0114):
			kbd->ctrlR = 1;
			kbd->ctrl = 1;
			break;
		case (0x0314):
			kbd->ctrlR = 0;
			if (!kbd->ctrlL) {
				kbd->ctrl = 0;
			}
			break;
		case (0x0111):
			kbd->altR = 1;
			kbd->alt = 1;
			break;
		case (0x0311):
			kbd->altR = 0;
			if (!kbd->altL) {
				kbd->alt = 0;
			}
			break;
		case (0x0170):
			i = kbd8042_outSeq("\033[2~", 4, kbd);
			break;
		case (0x016c):
			i = kbd8042_outSeq(modeApp ? "\033OH" : "\033[H", 3, kbd);
			break;
		case (0x017d):
			i = kbd8042_outSeq("\033[5~", 4, kbd);
			break;
		case (0x0171):
			i = kbd8042_outSeq("\033[3~", 4, kbd);
			break;
		case (0x0169):
			i = kbd8042_outSeq(modeApp ? "\033OF" : "\033[F", 3, kbd);
			break;
		case (0x017a):
			i = kbd8042_outSeq("\033[6~", 4, kbd);
			break;
		case (0x014a):
			i = kbd8042_outChar(0x2f, kbd);
			break;
		default:
			break;
	}
	if (i) {
		return (-1);
	}
	if (state == kbd->state) {
		kbd->state = 0;
	}
	return 0;
}
void kbd8042_serviceIRQ(struct Keyboard8042* kbd) {// Mutex `bufLock' must have been acquired and is NOT released
	if (kbd->ID != 0x00) {
		bugCheckNum(0x0102 | FAILMASK_KBD8042);
	}
	int i = 0;
	while (1) {
		if (kbd->avail == i) {
			kbd->avail = 0;
			i = 0;
			break;
		}
		if (kbd8042_process(kbd->buf[i], kbd)) {
			if (i) {
				kbd->avail -= i;
				move(kbd->buf, kbd->buf + i, kbd->avail);
			}
			i = 1;
			break;
		}
		i++;
	}
	if (!i) {
		while (1) {
			if (!(bus_in_u8(0x0064) & 0x01)) {
				return;
			}
			unsigned char res = bus_in_u8(0x0060);
			if (kbd8042_process(res, kbd)) {
				kbd->buf[kbd->avail = 1] = res;
				break;
			}
		}
	}
	while (1) {
		if (kbd->avail == kbd->bufSize) {
			return;
		}
		if (!(bus_in_u8(0x0064) & 0x01)) {
			return;
		}
		kbd->buf[kbd->avail] = bus_in_u8(0x0060);
		kbd->avail++;
	}
}
void kbd8042_onIRQ(struct Keyboard8042* kbd) {
	int service = Mutex_tryAcquire(&(kbd->bufLock));
	if (!service) {
		return;//TODO URGENT Set timer to service the IRQ when it can
	}
	kbd8042_serviceIRQ(kbd);
	Mutex_release(&(kbd->bufLock));
	return;
}
ssize_t kbd8042_readGiven(void* dat, size_t count, struct Keyboard8042* kbd) {
	size_t oCount = count;
	unsigned char* dats = (unsigned char*) dat;
	Mutex_acquire(&(kbd->bufLock));
	while (1) {
		if (kbd->availTerm == 0) {
			Mutex_release(&(kbd->bufLock));
			Mutex_wait();
			Mutex_acquire(&(kbd->bufLock));
			continue;
		}
		if (kbd->availTerm >= count) {
			cpy(dats, kbd->bufTerm, count);
			kbd->availTerm -= count;
			move(kbd->bufTerm, kbd->bufTerm + count, kbd->availTerm);
			Mutex_release(&(kbd->bufLock));
			return oCount;
		}
		count -= kbd->availTerm;
		cpy(dats, kbd->bufTerm, kbd->availTerm);
		kbd->availTerm = 0;
	}
}
ssize_t kbd8042_read(int kfd, void* dat, size_t len) {
	if (kfd != 1) {
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
