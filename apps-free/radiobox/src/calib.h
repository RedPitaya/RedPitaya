/**
 * @brief Red Pitaya RadioBox Calibration Module.
 *
 * @author Ulrich Habel (DF4IAH) <espero7757@gmx.net>
 *         
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef __CALIB_H
#define __CALIB_H

#include <stdint.h>


/** @defgroup calib_h Calibration data
 * @{
 */

/** @brief  Calibration parameters stored in the EEPROM device
 */
typedef struct rp_calib_params_s {

	/** @brief High gain front end full scale voltage, channel 1 */
	uint32_t fe_ch1_fs_g_hi;

	/** @brief High gain front end full scale voltage, channel 2 */
	uint32_t fe_ch2_fs_g_hi;

	/** @brief Low gain front end full scale voltage, channel 1  */
	uint32_t fe_ch1_fs_g_lo;

	/** @brief Low gain front end full scale voltage, channel 2  */
	uint32_t fe_ch2_fs_g_lo;

	/** @brief Front end DC offset, channel 1  */
	int32_t  fe_ch1_dc_offs;

	/** @brief Front end DC offset, channel 2  */
	int32_t  fe_ch2_dc_offs;

	/** @brief Back end full scale voltage, channel 1  */
	uint32_t be_ch1_fs;

	/** @brief Back end full scale voltage, channel 2  */
	uint32_t be_ch2_fs;

	/** @brief Back end DC offset, channel 1 */
	int32_t  be_ch1_dc_offs;

	/** @brief Back end DC offset, on channel 2 */
	int32_t  be_ch2_dc_offs;

	/** @brief Base attributes: real frequency of the 125 MHz ADC clock in Hz */
	double	 base_osc125mhz_realhz;

} rp_calib_params_t;


/**
 * @brief Read calibration parameters from EEPROM device.
 *
 * Function reads calibration parameters from EEPROM device and stores them to the
 * specified buffer. Communication to the EEPROM device is taken place through
 * appropriate system driver accessed through the file system device
 * /sys/bus/i2c/devices/0-0050/eeprom.
 *
 * @param[out]   calib_params  Pointer to destination buffer.
 * @retval       0 Success
 * @retval      -1 Failure, error message is put on stderr device
 *
 */
int rp_read_calib_params(rp_calib_params_t* calib_params);

/**
 * @brief Write calibration parameters to EEPROM device.
 *
 * Function writes calibration parameters to EEPROM device.
 * Communication to the EEPROM device is taken place through
 * appropriate system driver accessed through the file system device
 * /sys/bus/i2c/devices/0-0050/eeprom.
 *
 * @param[out]   calib_params  Pointer to source buffer.
 * @retval       0 Success
 * @retval      -1 Failure, error message is put on stderr device
 *
 */
int rp_write_calib_params(rp_calib_params_t* calib_params);

/**
 * Initialize calibration parameters to default values.
 *
 * @param[out] calib_params  Pointer to target buffer to be initialized.

 * @retval     0             Success, could never fail.
 */
int rp_default_calib_params(rp_calib_params_t* calib_params);


/*
 * Information about data representation
 * =====================================
 *
 * max_adc_v = 100.0f * fe_gain_fs     / ((float) (1ULL << 32) * probe_att_fact;  // having probe_att_fact = 1 or 10
 * max_dac_v = 100.0f * be_chX_dc_offs / ((float) (1ULL << 32));
 *
 */

/**
 * @brief Calculates maximum [V] in respect to calibration parameters
 *
 * Function is used to calculate the maximum voltage which can be applied on an ADC input.
 * This calculation is based on the calibrated front-end full scale gain setting and the
 * configured probe attenuation.
 *
 * @param[in] fe_gain_fs     Front End Full Scale Gain
 * @param[in] probe_att      Probe attenuation
 * @retval    float          Maximum voltage, expressed in [V]
 */
float rp_calib_calc_max_v(uint32_t fe_gain_fs, int probe_att);

/** @} */


#endif //__CALIB_H
