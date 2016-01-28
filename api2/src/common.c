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

int FpgaRegDump(uint32_t a_addr, uint32_t * a_data, uint32_t a_len){
	uint32_t addr=a_addr;
	printf("\n\r fpga reg. dump \n\r");
	printf("\n\r index, addr, value\n\r");
	for(int i=0; i<a_len; i++){
		addr+=0x4;
		printf("0x%04d,0x%08x,0x%08x\n\r",i,addr,a_data[i]);
	}
	return RP_OK;
}

int common_Open(char *dev, rp_handle_uio_t *handle) {
    // make a copy of the device path
    handle->dev = (char*) malloc((strlen(dev)+1) * sizeof(char));
    strncpy(handle->dev, dev, strlen(dev)+1);
    // try opening the device
    handle->fd = open(handle->dev, O_RDWR);
    if (!handle->fd) {
        return -1;
    }
    // get regset pointer
    handle->regset = mmap(NULL, handle->length, PROT_READ|PROT_WRITE, MAP_SHARED, handle->fd, 0x0);
    if (handle->regset == NULL) {
        return -1;
    }
    return RP_OK;
}

// TODO add error checking
int common_Close(rp_handle_uio_t *handle) {
    // release regset
    munmap((void *) handle->regset, handle->length);
    // close device
    close (handle->fd);
    // free device path
    free(handle->dev);
    // free name
    // TODO
    return RP_OK;
}
