#ifndef __EXTERNAL_H__
#define __EXTERNAL_H__ 1

/*
lots of extern statements for interfacing with the FS
*/

#include "./fstypes.h"

typedef struct FSReturn FSRet;

// #ifndef PROCPRINTF
// int printf(const char*__restrict s, ...) {}
// #endif

extern char SEEK_TRACING;
extern void awrite_buf(void*, const void*, _kernel_size_t);
extern int bufcmp(void const*, void const*, int);
extern int tsfs_cmp_name(void const*, void const*);
extern int tsfs_cmp_cename(void const*, void const*);
extern int tsfs_cmp_ce(TSFSSBChildEntry*, TSFSSBChildEntry*);
extern _kernel_u64 tsfs_tell(FileSystem*);
extern int _real_longseek(FileSystem*, _kernel_loff_t, int, long, const char*, const char*);
extern int _real_seek(FileSystem*, _kernel_off_t, int, long, const char*, const char*);
extern int _real_block_seek(FileSystem*, _kernel_s32, char, long, const char*, const char*);
#define seek(fs, offset, whence) _real_seek(fs, offset, whence, __LINE__, __func__, __FILE__)
#define longseek(fs, offset, whence) _real_longseek(fs, offset, whence, __LINE__, __func__, __FILE__)
#define block_seek(fs, offset, abs) _real_block_seek(fs, offset, abs, __LINE__, __func__, __FILE__)
// extern int longseek(FileSystem*, _kernel_loff_t, int);
// extern int seek(FileSystem*, _kernel_off_t, int);
// extern int block_seek(FileSystem*, _kernel_s32, char);
extern _kernel_u32 tsfs_loc_to_block(_kernel_u64);
extern int loc_seek(FileSystem*, _kernel_u64);
extern void read_childentry(FileSystem*, TSFSSBChildEntry*);
extern void write_childentry(FileSystem*, TSFSSBChildEntry*);
extern void tsfs_mk_ce_name(void*, char const*, _kernel_size_t);
extern void read_rootblock(FileSystem*, TSFSRootBlock*);
extern void read_datablock(FileSystem*, TSFSDataBlock*);
extern void read_dataheader(FileSystem*, TSFSDataHeader*);
extern void read_structblock(FileSystem*, TSFSStructBlock*);
extern void read_structnode(FileSystem*, TSFSStructNode*);
extern void write_rootblock(FileSystem*, TSFSRootBlock*);
extern void write_datablock(FileSystem*, TSFSDataBlock*);
extern void write_dataheader(FileSystem*, TSFSDataHeader*);
extern void write_structblock(FileSystem*, TSFSStructBlock*);
extern void write_structnode(FileSystem*, TSFSStructNode*);
extern FSRet createFS(struct FileDriver*, int, _kernel_loff_t);
extern FSRet loadFS(struct FileDriver*, int, _kernel_loff_t);
extern void releaseFS(FileSystem*);
extern _kernel_size_t data_write(FileSystem*, TSFSStructNode*, _kernel_u64, const void*, _kernel_size_t);
extern _kernel_size_t data_read(FileSystem*, TSFSStructNode*, _kernel_u64, void*, _kernel_size_t);
extern int tsfs_exists(FileSystem*, TSFSStructBlock*, char const*);
extern int tsfs_unlink(FileSystem*, TSFSStructNode*);
extern int tsfs_rmdir(FileSystem*, TSFSStructNode*);
extern int tsfs_mk_dir(FileSystem*, TSFSStructNode*, char const*, TSFSStructBlock*);
extern int tsfs_mk_file(FileSystem*, TSFSStructNode*, const char*);
extern _kernel_u32 tsfs_resolve_path(FileSystem*, const char*);
extern int dmanip_fill(FileSystem*, _kernel_u32, _kernel_u32, unsigned char);
extern int dmanip_null(FileSystem*, _kernel_u32, _kernel_u32);
extern int dmanip_shift_right(FileSystem*, _kernel_u32, _kernel_u32, _kernel_u32);
extern int dmanip_shift_left(FileSystem*, _kernel_u32, _kernel_u32, _kernel_u32);
extern int dmanip_null_shift_right(FileSystem*, _kernel_u32, _kernel_u32, _kernel_u32);
extern int dmanip_null_shift_left(FileSystem*, _kernel_u32, _kernel_u32, _kernel_u32);
extern _kernel_u64 tsfs_sbcs_foreach(FileSystem*, TSFSStructBlock*, int(*)(FileSystem*, TSFSSBChildEntry*, void*), void*);
extern TSFSStructBlock* _tsmagic_block(FileSystem*);
extern TSFSStructNode* _tsmagic_node(FileSystem*);
extern TSFSDataBlock* _tsmagic_data(FileSystem*);
extern TSFSDataHeader* _tsmagic_head(FileSystem*);
extern void _tsmagic_deblock(FileSystem*, TSFSStructBlock*);
extern void _tsmagic_denode(FileSystem*, TSFSStructNode*);
extern void _tsmagic_dedata(FileSystem*, TSFSDataBlock*);
extern void _tsmagic_dehead(FileSystem*, TSFSDataHeader*);
extern void tsfs_unload(FileSystem*, void*);
extern void _tsmagic_force_release(FileSystem*, void*);
extern TSFSStructBlock* tsfs_load_block(FileSystem*, _kernel_u32);
extern TSFSStructNode* tsfs_load_node(FileSystem*, _kernel_u32);
extern TSFSDataBlock* tsfs_load_data(FileSystem*, _kernel_u32);
extern TSFSDataHeader* tsfs_load_head(FileSystem*, _kernel_u32);

// macro'd funcs
extern void _magic_smoke(unsigned long, long, const char*, const char*);
extern void __DBG_print_root(TSFSRootBlock*, long, const char*, const char*);
extern void __DBG_print_block(TSFSStructBlock*, long, const char*, const char*);
extern void __DBG_print_node(TSFSStructNode*, long, const char*, const char*);
extern void __DBG_print_head(TSFSDataHeader*, long, const char*, const char*);
extern void __DBG_print_data(TSFSDataBlock*, long, const char*, const char*);
extern void __DBG_print_cename(char const*, long, const char*, const char*);
extern void __DBG_print_child(TSFSSBChildEntry*, long, const char*, const char*);
extern void __DBG_here(long, const char*, const char*);
#define magic_smoke(code) _magic_smoke(code, __LINE__, __FILE__, __func__)
#define _DBG_print_root(rb) __DBG_print_root(rb, __LINE__, __FILE__, __func__)
#define _DBG_print_block(sn) __DBG_print_block(sn, __LINE__, __FILE__, __func__)
#define _DBG_print_node(sn) __DBG_print_node(sn, __LINE__, __FILE__, __func__)
#define _DBG_print_cename(name) __DBG_print_cename(name, __LINE__, __FILE__, __func__)
#define _DBG_print_child(ce) __DBG_print_child(ce, __LINE__, __FILE__, __func__)
#define _DBG_print_head(dh) __DBG_print_head(dh, __LINE__, __FILE__, __func__)
#define _DBG_print_data(db) __DBG_print_data(db, __LINE__, __FILE__, __func__)
#define _DBG_here() __DBG_here(__LINE__, __FILE__, __func__)

#endif
