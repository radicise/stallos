extern void bugCheck(void);
extern void irupt_null(void);
extern void set_idt(void*, int);
extern void irupt_discall(void);
extern int runELF(void* elf, void* loadTo, int* code);
extern void farRunELF(void);
extern void irupt_hang(void);
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
int setup() {/* setup() IS NOT ABLE TO ISSUE ANY ERRORS */
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
int executeSystem() {
	int retVal = 0;
	/*
	*((char*) 0xb8000) = *((char*) 0x00010000);
	while (1) {
	}
	*/
	(*((void**) 0x7f7f8)) = (void*) farRunELF;
	(*((int*) 0x7f7fc)) = 0x00000008;
	int i = runELF((void*) 0x00010000, (void*) 0x00040000, &retVal);
	return i;
}
