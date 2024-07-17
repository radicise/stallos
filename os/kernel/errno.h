#ifndef __ERRNO_H__
#define __ERRNO_H__ 1
/*
 *
 * Definitions of `errno' values as specified by the Linux kernel-userspace interface
 *
 */
#ifndef TARGETNUM
#error "'TARGETNUM' is not set"
#endif
#if TARGETNUM == 1
#include "machine/x86_32/errno.h"
#elif TARGETNUM == 2
#include "machine/x86_64/errno.h"
#else
#error "Target is not supported"
#endif
#endif
