#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"

const char c_dummy_dev[10]="/dev/dummy";

int FpgaRegDump(char * desc, uint32_t a_addr, uint32_t * a_data, uint32_t a_len){
    uint32_t addr=a_addr;
    printf("\n\r %s\n\r",desc);
    printf("\n\r index, addr, value\n\r");
    for(int i=0; i<a_len; i++){
        printf("0x%04d,0x%08x,0x%08x\n\r",i,addr,a_data[i]);//ioread32(&a_data[i]));
        addr+=0x4;
    }
    return RP_OK;
}

int common_Open(const char *dev, rp_handle_uio_t *handle) {
    // make a copy of the device path
    handle->dev = (char*) malloc((strlen(dev)+1) * sizeof(char));
    strncpy(handle->dev, dev, strlen(dev)+1);

    // if this is dummy device, just allocate dummy memeory space
    if(strncmp(c_dummy_dev, handle->dev, sizeof(c_dummy_dev))==0){
        handle->regset = malloc(handle->struct_size);
    }
    else{
        // try opening the device
        handle->fd = open(handle->dev, O_RDWR);
        if (handle->fd == -1) {
            return -1;
        }
        // get regset pointer
        handle->regset = mmap(NULL, handle->length, PROT_READ|PROT_WRITE, MAP_SHARED, handle->fd, 0x0);
        //printf("1:0x%08x\n\r",(uint32_t)handle->regset);
        if (handle->regset == MAP_FAILED) {
            //printf("2:0x%08x\n\r",(uint32_t)handle->regset);
            return -1;
        }
    }
    return RP_OK;
}

int common_Close(rp_handle_uio_t *handle) {
    int r=RP_OK;
    // if this is dummy device, just allocate dummy memeory space
    if(strncmp(c_dummy_dev, handle->dev, sizeof(c_dummy_dev))==0){
        free((void *) handle->regset);
        free(handle->dev);
    }
    else{
        // release regset
        if(munmap((void *) handle->regset, handle->length)==-1){
            r=-1;
        }
        // close device
        if(close (handle->fd)==-1){
            r=-1;

        }
        // free device path
        free(handle->dev);
        // free name
        // TODO
    }
    return r;
}

/**
 *  Check if value x is in range
 *  @return true if value x is in [minval,maxval] range
 */
bool inrangeUint32(uint32_t x, uint32_t minval, uint32_t maxval)
{
    return (x >= minval) && (x <= maxval);
}

bool inrangeDouble(double x, double minval, double maxval)
{
    return (x >= minval) && (x <= maxval);
}

