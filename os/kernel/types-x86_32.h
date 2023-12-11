#ifndef __TYPES_x86_32_H__
#define __TYPES_x86_32_H__ 1
/*
 *
 * Definitions for POSIX.1 types as in 32-bit i386 Linux
 * Source: '/arch/x86/include/uapi/asm/posix_types_32.h'
 *
 */
typedef unsigned int size_t;
typedef int ssize_t;
typedef int pid_t;
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
#endif
