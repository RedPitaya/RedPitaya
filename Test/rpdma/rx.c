#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include "rpdma.h"

#define SIZE SGMNT_CNT*SGMNT_SIZE

int main(int argc, char *argv[]) {
    int fd;
    int status;
    char buf[256];
    unsigned char* map=NULL;
    int periods=SGMNT_CNT+1;

    if(argc==2)periods=atoi(argv[1]);
    printf("rpdma read test\n");
    printf("SGMNT_CNT  = %u\n", SGMNT_CNT );
    printf("SGMNT_SIZE = %u\n", SGMNT_SIZE);

    // open DMA driver device
    fd = open("/dev/rprx", O_RDWR);
    if (fd < 1) {
        printf("Unable to open device file");
        return -1;
    }

    // allocate data buffer memory
    map = (unsigned char *) mmap(NULL, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (map==NULL) {
        printf("Failed to mmap\n");
        if (fd) close(fd);
        return -1;
    }

    memset(map, 0, SIZE);
    // release memory
    status = munmap (map, SIZE);

    // DMA prepare
//    ioctl(fd, SINGLE_RX, 0);  // single RX
    ioctl(fd, CYCLIC_RX, 0);  // cyclic RX

    for (int b=0; b<periods; b++) {
      // blocking read waiting for DMA data
      status = read(fd, buf, 1);
      if (status<0) {
          printf("read error\n");
      }
      printf("Block %i received.\n", b);
    }

    // allocate data buffer memory
    map = (unsigned char *) mmap(NULL, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (map==NULL) {
        printf("Failed to mmap\n");
        if (fd) close(fd);
        return -1;
    }

    // DMA stop
    ioctl(fd,STOP_RX,0);

    // printout data
    for (int i=0; i<SIZE; i++) {
        if ((i%64)==0 ) printf("@%08x: ", i);
        printf("%02x",(char)map[i]);
        if ((i%64)==63) printf("\n");
    }

    // release memory
    status = munmap (map, SIZE);

    if (fd) close(fd);
    return 0;
}
