#ifndef __MACHINE_X86_64_UTIL_H__
#define __MACHINE_X86_64_UTIL_H__ 1
typedef volatile unsigned char SimpleMutex;// Not reentrant, acquisition also acts as a memory fence for the thread
typedef volatile unsigned long AtomicULong;
#endif
