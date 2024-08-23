#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <stdlib.h>
#define FSEXTERNAL 1
#define PROCPRINTF 1
// #define TARGET_PLATFORM "ARM"
// #include "os/kernel/filesystems/tsfs.h"
#include "os/kernel/filesystems/fstypes.h"
#include "os/kernel/FileDriver.h"
#include "os/kernel/filesystems/fsmacros.h"
#include "os/kernel/filesystems/external.h"
#include "./fstutils/input.h"

// #define MDISK_SIZE 21
#define MDISK_SIZE 1024*1024*4
// #define MDISK_SIZE 4096

void kernelWarnMsg(const char* msg) {
    puts(msg);
}
void kernelMsg(const char* msg) {
    puts(msg);
}
void kernelMsgULong_hex(unsigned long l) {
    printf("%lu", l);
}

void bugCheckNum(long n) {
    printf("BUGCHK: %li\n", n);
    exit(1);
}

void* alloc(_kernel_size_t size) {
    return malloc(size);
}
void dealloc(void* ptr, _kernel_size_t size) {
    free(ptr);
}

FILE* fp;
char* bigdata[1024];

_kernel_off_t fdrive_tell(int _) {
    return ftell(fp);
}
_kernel_ssize_t fdrive_write(int _, const void* data, _kernel_size_t len) {
    if (fdrive_tell(0) + len > MDISK_SIZE) {
        printf("BOUNDRY EXCEEDED\n");
        exit(2);
        return 0;
    }
    if (len % 16 == 0) {
        return fwrite(data, 16, len / 16, fp);
    } else if (len % 8 == 0) {
        return fwrite(data, 8, len / 8, fp);
    } else {
        return fwrite(data, 1, len, fp);
    }
}
_kernel_ssize_t fdrive_read(int _, void* data, _kernel_size_t len) {
    if (len % 16 == 0) {
        return fread(data, 16, len / 16, fp);
    } else if (len % 8 == 0) {
        return fread(data, 8, len / 8, fp);
    } else {
        return fread(data, 1, len, fp);
    }
}
_kernel_off_t fdrive_lseek(int _, _kernel_off_t offset, int whence) {
    fseek(fp, offset, whence);
    return ftell(fp);
}
int fdrive__llseek(int _, _kernel_off_t offhi, _kernel_off_t offlo, _kernel_loff_t* posptr, int whence) {
    fseek(fp, (((long)offhi) << 32) | (long)(offlo), whence);
    (*posptr) = ftell(fp);
    return 0;
}

#undef fsflush
#define fsflush(fs) fflush(fp)

int _fstest_sbcs_fe_do(FileSystem* s, TSFSSBChildEntry* ce, void* data) {
    if (ce->flags != TSFS_CF_EXTT) {
        _kernel_u64 l = tsfs_tell(s);
        block_seek(s, ce->dloc, BSEEK_SET);
        // printf("inip: %llu\nseekp: %llu\n", l, ce->dloc);
        TSFSStructNode sn = {0};
        read_structnode(s, &sn);
        char tc = 'i';
        if (sn.storage_flags == TSFS_KIND_DIR) {
            tc = 'd';
        } else if (sn.storage_flags == TSFS_KIND_FILE) {
            tc = 'f';
        } else if (sn.storage_flags == TSFS_KIND_HARD) {
            tc = 'h';
        } else if (sn.storage_flags == TSFS_KIND_LINK) {
            tc = 'l';
        } else {
            tc = 'i';
        }
        printf("@ %u : -%c- %s\n", ce->dloc, tc, sn.name);
        loc_seek(s, l);
    }
    return 0;
}

int regen_mock(int fd) {
    ftruncate(fd, 0);
    ftruncate(fd, MDISK_SIZE);
    return 0;
}
_kernel_time_t fetch_time(_kernel_time_t* t) {
    return (_kernel_time_t)time((time_t*)t);
}
int data_test(struct FileDriver* fdrive, int fd, char regen) {
    FileSystem* s = 0;
    if (regen) {
        printf("REGEN\n");
        regen_mock(fd);
        s = (createFS(fdrive, 0, MDISK_SIZE)).retptr;
    }
    if (s == 0) {
        s = (loadFS(fdrive, 0, MDISK_SIZE)).retptr;
    }
    if (s->rootblock == 0) {
        printf("ERROR LOADING FS\n");
        free(s);
        return 0;
    }
    if (s->rootblock->breakver == 0) {
        printf("BAD CHECKSUM\n");
        goto dealloc;
    }
    printf("psize (blocks): %lu\n", 2lu<<(s->rootblock->partition_size - 1));
    printf("bsize: %d\n", BLOCK_SIZE);
    printf("creation time: %s", ctime((const time_t*)&(s->rootblock->creation_time)));
    TSFSStructNode sn;
    _kernel_u16 bsize = BLOCK_SIZE;
    char nbuf[14];
    awrite_buf(nbuf, "/testfile.txt", 14);
    if (regen) {
        block_seek(s, s->rootblock->top_dir, BSEEK_SET);
        TSFSStructNode topdir = {0};
        read_structnode(s, &topdir);
        _DBG_print_node(&topdir);
        tsfs_mk_file(s, &topdir, "testfile.txt");
        TSFSStructBlock* sb = tsfs_load_block(s, topdir.parent_loc);
        _DBG_print_block(sb);
        tsfs_sbcs_foreach(s, sb, _fstest_sbcs_fe_do, 0);
        tsfs_unload(s, sb);
        _kernel_u32 llc = tsfs_resolve_path(s, nbuf);
        printf("disk addr of file node: %u\n", llc);
        if (llc > 7) {
            printf("BAD LLC\n");
            goto dealloc;
        }
        if (llc == 0) {
            printf("COULDN'T FIND FILE AGAIN\n");
            goto dealloc;
        }
        block_seek(s, llc, BSEEK_SET);
        read_structnode(s, &sn);
        // _tsfs_make_data(s, &sn);
        printf("write data: ");
        char sequence[11];
        int c = 0;
        while (c < 10) {
            char s = (char) getchar();
            if (s == 10) {
                break;
            }
            sequence[c] = s;
            c ++;
        }
        char v = (char) c;
        if (data_write(s, &sn, 0, &v, 1) == 0) {
            printf("WRITE SIZE FAILURE\n");
            goto dealloc;
        }
        if (data_write(s, &sn, 1, sequence, c) < c) {
            printf("WRITE DATA FAILURE\n");
            goto dealloc;
        }
        printf("done writing\n");
    } else {
        // longseek(s, (loff_t)(bsize*3), SEEK_SET);
        // read_structnode(s, &sn);
        _kernel_u32 llc = tsfs_resolve_path(s, nbuf);
        if (llc == 0) {
            printf("COULDN'T FIND FILE AGAIN\n");
            goto dealloc;
        }
        block_seek(s, llc, BSEEK_SET);
        read_structnode(s, &sn);
    }
    char x[11];
    printf("sequence length:\n");
    char l = 0;
    data_read(s, &sn, 0, &l, 1);
    printf("%d\n", l);
    printf("readback sequence\n");
    data_read(s, &sn, 1, x, l);
    x[l] = 0;
    printf("print readback:\n");
    printf("%s\n", x);
    dealloc:
    free(s->rootblock);
    free(s);
    return 0;
}

int manip_test(struct FileDriver* fdrive, int fd, char kind) {
    if (kind == 0) {
        printf("bad test kind\n");
        return 0;
    }
    if (kind == 1) {
        regen_mock(fd);
        FileSystem* s = 0;
        s = (createFS(fdrive, 0, MDISK_SIZE)).retptr;
        dmanip_fill(s, 0, 2, 97);
        dmanip_null_shift_right(s, 0, 2, 2);
        fflush(fp);
        getchar();
        dmanip_null_shift_left(s, 2, 2, 1);
        free(s->rootblock);
        free(s);
    }
    return 0;
}

int fmt_test(struct FileDriver* fdrive, int fd, char f) {
    regen_mock(fd);
    FileSystem* s = (createFS(fdrive, fd, MDISK_SIZE)).retptr;
    releaseFS(s);
    return 0;
}

int struct_test(struct FileDriver* fdrive, int fd, char f) {
    regen_mock(fd);
    FileSystem* s = (createFS(fdrive, fd, MDISK_SIZE)).retptr;
    // TSFSStructBlock sb = {0};
    block_seek(s, s->rootblock->top_dir, BSEEK_SET);
    TSFSStructNode topdir = {0};
    read_structnode(s, &topdir);
    // u64 x = topdir.parent_loc;
    // longseek(s, x, SEEK_SET);
    // read_structblock(s, &sb);
    TSFSStructBlock* sb = tsfs_load_block(s, topdir.parent_loc);
    TSFSStructBlock* osb = sb;
    printf("OPTR: %p\n", (void*)sb);
    tsfs_sbcs_foreach(s, sb, _fstest_sbcs_fe_do, 0);
    TSFSStructBlock sb2 = {0};
    tsfs_mk_dir(s, &topdir, "hi", &sb2);
    tsfs_mk_dir(s, &topdir, "my", &sb2);
    tsfs_mk_dir(s, &topdir, "guy", &sb2);
    tsfs_mk_dir(s, &topdir, "bye", &sb2);
    // loc_seek(s, topdir.parent_loc);
    tsfs_sbcs_foreach(s, sb, _fstest_sbcs_fe_do, 0);
    // read_structblock(s, &sb);
    _DBG_print_block(sb);
    printf("BEFORE DEL\n");
    getchar();
    // printf("BEFORE LOAD\n");
    TSFSStructNode* sn = tsfs_load_node(s, sb2.disk_ref);
    // printf("BEFORE RM\n");
    tsfs_rmdir(s, sn);
    // printf("AFTER RM\n");
    fsflush(s);
    getchar();
    printf("SBPTRO = %p\nSBPTRC = %p\n", (void*)osb, (void*)sb);
    _DBG_print_block(sb);
    tsfs_sbcs_foreach(s, sb, _fstest_sbcs_fe_do, 0);
    tsfs_unload(s, sb);
    rel:
    releaseFS(s);
    return 0;
}

int magic_test(struct FileDriver* fdrive, int kf, char flags) {
    printf("MAGIC\n");
    if (flags == 1) {
        magic_smoke(FEWAND | FEALLOC);
    } else if (flags == 2) {
        puts("BEFORE");
        FileSystem* s = (createFS(fdrive, 0, MDISK_SIZE)).retptr;
        puts("FS DONE");
        TSFSStructBlock* blk1 = _tsmagic_block(s);
        puts("BLK1 DONE");
        TSFSStructBlock* blk2 = _tsmagic_block(s);
        puts("BLK2 DONE");
        TSFSStructNode* nod1 = _tsmagic_node(s);
        puts("NOD1 DONE");
        TSFSDataBlock* dat1 = _tsmagic_data(s);
        puts("DAT1 DONE");
        printf("%zu, %zu\n", blk2->magicno, *((_kernel_size_t*)blk2));
        printf("%zu, %zu\n", nod1->magicno, *((_kernel_size_t*)nod1));
        printf("%zu, %zu\n", dat1->magicno, *((_kernel_size_t*)dat1));
        puts("PRINT DONE");
        _tsmagic_deblock(s, blk1);
        puts("DEB1 DONE");
        _tsmagic_deblock(s, blk2);
        puts("DEB2 DONE");
        _tsmagic_denode(s, nod1);
        puts("DEN1 DONE");
        _tsmagic_dedata(s, dat1);
        puts("DED1 DONE");
    }
    return 0;
}

int list_test(struct FileDriver* fdrive, int fd, char f) {
    FileSystem* s = (loadFS(fdrive, fd, MDISK_SIZE)).retptr;
    // TSFSStructBlock sb = {0};
    block_seek(s, s->rootblock->top_dir, BSEEK_SET);
    TSFSStructNode topdir = {0};
    read_structnode(s, &topdir);
    // u64 x = topdir.parent_loc;
    // longseek(s, x, SEEK_SET);
    // read_structblock(s, &sb);
    TSFSStructBlock* sb = tsfs_load_block(s, topdir.parent_loc);
    TSFSStructNode sd = {0};
    block_seek(s, tsfs_resolve_path(s, "/happy"), BSEEK_SET);
    read_structnode(s, &sd);
    TSFSStructBlock* sb2 = tsfs_load_block(s, sd.parent_loc);
    printf("OPTR: %p\n", (void*)sb);
    printf("OPTR2: %p\n", (void*)sb2);
    tsfs_sbcs_foreach(s, sb, _fstest_sbcs_fe_do, 0);
    tsfs_sbcs_foreach(s, sb2, _fstest_sbcs_fe_do, 0);
    tsfs_unload(s, sb);
    tsfs_unload(s, sb2);
    rel:
    releaseFS(s);
    return 0;
}

extern int tsfs_free_data(FileSystem*, _kernel_u32);

int delete_test(struct FileDriver* fdrive, int fd, char f) {
    regen_mock(fd);
    FileSystem* s = (createFS(fdrive, fd, MDISK_SIZE)).retptr;
    _DBG_print_root(s->rootblock);
    int dfd = open("bigdata.txt", O_RDONLY);
    if (dfd == -1) {
        goto rel;
    }
    read(dfd, bigdata, 1024);
    close(dfd);
    // TSFSStructBlock sb = {0};
    block_seek(s, s->rootblock->top_dir, BSEEK_SET);
    TSFSStructNode topdir = {0};
    read_structnode(s, &topdir);
    tsfs_mk_file(s, &topdir, "test");
    tsfs_mk_file(s, &topdir, "ctrl");
    _kernel_u32 tblkno = tsfs_resolve_path(s, "/test");
    printf("FILE BLOCK NO: %lu\n", tblkno);
    TSFSStructNode* fil = tsfs_load_node(s, tblkno);
    _DBG_print_node(fil);
    for (int i = 0; i < 8; i ++) {
        data_write(s, fil, 1024*i, bigdata, 1024);
    }
    _kernel_u32 p = resolve_itable_entry(s, fil->ikey);
    printf("DLOC: %lx\n", p);
    block_seek(s, p, BSEEK_SET);
    TSFSDataHeader dh = {0};
    read_dataheader(s, &dh);
    _DBG_print_head(&dh);
    // tsfs_unload(s, fil);
    // goto rel;
    // tsfs_unload(s, fil);
    // tsfs_free_data(s, p);
    tsfs_unlink(s, fil);
    // _DBG_print_root(s->rootblock);
    // block_seek(s, 0, BSEEK_SET);
    // TSFSRootBlock rbt = {0};
    // read_rootblock(s, &rbt);
    // _DBG_print_root(&rbt);
    rel:
    releaseFS(s);
    return 0;
}
#define TSFSDBLOCKDSIZE 19
int full_test(struct FileDriver* fdrive, int fd, char flags) {
    FileSystem* s = 0;
    SEEK_TRACING = 0;
    if (flags) {
        printf("REGEN\n");
        regen_mock(fd);
        s = (createFS(fdrive, 0, MDISK_SIZE)).retptr;
    }
    if (s == 0) {
        s = (loadFS(fdrive, 0, MDISK_SIZE)).retptr;
    }
    if (s == 0) {
        printf("ERROR LOADING FS\n");
        return 0;
    }
    if (s->rootblock == 0) {
        printf("ERROR LOADING FS\n");
        free(s);
        return 0;
    }
    if (s->rootblock->breakver == 0) {
        printf("BAD CHECKSUM\n");
        goto dealloc;
    }
    printf("FS CREATE/LOAD OK\nSTARTING CLI\n");
    char* cwd = (char*)malloc(2);
    cwd[0] = '/';
    cwd[1] = 0;
    while (1) {
        CLIData clidata = clihelper();
        int clicode = clidata.clicode;
        if (clicode < 0) {
            printf("ERROR: %d\n", clicode);
            break;
        }
        if (clicode == 0) {
            break;
        } else if (clicode == 1) {
            cwd = strmove(clidata.data);
            free(clidata.data);
        } else if (clicode < 4) {
            char* name = strmove(clidata.data);
            free(clidata.data);
            TSFSStructNode* par = tsfs_load_node(s, tsfs_resolve_path(s, cwd));
            if (clicode == 2) {
                TSFSStructBlock blk = {0};
                tsfs_mk_dir(s, par, name, &blk);
            } else {
                tsfs_mk_file(s, par, name);
            }
            tsfs_unload(s, par);
        } else if (clicode == 4) {
            TSFSStructNode dir = {0};
            block_seek(s, tsfs_resolve_path(s, cwd), BSEEK_SET);
            read_structnode(s, &dir);
            TSFSStructBlock* sb = tsfs_load_block(s, dir.parent_loc);
            tsfs_sbcs_foreach(s, sb, _fstest_sbcs_fe_do, 0);
            tsfs_unload(s, sb);
        } else if (clicode == 5) {
            SEEK_TRACING ^= 1; // toggle seek tracing
        } else if (clicode == 6) {
            TSFSStructNode dir = {0};
            char* fullpath = stringcmp(clidata.data, ".") ? strmove(cwd) : strjoin(cwd, clidata.data);
            free(clidata.data);
            _kernel_u32 tarblock = tsfs_resolve_path(s, fullpath);
            free(fullpath);
            if (tarblock == 0) {
                printf("BAD NAME\n");
                continue;
            }
            block_seek(s, tarblock, BSEEK_SET);
            read_structnode(s, &dir);
            _DBG_print_node(&dir);
            if (dir.storage_flags == TSFS_KIND_FILE) {
                TSFSDataHeader head = {0};
                block_seek(s, resolve_itable_entry(s, dir.ikey), BSEEK_SET);
                read_dataheader(s, &head);
                _DBG_print_head(&head);
            }
        } else if (clicode == 7) {
            TSFSStructNode dir = {0};
            char* fullpath = stringcmp(clidata.data, ".") ? strmove(cwd) : strjoin(cwd, clidata.data);
            free(clidata.data);
            _kernel_u32 tarblock = tsfs_resolve_path(s, fullpath);
            free(fullpath);
            if (tarblock == 0) {
                printf("BAD NAME\n");
                continue;
            }
            block_seek(s, tarblock, BSEEK_SET);
            read_structnode(s, &dir);
            _DBG_print_node(&dir);
            if (dir.storage_flags == TSFS_KIND_FILE) {
                TSFSDataHeader head = {0};
                block_seek(s, resolve_itable_entry(s, dir.ikey), BSEEK_SET);
                read_dataheader(s, &head);
                _DBG_print_head(&head);
                TSFSDataBlock cdb = {0};
                block_seek(s, head.head, BSEEK_SET);
                read_datablock(s, &cdb);
                _DBG_print_data(&cdb);
                while (cdb.next_block) {
                    block_seek(s, cdb.next_block, BSEEK_SET);
                    read_datablock(s, &cdb);
                    _DBG_print_data(&cdb);
                }
            }
        } else if (clicode < 10) {
            TSFSStructNode dir = {0};
            char* name = strmove(clidata.data);
            char* fullpath = strjoin(cwd, clidata.data);
            free(clidata.data);
            _kernel_u32 tarblock = tsfs_resolve_path(s, fullpath);
            free(fullpath);
            if (tarblock == 0) {
                printf("BAD NAME\n");
                continue;
            }
            block_seek(s, tarblock, BSEEK_SET);
            read_structnode(s, &dir);
            _DBG_print_node(&dir);
            if (dir.storage_flags != TSFS_KIND_FILE) {
                printf("BAD NODE TYPE\n");
                continue;
            }
            TSFSDataHeader head = {0};
            block_seek(s, resolve_itable_entry(s, dir.ikey), BSEEK_SET);
            read_dataheader(s, &head);
            TSFSDataBlock cdb = {0};
            block_seek(s, head.head, BSEEK_SET);
            read_datablock(s, &cdb);
            char* dumpdata = (char*)malloc(1006);
            if (dumpdata == NULL) {
                printf("MALLOC FAILED");
                break;
            }
            for (int i = 0; i < head.blocks; i ++) {
                block_seek(s, cdb.disk_loc, BSEEK_SET);
                seek(s, TSFSDBLOCKDSIZE, SEEK_CUR);
                read_buf(s, dumpdata, cdb.data_length);
                dumpdata[cdb.data_length] = 0;
                if (clicode == 8) {
                    // text dump
                    printf("TXT DATA @ {%lx}\n%s\nLEN: %u\n", cdb.disk_loc, dumpdata, stringlen(dumpdata));
                } else {
                    // binary dump
                    printf("BIN DATA @ {%lx}\n", cdb.disk_loc);
                    for (int j = 1; j < cdb.data_length+1; j ++) {
                        printf("%x ", dumpdata[j-1]);
                        if (j % 8 == 0) {
                            printf("\n");
                        }
                    }
                    if (cdb.data_length % 8) {
                        printf("\n");
                    }
                }
                if (cdb.next_block != 0) {
                    block_seek(s, cdb.next_block, BSEEK_SET);
                    read_datablock(s, &cdb);
                }
            }
            free(dumpdata);
        } else if (clicode == 10) {
            char* ikeytxt = strmove(clidata.data);
            free(clidata.data);
            unsigned long ikey = parse_ulonghex(ikeytxt);
            free(ikeytxt);
            printf("IKEY: %lx\nIKEYRES: %lx\n", ikey, resolve_itable_entry(s, ikey));
        } else if (clicode == 11) {
            DMAN_TRACING ^= 1; // toggle disk manip tracing
        } else if (clicode == 12) {
            char* path = strjoin(cwd, clidata.data);
            free(clidata.data);
            _kernel_u32 blockno = tsfs_resolve_path(s, path);
            free(path);
            if (blockno == 0) {
                printf("BAD NAME\n");
                continue;
            }
            TSFSStructNode* dnode = tsfs_load_node(s, blockno);
            if (dnode->storage_flags == TSFS_KIND_FILE) {
                tsfs_unlink(s, dnode);
            } else {
                if (tsfs_rmdir(s, dnode)) {
                    tsfs_unload(s, dnode);
                };
            }
        }
    }
    dealloc:
    free(s->rootblock);
    free(s);
    return 0;
}

int harness(const char* fpath, char flag, int(*tst)(struct FileDriver*, int, char)) {
    int retc = 0;
    int fd;
    printf("%s\n", fpath);
    if ((fd=open(fpath, O_RDWR | O_CREAT)) == -1) {
        printf("ERROR\n");
        goto bottom;
    }
    if ((fp=fdopen(fd, "rb+")) == 0) {
        printf("ERROR\n");
        goto closefd;
    }
    struct FileDriver fdrive = {.write=fdrive_write,.read=fdrive_read,.lseek=fdrive_lseek,._llseek=fdrive__llseek};
    retc = tst(&fdrive, fd, flag);
    closemock:
    fclose(fp);
    closefd:
    close(fd);
    bottom:
    return retc;
}

int generic_test(char f1, char f2) {
    switch (f1) {
        case -1:
            printf("%sRED\n%sGRN\n%sYEL%s\n", TSFS_ANSI_RED, TSFS_ANSI_GRN, TSFS_ANSI_YEL, TSFS_ANSI_NUN);
            break;
        case -2:
            printf("> ");
            char* str = readline();
            if (str == NULL) {
                printf("BAD STR PTR\n");
                break;
            }
            printf("ECHO: %s\n", str);
            free(str);
            break;
        default:
            printf("BAD FLAG\n");
            break;
    }
    return 0;
}

int main(int argc, char** argv) {
    char flags[] = {0, 0};
    char* fpath = argv[1];
    for (int i = 2; i < argc; i ++) {
        if (stringcmp(argv[i], "-regen")) {
            flags[1] = 1;
        } else if (stringcmp(argv[i], "data")) {
            flags[0] = 1;
        } else if (stringcmp(argv[i], "manip")) {
            flags[0] = 2;
        } else if (stringcmp(argv[i], "-shift")) {
            flags[1] = 1;
        } else if (stringcmp(argv[i], "format")) {
            flags[0] = 3;
        } else if (stringcmp(argv[i], "struct")) {
            flags[0] = 4;
        } else if (stringcmp(argv[i], "magic")) {
            flags[0] = 5;
        } else if (stringcmp(argv[i], "-smoke")) {
            flags[1] = 1;
        } else if (stringcmp(argv[i], "-no")) {
            flags[1] = 2;
        } else if (stringcmp(argv[i], "palette")) {
            flags[0] = -1;
        } else if (stringcmp(argv[i], "meta-readline")) {
            flags[0] = -2;
        } else if (stringcmp(argv[i], "full")) {
            flags[0] = 6;
        } else if (stringcmp(argv[i], "list")) {
            flags[0] = 7;
        } else if (stringcmp(argv[i], "delete")) {
            flags[0] = 8;
        } else if (stringcmp(argv[i], "-sktrace")) {
            SEEK_TRACING = 1;
        }
    }
    printf("{%d, %d}\n", flags[0], flags[1]);
    if (flags[0] < 0) {
        generic_test(flags[0], flags[1]);
        return 0;
    }
    // char* fpath = "FSMOCKFILE.mock";
    switch (flags[0]) {
        case 1:
            harness(fpath, flags[1], data_test);
            break;
        case 2:
            harness(fpath, flags[1], manip_test);
            break;
        case 3:
            harness(fpath, flags[1], fmt_test);
            break;
        case 4:
            harness(fpath, flags[1], struct_test);
            break;
        case 5:
            harness(fpath, flags[1], magic_test);
            break;
        case 6:
            harness(fpath, flags[1], full_test);
            break;
        case 7:
            harness(fpath, flags[1], list_test);
            break;
        case 8:
            harness(fpath, flags[1], delete_test);
            break;
        default:
            printf("bad args\n");
            break;
    }
    return 0;
}
