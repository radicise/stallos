#ifndef __PAGING_H__
#define __PAGING_H__ 1
/*
 * Defines:
 * PAGE_SIZE
 * struct MemSpace
 * struct MemSpace* MemSpace_create(void);
 * void MemSpace_destroy(struct MemSpace*);
 * int mapPage(uintptr, void*, int, struct MemSpace*);// The argument of type `uintptr' represents a page-aligned address, the argument of type `void*' is a page-aligned address, the return value indicates the status: 0 represents success and (-1) represents failure due to the virtual address already having been mapped
 * void unmapPage(uintptr, struct MemSpace*);// Arguments must match with those of the corresponding `mapPage' call, the return value indicates the status: 0 represents success and (-1) represents failure due to the virtual address not already being mapped
 * int pageExists(uintptr, struct MemSpace*);
 * void* pageMapping(uintptr, struct MemSpace*);
 * void initPaging(void);
 * MemSpace* MemSpace_kernel;// This is initialised within initPaging
 *
 */
#ifndef TARGETNUM
#error "`TARGETNUM' is not set"
#endif
#if TARGETNUM == 1
#include "machine/x86_32/paging.h"
#else
#error "Target is not supported"
#endif
#endif
