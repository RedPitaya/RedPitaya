

#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#include "common.h"
#include "rpdma.h"

#include "rp_dma.h"

#define STOP_TX     0
#define STOP_RX     1
#define SET_TX      2
#define SET_RX      3
#define START_RX    4
#define START_TX    5


#define SGMNT_CNT 4
#define SGMNT_SIZE 4*1024
#define SIZE SGMNT_CNT*SGMNT_SIZE

int fd;
int status;
char buf[256];
unsigned char* map=NULL;

int rp_DmaOpen(const char *dev, rp_handle_uio_t *handle)
{
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

    int s = munmap (map, SIZE);
    if(s==-1){
        printf("Failed to munmap\n");
    }

    return RP_OK;
}

int rp_DmaCtrl(rp_handle_uio_t *handle, RP_DMA_CTRL ctrl)
{
    switch(ctrl){
        //case RP_DMA_SINGLE:
            //ioctl(fd, 10, 0);  // single RX
        //break;
        case RP_DMA_CYCLIC:
            ioctl(fd, CYCLIC_RX, 0);  // cyclic RX
        break;
        case RP_DMA_STOP_RX:
            ioctl(fd, STOP_RX, 0);
        break;
        default:
            return RP_EOOR;
    }
    return RP_OK;
}

int rp_DmaRead(rp_handle_uio_t *handle)
{
    //for (int b=0; b<1; b++) {
        // blocking read waiting for DMA data
    printf("rp_DmaRead.\n");
    status = read(fd, buf, 1);
    if (status<0) {
      printf("read error\n");
    }
    printf("Read unblocked.\n");
    //}

    rp_DmaCtrl(handle, RP_DMA_STOP_RX);

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

    int s = munmap (map, SIZE);
    if(s==-1){
        printf("Failed to munmap\n");
    }

    return RP_OK;
}

int rp_DmaClose(rp_handle_uio_t *handle)
{
    // release memory
    // status = munmap (map, 4*1024);

    if (fd)
        close(fd);
    return RP_OK;
}
