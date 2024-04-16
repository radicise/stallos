#ifndef __MACHINE_X86_64_TYPES_H__
#define __MACHINE_X86_64_TYPES_H__ 1
#define NULL ((void*) 0)
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
typedef unsigned int u32;
typedef signed int s32;
typedef unsigned long u64;
typedef signed long s64;
/*
 *
 * Definitions: Linux '/arch/x86/include/uapi/asm/posix_types_64.h', Linux 'include/linux/kdev_t.h'
 *
 */
typedef unsigned long size_t;
typedef long ssize_t;
typedef int pid_t;
typedef long time_t;
typedef long long time64_t;
typedef long off_t;
typedef long long loff_t;
typedef unsigned int uid32_t;
typedef uid32_t uidnatural_t;
typedef unsigned int mode_t;
typedef u32 kdev_t;
typedef u16 udev_old_t;
typedef u32 udev_new_t;
uidnatural_t uid32_to_uidnatural(uid32_t val) {
	return val;
}
/*
 *
 * Other
 *
 */
typedef unsigned long uintptr;
#endif
