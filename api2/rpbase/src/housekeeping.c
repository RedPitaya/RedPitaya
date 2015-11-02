/**
 * @brief Red Pitaya library API interface implementation
 * @Author Red Pitaya
 * (c) Red Pitaya  http://www.redpitaya.com
 */

#include <stdio.h>
#include <stdint.h>

#include "common.h"
#include "housekeeping.h"

static volatile housekeeping_control_t *hk = NULL;

int hk_Init() {
    cmn_Map(HOUSEKEEPING_BASE_SIZE, HOUSEKEEPING_BASE_ADDR, (void**)&hk);
    return RP_OK;
}

int hk_Release() {
    cmn_Unmap(HOUSEKEEPING_BASE_SIZE, (void**)&hk);
    return RP_OK;
}

/**
 * Identification
 */

int rp_IdGetID(uint32_t *id) {
    *id = ioread32(&hk->id);
    return RP_OK;
}

int rp_IdGetDNA(uint64_t *dna) {
    *dna = ((uint64_t) ioread32(&hk->dna_hi) << 32)
         | ((uint64_t) ioread32(&hk->dna_lo) <<  0);
    return RP_OK;
}

/**
 * LED methods
 */

int rp_LEDSetState(uint32_t state) {
    iowrite32(state, &hk->led_control);
    return RP_OK;
}

int rp_LEDGetState(uint32_t *state) {
    *state = ioread32(&hk->led_control);
    return RP_OK;
}

/**
 * GPIO methods
 */

int rp_GPIOnSetDirection(uint32_t direction) {
    iowrite32(direction, &hk->ex_cd_n);
    return RP_OK;
}

int rp_GPIOnGetDirection(uint32_t *direction) {
    *direction = ioread32(&hk->ex_cd_n);
    return RP_OK;
}

int rp_GPIOnSetState(uint32_t state) {
    iowrite32(state, &hk->ex_co_n);
    return RP_OK;
}

int rp_GPIOnGetState(uint32_t *state) {
    *state = ioread32(&hk->ex_ci_n);
    return RP_OK;
}

int rp_GPIOpSetDirection(uint32_t direction) {
    iowrite32(direction, &hk->ex_cd_p);
    return RP_OK;
}

int rp_GPIOpGetDirection(uint32_t *direction) {
    *direction = ioread32(&hk->ex_cd_p);
    return RP_OK;
}

int rp_GPIOpSetState(uint32_t state) {
    iowrite32(state, &hk->ex_co_p);
    return RP_OK;
}

int rp_GPIOpGetState(uint32_t *state) {
    *state = ioread32(&hk->ex_ci_p);
    return RP_OK;
}

/**
 * Digital Pin Input Output methods
 */

int rp_DpinReset() {
    iowrite32(0, &hk->ex_cd_p);
    iowrite32(0, &hk->ex_cd_n);
    iowrite32(0, &hk->ex_co_p);
    iowrite32(0, &hk->ex_co_n);
    iowrite32(0, &hk->led_control);
    iowrite32(0, &hk->digital_loop);
    return RP_OK;
}

int rp_DpinSetDirection(rp_dpin_t pin, rp_pinDirection_t direction) {
    uint32_t tmp;
    if (pin < RP_DIO0_P) {
        // LEDS
        return RP_ELID;
    } else if (pin < RP_DIO0_N) {
        // DIO_P
        pin -= RP_DIO0_P;
        tmp = ioread32(&hk->ex_cd_p);
        iowrite32((tmp & ~(1 << pin)) | ((direction << pin) & (1 << pin)), &hk->ex_cd_p);
    } else {
        // DIO_N
        pin -= RP_DIO0_N;
        tmp = ioread32(&hk->ex_cd_n);
        iowrite32((tmp & ~(1 << pin)) | ((direction << pin) & (1 << pin)), &hk->ex_cd_p);
    }
    return RP_OK;
}

int rp_DpinGetDirection(rp_dpin_t pin, rp_pinDirection_t* direction) {
    if (pin < RP_DIO0_P) {
        // LEDS
        *direction = RP_OUT;
    } else if (pin < RP_DIO0_N) {
        // DIO_P
        pin -= RP_DIO0_P;
        *direction = (ioread32(&hk->ex_cd_p) >> pin) & 0x1;
    } else {
        // DIO_N
        pin -= RP_DIO0_N;
        *direction = (ioread32(&hk->ex_cd_n) >> pin) & 0x1;
    }
    return RP_OK;
}

int rp_DpinSetState(rp_dpin_t pin, rp_pinState_t state) {
    uint32_t tmp;
    rp_pinDirection_t direction;
    rp_DpinGetDirection(pin, &direction);
    if (!direction) {
        return RP_EWIP;
    }
    if (pin < RP_DIO0_P) {
        // LEDS
        tmp = ioread32(&hk->led_control);
        iowrite32((tmp & ~(1 << pin)) | ((state << pin) & (1 << pin)), &hk->led_control);
    } else if (pin < RP_DIO0_N) {
        // DIO_P
        pin -= RP_DIO0_P;
        tmp = ioread32(&hk->ex_co_p);
        iowrite32((tmp & ~(1 << pin)) | ((state << pin) & (1 << pin)), &hk->ex_co_p);
    } else {
        // DIO_N
        pin -= RP_DIO0_N;
        tmp = ioread32(&hk->ex_co_n);
        iowrite32((tmp & ~(1 << pin)) | ((state << pin) & (1 << pin)), &hk->ex_co_n);
    }
    return RP_OK;
}

int rp_DpinGetState(rp_dpin_t pin, rp_pinState_t* state) {
    if (pin < RP_DIO0_P) {
        // LEDS
        *state = (ioread32(&hk->led_control) >> pin) & 0x1;
    } else if (pin < RP_DIO0_N) {
        // DIO_P
        pin -= RP_DIO0_P;
        *state = (ioread32(&hk->ex_ci_p) >> pin) & 0x1;
    } else {
        // DIO_N
        pin -= RP_DIO0_N;
        *state = (ioread32(&hk->ex_ci_n) >> pin) & 0x1;
    }
    return RP_OK;
}


/**
 * Digital loop
 */

int rp_EnableDigitalLoop(bool enable) {
    iowrite32((uint32_t) enable, &hk->digital_loop);
    return RP_OK;
}

