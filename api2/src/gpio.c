/**
 * @brief Red Pitaya library API interface implementation
 * @Author Red Pitaya
 * (c) Red Pitaya  http://www.redpitaya.com
 */

#include <stdlib.h>
#include <stdint.h>

#include "common.h"
#include "gpio.h"

int rp_GpioOpen(char *dev, rp_handle_uio_t *handle) {
    handle->length = GPIO_BASE_SIZE;
    int status = common_Open (dev, handle);
    if (status != RP_OK) {
        return status;
    }
    return RP_OK;
}

int rp_GpioClose(rp_handle_uio_t *handle) {
    int status = common_Close (handle); 
    if (status != RP_OK) {
        return status;
    }
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

