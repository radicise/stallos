#ifndef __MACHINE_X86_64_UTIL_H__
#define __MACHINE_X86_64_UTIL_H__ 1
#include "../../types.h"
AtomicULong uidnatural_overflow = (AtomicULong) 65534;
uidnatural_t uid_to_uidnatural(kuid_t val) {
	return val;
}
#endif
