#ifndef __MEMFORK_H__
#define __MEMFORK_H__ 1
#include "types.h"
#include "kmemman.h"
#include "paging.h"
#include "util.h"
void* fork_user_page(void* page) {// The `struct MemSpace' corresponding to the memory space of the thread having its pages forked must be locked so as to prevent pages from being removed from the memory space (and thus possibly killed) during the page forking; it is expected that the page pointed to by `page' is not killed if that page has a valid associated page description
	{
		uintptr adui = (uintptr) page;
		if ((adui < (KMEM_LB_DATSTART - RELOC)) || (adui >= (KMEM_LB_DATSTART + KMEM_LB_AMNT - RELOC))) {
			return page;
		}
	}
	struct PageDescData pdd;
	fetchPageData(page, &pdd);
	if (pdd.refs) {
		void* npg;
		switch (pdd.type) {
			case (1):
				npg = alloc_lb();
				moveExv(npg, page, PAGE_SIZE);
				initPageRefBased(1, npg, page);
				return npg;
			case (2):
				addPageRef(page);
				return page;
			default:
				bugCheckNum(0x0103 | FAILMASK_PAGING);
		}
	}
	return page;
}
#endif
