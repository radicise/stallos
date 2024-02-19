#ifndef __KMEMMAN_H__
#define __KMEMMAN_H__ 1
#include "types.h"
#define KMEM_ADDR 0x1800000
#define KMEM_AMNT 0x1800000
#define KMEM_BS 16
#define KMEM_DATSTART (KMEM_ADDR + (KMEM_AMNT / (KMEM_BS * CHAR_BIT)))
Mutex kmem_access;
unsigned long long memAllocated = 0;
void kmem_init(void) {
	set((void*) (KMEM_ADDR - RELOC), 0x00, KMEM_DATSTART - KMEM_ADDR);
	Mutex_initUnlocked(&kmem_access);
	return;
}
void* alloc(size_t siz) {// Allocated memory is guaranteed to not be at NULL and to not be at (void*) (-1) and to not be at (uintptr*) (-1); avoidance of deadlock must be sure
	if (siz == 0) {
		return NULL;
	}
	uintptr j = 0;
	uintptr k = 0;
	size_t a = siz;
	Mutex_acquire(&kmem_access);
	while (1) {
		if (k == KMEM_AMNT) {
			Mutex_release(&kmem_access);
			bugCheckNum(0xd00d25ad);// Out of heap memory
		}
		if ((((char*) (KMEM_ADDR - RELOC))[k / CHAR_BIT]) & (0x01 << (k % CHAR_BIT))) {
			k++;
			j = k;
			a = siz;
			continue;
		}
		if (a <= KMEM_BS) {
			while (1) {
				((char*) (void*) (KMEM_ADDR - RELOC))[k / CHAR_BIT] |= (0x01 << (k % CHAR_BIT));
				if ((k--) == j) {
					break;
				}
			}
			memAllocated += ((siz + KMEM_BS - 1) / KMEM_BS) * KMEM_BS;
			Mutex_release(&kmem_access);
			return (void*) ((KMEM_DATSTART - RELOC) + (j * KMEM_BS));
		}
		a -= KMEM_BS;
		k++;
	}
}
void dealloc(void* obj, size_t siz) {
	if (obj == NULL) {
		return;
	}
	size_t sz = siz;
	uintptr k = (((uintptr) obj) - KMEM_DATSTART + RELOC) / KMEM_BS;
	Mutex_acquire(&kmem_access);
	while (1) {
		((char*) (void*) (KMEM_ADDR - RELOC))[k / CHAR_BIT] &= (~(0x01 << (k % CHAR_BIT)));
		if (siz <= KMEM_BS) {
			memAllocated -= ((sz + KMEM_BS - 1) / KMEM_BS) * KMEM_BS;
			Mutex_release(&kmem_access);
			return;
		}
		siz -= KMEM_BS;
		k++;
	}
}
#endif
