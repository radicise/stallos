#ifndef __MACHINE_X86_32_THREADSTATE_H__
#define __MACHINE_X86_32_THREADSTATE_H__ 1
#include "segments.h"
struct Thread_state {// Update all definitions of this structure if and when any of them are updated
	TSS tss;
	u32 pageDirectory;
};
void Threadstate_fill(uintptr stack, uintptr entry, struct MemSpace* mem, struct Thread_state* state) {
	TSSmk(13 * 8, (6 * 8) + 0x0003, (5 * 8) + 0x0003, entry, stack, 0, ((uintptr) (mem->dir)) + RELOC, 128, &(state->tss));
	return;
}
#endif
