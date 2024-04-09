#ifndef __DRIVER_MEMDISK_H__
#define __DRIVER_MEMDISK_H__ 1
#include "../paging.h"
#define MEMDISK_BS PAGE_SIZE
struct Memdisk {
	void* mem;
	uintptr len;// Length, in blocks
};
int Memdisk_readBlock(unsigned long long block, unsigned long long amnt, void* dst, void* obj) {
	if ((block + amnt) > (unsigned long long) (((struct Memdisk*) obj)->len)) {
		return (-1);
	}
	void* area = ((char*) mem) + (block * (unsigned long long) MEMDISK_BS);
	cpy(dst, area, amnt * (unsigned long long) MEMDISK_BS);
	return 0;
}
int Memdisk_writeBlock(unsigned long long block, unsigned long long amnt, const void* src, void* obj) {
	if ((block + amnt) > (unsigned long long) (((struct Memdisk*) obj)->len)) {
		return (-1);
	}
	void* area = ((char*) mem) + (block * (unsigned long long) MEMDISK_BS);
	cpy(area, src, amnt * (unsigned long long) MEMDISK_BS);
	return 0;
}
#endif
