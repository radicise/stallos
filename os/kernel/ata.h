#ifndef __ATA_H__
#define __ATA_H__ 1
#include "types.h"
#define FAILMASK_ATA 0x00040000





int ata_PIO_readSectors(u16 ata, u32 LBA, void* dest,  count) {
	if (LBA & 0xf0000000) {
		bugCheckNum(0x0001 | FAILMASK_ATA);// Not within LBA28 range
	}
	bus_out_u8(ata + 1, 0x00);
	bus_out_u8(ata + 2, 
	LBA++;
}









#endif
