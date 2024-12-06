
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#include <poll.h>
#include <stdio.h>

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "dma.h"
#include "rp.h"


int dma_getReservedMemory(uint32_t *_startAddress,uint32_t *_size){
    *_startAddress = 0;
    *_size = 0;
    int fd = 0;
    if((fd = open("/sys/firmware/devicetree/base/reserved-memory/buffer@1000000/reg", O_RDONLY)) == -1) {
        FATAL("Error open: /sys/firmware/devicetree/base/reserved-memory/buffer@1000000/reg\n");
        return RP_EOMD;
    }
    char data[8] = {0,0,0,0,0,0,0,0};
    int sz = read(fd, &data, 8);

    if (close(fd) < 0) {
        return RP_EOMD;
    }
    if (sz == 8){
        *_startAddress = data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3];
        *_size = data[4] << 24 | data[5] << 16 | data[6] << 8 | data[7];
    }else{
        return RP_EOMD;
    }
    return RP_OK;
}

int rp_dmaOpen(const std::string dev, rp_handle_uio_t *handle) {

    uint32_t start = 0, size=0;
    if (dma_getReservedMemory(&start,&size) != RP_OK){
        ERROR_LOG("Error getting reserved memory size.")
        return -1;
    }

    if (size <= RP_SGMNT_CNT * RP_SGMNT_SIZE){
        ERROR_LOG("Reserved memory size %d is less than required %d", size, RP_SGMNT_CNT * RP_SGMNT_SIZE)
        return -1;
    }

    handle->dma_dev = dev;
    handle->dma_fd = open(handle->dma_dev.c_str(), O_RDWR);

    if (handle->dma_fd == -1) {
        ERROR_LOG("Unable to open device file");
        return -1;
    }

    handle->dma_size = RP_SGMNT_CNT * RP_SGMNT_SIZE;
    rp_setSgmntC(handle, RP_SGMNT_CNT);
    rp_setSgmntS(handle, RP_SGMNT_SIZE);
    return RP_OK;
}

int rp_dmaCtrl(rp_handle_uio_t *handle, RP_DMA_CTRL ctrl) {
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

int rp_setSgmntC(rp_handle_uio_t *handle, unsigned long no) {
    ioctl(handle->dma_fd, SET_RX_SGMNT_CNT, no);
    return RP_OK;
}

int rp_setSgmntS(rp_handle_uio_t *handle, unsigned long no) {
    ioctl(handle->dma_fd, SET_RX_SGMNT_SIZE, no);
    return RP_OK;
}

int rp_dmaRead(rp_handle_uio_t *handle, int timeout_s, bool *timeOut) {

    TRACE_SHORT("Read")
    *timeOut = false;
    ioctl(handle->dma_fd, TIMEOUT, timeout_s); // Set timeout in sec
    int s = read(handle->dma_fd, NULL, 1);
    if (s < 0) {
        ERROR_LOG("Read error");
        return -1;
    } else if (s == 0){
        *timeOut = true;
        TRACE_SHORT("Read timeout");
    }
    return RP_OK;
}

int rp_dmaClose(rp_handle_uio_t *handle) {
    if(handle->dma_fd){
        if(close(handle->dma_fd)==-1){
            return -1;
        }
    }
    return RP_OK;
}
