#ifndef __TYPES_X86_32_H__
#define __TYPES_X86_32_H__ 1
/*
 *
 * Definitions for Linux kernel-userspace interface types as in 32-bit i386 Linux
 * Definitions: Linux '/arch/x86/include/uapi/asm/posix_types_32.h'
 *
 */
typedef unsigned int size_t;
typedef int ssize_t;
typedef int pid_t;
typedef long time_t;
typedef long long time64_t;
typedef long off_t;
typedef long long loff_t;
typedef unsigned short uid16_t;// For certain legacy architectures
#define UID_OVERFLOW 65534
typedef unsigned int uid32_t;
typedef unsigned short mode_t;
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
#endif
