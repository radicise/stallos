#define RELOC 0x00020000
void bugCheck() {//Fatal kernel errors
	while (1) {
		(*((unsigned char*) (0xb8000 - RELOC)))++;
	}
}
#include "kernel/VGATerminal.h"
unsigned int strlen(char* str) {
	char* n = str;
	while (*n++) {
	}
	return n - str - 1;
}
struct VGATerminal mainTerm;
int kernelMsg(char* msg) {
	unsigned int len = strlen(msg);
	if (len != VGATerminalWrite(&mainTerm, msg, len)) {
		return (-1);
	}
	return 0;
}
#include "kernel/syscalls.h"
void _start() {
	initializeVGATerminal(&mainTerm, 80, 25, (struct VGACell*) (0x000b8000 - RELOC));
	for (unsigned int i = (0x000b8000 - RELOC); i < (0x000b8fa0 - RELOC); i += 2) {
		((struct VGACell*) i)->format = mainTerm.format;
		((struct VGACell*) i)->text = 0x20;
	}
	/* Style */
	((struct VGACell*) (0x000b8000 - RELOC))->format ^= 0x77;
	mainTerm.onlcr = 1;
	mainTerm.cursor = 1;
	/* End-of-style */
	while (1) {
	}
}
