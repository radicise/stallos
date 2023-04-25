#include <stdint.h>
typedef uint8_t U8;
typedef int8_t S8;
typedef uint16_t U16;
typedef int16_t S16;
typedef uint32_t U32;
typedef int32_t S32;
typedef struct {
	U8 used;
	U16 perms;
	U16 refCount;
	U32 length;
} Fileblock;
typedef struct {
	U8 used;
	U16 blockLen;
	U16 prev;
	U16 next;
	U16 pad0;
	U16 pad1;
	U16 pad2;
	U16 pad3;
	U8 data;
} Datablock;
typedef struct {
	U8 name;
	U8 pad0;
	U8 pad1;
	U8 pad2;
	U8 pad3;
	U8 pad4;
	U8 pad5;
	U8 pad6;
	U8 pad7;
	U8 pad8;
	U8 pad9;
	U8 pad10;
	U8 pad11;
	U8 pad12;
	U8 pad13;
	U8 pad14;
	U8 pad15;
	U8 pad16;
	U8 pad17;
	U8 pad18;
	U8 pad19;
	U8 pad20;
	U8 pad21;
	U8 pad22;
	U8 pad23;
	U8 pad24;
	U8 pad25;
	U8 pad26;
	U8 pad27;
	U8 pad28;
	U8 pad29;
	U8 pad30;
	U8 type;
	U16 bNum;
} Dirsect;
typedef struct{
	U8 used;
	U16 perms;
	U16 numChildren;
	U16 prev;
	U16 next;
	U16 pad0;
	U16 pad1;
	U16 pad2;
	Dirsect entries;
} Dirblock;
typedef struct {
	U8 used;
	U8 blockLen;
	U16 sizeBlocks;
	U16 root;
} FSDef;
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
char* buf;
size_t bs;
Fileblock* bf;
Datablock* bu;
FILE* stream;
int ents;
U16 blockNum;
size_t dataSpread;
int add_dir(DIR* dir, int dirfd) {
	struct dirent* ent;
	int curEnt = 0;
	char b[bs];
	memset(b 0, (size_t) bs);
	Dirblock* bd = b;// TODO maybe avoid the pointer
	bd->used = 1;
	bd->perms = 0x1ff;// TODO actually do
	Dirsect* entry = &(bd->entries);
	struct stat state;
	int fd;
	for (ent = readdir(dir); ent != NULL; ent = readdir(dir)) {
		if ((fd = openat(dirfd, ent.d_name)) == (-1)) {
			return errno;
		}
		if (fstatat(dirfd, ent.d_name, &state, AT_SYMLINK_NOFOLLOW) == (-1)) {
			return errno;
		}
		if (S_ISDIR(state.st_mode)) {
			// TODO do
		}
		else if (S_ISREG(state.st_mode)) {
			bf->used = 1;
			bf->perms = 0x1ff;// TODO actually do
			bf->refCount = 1;
			if (state.st_size > 0xffff) {
				return 1;// TODO error file too long
			}
			off_t pos;
			bf->length = pos = state.st_size;
			add_block(buf);
			for (; pos >= 0; pos -= dataSpread) {
				bu->used = 1;
				bu->blockLen = ((pos > dataSpread) ? dataSpread : pos);
				bu->prev = last;
				if (pos == state.st_size) {
					bu->prev = 0;
				}
				else {
					bu->prev = blockNum;
				}
				if (pos < dataSpread) {
					bu->next = 0;
				}
				else {
					bu->next = blockNum + 2;
				}
				&(bd->data);			
				
				add_block(buf);
		}
		curEnt++;
		if (curEnt == 0) {
			// TODO exit: too many files
		}
		if ((curEnt % ents) == 0) {
			// TODO new block
		}
		else {
			entry = entry[1];
		}
	}
	// TODO flush
	return 0;
}
int add_block(char* b) {
	if (fwrite(b, (size_t) 1, bs, stream) != bs) {
		return EIO;
	}
	memset(b, 0, bs);
	blockNum++;
	return 0;
}
int main(int argc, char** argv) {
	if (argc < 3) {
		return EINVAL;
	}
	bs = 2;
	bs *= 512;
	ents = (bs - 16) / 48;
	dataSpread = bs - 16;
	blockNum = 0;
	FILE* dirf = fopen(argv[1]);
	if (dirf == NULL) {
		return errno;
	}
	int dirfd = fileno(dirf);
	if (dirfd == (-1)) {
		return errno;
	}
	DIR* dir = fdopendir(dirfd);
	if (dir == NULL) {
		return errno;
	}
	stream = fopen(argv[2], "w");
	if (stream == NULL) {
		return errno;
	}
	buf = malloc(bs);
	if (buf == NULL) {
		return ENOMEM;
	}
	bd = bf = buf;
	memset(buf, 0, bs);
	//TODO filesystem header, use add_block
	int v = add_dir(dir, dirf);
	fclose(dirf);
	exit(v);
}
