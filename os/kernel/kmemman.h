#ifndef __KMEMMAN_H__
#define __KMEMMAN_H__ 1
#include "types.h"
#define KMEM_ADDR 0x1800000
#define KMEM_AMNT 0x1600000
#define KMEM_BS 16
/* `KMEM_AMNT' MUST be an integer multiple of `KMEM_BS' */
/* `KEMM_ADDR' MUST be an integer multiple of `KMEM_BS' */
#define KMEM_AMNTMEMBITMAPBYTES (KMEM_AMNT / (KMEM_BS * CHAR_BIT))
#define KMEM_AMNTMEMSPACES (KMEM_AMNT / KMEM_BS)
#define KMEM_DATSTART (KMEM_ADDR + KMEM_AMNTMEMBITMAPBYTES + KMEM_BS - ((KMEM_ADDR + KMEM_AMNTMEMBITMAPBYTES - 1) % KMEM_BS) - 1)
#define KMEM_LB_ADDR 0x3000000
#define KMEM_LB_AMNT 0x0e00000
#define KMEM_LB_BS 4096
/* `KMEM_LB_AMNT' MUST be an integer multiple of `KMEM_LB_BS' */
/* `KEMM_LB_ADDR' MUST be an integer multiple of `KMEM_LB_BS' */
#define KMEM_LB_AMNTMEMBITMAPBYTES (KMEM_LB_AMNT / (KMEM_LB_BS * CHAR_BIT))
#define KMEM_LB_AMNTMEMSPACES (KMEM_LB_AMNT / KMEM_LB_BS)
#define KMEM_LB_DATSTART (KMEM_LB_ADDR + KMEM_LB_AMNTMEMBITMAPBYTES + KMEM_LB_BS - ((KMEM_LB_ADDR + KMEM_LB_AMNTMEMBITMAPBYTES - 1) % KMEM_LB_BS) - 1)
Mutex kmem_access;
Mutex kmem_lb_access;
volatile unsigned long long memAllocated = 0;
volatile unsigned long long memAllocated_lb = 0;
void kmem_init(void) {
	set((void*) (KMEM_ADDR - RELOC), 0x00, KMEM_DATSTART - KMEM_ADDR);
	set((void*) (KMEM_LB_ADDR - RELOC), 0x00, KMEM_LB_DATSTART - KMEM_LB_ADDR);
	Mutex_initUnlocked(&kmem_access);
	Mutex_initUnlocked(&kmem_lb_access);
	return;
}
unsigned long long getMemUsage(void) {
	unsigned long long n = KMEM_LB_DATSTART - KMEM_AMNT;
	Mutex_acquire(&kmem_access);
	n += memAllocated;
	Mutex_release(&kmem_access);
	Mutex_acquire(&kmem_lb_access);
	n += memAllocated_lb;
	Mutex_release(&kmem_lb_access);
	return n;
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
		if (k == KMEM_AMNTMEMSPACES) {
			Mutex_release(&kmem_access);
			bugCheckNum(0xd00d25ad);// Out of small-block heap memory
		}
		if ((((volatile char*) (KMEM_ADDR - RELOC))[k / CHAR_BIT]) & (0x01 << (k % CHAR_BIT))) {
			k++;
			j = k;
			a = siz;
			continue;
		}
		if (a <= KMEM_BS) {
			while (1) {
				((volatile char*) (volatile void*) (KMEM_ADDR - RELOC))[k / CHAR_BIT] ^= (0x01 << (k % CHAR_BIT));
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
		((volatile char*) (volatile void*) (KMEM_ADDR - RELOC))[k / CHAR_BIT] &= (~(0x01 << (k % CHAR_BIT)));
		if (siz <= KMEM_BS) {
			memAllocated -= ((sz + KMEM_BS - 1) / KMEM_BS) * KMEM_BS;
			Mutex_release(&kmem_access);
			return;
		}
		siz -= KMEM_BS;
		k++;
	}
}
void* alloc_lb(void) {// Allocated memory is guaranteed to not be at NULL and to not be at (void*) (-1) and to not be at (uintptr*) (-1); avoidance of deadlock must be sure
	uintptr k = 0;
	Mutex_acquire(&kmem_lb_access);
	while (1) {
		if ((((volatile char*) (KMEM_LB_ADDR - RELOC))[k / CHAR_BIT]) & (0x01 << (k % CHAR_BIT))) {
			k++;
			if (k == KMEM_LB_AMNTMEMSPACES) {
				Mutex_release(&kmem_lb_access);
				bugCheckNum(0x25add00d);// Out of large-block heap memory
			}
			continue;
		}
		(((volatile char*) (KMEM_LB_ADDR - RELOC))[k / CHAR_BIT]) ^= (0x01 << (k % CHAR_BIT));
		memAllocated_lb += KMEM_LB_BS;
		Mutex_release(&kmem_lb_access);
		return (void*) ((KMEM_LB_DATSTART - RELOC) + (k * KMEM_LB_BS));
	}
}
void dealloc_lb(void* obj) {
	if (obj == NULL) {
		return;
	}
	uintptr k = (((uintptr) obj) - KMEM_LB_DATSTART + RELOC) / KMEM_LB_BS;
	Mutex_acquire(&kmem_lb_access);
	((volatile char*) (volatile void*) (KMEM_LB_ADDR - RELOC))[k / CHAR_BIT] &= (~(0x01 << (k % CHAR_BIT)));
	memAllocated_lb -= KMEM_LB_BS;
	Mutex_release(&kmem_lb_access);
}
#endif
