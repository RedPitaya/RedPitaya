/**
 * @brief Red Pitaya library API interface implementation
 * @Author Red Pitaya
 * (c) Red Pitaya  http://www.redpitaya.com
 */

// for Init
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <stdlib.h>
#include <stdint.h>

#include "common.h"
#include "gpio.h"

int rp_GpioInit(char *dev, rp_handle_uio_t *handle) {
    // make a copy of the device path
    handle->dev = (char*) malloc((strlen(dev)+1) * sizeof(char));
    strncpy(handle->dev, dev, strlen(dev)+1);
    // try opening the device
    handle->fd = open(handle->dev, O_RDWR);
    if (!handle->fd) {
        return -1;
    } else {
        // get regset pointer
        handle->regset = mmap(NULL, GPIO_BASE_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, handle->fd, 0x0);
        if (handle->regset == NULL) {
            return -1;
        }
    }
    return RP_OK;
}

int rp_GpioRelease(rp_handle_uio_t *handle) {
    // release regset
    munmap((void *) handle->regset, GPIO_BASE_SIZE);
    // close device
    close (handle->fd);
    // free device path
    free(handle->dev);
    // free name
    // TODO
    return RP_OK;
}

int rp_GpioReset(rp_handle_uio_t *handle) {
    gpio_regset_t *regset = (gpio_regset_t *) handle->regset;
    iowrite32(0, &regset->e);
    iowrite32(0, &regset->o);
    return RP_OK;
}

int rp_GpioSetEnable(rp_handle_uio_t *handle, uint32_t enable) {
    gpio_regset_t *regset = (gpio_regset_t *) handle->regset;
    iowrite32(enable, &regset->e);
    return RP_OK;
}

int rp_GpioGetEnable(rp_handle_uio_t *handle, uint32_t *enable) {
    gpio_regset_t *regset = (gpio_regset_t *) handle->regset;
    *enable = ioread32(&regset->e);
    return RP_OK;
}

int rp_GpioSetState(rp_handle_uio_t *handle, uint32_t state) {
    gpio_regset_t *regset = (gpio_regset_t *) handle->regset;
    iowrite32(state, &regset->o);
    return RP_OK;
}

int rp_GpioGetState(rp_handle_uio_t *handle, uint32_t *state) {
    gpio_regset_t *regset = (gpio_regset_t *) handle->regset;
    *state = ioread32(&regset->i);
    return RP_OK;
}

