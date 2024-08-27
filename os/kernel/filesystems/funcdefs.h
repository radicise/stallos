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
u32 _allocate_blocks(FileSystem*, u8, u16, unsigned long, const char*, const char*);
int _tsfs_free_structure(FileSystem*, u32, unsigned long, const char*, const char*);
int tsfs_free_centered(FSP, u32);
int _tsfs_free_data(FileSystem*, u32, unsigned long, const char*, const char*);
#define tsfs_free_data(fs, block_no) _tsfs_free_data(fs, block_no, __LINE__, __FILE__, __func__)
#define tsfs_free_structure(fs, block_no) _tsfs_free_structure(fs, block_no, __LINE__, __FILE__, __func__)
#define allocate_blocks(fs, area, count) _allocate_blocks(fs, area, count, __LINE__, __FILE__, __func__)
void getcreat_databloc(FileSystem*, u32, u32, TSFSDataBlock*, int*);
size_t data_write(FSP, SNP, u64, const void*, size_t);
size_t data_read(FSP, SNP, u64, void*, size_t);
u32 tsfs_resolve_path(FSP, const char*);
size_t tsfs_strlen(const char*);
int append_datablocks(FileSystem*, SN*, u16);
u32 aquire_itable_slot(FileSystem*, u32);
int release_itable_slot(FileSystem*, u32);
int update_itable_entry(FileSystem*, u32, u32);
u32 resolve_itable_entry(FileSystem*, u32);
char test_dataheader(FSP, u32);
char test_datablock(FSP, u32);
char test_structnode(FSP, u32);
char test_structblock(FSP, u32);
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
