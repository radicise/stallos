#ifndef __TSFS_H__
#define __TSFS_H__ 1
/*
Anything not explicitly marked as something for use outside of the file system code should NOT be used
Doing so is undefined behavior
- Tristan
*/
<<<<<<< HEAD:os/kernel/filesystems/tsfs.c
#define __STALLOS__ 1
#define printf(...)
=======
#define __STALLOS__
>>>>>>> 8b9a311313279530e93daca3e1dfb0b9e1acf8d3:os/kernel/filesystems/tsfs.h
#include "tsfscore.h"
#include "tsfsmanage.h"
#include "tsfsinterface.h"
#endif
