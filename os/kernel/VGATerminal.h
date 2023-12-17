#ifndef __KERNEL__VGATERMINAL_H__
#define __KERNEL__VGATERMINAL_H__ 1
#define FAILMASK_VGATERMINAL 0x00030000
#include "types.h"
struct VGACell {
	unsigned char text;
	unsigned char format;
};
struct VGATerminal {
	/* Basic information */
	unsigned char extra[15];
	unsigned char format;
	unsigned int pos;
	unsigned int width;
	unsigned int total;
	struct VGACell* screen;
	/* Terminal settings */
	unsigned char onlcr;
	unsigned char cursor;
	ssize_t (*read)(int, void*, size_t);
};
extern struct VGATerminal mainTerm;
void initializeVGATerminal(struct VGATerminal* term, unsigned int width, unsigned int height, void* screen, ssize_t (*read)(int, void*, size_t)) {
	term->extra[0] = 0;
	term->format = 0x07;
	term->pos = 0;
	term->width = width;
	term->total = width * height;
	term->screen = screen;
	term->onlcr = 0;
	term->cursor = 0;
	term->read = read;
	return;
}
void VGATerminalAdjustVis(struct VGATerminal* term) {
	unsigned int w = term->width;
	unsigned int t = term->total;
	struct VGACell* screen = term->screen;
	unsigned char format = term->format;
	while (term->pos >= t) {
		term->pos -= w;
		unsigned int i = 0;
		for (unsigned int o = w; o < t; o++) {
			screen[i].text = screen[o].text;
			screen[i].format = screen[o].format;
			i++;
		}
		for (unsigned int o = t - w; o < t; o++) {
			screen[o].text = 0x20;
			screen[o].format = format;
		}
	}
	return;
}
void VGATerminalWriteCodePoint(struct VGATerminal* term, unsigned int cp) {
	if (cp >= 127) {
		return;/* TODO Actual support for non-ascii characters in CP437 */
	}
	if (cp < 32) {
		if (cp == 0x0a) {
			if (term->onlcr) {
				term->pos = term->pos - (term->pos % term->width);
			}
			term->pos += term->width;
		}
		else if (cp == 0x0d) {
			term->pos = term->pos - (term->pos % term->width);
		}
		else {
			return;/* TODO Support all applicable C0 control codes */
		}
		VGATerminalAdjustVis(term);
		return;
	}
	term->screen[term->pos].text = (unsigned char) cp;
	term->screen[term->pos].format = term->format;
	term->pos++;
	if (term->pos >= term->total) {
		VGATerminalAdjustVis(term);
	}
	return;
}
unsigned int getCodePoint(unsigned char* data, unsigned int* len) {/* Returns the next UTF-8-encoded Unicode code point and lowers the value at `len' accordingly; if there is not sufficient length for decoding, 0 is returned and the value at `len' remains unchanged; if there is a decoding error, 0 is returned and the value at `len' is adjusted according to the amount of data consumed; 0 cannot be returned with the value at `len' unchanged if the value of `len' at entry is at least 8 *//* TODO Implement correct UTF-8 support */
	if ((*len) == 0) {
		return 0;
	}
	(*len)--;
	return *data;
}
unsigned int writeDataVGATerminal(struct VGATerminal* term, unsigned char* data, unsigned int len) {/* For UTF-8 */
	if (len == 0) {
		return 0;
	}
	if (term->extra[0] != 0) {/* Safe since 0x00 is nul in UTF-8 */
		unsigned int i;
		for (i = 1; i < 15; i++) {
			if (term->extra[i] == 0) {
				break;
			}
		}
		unsigned int m = 15 - i;
		if (len < m) {
			m = len;
		}
		m += i;
		for (int o = 0; i < m; i++) {
			term->extra[i] = data[o];
			o++;
		}
		int k = m;
		unsigned int cp = getCodePoint(term->extra, &m);
		if (m == k) {
			if (m == 15) {
				bugCheck();
			}
			term->extra[m] = 0x00;
			return len;
		}
		VGATerminalWriteCodePoint(term, cp);
		term->extra[0] = 0x00;
		len -= (k - m);
		data += (k - m);
		return (k - m) + writeDataVGATerminal(term, data, len);
	}
	int j;
	int cp;
	int l1 = len;
	while (1) {
		j = len;
		cp = getCodePoint(data, &len);
		if (len == j) {
			if (len > 15) {
				bugCheck();
			}
			for (int i = 0; i < len; i++) {
				term->extra[i] = data[i];
			}
			if (len != 15) {
				term->extra[len] = 0x00;
			}
			return l1;
		}
		VGATerminalWriteCodePoint(term, cp);
		if (len == 0) {
			return l1;
		}
		data += (j - len);
	}
}
ssize_t VGATerminalWrite(struct VGATerminal* term, unsigned char* data, unsigned int len) {/* For UTF-8 */
	if (term->cursor) {
		(term->screen[term->pos].format) ^= 0x77;
	}
	unsigned int res = writeDataVGATerminal(term, data, len);
	if (term->cursor) {
		(term->screen[term->pos].format) ^= 0x77;
	}
	return res;
}
#include "types.h"
#include "errno.h"
#include "perProcess.h"
ssize_t VGATerminal_write(int kfd, const void* data, size_t len) {
	if (kfd != 1) {
		bugCheckNum(FAILMASK_VGATERMINAL | 0x0100 | EBADF);
	}
	return VGATerminalWrite(&mainTerm, (unsigned char*) data, len);
}
ssize_t VGATerminal_read(int kfd, void* data, size_t siz) {
	if (kfd != 0) {
		bugCheckNum(FAILMASK_VGATERMINAL | 0x0200 | EBADF);
	}
	return mainTerm.read(kfd, data, siz);
}
off_t VGATerminal_lseek(int kfd, off_t off, int how) {
	if (kfd & (~1)) {
		bugCheckNum(FAILMASK_VGATERMINAL | 0x0400 | EBADF);
	}
	errno = ESPIPE;
	return (-1);
}
int VGATerminal__llseek(int kfd, off_t hi32, off_t lo32, loff_t* res, int how) {
	if (kfd & (~1)) {
		bugCheckNum(FAILMASK_VGATERMINAL | 0x0400 | EBADF);
	}
	errno = ESPIPE;
	return (-1);
}
#include "FileDriver.h"
struct FileDriver FileDriver_VGATerminal = (struct FileDriver){VGATerminal_write, VGATerminal_read, VGATerminal_lseek, VGATerminal__llseek};
#endif
