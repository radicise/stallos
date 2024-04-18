#ifndef __MACHINE_X86_32_TYPES_H__
#define __MACHINE_X86_32_TYPES_H__ 1
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
