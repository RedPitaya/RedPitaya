/**
 * $Id: $
 *
 * @brief Red Pitaya library Analog Mixed Signals (AMS) module interface
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef __ANALOG_MIXED_SIGNALS_H
#define __ANALOG_MIXED_SIGNALS_H

// Base Analog Mixed Signals address
static const int PDM_BASE_SIZE = 0x30;

typedef struct {
    uint32_t pdm_cfg [4];
} pdm_regset_t;

static const uint32_t PDM_MASK            = 0xFF;

static const float    PDM_MAX_VAL         = 1.8;
static const float    PDM_MIN_VAL         = 0.0;
static const uint32_t PDM_MAX_VAL_INTEGER = 255;

int rp_PdmOpen(char *dev, rp_handle_uio_t *handle);
int rp_PdmClose(rp_handle_uio_t *handle);

/**
* Sets analog outputs to default values (0V).
*/
int rp_PdmReset(rp_handle_uio_t *handle);

/**
 * Gets value from analog pin in volts.
 * @param pin    Analog output pin index.
 * @param value  Value on analog pin in volts
 * @return       RP_OK - successful, RP_E* - failure
 */
int rp_PdmGetValue(rp_handle_uio_t *handle, int unsigned pin, float* value);
int rp_PdmSetValue(rp_handle_uio_t *handle, int unsigned pin, float value);

/**
 * Gets raw value from analog pin.
 * @param pin    Analog output pin index.
 * @param value  Raw value on analog pin
 * @return       RP_OK - successful, RP_E* - failure
 */
int rp_PdmGetValueRaw(rp_handle_uio_t *handle, int unsigned pin, uint32_t* value);
int rp_PdmSetValueRaw(rp_handle_uio_t *handle, int unsigned pin, uint32_t value);

/**
 * Gets range in volts on specific pin.
 * @param pin      Analog input output pin index.
 * @param min_val  Minimum value in volts on given pin.
 * @param max_val  Maximum value in volts on given pin.
 * @return       RP_OK - successful, RP_E* - failure
 */
int rp_PdmGetRange(rp_handle_uio_t *handle, int unsigned pin, float* min_val,  float* max_val);

#endif //__ANALOG_MIXED_SIGNALS_H
