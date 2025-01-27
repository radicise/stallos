#ifndef __TSFS_LOCKS_H__
#define __TSFS_LOCKS_H__ 1

#include "./fsdefs.h"

void tslocks_make(FileSystem* fs) {
    fs->locks = (FSLocks*) allocate(sizeof(FSLocks));
    if (fs->locks == 0) {
        magic_smoke(FEALLOC | FELOCK);
    }
    FSLocks* locks = fs->locks;
    locks->drive_lock = (Mutex*) allocate(sizeof(Mutex));
    if (locks->drive_lock == 0) {
        magic_smoke(FEALLOC | FELOCK);
    }
    locks->ddata_lock = (Mutex*) allocate(sizeof(Mutex));
    if (locks->ddata_lock == 0) {
        magic_smoke(FEALLOC | FELOCK);
    }
    // locks->dcent_lock = (Mutex*) allocate(sizeof(Mutex));
    // if (locks->dcent_lock == 0) {
    //     magic_smoke(FEALLOC | FELOCK);
    // }
    locks->dstct_lock = (Mutex*) allocate(sizeof(Mutex));
    if (locks->dstct_lock == 0) {
        magic_smoke(FEALLOC | FELOCK);
    }
    #ifndef __FEATURE_NO_MUTEX
    Mutex_initUnlocked(locks->drive_lock);
    Mutex_initUnlocked(locks->ddata_lock);
    // Mutex_initUnlocked(locks->dcent_lock);
    Mutex_initUnlocked(locks->dstct_lock);
    #endif
}
void tslocks_release(FileSystem* fs) {
    FSLocks* locks = fs->locks;
    deallocate(locks->drive_lock, sizeof(Mutex));
    deallocate(locks->ddata_lock, sizeof(Mutex));
    // deallocate(locks->dcent_lock, sizeof(Mutex));
    deallocate(locks->dstct_lock, sizeof(Mutex));
    deallocate(fs->locks, sizeof(FSLocks));
}

#endif
