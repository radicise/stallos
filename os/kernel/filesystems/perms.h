#ifndef __PERMS_H__
#define __PERMS_H__ 1
#include "./fsdefs.h"

static uidnatural_t userid = 0;
static uidnatural_t grupid = 0;

uidnatural_t get_uid(void) {
    return userid;
}
uidnatural_t get_gid(void) {
    return grupid;
}

void set_uid(uidnatural_t id) {
    userid = id;
}
void set_gid(uidnatural_t id) {
    grupid = id;
}

/*
internal representation of permissions
LSB to LSB+2 is RWX in order
*/
typedef mode_t _internal_perm_t;


void set_fperm(TSFSDataHeader* dh, int perm_flag, int value) {
    u32 p = dh->perms;
    u32 f = (u32)perm_flag;
    u32 v = (u32)value;
    dh->perms = (p & ~f) | (v & f);
}
int get_fperm(TSFSDataHeader* dh, int perm_flag) {
    u32 p = dh->perms;
    u32 f = (u32)perm_flag;
    return (p & f) != 0;
}
void set_dperm(TSFSStructNode* sn, int perm_flag, int value) {
    u32 p = sn->ikey;
    u32 f = (u32)perm_flag;
    u32 v = (u32)value;
    sn->ikey = (p & ~f) | (v & f);
}
int get_dperm(TSFSStructNode* sn, int perm_flag, int value) {
    u32 p = sn->ikey;
    u32 f = (u32)perm_flag;
    return (p & f) != 0;
}
inline _internal_perm_t get_fperms(TSFSDataHeader* dh, kuid_t id) {
    char which = (id == dh->oid || id == 0) ? 0 : ((id == dh->gid) ? 1 : 2);
    return (dh->perms & ((which == 0) ? S_IRWXU : ((which == 1) ? S_IRWXG : S_IRWXO))) >> ((which == 0) ? 8 : ((which == 1) ? 4 : 0));
}
inline _internal_perm_t get_dperms(TSFSStructNode* sn, kuid_t id) {
    char which = (id == sn->oid || id == 0) ? 0 : ((id == sn->gid) ? 1 : 2);
    return (sn->ikey & ((which == 0) ? S_IRWXU : ((which == 1) ? S_IRWXG : S_IRWXO))) >> ((which == 0) ? 8 : ((which == 1) ? 4 : 0));
}
int has_adperm(TSFSStructNode* sn, int perm, kuid_t uid, kuid_t gid, u64 cap) {
    if ((sn->ikey & S_IRWXO) & perm) return 1;
    if (uid == sn->oid) {
        if (((sn->ikey & S_IRWXU) >> 8) & perm) return 1;
    }
    if (gid == sn->gid) {
        if (((sn->ikey & S_IRWXG) >> 8) & perm) return 1;
    }
    if (cap & CAP_DAC_READ_SEARCH) { // DAC_READ_SEARCH grants only RX on directories
        if (perm != 2) return 1;
    }
    if (cap & CAP_DAC_OVERRIDE) {
        if (perm == 4) { // DAC_OVERRIDE can only give execute perms when at least one execute bit is set
            return (sn->ikey & S_IXUSR || sn->ikey & S_IXGRP || sn->ikey & S_IXOTH);
        }
        return 1;
    }
}
int has_fperm(TSFSDataHeader* dh, kuid_t id, int perm) {
    return (get_fperms(dh, id) & perm) != 0;
}
int has_dperm(TSFSStructNode* dh, kuid_t id, int perm) {
    return (get_dperms(dh, id) & perm) != 0;
}

#endif
