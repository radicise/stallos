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
// #define printf(...)
extern int printf(const char*, ...);
#include "tsfscore.h"
#include "tsfsmanage.h"
#include "tsfsinterface.h"
#endif
