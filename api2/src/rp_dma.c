
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "rpdma.h"

#include "rp_dma.h"

static int rp_DmaMemSet(rp_handle_uio_t *handle, int value)
{
    // allocate data buffer memory
    unsigned char* map=NULL;
    map = (unsigned char *) mmap(NULL, handle->dma_size, PROT_READ | PROT_WRITE, MAP_SHARED, handle->dma_fd, 0);
    if (map==NULL) {
        printf("Failed to mmap\n");
        if (handle->dma_fd) close(handle->dma_fd);
        return -1;
    }

    // clear buffer
    for (int l=0; l<handle->dma_size; l++)
        map[l] = value;

    if(munmap (map, handle->dma_size)==-1){
        printf("Failed to munmap\n");
    }
    return RP_OK;
}

int rp_DmaOpen(const char *dev, rp_handle_uio_t *handle)
{
    // make a copy of the device path
    handle->dma_dev = (char*) malloc((strlen(dev)+1) * sizeof(char));
    strncpy(handle->dma_dev, dev, strlen(dev)+1);

    // open DMA driver device
    handle->dma_fd = open(handle->dma_dev, O_RDWR);
    if (handle->dma_fd < 1) {
        printf("Unable to open device file");
        return -1;
    }
    // set mem. to zero
    rp_DmaMemSet(handle, 0);

    return RP_OK;
}

int rp_DmaCtrl(rp_handle_uio_t *handle, RP_DMA_CTRL ctrl)
{
    switch(ctrl){
        case RP_DMA_CYCLIC:
            ioctl(handle->dma_fd, CYCLIC_RX, 0);
        break;
        case RP_DMA_STOP_RX:
            ioctl(handle->dma_fd, STOP_RX, 0);
        break;
        default:
            return RP_EOOR;
    }
    return RP_OK;
}

int rp_DmaMemDump(rp_handle_uio_t *handle)
{
    unsigned char* map=NULL;
    // allocate data buffer memory
    map = (unsigned char *) mmap(NULL, handle->dma_size, PROT_READ | PROT_WRITE, MAP_SHARED, handle->dma_fd, 0);
    if (map==NULL) {
        printf("Failed to mmap\n");
        if (handle->dma_fd) {
            close(handle->dma_fd);
        }
        return -1;
    }

    // printout data
    for (int i=0; i<handle->dma_size; i++) {
        if ((i%64)==0 ) printf("@%08x: ", i);
        printf("%02x",(char)map[i]);
        if ((i%64)==63) printf("\n");
    }

    if(munmap (map, handle->dma_size)==-1){
        printf("Failed to munmap\n");
        return -1;
    }
    return RP_OK;
}

int rp_DmaRead(rp_handle_uio_t *handle)
{
    int s = read(handle->dma_fd, NULL, 1);
    if (s<0) {
      printf("read error\n");
      return -1;
    }
    return RP_OK;
}

int rp_DmaClose(rp_handle_uio_t *handle)
{
    if(handle->dma_fd){
        if(close(handle->dma_fd)==-1){
            return -1;
        }
    }

    free(handle->dma_dev);

    return RP_OK;
}
