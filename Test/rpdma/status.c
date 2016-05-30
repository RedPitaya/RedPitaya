#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "rpdma.h"


int main(int argc, char *argv[]) {
        int fd;
        char status;
        fd = open("/dev/rprx", O_RDWR);
        if (fd < 1) { 
                printf(" unable to open device file"); return -1;
        }else{                
                status=0xf0;
                ioctl(fd,STATUS,&status);
                printf("status:%d",(int)status);
        }
        if (fd) close(fd);
    return 0;
}
