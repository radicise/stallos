#ifndef __PAGING_H__
#define __PAGING_H__ 1
#include "kmemman.h"
/*
 *
 * Defines:
 *
 * PAGE_SIZE
 * // The value of `PAGE_SIZE' must be a power of two and at least one
 *
 * PAGE_LOMASK
 * // The type of the value of `PAGE_LOMASK' is `uintptr'
 *
 * PAGE_BITS
 * // The value of `PAGE_SIZE' is equal to the value of `(((uintptr) 0x1) << PAGE_BITS)'
 *
 * PAGEOF(x)
 * // The type of the value of `PAGEOF(x)' is `uintptr'
 *
 * MACHINE_SUPPORT_WRONLYMEM
 * // 1 if the machine supports pages that are user-writable but not user-readable, 0 otherwise
 *
 * struct MemSpace
 * // struct MemSpace has an operations lock `lock' of type `Mutex' that is utilised appropriately
 *
 * struct MemSpace* MemSpace_create(void);
 *
 * void MemSpace_destroy(struct MemSpace*);// Does not attempt to use `dealloc' to free the memory backing the memory mappings
 *
 * int mapPage(uintptr vPage, void* addr, int userReadable, int userWritable, struct MemSpace* mem);// The argument of type `uintptr' represents a page-aligned address, the argument of type `void*' is a page-aligned address, the return value indicates the status: 0 represents success and (-1) represents failure due to the virtual address already having been mapped
 *
 * void unmapPage(uintptr vPage, struct MemSpace* mem);// Arguments must match with those of the corresponding `mapPage' call, the return value indicates the status: 0 represents success and (-1) represents failure due to the virtual address not already being mapped
 *
 * int pageExists(uintptr, struct MemSpace*);
 *
 * void* pageMapping(uintptr, struct MemSpace*);// Returns `NULL' if the specified page is not mapped; a return value of `NULL' does not imply that the specified page is not mapped
 *
 * MemSpace* MemSpace_kernel;// This is initialised within `initPaging'
 *
 * void initPaging(void);
 *
 * void MemSpace_forEach(void (*)(uintptr, void*, void*), void*, struct MemSpace*);
 *
 * int pinPage(uintptr vPage, struct MemSpace* mem, int implyReadable, int implyWritable);// Increments the pinning level of a page; Returns 0 upon success; returns (-1) upon failing due to the page not being mapped or not having sufficient userspace privilege
 *
 * void unpinPage(uintptr, struct MemSpace*);// Decrements the pinning level of a page that has a nonzero pinning level
 *
 * int chprot(uintptr vPage, int read, int write, int execute, struct MemSpace* mem);// Applies the given protection to the page specified in `vPage'; `vPage' represents a page-aligned address; Returns 0 upon success; returns (-1) upon failure due to the page not being allocated to the user; returns (-2) upon failure due to the user not having sufficient privilege to perform the specified protection change
 *
 * struct MemSpace* MemSpace_clone(struct MemSpace*);
 *
 * struct MemSpace* MemSpace_fork(struct MemSpace*);
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
#include "memfork.h"
void freeUserPage(uintptr vAddr, volatile void* addr, void* arb) {
	uintptr adui = (uintptr) addr;
	if ((adui < (KMEM_LB_DATSTART - RELOC)) || (adui >= (KMEM_LB_DATSTART + KMEM_LB_AMNT - RELOC))) {
		return;
	}
	subPageRef(addr);
}
void userPageDealloc(volatile void* obj) {
	dealloc_lb(obj);
	return;
}
void MemSpace_mkUserData(const void* ptr, uintptr target, uintptr len, int newPagesUserReadable, int newPagesUserWritable, struct MemSpace* mem) {
	if (len == 0) {
		return;
	}
	Mutex_acquire(&(mem->lock));
	while (1) {
		uintptr page = PAGEOF(target);
		void* usrmem;
		if (pageExists(page, mem)) {
			usrmem = pageMapping(page, mem);
		}
		else {
			usrmem = alloc_lb_wiped();
			initPageRef(1, userPageDealloc, usrmem, 1);
			int r = mapPage(page, usrmem, newPagesUserReadable, newPagesUserWritable, mem);
			if (r) {
				bugCheckNum(0x0101 | FAILMASK_PAGING);
			}
		}
		uintptr avail = page + PAGE_SIZE - target;
		if (len <= avail) {
			memmove((void*) (((char*) usrmem) + (target - page)), ptr, len);// TODO Ensure that PAGE_SIZE is not greater than the maximum value of the type `size_t'
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
void MemSpace_mkUserFill(char val, uintptr target, uintptr len, int newPagesUserReadable, int newPagesUserWritable, struct MemSpace* mem) {
	if (len == 0) {
		return;
	}
	Mutex_acquire(&(mem->lock));
	while (1) {
		uintptr page = PAGEOF(target);
		void* usrmem;
		if (pageExists(page, mem)) {
			usrmem = pageMapping(page, mem);
		}
		else {
			usrmem = alloc_lb_wiped();
			initPageRef(1, userPageDealloc, usrmem, 1);
			int r = mapPage(page, usrmem, newPagesUserReadable, newPagesUserWritable, mem);
			if (r) {
				bugCheckNum(0x0102 | FAILMASK_PAGING);
			}
		}
		uintptr avail = page + PAGE_SIZE - target;
		if (len <= avail) {
			memset((void*) (((char*) usrmem) + (target - page)), val, len);// TODO Ensure that PAGE_SIZE is not greater than the maximum value of the type `size_t'
			Mutex_release(&(mem->lock));
			return;
		}
		memset((void*) (((char*) usrmem) + (target - page)), val, avail);// TODO Ensure that PAGE_SIZE is not greater than the maximum value of the type `size_t'
		target += avail;
		len -= avail;
	}
	return;
}
#endif
