/**
 * $Id: $
 *
 * @brief Red Pitaya library common module interface
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 */

#ifndef __RP_LA_COMMON_H
#define __RP_LA_COMMON_H

#include <stdbool.h>
#include <stdint.h>
#include <string>
#include "structs.h"

// unmasked IO read/write (p - pointer, v - value)
#define ioread32(p) (*(volatile uint32_t*)(p))
#define iowrite32(v, p) (*(volatile uint32_t*)(p) = (v))
#define ioread64(p) (*(volatile uint64_t*)(p))
#define iowrite64(v, p) (*(volatile uint64_t*)(p) = (v))

// open/close UIO device
int common_Open(const std::string dev, rp_handle_uio_t* handle);
int common_Close(rp_handle_uio_t* handle);

bool inrangeUint32(uint32_t x, uint32_t minval_t, uint32_t maxval);
bool inrangeDouble(double x, double minval_t, double maxval);
uint32_t getMaxFreq();

const double c_max_dig_sampling_rate = getMaxFreq();
const double c_max_dig_sampling_rate_time_interval_ns = 1e9 / c_max_dig_sampling_rate;

auto getClockMs() -> double;

#endif