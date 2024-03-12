extern void bugCheck(void);
extern void irupt_null(void);
extern void set_idt(void*, int);
extern void irupt_discall(void);
extern void farRunELF(void);
extern void irupt_hang(void);
#define CHAR_BIT 8
/* TODO Interrupt handler to fail the running program after divide-by zero exceptions &c */
void set_irupt(void* base, unsigned int index, void(*func)(void), unsigned char trap, unsigned short seg) {
	unsigned char* pos = ((unsigned char*) base) + (index * 8);
	0[pos] = (unsigned char) (unsigned int) func;/* EIP byte 0 */
	1[pos] = (unsigned char) (((unsigned int) func) >> 8);/* EIP byte 1 */
	2[pos] = seg;
	3[pos] = seg >> 8;
	4[pos] = 0x00;
	5[pos] = 0x8e ^ (trap & 0x01);
	6[pos] = (unsigned char) (((unsigned int) func) >> 16);/* EIP byte 2 */
	7[pos] = (unsigned char) (((unsigned int) func) >> 24);/* EIP byte 3 */
}
#define SEGBMP 0x07f0
#define SEGLEN 64
int setup() {/* setup() IS NOT ABLE TO ISSUE ANY ERRORS */
	for (int i = 0; i < SEGLEN; i++) {
		((unsigned char*) SEGBMP)[i / CHAR_BIT] &= (((unsigned char) 0xfe) << (i % CHAR_BIT));
	}
	(*((unsigned char*) SEGBMP)) ^= 0x03;// Reservation for the null descriptor
	for (int i = 0; i < 256; i++) {
		set_irupt((void*) 0x7f800, i, bugCheck, 0x00, 0x08);// Unknown interrupts
	}
	for (int i = 0x70; i < 0x80; i++) {
		set_irupt((void*) 0x7f800, i, bugCheck, 0x00, 0x08);// IRQ 0 to 15, EIP and %cs values are later substituted
	}
	set_irupt((void*) 0x7f800, 0x80, bugCheck, 0x01, 0x08);// System call interface for x86_32 programs, EIP and %cs values are later substituted
	set_idt((void*) 0x00007f800, 2047);
	return 0;
}
struct Thread_state {
	unsigned long eax;
	unsigned long ebx;
	unsigned long ecx;
	unsigned long edx;
	unsigned long ebp;
	unsigned long esp;
	unsigned long esi;
	unsigned long edi;
	unsigned long eip;
	unsigned long eflags;
	unsigned short cs;
	unsigned short ds;
	unsigned short ss;
	unsigned short es;
	unsigned short fs;
	unsigned short gs;
};
extern int runELF(void*, void*, struct Thread_state*);
extern void Thread_run(struct Thread_state*);
int executeSystem() {
	/*
	*((char*) 0xb8000) = *((char*) 0x00010000);
	while (1) {
	}
	*/
	(*((void**) 0x7f7f8)) = (void*) farRunELF;
	(*((int*) 0x7f7fc)) = 0x00000008;
	struct Thread_state state;
	int i = runELF((void*) 0x00010000, (void*) 0x00040000, &state);
	if (i) {
		return i;
	}
	state.eflags = 0x00000002;
	Thread_run(&state);
	return 0;
}
#define GDTD 0x7fa
unsigned long findSeg(void) {// Changes LGDT descriptor data, finds the offset for the next segment pair; returns the value 0 if there are no available segment pairs
	unsigned long i = 0;
	unsigned char t = 0x01;
	unsigned char* pos = (unsigned char*) SEGBMP;
	while (1) {
		while (1) {
			if (!(t & (*pos))) {
				(*pos) ^= t;
				i <<= 4;
				if (((*((unsigned short*) GDTD)) + 9) == i) {
					(*((unsigned short*) GDTD)) += 0x10;
				}
				return i - 8;
			}
			i++;
			if (i == SEGLEN) {
				return 0;
			}
			t <<= 1;
			if (t == 0) {
				break;
			}
		}
		t = 0x01;
		pos += 1;
	}
}
unsigned long removeSeg(unsigned long seg) {//Changes the LGDT descriptor data; this does not invalidate the GDT entry; 0: Success; 1: Failure
	seg += 8;
	if ((seg + 16) == ((*((unsigned short*) GDTD)) + 9)) {
		(*((unsigned short*) GDTD)) -= 0x10;
	}
	seg >>= 4;
	int p = seg / CHAR_BIT;
	seg %= CHAR_BIT;
	((unsigned char*) SEGBMP)[p] ^= (((unsigned char) 0x01) << seg);
}
