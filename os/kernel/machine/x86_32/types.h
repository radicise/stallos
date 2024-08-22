#ifndef __MACHINE_X86_32_TYPES_H__
#define __MACHINE_X86_32_TYPES_H__ 1
/*
 *
 * Definition of CHAR_BIT
 *
 */
#define CHAR_BIT 8
/*
 *
 * Definitions for fundamental integer types
 *
 */
typedef unsigned char u8;
typedef signed char s8;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned long u32;
typedef signed long s32;
typedef unsigned long long u64;
typedef signed long long s64;
/*
 *
 * Other
 *
 */
typedef unsigned long uintptr;
struct __old_kernel_stat {
	unsigned short st_dev;
	unsigned short st_ino;
	unsigned short st_mode;
	unsigned short st_nlink;
	unsigned short st_uid;
	unsigned short st_gid;
	unsigned short st_rdev;
	unsigned long st_size;
	unsigned long st_atime;
	unsigned long st_mtime;
	unsigned long st_ctime;
};
#endif
