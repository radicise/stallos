#ifndef __MACHINE_X86_32_UTIL_H__
#define __MACHINE_X86_32_UTIL_H__ 1
typedef volatile unsigned char SimpleMutex;// Not reentrant, acquisition also acts as a memory fence for the thread
typedef volatile unsigned long AtomicULong;
AtomicULong uidnatural_overflow = (AtomicULong) 65534;
unsigned long AtomicULong_get(AtomicULong*);
uidnatural_t kuid_to_uidnatural(kuid_t val) {
	if (val >= 65535) {
		return (uidnatural_t) AtomicULong_get(&uidnatural_overflow);
	}
	return (uidnatural_t) val;
}
#endif
