#include "kernel/types.h"
#define __TESTING__ 1
#if __TESTING__ == 1
extern unsigned long testcall(unsigned long);
#endif
extern ssize_t write(int, const void*, size_t);
extern ssize_t read(int, void*, size_t);
extern time_t time(time_t*);
extern int stime(const time_t*);
unsigned char hex[] = {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66};
void printLong(long n) {
	if (n == 0) {
		write(1, "0", 1);
		return;
	}
	int neg = 0;
	if (n < 0) {
		neg = 1;
		n = (-n);
	}
	int count = 0;
	long m = n;
	while (m) {
		m /= 10;
		count++;
	}
	char buf[count + neg];
	if (neg) {
		buf[0] = 0x2d;
	}
	char* bb = buf + neg;
	int i = count - 1;
	while (1) {
		bb[i] = 0x30 + (n % 10);
		n /= 10;
		if (i == 0) {
			break;
		}
		i--;
	}
	write(1, buf, count + neg);
	return;
}
void printLine(void) {
	write(1, "\n", 1);
	return;
}
void _start() {
	/*
	unsigned long i = 0;
	while (1) {
		printLong(i);
		printLine();
		i++;
	}
	*/
	time_t timeOld = time(NULL);
	printLong(timeOld);
	printLine();
	time_t timeNow;
	while (1) {
#if __TESTING__ == 1
		testcall(0);
#endif
		while ((timeNow = time(NULL)) == timeOld) {
		}
		printLong(timeNow);
		printLine();
		timeOld = timeNow;
	}
	unsigned char c;
	ssize_t n;
	time_t desTime = 0;
	unsigned char dat[512];
	//n = read(3, dat, 512);
	while (1) {
		write(1, "the thing is that ", 18);
	}
	if (n != 512) {
		write(1, "eeee", 4);
		while (1) {
		}
	}
	n = write(1, dat, 512);
	if (n != 512) {
		write(1, "eeee", 4);
		while (1) {
		}
	}
	while (1) {
	}
}
