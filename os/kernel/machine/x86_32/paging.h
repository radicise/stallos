#ifndef __MACHINE_X86_32_PAGING_H__
#define __MACHINE_X86_32_PAGING_H__ 1
#define PAGE_SIZE 4096
#define PAGE_LOMASK ((uintptr) 0x00000fff)
#define PAGEOF(x) ((uintptr) ((((uintptr) (x)) / PAGE_SIZE) * PAGE_SIZE))
#define MACHINE_SUPPORT_WRONLYMEM 0
const uintptr amntMem = 64 * 1024 * 1024;// The value of `amntMem' must be an integer multiple of the value of `PAGE_SIZE'
#define USERMEMHIGH (((uintptr) 0) - ((uintptr) 1))
// TODO URGENT Ensure that `uintptr' wrap-around works as expected
#include "../../kmemman.h"
#if KMEM_LB_BS != PAGE_SIZE
#error "Incompatible large-block memory allocation block size"
#endif
#include "../../types.h"
extern void CR3Load(unsigned long);
extern void WPPGSetup(void);
extern void TLBReload(void);
typedef u32 PGDEnt;
#define PG_P ((PGDEnt) (u32) 0x00000001)
#define PG_RW ((PGDEnt) (u32) 0x00000002)
#define PG_US ((PGDEnt) (u32) 0x00000004)
#define PG_A ((PGDEnt) (u32) 0x00000020)
#define PG_D ((PGDEnt) (u32) 0x00000040)
#define PG_PINNED ((PGDEnt) (u32) 0x00000e00)
void PGDPinAdj(volatile PGDEnt* ent, int dec) {
	u32 val = AtomicULong_get((AtomicULong*) ent);
	unsigned int pins = (val >> 9) & 0x00000007;// TODO Support more than 7 concurrent page pins
	if (!dec) {
		pins++;
		pins &= 0x00000007;
	}
	if (!pins) {
		if (dec) {
			bugCheckNum(0x0007 | FAILMASK_PAGING);
		}
		bugCheckNum(0x0009 | FAILMASK_PAGING);
	}
	if (dec) {
		pins--;
	}
	val &= (u32) 0xffff1ff;
	val |= ((u32) pins) << 9;
	AtomicULong_set((AtomicULong*) ent, val);
	return;
}
int PGDTest(volatile PGDEnt* valptr, PGDEnt cond) {
	return (((PGDEnt) AtomicULong_get((AtomicULong*) valptr)) & cond) ? 1 : 0;
}
void PGDSet(volatile PGDEnt* valptr, PGDEnt cond) {
	PGDEnt p = (PGDEnt) AtomicULong_get((AtomicULong*) valptr);
	p |= cond;
	AtomicULong_set((AtomicULong*) valptr, (unsigned long) p);
	return;
}
void PGDUnset(volatile PGDEnt* valptr, PGDEnt cond) {
	PGDEnt p = (PGDEnt) AtomicULong_get((AtomicULong*) valptr);
	p &= (~cond);
	AtomicULong_set((AtomicULong*) valptr, (unsigned long) p);
	return;
}
unsigned long PGDReadPins(volatile PGDEnt* valptr) {
	PGDEnt p = (PGDEnt) AtomicULong_get((AtomicULong*) valptr);
	return (((u32) p) >> 9) & ((u32) 0x00000007);
}
void PGDWritePins(volatile PGDEnt* valptr, unsigned long datt) {
	if (((u32) datt) & ~((u32) 0x00000007)) {
		bugCheckNum(0x0001 | FAILMASK_PAGING);
	}
	PGDEnt p = (PGDEnt) AtomicULong_get((AtomicULong*) valptr);
	p = (PGDEnt) ((((u32) p) & ((u32) 0xffffff1ff)) ^ (((u32) datt) << 9));
	AtomicULong_set((AtomicULong*) valptr, (unsigned long) p);
	return;
}
void PGDSetAddr(volatile PGDEnt* valptr, volatile void* ptr) {
	u32 b = (u32) (((uintptr) ptr) + ((uintptr) RELOC));
	if (b & 0x00000fff) {
		bugCheckNum(0x0002 | FAILMASK_PAGING);
	}
	PGDEnt p = (PGDEnt) AtomicULong_get((AtomicULong*) valptr);
	p = (PGDEnt) ((((u32) p) & ((u32) 0x00000fff)) ^ b);
	AtomicULong_set((AtomicULong*) valptr, (unsigned long) p);
	return;
}
void* PGDGetAddr(volatile PGDEnt* valptr) {
	PGDEnt p = (PGDEnt) AtomicULong_get((AtomicULong*) valptr);
	return (void*) (((uintptr) (((u32) p) & ((u32) 0xfffff000))) - ((uintptr) RELOC));
}
void nullifyEntry(volatile PGDEnt* entry) {
	(*((u32*) entry)) = 0x00000000;
}
void initPageTable(volatile PGDEnt* table) {
	for (int i = 0; i < 1024; i++) {
		nullifyEntry(table + i);
	}
	return;
}
void initEntry(volatile PGDEnt* ent, int userWritable, int privileged, volatile void* addr) {
	nullifyEntry(ent);
	if (!privileged) {
		PGDSet(ent, PG_US);
	}
	if (userWritable) {
		PGDSet(ent, PG_RW);
	}
	PGDSetAddr(ent, addr);
	PGDSet(ent, PG_P);
	return;
}
struct MemSpace {
	PGDEnt* dir;
	Mutex lock;
	uintptr offset;
};
struct MemSpace* MemSpace_kernel;
void TLBFlush(struct MemSpace* ms) {
	if (ms == MemSpace_kernel) {
		TLBReload();
	}
	return;
}
volatile PGDEnt* findPage(uintptr vAddr, struct MemSpace* ms) {
	u32 p = vAddr;
	if (p & ((u32) 0x00000fff)) {
		bugCheckNum(0x0006 | FAILMASK_PAGING);
	}
	int i = (p >> 22);
	int j = ((p >> 12) & ((u32) 0x000003ff));
	if (!(PGDTest((ms->dir) + i, PG_P))) {
		return NULL;
	}
	volatile PGDEnt* tbl = PGDGetAddr((ms->dir) + i);
	if (!(PGDTest(tbl + j, PG_P))) {
		return NULL;
	}
	return tbl + j;
}
struct MemSpace* MemSpace_createOffset(uintptr off) {// The value of `off' MUST be an integer multiple of the value of `PAGE_SIZE'
	struct MemSpace* ms = alloc(sizeof(struct MemSpace));
	Mutex_initUnlocked(&(ms->lock));
	ms->offset = off;
	(ms->dir) = alloc_lb();
	initPageTable(ms->dir);
	return ms;
}
struct MemSpace* MemSpace_create(void) {
	return MemSpace_createOffset((uintptr) 0);
}
void MemSpace_destroy(struct MemSpace* ms) {
	Mutex_acquire(&(ms->lock));
	volatile PGDEnt* dir = (ms->dir);
	for (int i = 0; i < 1024; i++) {
		if (PGDTest(dir + i, PG_P)) {
			volatile PGDEnt* tbl = PGDGetAddr(dir + i);
			dealloc_lb(tbl);
		}
	}
	dealloc_lb(dir);
	dealloc(ms, sizeof(struct MemSpace));
	return;
}
void MemSpace_forEach(void (*operation)(uintptr, volatile void*, void*), void* obj, struct MemSpace* mem) {
	Mutex_acquire(&(mem->lock));
	volatile PGDEnt* dir = (mem->dir);
	for (int i = 0; i < 1024; i++) {
		if (PGDTest(dir + i, PG_P)) {
			volatile PGDEnt* tbl = PGDGetAddr(dir + i);
			for (int j = 0; j < 1024; j++) {
				if (PGDTest(tbl + j, PG_P)) {
					volatile void* addr = PGDGetAddr(tbl + j);
					operation(((((uintptr) i) << 22) | (((uintptr) j) << 12)) - ((uintptr) (mem->offset)), addr, obj);
				}
			}
		}
	}
	Mutex_release(&(mem->lock));
	return;
}
int mapPageSpecificPrivilege(uintptr vAddr, void* ptr, int userWritable, int privileged, struct MemSpace* ms) {
	Mutex_acquire(&(ms->lock));
	vAddr += ms->offset;
	u32 p = vAddr;
	if (p & ((u32) 0x00000fff)) {
		bugCheckNum(0x0004 | FAILMASK_PAGING);
	}
	int i = (p >> 22);
	int j = ((p >> 12) & ((u32) 0x000003ff));
	if (!(PGDTest((ms->dir) + i, PG_P))) {
		
		//kernelMsgULong((unsigned long) i);
		//kernelMsg("\n");
		//kernelMsgULong((unsigned long) j);
		//kernelMsg("\n");
		//kernelMsg("\n");

		volatile PGDEnt* tbl = alloc_lb();
		initPageTable(tbl);
		initEntry((ms->dir) + i, 1, 0, tbl);
		initEntry(tbl + j, userWritable, privileged, ptr);
		TLBFlush(ms);
		Mutex_release(&(ms->lock));
		return 0;
	}
	volatile PGDEnt* tbl = PGDGetAddr((ms->dir) + i);
	if (PGDTest(tbl + j, PG_P)) {
		Mutex_release(&(ms->lock));
		return (-1);
	}
	initEntry(tbl + j, userWritable, privileged, ptr);
	TLBFlush(ms);
	Mutex_release(&(ms->lock));
	return 0;
}
int mapPage(uintptr vAddr, void* ptr, int userReadable, int userWritable, struct MemSpace* ms) {// TODO URGENT Distinguish between userspace-held pages with no permissions and kernel-held pages
	//kernelWarnMsgCode("", (unsigned long) ptr);
	//kernelWarnMsgCode("", (unsigned long) vAddr);
	//kernelMsg("\n");
	if (userReadable) {
		return mapPageSpecificPrivilege(vAddr, ptr, userWritable, 0, ms);
	}
	if (userWritable) {
		bugCheckNum(0xbadacce5);
	}
	return mapPageSpecificPrivilege(vAddr, ptr, 0, 1, ms);
}
int pinPage(uintptr vAddr, struct MemSpace* ms, int implyReadable, int implyWritable) {
	Mutex_acquire(&(ms->lock));

	// kernelMsgCode("Pin: ", vAddr);

	vAddr += ms->offset;
	volatile PGDEnt* ent = findPage(vAddr, ms);
	if (ent == NULL) {
		Mutex_release(&(ms->lock));
		return (-1);
	}
	if (implyReadable | implyWritable) {
		if (!(PGDTest(ent, PG_US))) {
			Mutex_release(&(ms->lock));
			return (-1);
		}
		if (implyWritable) {
			if (!(PGDTest(ent, PG_RW))) {
				Mutex_release(&(ms->lock));
				return (-1);
			}
		}
	}
	PGDPinAdj(ent, 0);
	Mutex_release(&(ms->lock));
	return 0;
}
void unpinPage(uintptr vAddr, struct MemSpace* ms) {
	Mutex_acquire(&(ms->lock));

	// kernelMsgCode("Unpin: ", vAddr);

	vAddr += ms->offset;
	volatile PGDEnt* ent = findPage(vAddr, ms);
	if (ent == NULL) {
		bugCheckNum(0x0008 | FAILMASK_PAGING);
	}
	PGDPinAdj(ent, 1);
	Mutex_release(&(ms->lock));
	return;
}
int unmapPage(uintptr vA, struct MemSpace* ms) {
	if (((u32) vA) & ((u32) 0x00000fff)) {
		bugCheckNum(0x0005 | FAILMASK_PAGING);
	}
	int i;
	int j;
	volatile PGDEnt* tbl = NULL;
	while (1) {
		Mutex_acquire(&(ms->lock));
		u32 p = (u32) (vA + (ms->offset));
		i = (p >> 22);
		j = ((p >> 12) & ((u32) 0x000003ff));
		if (!(PGDTest((ms->dir) + i, PG_P))) {
			Mutex_release(&(ms->lock));
			return (-1);
		}
		tbl = PGDGetAddr((ms->dir) + i);
		if (!(PGDTest(tbl + j, PG_P))) {
			Mutex_release(&(ms->lock));
			return (-1);
		}
		if (!(PGDTest(tbl + j, PG_PINNED))) {
			break;
		}
		Mutex_release(&(ms->lock));
		Mutex_wait();
	}
	nullifyEntry(tbl + j);
	for (int k = 0; k < 1024; k++) {
		if (k == j) {
			continue;
		}
		if (PGDTest(tbl + k, PG_P)) {
			TLBFlush(ms);
			Mutex_release(&(ms->lock));
			return 0;
		}
	}
	nullifyEntry((ms->dir) + i);
	dealloc_lb(tbl);
	TLBFlush(ms);
	Mutex_release(&(ms->lock));
	return 0;
}
int pageExists(uintptr vAddr, struct MemSpace* ms) {
	Mutex_acquire(&(ms->lock));
	vAddr += ms->offset;
	u32 p = vAddr;
	int i = (p >> 22);
	int j = ((p >> 12) & ((u32) 0x000003ff));
	if (!(PGDTest((ms->dir) + i, PG_P))) {
		Mutex_release(&(ms->lock));
		return 0;
	}
	volatile PGDEnt* tbl = PGDGetAddr((ms->dir) + i);
	if (!(PGDTest(tbl + j, PG_P))) {
		Mutex_release(&(ms->lock));
		return 0;
	}
	Mutex_release(&(ms->lock));
	return 1;
}
void* pageMapping(uintptr vAddr, struct MemSpace* ms) {
	Mutex_acquire(&(ms->lock));
	vAddr += ms->offset;
	u32 p = vAddr;
	int i = (p >> 22);
	int j = ((p >> 12) & ((u32) 0x000003ff));
	if (!(PGDTest((ms->dir) + i, PG_P))) {
		Mutex_release(&(ms->lock));
		return NULL;
	}
	volatile PGDEnt* tbl = PGDGetAddr((ms->dir) + i);
	if (!(PGDTest(tbl + j, PG_P))) {
		Mutex_release(&(ms->lock));
		return NULL;
	}
	void* b = PGDGetAddr(tbl + j);
	Mutex_release(&(ms->lock));
	return b;
}
// TODO URGENT Make TLB flushing work on multi-processor systems
// TODO Use `invlpg' appropriately
void initPaging(void) {
	MemSpace_kernel = MemSpace_createOffset((uintptr) RELOC);
	uintptr m = ((uintptr) 0) - ((uintptr) RELOC);
	while (m != (amntMem - ((uintptr) RELOC))) {
		if (mapPage(m, (void*) m, 0, 0, MemSpace_kernel)) {// TODO URGENT Ensure that casting an underflowed `uintptr' to a `void*' works as it was taken to work
			bugCheckNum(0x0003 | FAILMASK_PAGING);
		}
		m += PAGE_SIZE;
	}
	CR3Load(((u32) (MemSpace_kernel->dir)) + ((u32) RELOC));
	WPPGSetup();
	return;
}
int chprot(uintptr vPage, int read, int write, int execute, struct MemSpace* ms) {
	bugCheckNum(0x000a | FAILMASK_PAGING);// TODO URGENT Implement
}
#include "../../memfork.h"
struct MemSpace* MemSpace_based(struct MemSpace* mem, int memfork) {
	Mutex_acquire(&(mem->lock));
	const PGDEnt* dir = (mem->dir);
	PGDEnt* ndir = alloc_lb();
	memcpy(ndir, dir, 4096);
	for (int i = 0; i < 1024; i++) {
		if (PGDTest(ndir + i, PG_P)) {
			const PGDEnt* tbl = PGDGetAddr(ndir + i);
			PGDEnt* ntbl = alloc_lb();
			memcpy(ntbl, tbl, 4096);
			if (memfork) {
				for (int j = 0; j < 1024; j++) {
					if (PGDTest(ntbl + j, PG_P)) {
						PGDSetAddr(ntbl + j, fork_user_page(PGDGetAddr(ntbl + j)));
						PGDWritePins(ntbl + j, 0);
					}
				}
			}
			PGDSetAddr(ndir + i, ntbl);
		}
	}
	struct MemSpace* nms = alloc(sizeof(struct MemSpace));
	memcpy(nms, mem, sizeof(struct MemSpace));
	nms->dir = ndir;
	Mutex_release(&(mem->lock));
	Mutex_release(&(nms->lock));
	return nms;
}
struct MemSpace* MemSpace_clone(struct MemSpace* mem) {
	return MemSpace_based(mem, 0);
}
struct MemSpace* MemSpace_fork(struct MemSpace* mem) {
	return MemSpace_based(mem, 1);
}
// TODO Remove volatility where not needed
#endif
