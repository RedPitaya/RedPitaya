#ifndef CLB_H
#define CLB_H

#include <stdint.h>

#include "redpitaya/uio.h"
#include "redpitaya/lin.h"

#define CLB_MAGIC 0xAABBCCDD
#define eeprom_device "/sys/bus/i2c/devices/0-0050/eeprom"
#define eeprom_offset_user    = 0x0008
#define eeprom_offset_factory = 0x1c08

// device regset structure
typedef struct {
    rp_lin_regset_t dac[2];
    rp_lin_regset_t adc[2];
} rp_clb_regset_t;

// floating point structure
typedef struct {
    float gain;    // multiplication
    float offset;  // summation
} rp_clb_channel_t;

typedef struct {
    rp_clb_channel_t lo;  //  1.0V range
    rp_clb_channel_t hi;  // 20.0V range
} rp_clb_range_t;

typedef struct {
    rp_clb_channel_t dac[2];  // generator
    rp_clb_range_t   adc[2];  // oscilloscope
} rp_clb_float_t;

// EEPROM structure
typedef struct {
    uint32_t adc_hi_gain  [2];
    uint32_t adc_lo_gain  [2];
     int32_t adc_lo_offset[2];
    uint32_t dac_gain     [2];
     int32_t dac_offset   [2];
    uint32_t magic           ;
     int32_t adc_hi_offset[2];
} rp_clb_eeprom_t;

typedef struct {
    rp_uio_t uio;
    volatile rp_clb_regset_t *regset;
    int unsigned num_dac;
    int unsigned num_adc;
    rp_lin_t dac[2];
    rp_lin_t adc[2];
} rp_clb_t;

int      rp_clb_init      (rp_clb_t *handle);
int      rp_clb_release   (rp_clb_t *handle);
int      rp_clb_default   (rp_clb_t *handle);
void     rp_clb_print     (rp_clb_t *handle);
    
int      rp_clb_eeprom_read   (rp_clb_t *handle);
int      rp_clb_eeprom_parse  (rp_clb_t *handle, rp_clb_eeprom_t eeprom_struct, rp_clb_float_t *clb_struct);

int      rp_clb_calib_show    (rp_clb_t *handle);
int      rp_clb_apply         (rp_clb_t *handle);

float    rp_clb_FullScaleToVoltage   (rp_clb_t *handle, int   value);
int      rp_clb_FullScaleFromVoltage (rp_clb_t *handle, float value);

#endif

