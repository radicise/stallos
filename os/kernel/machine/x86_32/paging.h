#ifndef __MACHINE_X86_32_PAGING_H__
#define __MACHINE_X86_32_PAGING_H__ 1
#define PAGE_SIZE 4096
uintptr amntMem = 72 * 1024 * 1024;// The value of `amntMem' must be an integer multiple of the value of `PAGE_SIZE'
#if KMEM_LB_BS != PAGE_SIZE
#error "Incompatible large-block memory allocation block size"
#endif
#include "../../types.h"
#include "../../kmemman.h"
#define FAILMASK_PAGING 0x000d0000
#ifdef NAMESPACE_PG
#error "ERROR: NAMESPACE COLLISION"
#endif
#define NAMESPACE_PG 1
extern void CR3Load(unsigned long);
extern void WPPGSetup(void);
typedef u32 PGDEnt;
#define PG_P ((PGDEnt) (u32) 0x00000001)
#define PG_RW ((PGDEnt) (u32) 0x00000002)
#define PG_US ((PGDEnt) (u32) 0x00000004)
#define PG_A ((PGDEnt) (u32) 0x00000020)
#define PG_D ((PGDEnt) (u32) 0x00000040)
int PGDTest(PGDEnt* valptr, PGDEnt cond) {
	return (((PGDEnt) AtomicULong_get((AtomicULong*) valptr)) & cond) ? 1 : 0;
}
void PGDSet(PGDEnt* valptr, PGDEnt cond) {
	PGDEnt p = (PGDEnt) AtomicULong_get((AtomicULong*) valptr);
	p |= cond;
	AtomicULong_set((AtomicULong*) valptr, (unsigned long) p);
	return;
}
void PGDUnset(PGDEnt* valptr, PGDEnt cond) {
	PGDEnt p = (PGDEnt) AtomicULong_get((AtomicULong*) valptr);
	p &= (~cond);
	AtomicULong_set((AtomicULong*) valptr, (unsigned long) p);
	return;
}
unsigned long PGDRead(PGDEnt* valptr) {
	PGDEnt p = (PGDEnt) AtomicULong_get((AtomicULong*) valptr);
	return (((u32) p) >> 9) & ((u32) 0x00000007);
}
void PGDWrite(PGDEnt* valptr, unsigned long datt) {
	if (((u32) datt) & ((u32) 0x00000007)) {
		bugCheckNum(0x0001 | FAILMASK_PAGING);
	}
	PGDEnt p = (PGDEnt) AtomicULong_get((AtomicULong*) valptr);
	p = (PGDEnt) ((((u32) p) & ((u32) 0xffffff1ff)) ^ (((u32) datt) << 9));
	AtomicULong_set((AtomicULong*) valptr, (unsigned long) p);
	return;
}
void PGDSetAddr(PGDEnt* valptr, void* ptr) {
	u32 b = (u32) (((uintptr) ptr) + ((uintptr) RELOC));
	if (b & 0x00000fff) {
		bugCheckNum(0x0002 | FAILMASK_PAGING);
	}
	PGDEnt p = (PGDEnt) AtomicULong_get((AtomicULong*) valptr);
	p = (PGDEnt) ((((u32) p) & ((u32) 0x00000fff)) ^ b);
	AtomicULong_set((AtomicULong*) valptr, (unsigned long) p);
	return;
}
void* PGDGetAddr(PGDEnt* valptr) {
	PGDEnt p = (PGDEnt) AtomicULong_get((AtomicULong*) valptr);
	return (void*) (((uintptr) (((u32) p) & ((u32) 0xfffff000))) - ((uintptr) RELOC));
}
void nullifyEntry(PGDEnt* entry) {
	set(entry, 0x00, 4);
}
void initPageTable(PGDEnt* table) {
	set(table, 0x00, 4096);
	return;
}
void initEntry(PGDEnt* ent, int userWritable, int privileged, void* addr) {
	set(ent, 0x00, 4);
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
	Mutex lock;
	PGDEnt* dir;
};
struct MemSpace* MemSpace_create(void) {
	struct MemSpace* ms = alloc(sizeof(struct MemSpace));
	Mutex_initUnlocked(&(ms->lock));
	(ms->dir) = alloc_lb();
	initPageTable(ms->dir);
	return ms;
}
void MemSpace_destroy(struct MemSpace* ms) {
	Mutex_acquire(&(ms->lock));
	PGDEnt* dir = (ms->dir);
	for (int i = 0; i < 1024; i++) {
		if (PGDTest(dir + i, PG_P)) {
			PGDEnt* tbl = PGDGetAddr(dir + i);
			dealloc_lb(tbl);
		}
	}
	dealloc_lb(dir);
	dealloc(ms, sizeof(struct MemSpace));
	return;
}
int mapPageSpecificPrivilege(uintptr vAddr, void* ptr, int userWritable, int privileged, struct MemSpace* ms) {
	Mutex_acquire(&(ms->lock));
	u32 p = vAddr;
	if (p & ((u32) 0x00000fff)) {
		bugCheckNum(0x0004 | FAILMASK_PAGING);
	}
	int i = (p >> 22);
	int j = ((p >> 12) & ((u32) 0x000003ff));
	if (!(PGDTest((ms->dir) + i, PG_P))) {
		PGDEnt* tbl = alloc_lb();
		initPageTable(tbl);
		initEntry((ms->dir) + i, 1, 0, tbl);
		initEntry(tbl + j, userWritable, privileged, ptr);
		Mutex_release(&(ms->lock));
		return 0;
	}
	PGDEnt* tbl = PGDGetAddr((ms->dir) + i);
	if (PGDTest(tbl + j, PG_P)) {
		Mutex_release(&(ms->lock));
		return (-1);
	}
	initEntry(tbl + j, userWritable, privileged, ptr);
	Mutex_release(&(ms->lock));
	return 0;
}
int mapPage(uintptr vAddr, void* ptr, int userWritable, struct MemSpace* ms) {
	return mapPageSpecificPrivilege(vAddr, ptr, userWritable, 0, ms);
}
int unmapPage(uintptr vAddr, struct MemSpace* ms) {
	Mutex_acquire(&(ms->lock));
	u32 p = vAddr;
	if (p & ((u32) 0x00000fff)) {
		bugCheckNum(0x0005 | FAILMASK_PAGING);
	}
	int i = (p >> 22);
	int j = ((p >> 12) & ((u32) 0x000003ff));
	if (!(PGDTest((ms->dir) + i, PG_P))) {
		Mutex_release(&(ms->lock));
		return (-1);
	}
	PGDEnt* tbl = PGDGetAddr((ms->dir) + i);
	if (!(PGDTest(tbl + j, PG_P))) {
		Mutex_release(&(ms->lock));
		return (-1);
	}
	nullifyEntry(tbl + j);
	for (int k = 0; k < 1024; k++) {
		if (k == j) {
			continue;
		}
		if (PGDTest(tbl + k, PG_P)) {
			Mutex_release(&(ms->lock));
			return 0;
		}
	}
	nullifyEntry((ms->dir) + i);
	dealloc_lb(tbl);
	Mutex_release(&(ms->lock));
	return 0;
}
int pageExists(uintptr vAddr, struct MemSpace* ms) {
	Mutex_acquire(&(ms->lock));
	u32 p = vAddr;
	int i = (p >> 22);
	int j = ((p >> 12) & ((u32) 0x000003ff));
	if (!(PGDTest((ms->dir) + i, PG_P))) {
		Mutex_release(&(ms->lock));
		return 0;
	}
	PGDEnt* tbl = PGDGetAddr((ms->dir) + i);
	if (!(PGDTest(tbl + j, PG_P))) {
		Mutex_release(&(ms->lock));
		return 0;
	}
	Mutex_release(&(ms->lock));
	return 1;
}
void* pageMapping(uintptr vAddr, struct MemSpace* ms) {
	Mutex_acquire(&(ms->lock));
	u32 p = vAddr;
	int i = (p >> 22);
	int j = ((p >> 12) & ((u32) 0x000003ff));
	if (!(PGDTest((ms->dir) + i, PG_P))) {
		Mutex_release(&(ms->lock));
		return NULL;
	}
	PGDEnt* tbl = PGDGetAddr((ms->dir) + i);
	if (!(PGDTest(tbl + j, PG_P))) {
		Mutex_release(&(ms->lock));
		return NULL;
	}
	void* b = PGDGetAddr(tbl + j);
	Mutex_release(&(ms->lock));
	return b;
}
// TODO TLB management
struct MemSpace* MemSpace_kernel;
void initPaging(void) {
	MemSpace_kernel = MemSpace_create();
	uintptr m = 0;
	while (m < amntMem) {
		if (mapPageSpecificPrivilege(m, (void*) (m - RELOC), 0, 1, MemSpace_kernel)) {
			bugCheckNum(0x0003 | FAILMASK_PAGING);
		}
		m += PAGE_SIZE;
	}
	CR3Load(((u32) (MemSpace_kernel->dir)) + ((u32) RELOC));
	WPPGSetup();
	return;
}
#undef NAMESPACE_PG
#endif
