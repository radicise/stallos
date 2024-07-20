#ifndef __MACHINE_X86_32_SEGMENTS_H__
#define __MACHINE_X86_32_SEGMENTS_H__ 1
#define FAILMASK_SEGMENTS 0x00130000
#include "../../paging.h"
typedef volatile struct {
	u8 b0;
	u8 b1;
	u8 b2;
	u8 b3;
	u8 b4;
	u8 b5;
	u8 b6;
	u8 b7;
} SegDesc;
void Seg_setDesc(uintptr base, uintptr limit, int g, int dt, int exec, int ec, int wr, int a, int dpl, SegDesc* seg) {
	seg->b0 = limit;
	limit >>= 8;
	seg->b1 = limit;
	limit >>= 8;
	seg->b2 = base;
	base >>= 8;
	seg->b3 = base;
	base >>= 8;
	seg->b4 = base;
	base >>= 8;
	if ((dpl < 0) || (dpl >= 4)) {
		bugCheckNum(0x0002 | FAILMASK_SEGMENTS);
	}
	seg->b5 = (((u8) dpl) << 5) | (dt ? 0x10 : 0x00) | (exec ? 0x08 : 0x00) | (ec ? 0x04 : 0x00) | (wr ? 0x02 : 0x00) | (a ? 0x01 : 0x00);
	seg->b6 = (limit & 0x0f) | (g ? 0x80 : 0x00) | 0x40;
	limit >>= 4;
	seg->b7 = base;
	base >>= 8;
	if (base || limit) {
		bugCheckNum(0x0001 | FAILMASK_SEGMENTS);
	}
	return;
}
void Seg_enable(SegDesc* seg) {
	seg->b5 |= (u8) 0x80;
	return;
}
void TG_setDesc(u16 segSel, int pl, SegDesc* seg) {
	seg->b0 = 0x00;
	seg->b1 = 0x00;
	seg->b2 = segSel;
	segSel >>= 8;
	seg->b3 = segSel;
	segSel >>= 8;
	if (segSel) {
		bugCheckNum(0x0004 | FAILMASK_SEGMENTS);
	}
	if ((pl < 0) || (pl > 3)) {
		bugCheckNum(0x0003 | FAILMASK_SEGMENTS);
	}
	seg->b4 = 0x00;
	seg->b5 = (((u8) pl) << 5) | ((u8) 0x05);
	seg->b6 = 0x00;
	seg->b7 = 0x00;
	return;
}
void TS_setDesc(uintptr base, uintptr limit, int g, int dpl, int busy, SegDesc* seg) {
	seg->b0 = limit;
	limit >>= 8;
	seg->b1 = limit;
	limit >>= 8;
	seg->b2 = base;
	base >>= 8;
	seg->b3 = base;
	base >>= 8;
	seg->b4 = base;
	base >>= 8;
	if ((dpl < 0) || (dpl >= 4)) {
		bugCheckNum(0x0006 | FAILMASK_SEGMENTS);
	}
	seg->b5 = (((u8) dpl) << 5) | (busy ? 0x02 : 0x00) | 0x09;
	seg->b6 = (limit & 0x0f) | (g ? 0x80 : 0x00);
	limit >>= 4;
	seg->b7 = base;
	base >>= 8;
	if (base || limit) {
		bugCheckNum(0x0005 | FAILMASK_SEGMENTS);
	}
	return;
}
int TS_isBusy(SegDesc* seg) {
	return (seg->b5 & 0x02) ? 1 : 0;
}
void SSS_setDesc(uintptr base, uintptr limit, int g, int dpl, int type, SegDesc* seg) {
	seg->b0 = limit;
	limit >>= 8;
	seg->b1 = limit;
	limit >>= 8;
	seg->b2 = base;
	base >>= 8;
	seg->b3 = base;
	base >>= 8;
	seg->b4 = base;
	base >>= 8;
	if ((dpl < 0) || (dpl >= 4)) {
		bugCheckNum(0x0009 | FAILMASK_SEGMENTS);
	}
	if ((type < 0) || (type >= 16)) {
		bugCheckNum(0x0007 | FAILMASK_SEGMENTS);
	}
	seg->b5 = (((u8) dpl) << 5) | type;
	seg->b6 = (limit & 0x0f) | (g ? 0x80 : 0x00) | 0x40;
	limit >>= 4;
	seg->b7 = base;
	base >>= 8;
	if (base || limit) {
		bugCheckNum(0x0008 | FAILMASK_SEGMENTS);
	}
	return;
}
typedef volatile struct {
	u16 prev;// offset=0x00;
	u16 z0;
	u32 esp0;
	u16 ss0;
	u16 z1;
	u32 esp1;
	u16 ss1;// offset=0x10
	u16 z2;
	u32 esp2;
	u16 ss2;
	u16 z3;
	u32 pdr;
	u32 eip;// offset=0x20
	u32 eflags;
	u32 eax;
	u32 ecx;
	u32 edx;// offset=0x30
	u32 ebx;
	u32 esp;
	u32 ebp;
	u32 esi;// offset=0x40
	u32 edi;
	u16 es;
	u16 z4;
	u16 cs;
	u16 z5;
	u16 ss;// offset=0x50
	u16 z6;
	u16 ds;
	u16 z7;
	u16 fs;
	u16 z8;
	u16 gs;
	u16 z9;
	u16 ldt;// offset=0x60
	u16 z10;
	u8 debugTrap;
	u8 z11;
	u16 iobase;// TODO Does this need to be set up for the kernel-space? An 80386 manual has internal conflicts surrounding the use of the I/O Permission Bitmap
	u8 data[24];// offset=0x68
} TSS;
void TSSmk(u16 prev, u16 dseg, u16 cseg, uintptr eip, uintptr esp, int disableIrupts, uintptr pdPhys, u16 iobase, TSS* tss) {
	tss->prev = prev;
	tss->z0 = 0x00;
	tss->esp0 = esp;
	tss->ss0 = dseg;
	tss->z1 = 0x00;
	tss->esp1 = esp;
	tss->ss1 = dseg;
	tss->z2 = 0x00;
	tss->esp2 = esp;
	tss->ss2 = dseg;
	tss->z3 = 0x00;
	tss->pdr = pdPhys;
	tss->eip = eip;
	tss->eflags = (disableIrupts ? 0x00000002 : 0x00000202);
	tss->eax = 0x00000000;
	tss->ecx = 0x00000000;
	tss->edx = 0x00000000;
	tss->ebx = 0x00000000;
	tss->esp = esp;
	tss->ebp = 0x00000000;
	tss->esi = 0x00000000;
	tss->edi = 0x00000000;
	tss->es = dseg;
	tss->z4 = 0x00;
	tss->cs = cseg;
	tss->z5 = 0x00;
	tss->ss = dseg;
	tss->z6 = 0x00;
	tss->ds = dseg;
	tss->z7 = 0x00;
	tss->fs = dseg;
	tss->z8 = 0x00;
	tss->gs = dseg;
	tss->z9 = 0x00;
	tss->ldt = 8 * 8;
	tss->z10 = 0x00;
	tss->debugTrap = 0;
	tss->z11 = 0x00;
	tss->iobase = iobase;
	tss->data[0] = 0x00;
	tss->data[1] = 0x00;
	tss->data[2] = 0x00;
	tss->data[3] = 0x00;
	tss->data[4] = 0x00;
	tss->data[5] = 0x00;
	tss->data[6] = 0x00;
	tss->data[7] = 0x00;
	tss->data[8] = 0x00;
	tss->data[9] = 0x00;
	tss->data[10] = 0x00;
	tss->data[11] = 0x00;
	tss->data[12] = 0x00;
	tss->data[13] = 0x00;
	tss->data[14] = 0x00;
	tss->data[15] = 0x00;
	tss->data[16] = 0x00;
	tss->data[17] = 0x00;
	tss->data[18] = 0x00;
	tss->data[19] = 0x00;
	tss->data[20] = 0x00;
	tss->data[21] = 0x00;
	tss->data[22] = 0x00;
	tss->data[23] = 0x00;
}
extern void loadGDT(unsigned long, unsigned long);
extern void loadIDT(unsigned long, unsigned long);
extern void loadTS(unsigned long);
extern unsigned long storeTS(void);
extern void loadLDT(unsigned long);
extern void setNT(void);
void initSegmentation(void) {
	/*
	 *
	 * GDT Segments:
	 * 0x0000 - "Null segment"
	 * 0x0008 - Non-offset kernel code
	 * 0x0010 - Non-offset kernel data
	 * 0x0018 - Offset kernel code
	 * 0x0020 - Offset kernel data
	 *
	 * LDT Segments: none
	 *
	 */
	SegDesc* GDT = (SegDesc*) (((volatile char*) physicalZero) + 0x800);
	GDT->b0 = 0x00;
	GDT->b1 = 0x00;
	GDT->b2 = 0x00;
	GDT->b3 = 0x00;
	GDT->b4 = 0x00;
	GDT->b5 = 0x00;
	GDT->b6 = 0x00;
	GDT->b7 = 0x00;
	Seg_setDesc(0, 0xfffff, 1, 1, 1, 0, 1, 1, 0, GDT + 1);
	Seg_enable(GDT + 1);/* Non-offset kernel code */
	Seg_setDesc(0, 0xfffff, 1, 1, 0, 0, 1, 1, 0, GDT + 2);
	Seg_enable(GDT + 2);/* Non-offset kernel data */
	Seg_setDesc(RELOC, 0xfffff, 1, 1, 1, 0, 1, 1, 0, GDT + 3);// TODO URGENT What happens with address-space wrap-around?
	Seg_enable(GDT + 3);/* Offset kernel code */
	Seg_setDesc(RELOC, 0xfffff, 1, 1, 0, 0, 1, 1, 0, GDT + 4);// TODO URGENT What happens with address-space wrap-around?
	Seg_enable(GDT + 4);/* Offset kernel data */
	Seg_setDesc(0, 0xfffff, 1, 1, 1, 0, 1, 1, 3, GDT + 5);
	Seg_enable(GDT + 5);/* User-space code */
	Seg_setDesc(0, 0xfffff, 1, 1, 0, 0, 1, 1, 3, GDT + 6);
	Seg_enable(GDT + 6);/* User-space data */
	TS_setDesc(0x0b00, 127, 0, 0, 0, GDT + 7);
	Seg_enable(GDT + 7);
	SSS_setDesc(0x0828, 15, 0, 3, 0x2, GDT + 8);// TODO URGENT What value should the DPL of the LDT take?
	Seg_enable(GDT + 8);
	TS_setDesc(0x0b80, 127, 0, 0, 0, GDT + 9);
	Seg_enable(GDT + 9);
	TS_setDesc(0x0c00, 127, 0, 0, 0, GDT + 10);
	Seg_enable(GDT + 10);
	TS_setDesc(0x0c80, 127, 0, 0, 0, GDT + 11);
	Seg_enable(GDT + 11);
	TS_setDesc(0x0d00, 127, 0, 0, 0, GDT + 12);
	Seg_enable(GDT + 12);
	TS_setDesc(0x0c80, 127, 0, 0, 1, GDT + 13);
	Seg_enable(GDT + 13);
	TS_setDesc(0x0e00, 127, 0, 0, 0, GDT + 14);
	Seg_enable(GDT + 14);
	TSS* tssp = (TSS*) (((volatile char*) physicalZero) + 0xb00);
	TSSmk(13 * 8, 4 * 8, 3 * 8, (uintptr) irupt_80h, 0x007ffff0 - RELOC, 0, ((uintptr) (MemSpace_kernel->dir)) + RELOC, 128, tssp);
	TSSmk(9 * 8, 4 * 8, 3 * 8, (uintptr) irupt_70h, 0x007f7f0 - RELOC, 1, ((uintptr) (MemSpace_kernel->dir)) + RELOC, 128, tssp + 1);
	TSSmk(10 * 8, 4 * 8, 3 * 8, (uintptr) irupt_71h, 0x007f7f0 - RELOC, 1, ((uintptr) (MemSpace_kernel->dir)) + RELOC, 128, tssp + 2);
	TSSmk(11 * 8, 4 * 8, 3 * 8, (uintptr) irupt_7dh, 0x007f7f0 - RELOC, 1, ((uintptr) (MemSpace_kernel->dir)) + RELOC, 128, tssp + 3);
	TSSmk(12 * 8, 4 * 8, 3 * 8, (uintptr) irupt_7fh, 0x007f7f0 - RELOC, 1, ((uintptr) (MemSpace_kernel->dir)) + RELOC, 128, tssp + 4);
	TSSmk(14 * 8, 4 * 8, 3 * 8, (uintptr) irupt_yield, 0x007f7f0 - RELOC, 1, ((uintptr) (MemSpace_kernel->dir)) + RELOC, 128, tssp + 6);// TODO Is it acceptable to disable interrupts during the CPU yielding sequence?
	SegDesc* IDT = (SegDesc*) (volatile char*) physicalZero;
	for (int i = 0; i < 256; i++) {// TODO URGENT Move the IRQ stack area
		TG_setDesc(11 * 8, 0, IDT + i);
		Seg_enable(IDT + i);
	}// TODO URGENT add guard-pages to all kernel stack areas
	TG_setDesc(7 * 8, 3, IDT + 0x80);
	Seg_enable(IDT + 0x80);
	TG_setDesc(9 * 8, 0, IDT + 0x70);
	Seg_enable(IDT + 0x70);
	TG_setDesc(10 * 8, 0, IDT + 0x71);
	Seg_enable(IDT + 0x71);
	TG_setDesc(12 * 8, 0, IDT + 0x7e);
	Seg_enable(IDT + 0x7e);
	TG_setDesc(12 * 8, 0, IDT + 0x7f);
	Seg_enable(IDT + 0x7f);
	TG_setDesc(14 * 8, 0, IDT + 0x40);
	Seg_enable(IDT + 0x40);
	loadGDT(0x800, (15 * 8) - 1);
	loadIDT(0x000, 0x7ff);
	loadLDT(8 * 8);
	loadTS(7 * 8);
}
#endif
