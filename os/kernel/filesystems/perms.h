#ifndef __PERMS_H__
#define __PERMS_H__ 1
#include "./fsdefs.h"

/*
internal representation of permissions
LSB to LSB+2 is RWX in order
*/
typedef mode_t _internal_perm_t;


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
inline _internal_perm_t get_fperms(TSFSDataHeader* dh, uidnatural_t id) {
    char which = (id == dh->oid || id == 0) ? 0 : ((id == dh->gid) ? 1 : 2);
    return (dh->perms & ((which == 0) ? S_IRWXU : ((which == 1) ? S_IRWXG : S_IRWXO))) >> ((which == 0) ? 8 : ((which == 1) ? 4 : 0));
}
inline _internal_perm_t get_dperms(TSFSStructNode* sn, uidnatural_t id) {
    char which = (id == sn->oid || id == 0) ? 0 : ((id == sn->gid) ? 1 : 2);
    return (sn->ikey & ((which == 0) ? S_IRWXU : ((which == 1) ? S_IRWXG : S_IRWXO))) >> ((which == 0) ? 8 : ((which == 1) ? 4 : 0));
}
inline int has_fperm(TSFSDataHeader* dh, uidnatural_t id, int perm) {
    return (get_fperms(dh, id) & perm) != 0;
}
inline int has_dperm(TSFSStructNode* dh, uidnatural_t id, int perm) {
    return (get_dperms(dh, id) & perm) != 0;
}

#endif
