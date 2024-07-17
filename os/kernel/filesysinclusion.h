#ifndef __FILESYSINCLUSION_H__
#define __FILESYSINCLUSION_H__ 1
/**
 * does inclusion of whatever file system is being used (in case I implement multiple) - Tristan
 */

#ifdef FSNAME
#if FSNAME == "EXT4"
#include "filesystems/ext4.h"
#endif
#if FSNAME == "TSFS"
#include "filesystems/tsfs.h"
#endif
#endif
#endif
