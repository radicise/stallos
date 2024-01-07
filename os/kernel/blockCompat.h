#ifndef __BLOCKCOMPAT_H__
#define __BLOCKCOMPAT_H__ 1
#include "types.h"
#include "FileDriver.h"
#include "perProcess.h"
struct BDSpec {
	u8 bs;
	int (*writeBlock)(unsigned long long, unsigned long long, const void*, void*);/* "block", "amnt", "src", "obj" */
	int (*readBlock)(unsigned long long, unsigned long long, void*, void*);/* "block", "amnt", "dest", "obj" */
	unsigned long long readMax;
	unsigned long long writeMax;
};
struct BlockFile {
	struct BDSpec* reliance;
	loff_t pos;// in address units
	unsigned long long amnt;// in blocks
	void* obj;
};// TODO Utilise `loff_t' appropriately throughout the functions and structures instead of using `unsigned long long'
ssize_t Block_write(const void* data, size_t count, struct BlockFile* file) {
	int (*writeBlock)(unsigned long long, unsigned long long, const void*, void*) = file->reliance->writeBlock;
	int (*readBlock)(unsigned long long, unsigned long long, void*, void*) = file->reliance->readBlock;
	u8 bs = file->reliance->bs;
	unsigned long long readMax = file->reliance->readMax;
	unsigned long long writeMax = file->reliance->writeMax;
	void* obj = file->obj;
	unsigned long long pos = file->pos;
	unsigned long long amnt = file->amnt;
	if (pos >= (amnt << bs)) {
		return 0;
	}
	unsigned long long mask = (((unsigned long long) 0) - 1) >> ((sizeof(unsigned long long) * CHAR_BIT) - bs);
	unsigned char buf[mask + 1];
	if ((pos + count) > (amnt << bs)) {
		count = (amnt << bs) - pos;
	}
	size_t oCount = count;
	if (pos & mask) {
		size_t cCount;
		if (((pos & mask) + count) > (mask + 1)) {
			cCount = mask + 1 - (pos & mask);
		}
		else {
			cCount = count;
		}
		while (readBlock(pos >> bs, 1, buf, obj)) {
		}
		cpy(buf + (pos & mask), data, cCount);
		while (writeBlock(pos >> bs, 1, buf, obj)) {
		}
		if (cCount == count) {
			file->pos += cCount;
			return oCount;
		}
		count -= cCount;
		data = ((const char*) data) + cCount;
		pos += cCount;
	}
	unsigned long long wm = (mask + 1) * writeMax;
	while ((count >> bs) >= writeMax) {
		while (writeBlock(pos >> bs, writeMax, data, obj)) {
		}
		pos += wm;
		count -= wm;
		data = ((const char*) data) + wm;
	}
	if (count >> bs) {
		while (writeBlock(pos >> bs, count >> bs, data, obj)) {
		}
		pos += count & (~mask);
		data = ((const char*) data) + (count & (~mask));
		count &= mask;
	}
	if (!count) {
		file->pos = pos;
		return oCount;
	}
	while (readBlock(pos >> bs, 1, buf, obj)) {
	}
	cpy(buf, data, count);
	while (writeBlock(pos >> bs, 1, buf, obj)) {
	}
	file->pos += oCount;
	return oCount;
}
ssize_t Block_read(void* data, size_t count, struct BlockFile* file) {
	int (*readBlock)(unsigned long long, unsigned long long, void*, void*) = file->reliance->readBlock;
	u8 bs = file->reliance->bs;
	unsigned long long readMax = file->reliance->readMax;
	void* obj = file->obj;
	unsigned long long pos = file->pos;
	unsigned long long amnt = file->amnt;
	if (pos >= (amnt << bs)) {
		return 0;
	}
	unsigned long long mask = (((unsigned long long) 0) - 1) >> ((sizeof(unsigned long long) * CHAR_BIT) - bs);
	unsigned char buf[mask + 1];
	if ((pos + count) > (amnt << bs)) {
		count = (amnt << bs) - pos;
	}
	size_t oCount = count;
	if (pos & mask) {
		size_t cCount;
		if (((pos & mask) + count) > (mask + 1)) {
			cCount = mask + 1 - (pos & mask);
		}
		else {
			cCount = count;
		}
		while (readBlock(pos >> bs, 1, buf, obj)) {
		}
		cpy(data, buf + (pos & mask), cCount);
		if (cCount == count) {
			file->pos += cCount;
			return oCount;
		}
		count -= cCount;
		data = ((char*) data) + cCount;
		pos += cCount;
	}
	unsigned long long wm = (mask + 1) * readMax;
	while ((count >> bs) >= readMax) {
		while (readBlock(pos >> bs, readMax, data, obj)) {
		}
		pos += wm;
		count -= wm;
		data = ((char*) data) + wm;
	}
	if (count >> bs) {
		while (readBlock(pos >> bs, count >> bs, data, obj)) {
		}
		pos += count & (~mask);
		data = ((char*) data) + (count & (~mask));
		count &= mask;
	}
	if (!count) {
		file->pos = pos;
		return oCount;
	}
	while (readBlock(pos >> bs, 1, buf, obj)) {
	}
	cpy(data, buf, count);
	file->pos += oCount;
	return oCount;
}
int Block__llseek(off_t hi, off_t lo, loff_t* res, int how, struct BlockFile* file) {
	loff_t rel = (((loff_t) hi) << (sizeof(off_t) * CHAR_BIT)) | ((loff_t) lo);
	loff_t max = file->amnt << file->reliance->bs;
	switch (how) {
		case (SEEK_END):
			rel += max - file->pos;
		case (SEEK_CUR):
			rel += file->pos;
		case (SEEK_SET):
			if (rel < 0) {
				errno = EINVAL;
				return (-1);
			}
			if (rel > max) {
				errno = EINVAL;
				return (-1);
			}
			(*res) = (file->pos = rel);
			return 0;
		case (SEEK_DATA):
			if (rel >= max) {
				errno = ENXIO;
				return (-1);
			}
			(*res) = (file->pos = rel);
			return 0;
		case (SEEK_HOLE):
			if (rel > max) {
				errno = ENXIO;
				return (-1);
			}
			(*res) = (file->pos = max);
			return 0;
		default:
			errno = EINVAL;
			return (-1);
	}
}
off_t Block_lseek(off_t off, int how, struct BlockFile* file) {
	unsigned long long pos = file->pos;
	loff_t res;
	if (Block__llseek((off < 0) ? (-1) : 0, off, &res, how, file)) {
		return (-1);
	}
	if (res >> (sizeof(off_t) * CHAR_BIT)) {
		file->pos = pos;
		errno = EOVERFLOW;
		return (-1);
	}
	return res;
}
#endif
