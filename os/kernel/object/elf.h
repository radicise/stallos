#ifndef __OBJECT_ELF_H__
#define __OBJECT_ELF_H__ 1
#ifndef TARGETNUM
#error "'TARGETNUM' is not set"
#endif
#if TARGETNUM == 1
// x86_32
#elif TARGETNUM == 4
// arm
#else
#error "Target is not supported"
#endif
/*
 *
 * TODO Relax structure packing and endianness requirements
 *
 */
#include "../types.h"
#include "../threads.h"
#include "../perThreadgroup.h"
#include "../paging.h"
#include "../kmemman.h"
#define FAILMASK_ELF 0x00120000
typedef u32 Elf32_Addr;
typedef u16 Elf32_Half;
typedef u32 Elf32_Off;
typedef s32 Elf32_Sword;
typedef u32 Elf32_Word;
typedef struct {
	u8 e_ident;
	u8 e_ident_1;
	u8 e_ident_2;
	u8 e_ident_3;
	u8 e_ident_4;
	u8 e_ident_5;
	u8 e_ident_6;
	u8 e_ident_7;
	u8 e_ident_8;
	u8 e_ident_9;
	u8 e_ident_10;
	u8 e_ident_11;
	u8 e_ident_12;
	u8 e_ident_13;
	u8 e_ident_14;
	u8 e_ident_15;
	Elf32_Half e_type;
	Elf32_Half e_machine;
	Elf32_Word e_version;
	Elf32_Addr e_entry;
	Elf32_Off e_phoff;
	Elf32_Off e_shoff;
	Elf32_Word e_flags;
	Elf32_Half e_ehsize;
	Elf32_Half e_phentsize;
	Elf32_Half e_phnum;
	Elf32_Half e_shentsize;
	Elf32_Half e_shnum;
	Elf32_Half e_shstrndx;
} Elf32_Ehdr;
typedef struct {
	Elf32_Word p_type;
	Elf32_Off p_offset;
	Elf32_Addr p_vaddr;
	Elf32_Addr p_paddr;
	Elf32_Word p_filesz;
	Elf32_Word p_memsz;
	Elf32_Word p_flags;
	Elf32_Word p_align;
} Elf32_Phdr;
int ELF_cpyUser(uintptr dest, const void* from, uintptr count, int userWritable, struct MemSpace* mem) {
	MemSpace_mkData(from, dest, count, userWritable, mem);// TODO How should overlapping segments that should all be loaded be handled?
	return 0;
}
int ELF_fillUser(uintptr dest, uintptr count, int userWritable, struct MemSpace* mem) {
	MemSpace_mkFill(0x00, dest, count, userWritable, mem);// TODO How should overlapping segments that should all be loaded be handled?
	return 0;
}
int loadSeg(const Elf32_Phdr* seg, const void* fileBase, uintptr* stack, struct MemSpace* mem) {
	Elf32_Word t = seg->p_type;
	if ((t == 0) | (t == 4) | (t == 6)) {
		return 0;
	}
	if (t == 2) {
		return 19;/* Dynamic linking is not supported */
	}
	if (t == 3) {
		return 20;/* Program interpretation is not supported */
	}
	if (t == 5) {
		return 21;/* Unsupported segment type */
	}
	if ((t > 0x70000000) & (t <= 0x7fffffff)) {
		return 22;/* Unrecognised processor-specific segment type */
		/* TODO Check for these in the appropriate ABI &c. standards */
	}
	if (t > 6) {
		if (t != (Elf32_Word) 0x6474e551) {/* Disregard of PT_GNU_STACK because all segments are executable *//* TODO Pay attention to the p_flags value of PT_GNU_STACK when p_flags becomes used for setting executability, in order to set executability of the stack accordingly */
			return 23;/* Unrecognised segment type */
		}
	}
	/* TODO Use p_flags for setting read and execute permissions */
	if ((seg->p_filesz) > (seg->p_memsz)) {
		return 24;/* Memory image is of smaller size than file image */
	}
	ELF_cpyUser((uintptr) (seg->p_vaddr), ((const char*) fileBase) + (seg->p_offset), seg->p_filesz, (seg->p_flags & 0x02) ? 1 : 0, mem);
	if ((seg->p_memsz) > (seg->p_filesz)) {
		ELF_fillUser((uintptr) ((seg->p_vaddr) + (seg->p_filesz)), (seg->p_memsz) - (seg->p_filesz), (seg->p_flags & 0x02) ? 1 : 0, mem);
	}
	uintptr s = seg->p_vaddr + seg->p_memsz;
	if (s > (*stack)) {
		(*stack) = s;
	}
	return 0;
}
int loadELF(const Elf32_Ehdr* prgm, uintptr* s, struct MemSpace* mem) {
	if (prgm->e_ident != 0x7F) {
		return 1;/* Invalid magic */
	}
	if (prgm->e_ident_1 != 0x45) {
		return 1;
	}
	if (prgm->e_ident_2 != 0x4C) {
		return 1;
	}
	if (prgm->e_ident_3 != 0x46) {
		return 1;
	}
	u8 c = prgm->e_ident_4;
	if (c == 0) {
		return 2;/* Invalid ELF class */
	}
	if (c == 2) {
		return 3;/* ELF class not supported */
	}
	if (c > 2) {
		return 4;/* ELF class not recognised */
	}
	c = prgm->e_ident_5;
	if (c == 0) {
		return 5;/* Invalid data encoding */
	}
	if (c == 2) {
		return 6;/* Data encoding not supported */
	}
	if (c > 2) {
		return 7;/* Unrecognised data encoding */
	}
	c = prgm->e_ident_6;
	if (c != (prgm->e_version)) {
		return 8;/* Inconsistent ELF version identifier */
	}
	if (c == 0) {
		return 9;/* Invalid ELF version */
	}
	if (c > 1) {
		return 10;/* Unrecognised ELF version identifier */
	}
	Elf32_Half t = prgm->e_type;
	if (t == 0) {
		return 11;/* No ELF file type */
	}
	if (t == 1) {
		return 12;/* ELF relocatable file */
	}
	if (t == 3) {
		return 13;/* ELF shared object file */
	}
	if (t == 4) {
		return 14;/* ELF core file */
	}
	if (t >= 0xff00) {
		return 15;/* Unrecognised processor-specific ELF file type */
		/* TODO Check for these in the appropriate ABI &c. standards */
	}
	if (t >= 0xfe00) {
		return 27;/* Unrecognised operating system-specific ELF file type */
		/* TODO Check for the meanings of these in Linux */
	}
	if (t != 2) {
		return 16;/* Unrecognised ELF file type */
	}
	t = prgm->e_machine;
	if (t != 0) {
#ifndef TARGETNUM
#error "'TARGETNUM' is not set"
#endif
#if TARGETNUM == 1
		// x86_32
		if (t != 3) {// TODO How should an `e_type' value of 6 be handled?
			return 17;/* Unrecognised or unsupported architecture */
		}
#elif TARGETNUM == 4
		// arm
		if (t != 40) {
			return 17;/* Unrecognised or unsupported architecture */
		}
#else
#error "Target is not supported"
#endif
	}
	const Elf32_Phdr* seg = (const Elf32_Phdr*) (((const char*) prgm) + (prgm->e_phoff));
	Elf32_Half ps = prgm->e_phentsize;
	Elf32_Half pnum = prgm->e_phnum;
	int res = 0;
	while (pnum--) {
		res = loadSeg(seg, prgm, s, mem);
		if (res != 0) {
			return res;
		}
		seg = (const Elf32_Phdr*) (((const char*) seg) + (uintptr) ps);
	}
	return 0;
}
void freePage(uintptr vAddr, void* addr, void* arb) {
	dealloc_lb(addr);
}
int runELF(const void* elf, struct Thread* thread) {// The object at `thread' should be filled out, excepting the contents at `thread->state' and the value of `thread->group->mem'
	thread->group->mem = MemSpace_create();
	uintptr s = 0;
	int i = loadELF((const Elf32_Ehdr*) elf, &s, thread->group->mem);
	if (i != 0) {
		MemSpace_forEach(freePage, NULL, thread->group->mem);
		MemSpace_destroy(thread->group->mem);
		return i;
	}
	uintptr sl = s;
	s += ((uintptr) 0x00200000);// TODO How much space should be given to the stack?
	if (s < (uintptr) 0x00200000) {// TODO Ensure that there is at least one available page in the stack
		MemSpace_forEach(freePage, NULL, thread->group->mem);
		MemSpace_destroy(thread->group->mem);
		return 28;// TODO Is this denial allowed?
	}
	//TODO URGENT Stop the assumption that the stack grows downward
#ifndef TARGETNUM
#error "'TARGETNUM' is not set"
#endif
#if TARGETNUM == 1
	// x86_32
	s = ((uintptr) s) - (((uintptr) s) % 4);
	if (mapPageSpecificPrivilege((uintptr) 0, (void*) (0 - RELOC), 0, 1, thread->group->mem)) {
		MemSpace_forEach(freePage, NULL, thread->group->mem);
		MemSpace_destroy(thread->group->mem);
		return 29;// TODO Is this denial allowed?
	}
#else
#error "Target is not supported"
#endif
	uintptr m = PAGEOF(s);
	sl = PAGEOF(sl + PAGE_SIZE);
	while (m != sl) {
		if (mapPage(m, alloc_lb_wiped(), 1, thread->group->mem)) {
			bugCheckNum(0x0001 | FAILMASK_ELF);
		}
		m = PAGEOF(m - 1);
	}
	// TODO Do not allocate the entirety of the stack at once when on x86, maybe
	Threadstate_fill(s, ((const Elf32_Ehdr*) elf)->e_entry, thread->group->mem, &(thread->state));
	return 0;
}
#endif