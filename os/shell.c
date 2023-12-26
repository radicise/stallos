#include "kernel/types.h"
extern ssize_t write(int, const void*, size_t);
extern ssize_t read(int, void*, size_t);
extern time_t time(time_t*);
extern int stime(const time_t*);
unsigned char hex[] = {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66};
void _start() {
	write(1, "Stall v0.0.2.0\n", 15);
	unsigned char c;
	ssize_t n;
	time_t desTime = 0;
	unsigned char dat[512];
	c = read(3, dat, 512);
	write(1, &(hex[c >> 4]), 1);
	write(1, &(hex[c & 0x0f]), 1);
	write(1, "\n", 1);
	for (int i = 0; i < 512; i++) {
		c = dat[i];
		write(1, &(hex[c >> 4]), 1);
		write(1, &(hex[c & 0x0f]), 1);
		write(1, "\n", 1);
	}
	while (1) {
		n = read(0, &c, (size_t) 1);
		if (n == (-1)) {
			break;
		}
		if (n == 0) {
			continue;
		}
		while (1) {
			n = write(1, &c, (size_t) 1);
			if (n == 0) {
				continue;
			}
			if (n == (-1)) {
				while (1) {
				}
			}
			break;
		}
	}
	while (1) {
	}
}
