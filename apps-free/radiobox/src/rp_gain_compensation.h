/**
 * @brief Red Pitaya RadioBox gain compensation module.
 *
 * @author Ulrich Habel (DF4IAH) <espero7757@gmx.net>
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef __GAIN_COMPENSATION_H
#define __GAIN_COMPENSATION_H


/** @defgroup gain_compensation_h FPGA RadioBox sub-module gain compensation for DAC output
 * signals with an open end or terminated with 50 ohms.
 * @{
 */

/** @brief Table of gain values measured at these frequencies with open end or terminated
 *
 * This structure  rb_gain_params_s  stores a table of gain values
 * for an open end or 50 ohms terminated connector of the DAC lines. A readout function enables
 * to compensate for this gain drift for any frequency needed at the DAC output.
 **/
typedef struct rb_gain_params_s {
    /** @brief frequency in hertz */
    float frequency_hz;

    /** @brief gain factor for the DAC terminated with 50 ohms */
    float gain_terminated50R;

    /** @brief gain factor for the DAC not terminated (open end) */
    float gain_openEnd;
} rb_gain_params_t;

#define RB_GAIN_PARAMS_BSPLINE_K    4
#define RB_GAIN_PARAMS_HW_1V1_NUM 113

enum rb_gain_params_columns {
    RB_GAIN_PARAMS_FREQUENCY = 0,
    RB_GAIN_PARAMS_GAIN_TERM,
    RB_GAIN_PARAMS_GAIN_OPEN
};


/**
 * @brief b-spline helper function to correct the access index by the k factor
 *
 * @param[in]  j               b-spline index position not corrected for the k value.
 * @param[in]  k               b-spline k order of smoothness.
 * @param[in]  n               b-spline count of table entries.
 * @retval     int             b-spline index position to look-up into the data table.
 */
int bspline_j_k_n(int j, int k, int n);

/**
 * @brief b-spline helper function to calculate the recursive part of N_i_k(t)
 *
 * @param[in]  i               b-spline i loop index of the sigma container function.
 * @param[in]  k               b-spline k order of smoothness.
 * @param[in]  t               b-spline t position.
 * @retval     float           b-spline weight value.
 */
float bspline_n_i_k(int i, int k, float t);

/**
 * @brief Calculates the compensation factor for the out amplifier
 *
 * @param[in]  frequency_hz    Frequency in hertz.
 * @param[in]  isTerminated    True if 50 ohms resistor is connected to the output line, False if the output line is open.
 * @retval     float           Compensation factor to be used for the output amplifier.
 */
float get_compensation_factor(float frequency_hz, int isTerminated);


/** @} */

#endif /* __GAIN_COMPENSATION_H*/
