#ifndef __MACHINE_X86_32_THREADSTATE_H__
#define __MACHINE_X86_32_THREADSTATE_H__ 1
struct Thread_state {// Update all definitions of this structure if and when any of them are updated
	u32 eax;// off=0x00
	u32 ebx;// off=0x04
	u32 ecx;// off=0x08
	u32 edx;// off=0x0c
	u32 ebp;// off=0x10
	u32 esp;// off=0x14; does not include pushed values from the interrupt procedure except, if the thread had a non-kernel privilege level, both %ss and %esp
	u32 esi;// off=0x18
	u32 edi;// off=0x1c
	u32 eip;// off=0x20
	u32 eflags;// off=0x24
	u16 cs;// off=0x28
	u16 ds;// off=0x2a
	u16 ss;// off=0x2c
	u16 es;// off=0x2e
	u16 fs;// off=0x30
	u16 gs;// off=0x32
};
#endif
