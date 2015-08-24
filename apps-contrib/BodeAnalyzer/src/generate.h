/** @file generate.h
 *
 * $Id: generate.h 882 2013-12-16 12:46:01Z crt.valentincic $
 *
 * @brief Red Pitaya simple signal/function generator with pre-defined signal types (C Header).
 * @author Jure Menart <juremenart@gmail.com>
 * @copyright Red Pitaya  http://www.redpitaya.com
 */

#ifndef __GENERATE_H
#define __GENERATE_H

#include "main.h"
#include "calib.h"

/** @defgroup generate_h Arbitrary Signal Generator
 * @{
 */

/** Signal types */
typedef enum awg_signal_e {
    eSignalSine = 0,     /* Sinusoidal waveform. */
    eSignalSquare,       /* Square waveform. */
    eSignalTriangle,     /* Triangular waveform. */
    eSignalFile          /* Waveform read from file */
} awg_signal_t;

/** AWG FPGA parameters */
typedef struct awg_param_s {
    int32_t  offsgain;   /* AWG offset & gain. */
    uint32_t wrap;       /* AWG buffer wrap value. */
    uint32_t step;       /* AWG step interval. */
} awg_param_t;

/** @} */

int generate_init(rp_calib_params_t *calib_params);
int generate_exit(void);

int generate_update(rp_app_params_t *params);

#endif // __GENERATE_H
