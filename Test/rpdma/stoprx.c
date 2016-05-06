#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <pthread.h>

#include <unistd.h>
#include <time.h>

#include "rpdma.h"


int main(int argc, char *argv[]) {
    int fd;
        fd = open("/dev/rprx", O_RDWR);                                        
        if (fd < 1) { printf(" unable to open device file"); return -1;
        }else{
        // DMA stop
          ioctl(fd,STOP_RX,0);
}
    if (fd) close(fd);
    return 0;
}
