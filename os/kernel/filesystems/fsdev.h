#ifndef __FILESYSTEMS_FSDEV_H__
#define __FILESYSTEMS_FSDEV_H__ 1
/*
 *
 * Filesystem driver for the '/dev' entries
 *
 */
#define FAILMASK_FSDEV 0x000f0000
struct Dev {
	long type;// 1: `Block'
	void* ref;
};
struct DevDescptn {
	struct Dev* dev;
	mode_t mode;
	int flags;

};
struct FSDev {
	Map* devs;// Map, char* -> struct Dev*
	Map* fds;// Map, int "kfd" -> struct DevDscptn*
};
int FSDev_addDev(const char* name, struct Dev* dev, struct FSDev* inst) {// No references change ownership
	if (Map_findByCompare(name, Map_cmpStr, inst->devs) != (uintptr) (-1)) {
		return (-1);// Device name is already in use
	}
	struct Dev* nd = alloc(sizeof(struct Dev));
	cpy(nd, dev, sizeof(struct Dev));
	size_t sl = strlen(name) + 1;
	char* nn = alloc(sl);
	cpy(nn, name, sl);
	if (Map_add((uintptr) nn, (uintptr) nd, inst->devs)) {
		bugCheckNum(0x0001 | FAILMASK_FSDEV);
	}
}


#include "../fsiface.h"
struct FSReturn FSDev_open(const char* filename, int flags, mode_t 




#endif
