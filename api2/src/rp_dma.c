
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

#define RP_SGMNT_CNT 4
#define RP_SGMNT_SIZE (4*1024)

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

    switch(handle->mem_type){
        case RP_MEM_DEV_DMA:
                // TODO: check for max. memory size..
                handle->dma_size=RP_SGMNT_CNT*RP_SGMNT_SIZE;
                rp_SetSgmntC(handle,RP_SGMNT_CNT);
                rp_SetSgmntS(handle,RP_SGMNT_SIZE);
            break;
        case RP_MEM_DEV_BRAM:
                handle->dma_size=65536;
            break;
        default:
                return RP_EIPV;
            break;
    }

    rp_DmaMemDump(handle);

    return RP_OK;
}

static int rp_BramReset(rp_handle_uio_t *handle){
    uint16_t * map=NULL;
    // allocate data buffer memory
    map = (uint16_t *) mmap(NULL, handle->dma_size, PROT_READ | PROT_WRITE, MAP_SHARED, handle->dma_fd, 0);
    if (map==NULL) {
        printf("Failed to mmap\n");
        if (handle->dma_fd) {
            close(handle->dma_fd);
        }
        return -1;
    }

    map[0]=0;

    if(munmap (map, handle->dma_size)==-1){
        printf("Failed to munmap\n");
        return -1;
    }
    return RP_OK;
}

int rp_DmaCtrl(rp_handle_uio_t *handle, RP_DMA_CTRL ctrl)
{
    if(handle->mem_type!=RP_MEM_DEV_DMA)
        return RP_OK;

    switch(handle->mem_type){
        case RP_MEM_DEV_DMA:
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
        break;
        case RP_MEM_DEV_BRAM:
            switch(ctrl){
                case RP_DMA_CYCLIC:
                    return RP_OK;
                break;
                case RP_DMA_STOP_RX:
                    return rp_BramReset(handle);
                break;
                default:
                    return RP_EOOR;
            }
        break;
        default:
            return RP_EOOR;
        break;
    }
    return RP_OK;
}

int rp_SetSgmntC(rp_handle_uio_t *handle, unsigned long no)
{
    ioctl(handle->dma_fd, SET_RX_SGMNT_CNT, no);
    return RP_OK;
}

int rp_SetSgmntS(rp_handle_uio_t *handle, unsigned long no)
{
    ioctl(handle->dma_fd, SET_RX_SGMNT_SIZE, no);
    return RP_OK;
}

int rp_DmaMemDump(rp_handle_uio_t *handle)
{
    uint32_t samples;
    rp_DmaSizeInSamples(handle, &samples);

    uint16_t * map=NULL;
    // allocate data buffer memory
    map = (uint16_t *) mmap(NULL, handle->dma_size, PROT_READ | PROT_WRITE, MAP_SHARED, handle->dma_fd, 0);
    if (map==NULL) {
        printf("Failed to mmap\n");
        if (handle->dma_fd) {
            close(handle->dma_fd);
        }
        return -1;
    }

    // printout data
    for (int i=0; i<samples; i++) {
        if ((i%64)==0 ) printf("@%02x: ", i);
        printf("%04x",(uint16_t)map[i]);
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
    if(handle->mem_type!=RP_MEM_DEV_DMA)
        return RP_OK;

    int s = read(handle->dma_fd, NULL, 1);
    if (s<0) {
      printf("read error\n");
      return -1;
    }
    return RP_OK;
}

int rp_DmaSizeInSamples(rp_handle_uio_t *handle, uint32_t * samples)
{

    *samples=handle->dma_size/(sizeof(int16_t));
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


