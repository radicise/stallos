#ifndef __DRIVER_PL011_H__
#define __DRIVER_PL011_H__ 1
#ifndef TARGETNUM
#error "'TARGETNUM' is not set"
#endif
#if TARGETNUM > 4
#error "Target is not supported"
#endif

/*
 *
 * This driver is for systems that have 32-bit `int' and pack them consecutively in structs and have `CHAR_BIT' equal to 8; TODO relax the aforementioned requirements
 *
 */
typedef volatile struct {
	unsigned int DR;
	unsigned int RSR;
	unsigned int Reserved_0[4];
	unsigned int FR;
	unsigned int Reserved_1[1];
	unsigned int ILPR;
	unsigned int IBRD;
	unsigned int FBRD;
	unsigned int LCR_H;
	unsigned int CR;
	unsigned int IFLS;
	unsigned int IMSC;
	unsigned int RIS;
	unsigned int MIS;
	unsigned int ICR;
	unsigned int DMACR;
	unsigned int Reserved_2[13];
	unsigned int Reserved_3[4];
	unsigned int Reserved_4[976];
	unsigned int Reserved_5[4];
	unsigned int PeriphID0;
	unsigned int PeriphID1;
	unsigned int PeriphID2;
	unsigned int PeriphID3;
	unsigned int PCellID0;
	unsigned int PCellID1;
	unsigned int PCellID2;
	unsigned int PCellID3;
} PL011UART;
/*
 * PL011WriteCharacter: Returns: The amount of words transmitted
 */
int PL011WriteCharacter(char val, PL011UART* uart) {
	while (uart->FR & 0x00000020) {
	}
	uart->DR = val;
	return 1;
}
/*
 * PL011WriteCharacters: Returns: The amount of words transmitted
 */
int PL011WriteCharacters(char* buf, unsigned int len, PL011UART* uart) {
	unsigned int i = len;
	while (i) {
		while (uart->FR & 0x00000020) {
		}
		uart->DR = *buf;
		buf++;
		i--;
	}
	return len;
}
/*
 * PL011ReadCharacter
 * Returns: -1 if an error concerning reception occurred, the data character value received otherwise
 */
int PL011ReadCharacter(PL011UART* uart) {
	while (uart->FR & 0x00000010) {
	}
	unsigned int i = uart->DR;
	if (i & 0x00000f00) {
		return -1;
	}
	return i & 0xff;
}
/*
 * PL011ReadCharacters
 * Returns: The amount of data characters written to the beginning of the specified buffer, which is less than the specified limit if and only if an error concerning transmission occurred
 */
int PL011ReadCharacters(char* buf, unsigned int len, PL011UART* uart) {
	unsigned int i = len;
	unsigned int c;
	while (i) {
		while (uart->FR & 0x00000010) {
		}
		c = uart->DR;
		if (c & 0x00000f00) {
			break;
		}
		*buf = c;
		buf++;
		i--;
	}
	return len - i;
}
#endif
