#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <stdlib.h>
#define __MOCKTEST
#define TARGET_PLATFORM "ARM"
#include "os/kernel/filesystems/tsfs.h"

FILE* fp;

ssize_t fdrive_write(int _, const void* data, size_t len) {
    if (len % 16 == 0) {
        return fwrite(data, 16, len / 16, fp);
    } else if (len % 8 == 0) {
        return fwrite(data, 8, len / 8, fp);
    } else {
        return fwrite(data, 1, len, fp);
    }
}
ssize_t fdrive_read(int _, void* data, size_t len) {
    if (len % 16 == 0) {
        return fread(data, 16, len / 16, fp);
    } else if (len % 8 == 0) {
        return fread(data, 8, len / 8, fp);
    } else {
        return fread(data, 1, len, fp);
    }
}
off_t fdrive_lseek(int _, off_t offset, int whence) {
    fseek(fp, offset, whence);
    return ftell(fp);
}
off_t fdrive_tell(int _) {
    return ftell(fp);
}
int fdrive__llseek(int _, off_t offhi, off_t offlo, loff_t* posptr, int whence) {
    fseek(fp, (((long)offhi) << 32) | (long)(offlo), whence);
    (*posptr) = ftell(fp);
    return 0;
}

#undef fsflush
#define fsflush(fs) fflush(fp)

#define MDISK_SIZE 23
// #define MDISK_SIZE 1024*1024*4

int _fstest_sbcs_fe_do(FileSystem* s, TSFSSBChildEntry* ce, void* data) {
    if (ce->flags != TSFS_CF_EXTT) {
        u64 l = tsfs_tell(s);
        block_seek(s, ce->dloc, BSEEK_SET);
        // printf("inip: %llu\nseekp: %llu\n", l, ce->dloc);
        TSFSStructNode sn = {0};
        read_structnode(s, &sn);
        printf("@ %lu : %s\n", ce->dloc, sn.name);
        loc_seek(s, l);
    }
    return 0;
}

int regen_mock(int fd) {
    ftruncate(fd, 0);
    ftruncate(fd, 2<<(MDISK_SIZE-1));
    return 0;
}

int data_test(struct FileDriver* fdrive, int fd, char regen) {
    FileSystem* s = 0;
    if (regen) {
        printf("REGEN\n");
        regen_mock(fd);
        s = createFS(fdrive, 0, MDISK_SIZE, (s64) time(NULL)).retptr;
    }
    if (s == 0) {
        s = loadFS(fdrive, 0).retptr;
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
    u16 bsize = BLOCK_SIZE;
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
        u32 llc = tsfs_resolve_path(s, nbuf);
        printf("disk addr of file node: %lu\n", llc);
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
        u32 llc = tsfs_resolve_path(s, nbuf);
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
        s = createFS(fdrive, 0, MDISK_SIZE, (s64) time(NULL)).retptr;
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

int stringcmp(const char* s1, const char* s2) {
    int c = 0;
    char c1;
    char c2;
    while ((c1 = s1[c]) != 0 && (c2 = s2[c]) != 0) {
        if (c1 != c2) {
            return 0;
        }
        c ++;
    }
    return 1;
}

int fmt_test(struct FileDriver* fdrive, int fd, char f) {
    regen_mock(fd);
    FileSystem* s = createFS(fdrive, fd, MDISK_SIZE, (s64)time(NULL)).retptr;
    releaseFS(s);
    return 0;
}

int struct_test(struct FileDriver* fdrive, int fd, char f) {
    regen_mock(fd);
    FileSystem* s = createFS(fdrive, fd, MDISK_SIZE, (s64)time(NULL)).retptr;
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
        FileSystem* s = createFS(fdrive, 0, MDISK_SIZE, (s64)time(NULL)).retptr;
        puts("FS DONE");
        TSFSStructBlock* blk1 = _tsmagic_block(s);
        puts("BLK1 DONE");
        TSFSStructBlock* blk2 = _tsmagic_block(s);
        puts("BLK2 DONE");
        TSFSStructNode* nod1 = _tsmagic_node(s);
        puts("NOD1 DONE");
        TSFSDataBlock* dat1 = _tsmagic_data(s);
        puts("DAT1 DONE");
        printf("%zu, %zu\n", blk2->magicno, *((size_t*)blk2));
        printf("%zu, %zu\n", nod1->magicno, *((size_t*)nod1));
        printf("%zu, %zu\n", dat1->magicno, *((size_t*)dat1));
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

int harness(char flag, int(*tst)(struct FileDriver*, int, char)) {
    int retc = 0;
    int fd;
    if ((fd=open("FSMOCKFILE.mock", O_RDWR | O_CREAT)) == -1) {
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
        default:
            printf("BAD FLAG\n");
            break;
    }
    return 0;
}

int main(int argc, char** argv) {
    char flags[] = {0, 0};
    for (int i = 1; i < argc; i ++) {
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
        }
    }
    if (flags[0] < 0) {
        generic_test(flags[0], flags[1]);
        return 0;
    }
    switch (flags[0]) {
        case 1:
            harness(flags[1], data_test);
            break;
        case 2:
            harness(flags[1], manip_test);
            break;
        case 3:
            harness(flags[1], fmt_test);
            break;
        case 4:
            harness(flags[1], struct_test);
            break;
        case 5:
            harness(flags[1], magic_test);
            break;
        default:
            printf("bad args\n");
            break;
    }
    return 0;
}
