/**
 * $Id: $
 *
 * @brief Red Pitaya library muxctl module interface
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef __MUXCTL_H
#define __MUXCTL_H

#include <stdint.h>
#include <stdbool.h>

// Base Muxctl address
static const int MUXCTL_BASE_SIZE = 0x30;

// Muxctl structure declaration
typedef struct {
  uint32_t gpio;
  uint32_t loop;
  uint32_t gen ;
  uint32_t lg  ;
} muxctl_regset_t;

int rp_MuxctlOpen(char *dev, rp_handle_uio_t *handle);
int rp_MuxctlClose(rp_handle_uio_t *handle);
int rp_MuxctlReset(rp_handle_uio_t *handle);

int rp_MuxctlSetGpio(rp_handle_uio_t *handle, uint32_t  mux);
int rp_MuxctlGetGpio(rp_handle_uio_t *handle, uint32_t *mux);

int rp_MuxctlSetLoop(rp_handle_uio_t *handle, uint32_t  mux);
int rp_MuxctlGetLoop(rp_handle_uio_t *handle, uint32_t *mux);

int rp_MuxctlSetGen (rp_handle_uio_t *handle, uint32_t  mux);
int rp_MuxctlGetGen (rp_handle_uio_t *handle, uint32_t *mux);

int rp_MuxctlSetLg  (rp_handle_uio_t *handle, uint32_t  mux);
int rp_MuxctlGetLg  (rp_handle_uio_t *handle, uint32_t *mux);

#endif //__MUXCTL_H
