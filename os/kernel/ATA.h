#ifndef __ATA_H__
#define __ATA_H__ 1
#include "types.h"
#include "blockCompat.h"
#define FAILMASK_ATA 0x00040000
#define ATA_SR_ERR 0x01
#define ATA_SR_IDX 0x02
#define ATA_SR_CORR 0x04
#define ATA_SR_DRQ 0x08
#define ATA_SR_DSC 0x10
#define ATA_SR_DWF 0x20
#define ATA_SR_DRDY 0x40
#define ATA_SR_BSY 0x80
struct ATA {
	u16 bus;
	unsigned char slaveLastUsed;
	unsigned char notUsed;
	Mutex inUse;// There is an operation
};
struct ATAFile {
	struct ATA* ata;
	unsigned char slave;
};
int ata_PIO_transferSectors(unsigned long long blockAddr, void* dest, unsigned long long amnt, unsigned char write, struct ATAFile* ataDrive) {
	if (amnt > 256) {
		bugCheckNum(0x0002 | FAILMASK_ATA);// Amount of sectors is too large
	}
	unsigned long count = amnt;
	if (blockAddr & (~((unsigned long long) 0x0fffffff))) {
		bugCheckNum(0x0001 | FAILMASK_ATA);// Not within LBA28 range, do not assume that the drive supports LBA48
	}
	u32 LBA = blockAddr;
	struct ATA* ata = ataDrive->ata;
	unsigned char slave = ataDrive->slave;
	Mutex_acquire(&(ata->inUse));
	u16 bus = ata->bus;
	bus_out_u8(bus + 6, 0xe0 | (slave << 4) | (LBA >> 24));
	if ((ata->slaveLastUsed != slave) || ata->notUsed) {
		ata->notUsed = 0x00;
		ata->slaveLastUsed = slave;
		for (int i = 0; i < 14; i++) {// For delay, suggested
			bus_in_u8(bus + 7);
		}
	}	
	u8 status = bus_in_u8(bus + 7);
	if (status & ATA_SR_BSY) {
		kernelWarnMsg("ATA command block register was accessed when busy");// TODO What if it was set or cleared between the access and the status register read
		Mutex_release(&(ata->inUse));
		return (-1);
	}
	if (status & ATA_SR_DRQ) {
		kernelWarnMsg("ATA unexpectedly expects payload data");
	}
	while (!(status & ATA_SR_DRDY)) {
		status = bus_in_u8(bus + 7);
	}
	bus_out_u8(bus + 1, 0x00);// TODO Is this necessary? Is this allowed?
	bus_out_u8(bus + 2, count);
	bus_out_u8(bus + 3, LBA);
	bus_out_u8(bus + 4, LBA >> 8);
	bus_out_u8(bus + 5, LBA >> 16);
	bus_out_u8(bus + 7, 0x20);
	for (int i = 0; i < 4; i++) {// For delay
		bus_in_u8(bus + 7);
	}
	status = bus_in_u8(bus + 7);
	if (status & ATA_SR_ERR) {
		kernelWarnMsgCode("ATA Drive: Error: ", bus_in_u8(bus + 1));
		Mutex_release(&(ata->inUse));
		return (-1);
	}
	if (status & ATA_SR_DWF) {
		kernelWarnMsg("ATA Drive: Drive Write Fault");
		Mutex_release(&(ata->inUse));
		return (-1);
	}
	while (!(status & ATA_SR_DRQ)) {
		bus_wait();
	}
	if (write) {
		amnt <<= 8;
		while (amnt--) {
			while (!(bus_in_u8(bus + 7) & ATA_SR_DRQ)) {
			}// TODO Do not run for first iteration
			bus_out_u16(bus, *((u16*) dest));
			dest = (char*) (dest + 2);
		}
		while (!(bus_in_u8(bus + 7) & ATA_SR_DRDY)) {
		}
		bus_out_u8(bus + 7, 0xe7);
		for (int i = 0; i < 4; i++) {// For delay
			bus_in_u8(bus + 7);
		}
		while (bus_in_u8(bus + 7) & ATA_SR_BSY) {
		}
		Mutex_release(&(ata->inUse));
	}
	else {
		amnt <<= 8;
		while (amnt--) {
			while (!(bus_in_u8(bus + 7) & ATA_SR_DRQ)) {
			}// TODO Do not run for first iteration
			(*((u16*) dest)) = bus_in_u16(bus);
			dest = (char*) (dest + 2);
		}
	}
	while (bus_in_u8(bus + 7) & ATA_SR_BSY) {
	}
	Mutex_release(&(ata->inUse));
	return 0;
}
int ata_readBlock(unsigned long long block, unsigned long long amnt, void* dst, void* drive) {
	return ata_PIO_transferSectors(block, dst, amnt, 0, (struct ATAFile*) drive);
}
int ata_writeBlock(unsigned long long block, unsigned long long amnt, const void* src, void* drive) {
	return ata_PIO_transferSectors(block, src, amnt, 1, (struct ATAFile*) drive);
}
struct ATA ATA_0;
struct ATA ATA_1;
struct ATAFile ATA_0m;// hda
struct ATAFile ATA_0s;// hdb
struct ATAFile ATA_1m;// hdc
struct ATAFile ATA_1s;// hdd
void initATA(void) {
	ATA_0.bus = 0x01f0;
	ATA_1.bus = 0x0170;
	ATA_0.slaveLastUsed = 0x00;
	ATA_1.slaveLastUsed = 0x00;
	ATA_0.notUsed = 0x01;
	ATA_1.notUsed = 0x01;
	Mutex_initUnlocked(&(ATA_0.inUse));
	Mutex_initUnlocked(&(ATA_1.inUse));
	ATA_0m.ata = &ATA_0;
	ATA_0m.slave = 0x00;
	ATA_0s.ata = &ATA_0;
	ATA_0s.slave = 0x01;
	ATA_1m.ata = &ATA_1;
	ATA_1m.slave = 0x00;
	ATA_1s.ata = &ATA_1;
	ATA_1s.slave = 0x01;
}
struct BDSpec ATA_BlockFile = (struct BDSpec) {9,
	ata_writeBlock,
	ata_readBlock,
	256,
	256};
struct BlockFile ATA_0m_file = (struct BlockFile) {&ATA_BlockFile,
	0,
	1024,// TODO Change
	&ATA_0m};
struct BlockFile ATA_0s_file = (struct BlockFile) {&ATA_BlockFile,
	0,
	1024,// TODO Change
	&ATA_0s};
struct BlockFile ATA_1m_file = (struct BlockFile) {&ATA_BlockFile,
	0,
	1024,// TODO Change
	&ATA_1m};
struct BlockFile ATA_1s_file = (struct BlockFile) {&ATA_BlockFile,
	0,
	1024,// TODO Change
	&ATA_1s};
ssize_t ATA_write(int kfd, const void* dat, size_t len) {
	switch (kfd) {
		case (2):
			return Block_write(dat, len, &ATA_0m_file);
		case (3):
			return Block_write(dat, len, &ATA_0s_file);
		case (4):
			return Block_write(dat, len, &ATA_1m_file);
		case (5):
			return Block_write(dat, len, &ATA_1s_file);
		default:
			bugCheckNum(0x0100 | EBADF | FAILMASK_ATA);
	}
	return 0;
}
ssize_t ATA_read(int kfd, void* dat, size_t len) {
	switch (kfd) {
		case (2):
			return Block_read(dat, len, &ATA_0m_file);
		case (3):
			return Block_read(dat, len, &ATA_0s_file);
		case (4):
			return Block_read(dat, len, &ATA_1m_file);
		case (5):
			return Block_read(dat, len, &ATA_1s_file);
		default:
			bugCheckNum(0x0200 | EBADF | FAILMASK_ATA);
	}
	return 0;
}
off_t ATA_lseek(int kfd, off_t off, int how) {
	switch (kfd) {
		case (2):
			return Block_lseek(off, how, &ATA_0m_file);
		case (3):
			return Block_lseek(off, how, &ATA_0s_file);
		case (4):
			return Block_lseek(off, how, &ATA_1m_file);
		case (5):
			return Block_lseek(off, how, &ATA_1s_file);
		default:
			bugCheckNum(0x0300 | EBADF | FAILMASK_ATA);
	}
	return 0;
}
int ATA__llseek(unsigned int kfd, off_t offHi, off_t offLo, loff_t* res, int how) {
	switch (kfd) {
		case (2):
			return Block__llseek(offHi, offLo, res, how, &ATA_0m_file);
		case (3):
			return Block__llseek(offHi, offLo, res, how, &ATA_0s_file);
		case (4):
			return Block__llseek(offHi, offLo, res, how, &ATA_1m_file);
		case (5):
			return Block__llseek(offHi, offLo, res, how, &ATA_1s_file);
		default:
			bugCheckNum(0x0400 | EBADF | FAILMASK_ATA);
	}
	return 0;
}
#include "FileDriver.h"
struct FileDriver ATA_FileDriver = (struct FileDriver) {ATA_write, ATA_read, ATA_lseek, ATA__llseek};
#endif