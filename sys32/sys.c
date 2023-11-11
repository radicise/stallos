extern void bugCheck(void);
extern void irupt_null(void);
extern void set_idt(void*, int);
extern void int_enable(void);
extern void irupt_discall(void);
extern int runELF(void* elf, void* loadTo, int* code);
/* TODO Interrupt handler to fail the running program after divide-by zero exceptions &c */
void set_irupt(void* base, unsigned int index, void(*func)(void), unsigned char trap) {
	unsigned char* pos = ((char*) base) + (index * 8);
	0[pos] = (unsigned char) (unsigned int) func;/* EIP byte 0 */
	1[pos] = (unsigned char) (((unsigned int) func) >> 8);/* EIP byte 1 */
	2[pos] = 0x08;/* Kernel code GDT index lobyte */
	3[pos] = 0x00;/* Kernel code GDT index hibyte */
	4[pos] = 0x00;
	5[pos] = 0x8e ^ (trap & 0x01);
	6[pos] = (unsigned char) (((unsigned int) func) >> 16);/* EIP byte 2 */
	7[pos] = (unsigned char) (((unsigned int) func) >> 24);/* EIP byte 3 */
}
int setup() {/* setup() IS NOT ABLE TO ISSUE ANY ERRORS */
	for (int i = 0; i < 128; i++) {
		set_irupt((void*) 0x7400, i, irupt_null, 0x00);/* TODO Differentiate between hardware interrupts and ISA exceptions */
	}
	for (int i = 128; i < 256; i++) {
		set_irupt((void*) 0x7400, i, irupt_null, 0x01);/* Traps */
	}
	for (int i = 0; i < 256; i++) {
		set_irupt((void*) 0x7400, i, bugCheck, 0x00);
	}
	set_idt((void*) 0x00007400, 2047);
	int_enable();
	return 0;
}
int executeSystem() {
	int retVal = 0;
	/*
	*((char*) 0xb8000) = *((char*) 0x00010000);
	while (1) {
	}
	*/

	int i = runELF((void*) 0x00010000, (void*) 0x00020000, &retVal);
	return i;
}
