#ifndef __FUNCDEFS_H__
#define __FUNCDEFS_H__ 1
#include "./fsdefs.h"
#include "../fsiface.h"
typedef struct FSReturn FSRet;
#define FSP FileSystem*
#define DBP TSFSDataBlock*
#define SNP TSFSStructNode*
#define SBP TSFSStructBlock*
#define DHP TSFSDataHeader*
#define CEP TSFSSBChildEntry*
#define DB TSFSDataBlock
#define SN TSFSStructNode
#define SB TSFSStructBlock
#define DH TSFSDataHeader
#define CE TSFSSBChildEntry
FSRet createFS(struct FileDriver*, int, loff_t);
void releaseFS(FSP);
FSRet loadFS(struct FileDriver*, int, loff_t);
u32 aquire_itable_slot(FSP, u32);
int release_itable_slot(FSP, u32);
u32 allocate_blocks(FSP, u8, u16);
int tsfs_free_structure(FSP, u32);
int tsfs_free_centered(FSP, u32);
int tsfs_free_data(FSP, u32);
void getcreat_databloc(FileSystem*, u32, u32, TSFSDataBlock*, int*);
size_t data_write(FSP, SNP, u64, const void*, size_t);
size_t data_read(FSP, SNP, u64, void*, size_t);
u32 tsfs_resolve_path(FSP, const char*);
size_t tsfs_strlen(const char*);
#undef FSP
#undef DBP
#undef SNP
#undef SBP
#undef DHP
#undef CEP
#undef DB
#undef SN
#undef SB
#undef DH
#undef CE
#endif
