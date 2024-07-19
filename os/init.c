#include "kernel/types.h"
#if __TESTING__ == 1
extern unsigned long testcall(unsigned long);
#endif
extern void _exit(int);
extern ssize_t write(int, const void*, size_t);
extern ssize_t read(int, void*, size_t);
extern time_t time(time_t*);
extern int stime(const time_t*);
extern void* brk(void*);
size_t strlen(const char* s) {
	size_t l = 0;
	while (s[l++]) {
	}
	return l - 1;
}
extern unsigned long TNN(void);
unsigned char hex[] = {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66};
void printULong_hex(unsigned long n) {
	int i;
	char buf[i = sizeof(unsigned long) * 2];
	while (1) {
		i--;
		int c = n & 0xf;
		if (c > 9) {
			c += 0x57;
		}
		else {
			c += 0x30;
		}
		buf[i] = c;
		if (!i) {
			break;
		}
		n >>= 4;
	}
	write(1, buf, sizeof(unsigned long) * 2);
	return;
}
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
void print(const char* d) {
	write(1, d, strlen(d));
	return;
}
void _start() {
	/*
	_exit(0xdeadc0de);
	int i = 0;
	while (i < 200) {
		char c[1];
		read(3, c, 1);
		write(1, c, 1);
		i += 1;
	}
	while (1) {}
	void* bm = brk(NULL);
	printULong_hex((unsigned long)bm);
	print("\n");
	// while (1) {}
	while (1) {
		print("0x");
		printULong_hex((unsigned long)(bm = brk((void*) (((char*) bm) + 4096 * 4))));
		print("\n");
		printLong(*(((char*) bm) - 1));
		print("\n");
		print("\n");
	}
	// TNN();
	*/
	/*
	while (1) {
		time(NULL);
	}
	*/
	/*
	char jee;
	while (1) {
		read(0, &jee, 1);
		write(1, &jee, 1);
	}
	*/
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
		//testcall(0);
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
