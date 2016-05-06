/**
 * @brief Red Pitaya library API interface implementation
 * @Author Red Pitaya
 * (c) Red Pitaya  http://www.redpitaya.com
 */

#include <stdlib.h>
#include <stdint.h>

#include "common.h"
#include "muxctl.h"

int rp_MuxctlOpen(char *dev, rp_handle_uio_t *handle) {
    handle->length = MUXCTL_BASE_SIZE;
    int status = common_Open (dev, handle);
    if (status != RP_OK) {
        return status;
    }
    return RP_OK;
}

int rp_MuxctlClose(rp_handle_uio_t *handle) {
    int status = common_Close (handle); 
    if (status != RP_OK) {
        return status;
    }
    return RP_OK;
}

int rp_MuxctlReset(rp_handle_uio_t *handle) {
    muxctl_regset_t *regset = (muxctl_regset_t *) handle->regset;
    iowrite32(0, &regset->gpio);
    iowrite32(0, &regset->loop);
    iowrite32(0, &regset->gen );
    iowrite32(0, &regset->lg  );
    return RP_OK;
}

// GPIO

int rp_MuxctlSetGpio(rp_handle_uio_t *handle, uint32_t mux) {
    muxctl_regset_t *regset = (muxctl_regset_t *) handle->regset;
    iowrite32(mux, &regset->gpio);
    return RP_OK;
}

int rp_MuxctlGetGpio(rp_handle_uio_t *handle, uint32_t *mux) {
    muxctl_regset_t *regset = (muxctl_regset_t *) handle->regset;
    *mux = ioread32(&regset->gpio);
    return RP_OK;
}

// Loop

int rp_MuxctlSetLoop(rp_handle_uio_t *handle, uint32_t mux) {
    muxctl_regset_t *regset = (muxctl_regset_t *) handle->regset;
    iowrite32(mux, &regset->loop);
    return RP_OK;
}

int rp_MuxctlGetLoop(rp_handle_uio_t *handle, uint32_t *mux) {
    muxctl_regset_t *regset = (muxctl_regset_t *) handle->regset;
    *mux = ioread32(&regset->loop);
    return RP_OK;
}

// Gen

int rp_MuxctlSetGen(rp_handle_uio_t *handle, uint32_t mux) {
    muxctl_regset_t *regset = (muxctl_regset_t *) handle->regset;
    iowrite32(mux, &regset->gen);
    return RP_OK;
}

int rp_MuxctlGetGen(rp_handle_uio_t *handle, uint32_t *mux) {
    muxctl_regset_t *regset = (muxctl_regset_t *) handle->regset;
    *mux = ioread32(&regset->gen);
    return RP_OK;
}

// LG

int rp_MuxctlSetLg(rp_handle_uio_t *handle, uint32_t mux) {
    muxctl_regset_t *regset = (muxctl_regset_t *) handle->regset;
    iowrite32(mux, &regset->lg);
    return RP_OK;
}

int rp_MuxctlGetLg(rp_handle_uio_t *handle, uint32_t *mux) {
    muxctl_regset_t *regset = (muxctl_regset_t *) handle->regset;
    *mux = ioread32(&regset->lg);
    return RP_OK;
}

