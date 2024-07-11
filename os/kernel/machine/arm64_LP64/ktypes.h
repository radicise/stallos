#ifndef __MACHINE_ARM64_LP64_KTYPES_H__
#define __MACHINE_ARM64_LP64_KTYPES_H__ 1
/*
 *
 * Definitions for fundamental integer types
 *
 */
typedef unsigned char _kernel_u8;
typedef signed char _kernel_s8;
typedef unsigned short _kernel_u16;
typedef signed short _kernel_s16;
typedef unsigned int _kernel_u32;
typedef signed int _kernel_s32;
typedef unsigned long _kernel_u64;
typedef signed long _kernel_s64;
/*
 *
 * Definitions for Linux kernel-userspace interface types
 *
 */
typedef unsigned long _kernel_size_t;
typedef long _kernel_ssize_t;
typedef int _kernel_pid_t;
typedef long _kernel_time_t;
typedef long long _kernel_time64_t;
typedef long _kernel_off_t;
typedef long long _kernel_loff_t;
typedef unsigned int _kernel_uid32_t;
typedef _kernel_uid32_t _kernel_uidnatural_t;
typedef _kernel_uid32_t _kernel_kuid_t;
typedef unsigned int _kernel_mode_t;
typedef _kernel_u32 _kernel_kdev_t;
typedef _kernel_u16 _kernel_udev_old_t;
typedef _kernel_u32 _kernel_udev_new_t;

// copied from the x86_64 ktypes to make arm64 compile - Tristan
// NOTE: these types are of unknown compatibility with Arm64 architecture
//       their sole purpose here is to make the file system testing code compile for Arm
//       additionally, that code currently makes no use whatsoever of these types, directly or indirectly
typedef volatile unsigned char _kernel_SimpleMutex;
typedef volatile unsigned long _kernel_AtomicULong;
#endif
