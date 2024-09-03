#ifndef __PERMS_H__
#define __PERMS_H__ 1
#include "./fsdefs.h"

inline void set_fperm(TSFSDataHeader* dh, int perm_flag, int value) {
    u32 p = dh->perms;
    u32 f = (u32)perm_flag;
    u32 v = (u32)value;
    dh->perms = (p & ~f) | (v & f);
}
inline int get_fperm(TSFSDataHeader* dh, int perm_flag) {
    u32 p = dh->perms;
    u32 f = (u32)perm_flag;
    return (p & f) != 0;
}
inline void set_dperm(TSFSStructNode* sn, int perm_flag, int value) {
    u32 p = sn->ikey;
    u32 f = (u32)perm_flag;
    u32 v = (u32)value;
    sn->ikey = (p & ~f) | (v & f);
}
inline int get_dperm(TSFSStructNode* sn, int perm_flag, int value) {
    u32 p = sn->ikey;
    u32 f = (u32)perm_flag;
    return (p & f) != 0;
}

#endif
