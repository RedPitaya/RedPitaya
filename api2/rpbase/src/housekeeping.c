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
#include "housekeeping.h"

int rp_HousekeepingInit(char *dev, rp_handle_uio_t *handle) {
    // make a copy of the device path
    handle->dev = (char*) malloc((strlen(dev)+1) * sizeof(char));
    strncpy(handle->dev, dev, strlen(dev)+1);
    // try opening the device
    handle->fd = open(handle->dev, O_RDWR);
    if (!handle->fd) {
        return -1;
    } else {
        // get regset pointer
        handle->regset = mmap(NULL, HOUSEKEEPING_BASE_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, handle->fd, 0x0);
        if (handle->regset == NULL) {
            return -1;
        }
    }
    return RP_OK;
}

int rp_HousekeepingRelease(rp_handle_uio_t *handle) {
    // release regset
    munmap((void *) handle->regset, HOUSEKEEPING_BASE_SIZE);
    // close device
    close (handle->fd);
    // free device path
    free(handle->dev);
    // free name
    // TODO
    return RP_OK;
}

/**
 * Identification
 */

int rp_IdGetID(rp_handle_uio_t *handle, uint32_t *id) {
    housekeeping_regset_t *regset = (housekeeping_regset_t *) handle->regset;
    *id = ioread32(&regset->id);
    return RP_OK;
}

int rp_IdGetDNA(rp_handle_uio_t *handle, uint64_t *dna) {
    housekeeping_regset_t *regset = (housekeeping_regset_t *) handle->regset;
    *dna = ((uint64_t) ioread32(&regset->dna_hi) << 32)
         | ((uint64_t) ioread32(&regset->dna_lo) <<  0);
    return RP_OK;
}

/**
 * LED methods
 */

int rp_LEDSetState(rp_handle_uio_t *handle, uint32_t state) {
    housekeeping_regset_t *regset = (housekeeping_regset_t *) handle->regset;
    iowrite32(state, &regset->led_control);
    return RP_OK;
}

int rp_LEDGetState(rp_handle_uio_t *handle, uint32_t *state) {
    housekeeping_regset_t *regset = (housekeeping_regset_t *) handle->regset;
    *state = ioread32(&regset->led_control);
    return RP_OK;
}

/**
 * GPIO methods
 */

int rp_GPIOnSetDirection(rp_handle_uio_t *handle, uint32_t direction) {
    housekeeping_regset_t *regset = (housekeeping_regset_t *) handle->regset;
    iowrite32(direction, &regset->ex_cd_n);
    return RP_OK;
}

int rp_GPIOnGetDirection(rp_handle_uio_t *handle, uint32_t *direction) {
    housekeeping_regset_t *regset = (housekeeping_regset_t *) handle->regset;
    *direction = ioread32(&regset->ex_cd_n);
    return RP_OK;
}

int rp_GPIOnSetState(rp_handle_uio_t *handle, uint32_t state) {
    housekeeping_regset_t *regset = (housekeeping_regset_t *) handle->regset;
    iowrite32(state, &regset->ex_co_n);
    return RP_OK;
}

int rp_GPIOnGetState(rp_handle_uio_t *handle, uint32_t *state) {
    housekeeping_regset_t *regset = (housekeeping_regset_t *) handle->regset;
    *state = ioread32(&regset->ex_ci_n);
    return RP_OK;
}

int rp_GPIOpSetDirection(rp_handle_uio_t *handle, uint32_t direction) {
    housekeeping_regset_t *regset = (housekeeping_regset_t *) handle->regset;
    iowrite32(direction, &regset->ex_cd_p);
    return RP_OK;
}

int rp_GPIOpGetDirection(rp_handle_uio_t *handle, uint32_t *direction) {
    housekeeping_regset_t *regset = (housekeeping_regset_t *) handle->regset;
    *direction = ioread32(&regset->ex_cd_p);
    return RP_OK;
}

int rp_GPIOpSetState(rp_handle_uio_t *handle, uint32_t state) {
    housekeeping_regset_t *regset = (housekeeping_regset_t *) handle->regset;
    iowrite32(state, &regset->ex_co_p);
    return RP_OK;
}

int rp_GPIOpGetState(rp_handle_uio_t *handle, uint32_t *state) {
    housekeeping_regset_t *regset = (housekeeping_regset_t *) handle->regset;
    *state = ioread32(&regset->ex_ci_p);
    return RP_OK;
}

/**
 * Digital Pin Input Output methods
 */

int rp_DpinReset(rp_handle_uio_t *handle) {
    housekeeping_regset_t *regset = (housekeeping_regset_t *) handle->regset;
    iowrite32(0, &regset->ex_cd_p);
    iowrite32(0, &regset->ex_cd_n);
    iowrite32(0, &regset->ex_co_p);
    iowrite32(0, &regset->ex_co_n);
    iowrite32(0, &regset->led_control);
    iowrite32(0, &regset->digital_loop);
    return RP_OK;
}

int rp_DpinSetDirection(rp_handle_uio_t *handle, rp_dpin_t pin, rp_pinDirection_t direction) {
    housekeeping_regset_t *regset = (housekeeping_regset_t *) handle->regset;
    uint32_t tmp;
    if (pin < RP_DIO0_P) {
        // LEDS
        return RP_ELID;
    } else if (pin < RP_DIO0_N) {
        // DIO_P
        pin -= RP_DIO0_P;
        tmp = ioread32(&regset->ex_cd_p);
        iowrite32((tmp & ~(1 << pin)) | ((direction << pin) & (1 << pin)), &regset->ex_cd_p);
    } else {
        // DIO_N
        pin -= RP_DIO0_N;
        tmp = ioread32(&regset->ex_cd_n);
        iowrite32((tmp & ~(1 << pin)) | ((direction << pin) & (1 << pin)), &regset->ex_cd_p);
    }
    return RP_OK;
}

int rp_DpinGetDirection(rp_handle_uio_t *handle, rp_dpin_t pin, rp_pinDirection_t* direction) {
    housekeeping_regset_t *regset = (housekeeping_regset_t *) handle->regset;
    if (pin < RP_DIO0_P) {
        // LEDS
        *direction = RP_OUT;
    } else if (pin < RP_DIO0_N) {
        // DIO_P
        pin -= RP_DIO0_P;
        *direction = (ioread32(&regset->ex_cd_p) >> pin) & 0x1;
    } else {
        // DIO_N
        pin -= RP_DIO0_N;
        *direction = (ioread32(&regset->ex_cd_n) >> pin) & 0x1;
    }
    return RP_OK;
}

int rp_DpinSetState(rp_handle_uio_t *handle, rp_dpin_t pin, rp_pinState_t state) {
    housekeeping_regset_t *regset = (housekeeping_regset_t *) handle->regset;
    uint32_t tmp;
    rp_pinDirection_t direction;
    rp_DpinGetDirection(handle, pin, &direction);
    if (!direction) {
        return RP_EWIP;
    }
    if (pin < RP_DIO0_P) {
        // LEDS
        tmp = ioread32(&regset->led_control);
        iowrite32((tmp & ~(1 << pin)) | ((state << pin) & (1 << pin)), &regset->led_control);
    } else if (pin < RP_DIO0_N) {
        // DIO_P
        pin -= RP_DIO0_P;
        tmp = ioread32(&regset->ex_co_p);
        iowrite32((tmp & ~(1 << pin)) | ((state << pin) & (1 << pin)), &regset->ex_co_p);
    } else {
        // DIO_N
        pin -= RP_DIO0_N;
        tmp = ioread32(&regset->ex_co_n);
        iowrite32((tmp & ~(1 << pin)) | ((state << pin) & (1 << pin)), &regset->ex_co_n);
    }
    return RP_OK;
}

int rp_DpinGetState(rp_handle_uio_t *handle, rp_dpin_t pin, rp_pinState_t* state) {
    housekeeping_regset_t *regset = (housekeeping_regset_t *) handle->regset;
    if (pin < RP_DIO0_P) {
        // LEDS
        *state = (ioread32(&regset->led_control) >> pin) & 0x1;
    } else if (pin < RP_DIO0_N) {
        // DIO_P
        pin -= RP_DIO0_P;
        *state = (ioread32(&regset->ex_ci_p) >> pin) & 0x1;
    } else {
        // DIO_N
        pin -= RP_DIO0_N;
        *state = (ioread32(&regset->ex_ci_n) >> pin) & 0x1;
    }
    return RP_OK;
}


/**
 * Digital loop
 */

int rp_EnableDigitalLoop(rp_handle_uio_t *handle, bool enable) {
    housekeeping_regset_t *regset = (housekeeping_regset_t *) handle->regset;
    iowrite32((uint32_t) enable, &regset->digital_loop);
    return RP_OK;
}

