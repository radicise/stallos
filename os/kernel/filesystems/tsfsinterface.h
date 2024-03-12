#ifndef __TSFSINTERFACE_H__
#define __TSFSINTERFACE_H__ 1

#include "fsdefs.h"

typedef struct {
    int tsfs_node_id;
    unsigned long long cursor;
} TSFS_FILE;

#endif