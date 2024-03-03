#ifndef __CAPABILITIES_H__
#define __CAPABILITIES_H__ 1
/*
 * Capability numerical definitions as in Linux
 * Definitions: Linux '/include/uapi/linux/capability.h'
 */
#define CAP_SYS_TIME 25
/*
 * Type definitions for the syscalls `capget' and `capset'
 * Definitions: "capget" Linux man-pages page of the section numbered 2
 */
struct cap_header {
	u32 version;
	int thid;
};
struct cap_data {
	u32 effective;
	u32 permitted;
	u32 inheritable;
};
typedef struct cap_header* cap_user_header_t;
typedef struct cap_data* cap_user_data_t;
#endif
