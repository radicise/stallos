#ifndef __TSFS_H__
#define __TSFS_H__ 1
/*
Anything not explicitly marked as something for use outside of the file system code should NOT be used
Doing so is undefined behavior
- Tristan
*/
#define __STALLOS__ 1
#ifndef TARGETNUM
#define TARGETNUM 2
#endif
#ifndef FST
#define FST 0
#endif
#if FST == 1
#define __FEATURE_NO_MUTEX 1
extern int printf(const char*, ...);
#else
#define printf(...)
#endif
#include "tsfscore.h"
#include "tsfsmanage.h"
#include "tsfsinterface.h"
#endif
