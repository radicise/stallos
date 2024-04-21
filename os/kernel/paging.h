#ifndef __PAGING_H__
#define __PAGING_H__ 1
/*
 * Defines:
 * PAGE_SIZE
 * // `PAGE_SIZE' must be a power of two and at least one
 * PAGEOF(x)
 * struct MemSpace
 * // struct MemSpace has an operations lock `lock' of type `Mutex' that is utilised appropriately
 * struct MemSpace* MemSpace_create(void);
 * void MemSpace_destroy(struct MemSpace*);// Does not attempt to use `dealloc' to free the memory backing the memory mappings
 * int mapPage(uintptr, void*, int, struct MemSpace*);// The argument of type `uintptr' represents a page-aligned address, the argument of type `void*' is a page-aligned address, the return value indicates the status: 0 represents success and (-1) represents failure due to the virtual address already having been mapped
 * void unmapPage(uintptr, struct MemSpace*);// Arguments must match with those of the corresponding `mapPage' call, the return value indicates the status: 0 represents success and (-1) represents failure due to the virtual address not already being mapped
 * int pageExists(uintptr, struct MemSpace*);
 * void* pageMapping(uintptr, struct MemSpace*);// Returns `NULL' if the specified page is not mapped
 * MemSpace* MemSpace_kernel;// This is initialised within `initPaging'
 * void initPaging(void);
 *
 */
#define FAILMASK_PAGING 0x000d0000
#ifndef TARGETNUM
#error "`TARGETNUM' is not set"
#endif
#if TARGETNUM == 1
#include "machine/x86_32/paging.h"
#else
#error "Target is not supported"
#endif
void MemSpace_mkData(const void* ptr, uintptr target, uintptr len, int newPagesUserWritable, struct MemSpace* mem) {
	if (len == 0) {
		return;
	}
	Mutex_acquire(&(mem->lock));
	while (1) {
		uintptr page = PAGEOF(target);
		void* usrmem = pageMapping(page, mem);
		if (usrmem == NULL) {
			int r = mapPage(page, usrmem = (alloc_lb()), newPagesUserWritable, mem);
			if (r) {
				bugCheckNum(0x1001 | FAILMASK_PAGING);
			}
		}
		uintptr avail = page + PAGE_SIZE - target;
		if (len <= avail) {
			memmove((void*) (((char*) usrmem) + (target - page)), ptr, len);
			Mutex_release(&(mem->lock));
			return;
		}
		memmove((void*) (((char*) usrmem) + (target - page)), ptr, avail);// TODO URGENT Ensure that PAGE_SIZE is not greater than the maximum value of the type `size_t'
		target += avail;
		len -= avail;
		ptr = (const void*) (((const char*) ptr) + avail);
	}
	return;
}
#endif
