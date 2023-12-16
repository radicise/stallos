#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#define __FSMOCKTEST
#include "os/kernel/filesystems/tsfs.h"

int main(int argc, char** argv) {
    int fd;
    FILE* fp;
    if ((fd=open("FSMOCKFILE.mock", O_RDWR | O_CREAT)) == -1) {
        goto bottom;
    }
    if ((fp=fdopen(fd, "rb+")) == 0) {
        goto closefd;
    }
    char* contents = "TESTCONTENTS"; 
    fwrite(contents, 1, 12, fp);
    closemock:
    fclose(fp);
    closefd:
    close(fd);
    bottom:
    return 0;
}