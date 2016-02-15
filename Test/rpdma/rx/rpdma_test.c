#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <pthread.h>

#include <unistd.h>
#include <time.h>

#include "rpdma.h"

#define SIZE SEGMENT_CNT*SGMNT_SIZE

int main(int argc, char *argv[]) {
    int fd;
    int status;
    char buf[256];
    unsigned char* map=NULL;

    printf("rpdma read test\n");

    // open DMA driver device
    fd = open("/dev/rpdma", O_RDWR);
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

    // clear buffer
    for (int l=0; l<SIZE; l++)
        map[l] = 0;

    // release memory
    status = munmap (map, 4*1024);

    // DMA prepare
    ioctl(fd, SINGLE_RX, 0);  // single RX
//    ioctl(fd, CYCLIC_RX, 0);  // cyclic RX

    for (int b=0; b<1; b++) {
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

    // printout data
    for (int i=0; i<SIZE; i++) {
        if ((i%64)==0 ) printf("@%08x: ", i);
        printf("%02x",(char)map[i]);
        if ((i%64)==63) printf("\n");
    }

    // release memory
    status = munmap (map, 4*1024);

    // DMA stop
    ioctl(fd,STOP_RX,0);

    if (fd) close(fd);
    return 0;
}
