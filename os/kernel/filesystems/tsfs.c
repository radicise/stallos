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
// TODO URGENT: determine what the mask value should actually be
#define FAILMASK_TSFS 0x00170000
// controls whether non-fatal problems cause bugcheck
#ifndef __FS_WARN_LOUD
#define __FS_WARN_LOUD 0
#endif
// allows runtime behavior to change while providing as a compile time option
char _FS_WARN_LOUD = __FS_WARN_LOUD;
#include "tsfscore.h"
#include "tsfsmanage.h"
#include "tsfsinterface.h"

/*

(W) denotes something normally a warning but can trigger bugCheckNum if _FS_WARN_LOUD is set
(E) denotes something that is always fatal
BugCheckNum codes:
0x01 (W) - path normalization anomaly, should be accompanied by kernelWarnMsg calls containing the arguments to the normalization function
0x02 (E) - path normalization anomaly, unrecoverable as semantics cannot be upheld

*/

#endif
