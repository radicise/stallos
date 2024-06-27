#ifndef __SWAP_H__
#define __SWAP_H__ 1
#define FAILMASK_SWAP 0x00140000
int page_not_present(uintptr addr, int user) {
	if (user) {
		bugCheckNum(0x0001 | FAILMASK_SWAP);// TODO Dynamic user stack and segfault interface
		return;
	}
	kernelMsgCode("Illegal memory access at address ", (unsigned long) addr);// TODO URGENT Ensure that unsigned long is at least as wide as uintptr
	while (1) {
	}
	return;
}
#endif
