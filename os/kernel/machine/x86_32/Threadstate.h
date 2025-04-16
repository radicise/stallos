#ifndef __MACHINE_X86_32_THREADSTATE_H__
#define __MACHINE_X86_32_THREADSTATE_H__ 1
#include "segments.h"
#include "../../paging.h"
struct Thread_state {
	TSS tss;
	TSS ktss;
	int kernelExecution;
	struct {
		unsigned long (*kfunc)(unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long, unsigned long);
		unsigned long data[8];
	} invocData;
};
void Threadstate_fill(uintptr stack, uintptr entry, struct MemSpace* mem, struct Thread_state* state) {
	TSSmk(13 * 8, (6 * 8) + 0x0003, (5 * 8) + 0x0003, entry, stack, 0, ((uintptr) (mem->dir)) + RELOC, 128, &(state->tss));
	TSSmk(13 * 8, 4 * 8, 3 * 8, (uintptr) irupt_80h, 0x007ffff0 - RELOC, 0, ((uintptr) (MemSpace_kernel->dir)) + RELOC, 128, &(state->ktss));
	state->kernelExecution = 0;
	return;
}
void flushThreadState(struct Thread_state* tsp) {
	moveExv(&(tsp->ktss), ((TSS*) (((volatile char*) physicalZero) + 0xb00)), sizeof(TSS));
	moveExv(&(tsp->tss), ((TSS*) (((volatile char*) physicalZero) + 0xb00)) + 5, sizeof(TSS));
	tsp->kernelExecution = (TS_isBusy(((SegDesc*) (((volatile char*) physicalZero) + 0x800)) + 7)) ? 1 : 0;
	return;
}
void prepare_fork_child(struct Thread_state* ts, void* stack, struct MemSpace* mem) {
	ts->tss.pdr = ((uintptr) mem->dir) + (uintptr) RELOC;
	ts->tss.eax = 0x00000000;
	ts->kernelExecution = 0;
	TSSmk(13 * 8, 4 * 8, 3 * 8, (uintptr) irupt_80h, ((uintptr) stack) & 0xfffffff0L, 0, ((uintptr) (MemSpace_kernel->dir)) + RELOC, 128, &(ts->ktss));// TODO Make the original kernel thread use a large-block-allocated block and make all kernel threads have a guard page
	return;
}
#endif
