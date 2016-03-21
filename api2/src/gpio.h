/**
 * $Id: $
 *
 * @brief Red Pitaya library gpio module interface
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef __GPIO_H
#define __GPIO_H

#include <stdint.h>
#include <stdbool.h>

// Base Gpio address
static const int GPIO_BASE_SIZE = 0x30;

// Gpio structure declaration
typedef struct {
    uint32_t e;
    uint32_t o;
    uint32_t i;
} gpio_regset_t;

int rp_GpioOpen(char *dev, rp_handle_uio_t *handle);
int rp_GpioClose(rp_handle_uio_t *handle);
int rp_GpioReset(rp_handle_uio_t *handle);

int rp_GpioSetEnable(rp_handle_uio_t *handle, uint32_t  enable);
int rp_GpioGetEnable(rp_handle_uio_t *handle, uint32_t *enable);
int rp_GpioSetState (rp_handle_uio_t *handle, uint32_t  state);
int rp_GpioGetState (rp_handle_uio_t *handle, uint32_t *state);

#endif //__GPIO_H
