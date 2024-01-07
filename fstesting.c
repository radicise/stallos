#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <stdlib.h>
#define __MOCKTEST
#define TARGET_PLATFORM "ARM"
#include "os/kernel/filesystems/tsfscore.h"

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

#define MDISK_SIZE 1024*1024*4

int main(int argc, char** argv) {
    int fd;
    if ((fd=open("FSMOCKFILE.mock", O_RDWR | O_CREAT)) == -1) {
        printf("ERROR\n");
        goto bottom;
    }
    if ((fp=fdopen(fd, "rb+")) == 0) {
        printf("ERROR\n");
        goto closefd;
    }
    // struct FileDriver fdrive = {.write=fdrive_write,.read=fdrive_read,.lseek=fdrive_lseek,.tell=fdrive_tell};
    struct FileDriver fdrive = {.write=fdrive_write,.read=fdrive_read,.lseek=fdrive_lseek,._llseek=fdrive__llseek};
    FileSystem* s = 0;
    if (argc > 3) {
        printf("REGEN\n");
        ftruncate(fd, 0);
        ftruncate(fd, MDISK_SIZE);
        s = createFS(&fdrive, 0, 0, MDISK_SIZE, 10, (u64) time(NULL));
    }
    if (s == 0) {
        s = loadFS(&fdrive, 0, 0);
    }
    if (s->rootblock == 0) {
        printf("ERROR LOADING FS\n");
        free(s);
        goto closemock;
    }
    if (s->rootblock->breakver == 0) {
        printf("BAD CHECKSUM\n");
        goto dealloc;
    }
    printf("psize: %llu\n", s->rootblock->partition_size);
    printf("bsize: %d\n", s->rootblock->block_size);
    printf("creation time: %s", ctime((const time_t*)&(s->rootblock->creation_time)));
    TSFSStructNode sn = {0};
    TSFSDataBlock db  = {0};
    u16 bsize = s->rootblock->block_size;
    sn.data_loc = (u64)(bsize*2);
    db.disk_loc = (u64)(bsize*2);
    printf("seek 1\n");
    longseek(s, (loff_t)bsize, SEEK_SET);
    printf("write struct node\n");
    write_structnode(s, &sn);
    printf("seek 2\n");
    longseek(s, (loff_t)db.disk_loc, SEEK_SET);
    printf("write data block\n");
    write_datablock(s, &db);
    printf("write data\n");
    data_write(s, &sn, 0, "hi", 2);
    printf("done writing\n");
    char x[3];
    printf("readback:\n");
    data_read(s, &sn, 0, x, 2);
    x[2] = 0;
    printf("%s\n", x);
    dealloc:
    free(s->rootblock);
    free(s);
    closemock:
    fclose(fp);
    closefd:
    close(fd);
    bottom:
    return 0;
}