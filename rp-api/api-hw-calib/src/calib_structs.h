/**
 * $Id: $
 *
 * @brief Red Pitaya API Calibration Module.
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 */

#ifndef __RP_HW_CALIB_STRUCT_H
#define __RP_HW_CALIB_STRUCT_H

#include <stdint.h>

/**
 * Calibration parameters, stored in the EEPROM device
 * used in 125-14 and 125-10
 */
typedef struct {
    char dataStructureId;
    char wpCheck;
    char reserved[6];

    uint32_t fe_ch1_fs_g_hi;    //!< High gain front end full scale voltage, channel A
    uint32_t fe_ch2_fs_g_hi;    //!< High gain front end full scale voltage, channel B
    uint32_t fe_ch1_fs_g_lo;    //!< Low gain front end full scale voltage, channel A
    uint32_t fe_ch2_fs_g_lo;    //!< Low gain front end full scale voltage, channel B
    int32_t  fe_ch1_lo_offs;    //!< Front end DC offset, channel A
    int32_t  fe_ch2_lo_offs;    //!< Front end DC offset, channel B
    uint32_t be_ch1_fs;         //!< Back end full scale voltage, channel A
    uint32_t be_ch2_fs;         //!< Back end full scale voltage, channel B
    int32_t  be_ch1_dc_offs;    //!< Back end DC offset, channel A
    int32_t  be_ch2_dc_offs;    //!< Back end DC offset, on channel B
	uint32_t magic;			    //!
    int32_t  fe_ch1_hi_offs;    //!< Front end DC offset, channel A
    int32_t  fe_ch2_hi_offs;    //!< Front end DC offset, channel B
    uint32_t low_filter_aa_ch1;  //!< Filter equalization coefficients AA for Low mode, channel A
    uint32_t low_filter_bb_ch1;  //!< Filter equalization coefficients BB for Low mode, channel A
    uint32_t low_filter_pp_ch1;  //!< Filter equalization coefficients PP for Low mode, channel A
    uint32_t low_filter_kk_ch1;  //!< Filter equalization coefficients KK for Low mode, channel A
    uint32_t low_filter_aa_ch2;  //!< Filter equalization coefficients AA for Low mode, channel B
    uint32_t low_filter_bb_ch2;  //!< Filter equalization coefficients BB for Low mode, channel B
    uint32_t low_filter_pp_ch2;  //!< Filter equalization coefficients PP for Low mode, channel B
    uint32_t low_filter_kk_ch2;  //!< Filter equalization coefficients KK for Low mode, channel B
    uint32_t  hi_filter_aa_ch1;  //!< Filter equalization coefficients AA for High mode, channel A
    uint32_t  hi_filter_bb_ch1;  //!< Filter equalization coefficients BB for High mode, channel A
    uint32_t  hi_filter_pp_ch1;  //!< Filter equalization coefficients PP for High mode, channel A
    uint32_t  hi_filter_kk_ch1;  //!< Filter equalization coefficients KK for High mode, channel A
    uint32_t  hi_filter_aa_ch2;  //!< Filter equalization coefficients AA for High mode, channel B
    uint32_t  hi_filter_bb_ch2;  //!< Filter equalization coefficients BB for High mode, channel B
    uint32_t  hi_filter_pp_ch2;  //!< Filter equalization coefficients PP for High mode, channel B
    uint32_t  hi_filter_kk_ch2;  //!< Filter equalization coefficients KK for High mode, channel B

} rp_calib_params_v1_t;


/**
 * Calibration parameters, stored in the EEPROM device
 *  used in 125-14 4Ch models
 */

typedef struct {
    char dataStructureId;
    char wpCheck;
    char reserved[6];

    uint32_t chA_g_hi;          //!< High gain front end full scale voltage, channel A
    uint32_t chB_g_hi;          //!< High gain front end full scale voltage, channel B
    uint32_t chC_g_hi;          //!< High gain front end full scale voltage, channel C
    uint32_t chD_g_hi;          //!< High gain front end full scale voltage, channel D

    uint32_t chA_g_low;         //!< Low gain front end full scale voltage, channel A
    uint32_t chB_g_low;         //!< Low gain front end full scale voltage, channel B
    uint32_t chC_g_low;         //!< Low gain front end full scale voltage, channel C
    uint32_t chD_g_low;         //!< Low gain front end full scale voltage, channel D

    int32_t  chA_hi_offs;       //!< Front end DC offset, channel A
    int32_t  chB_hi_offs;       //!< Front end DC offset, channel B
    int32_t  chC_hi_offs;       //!< Front end DC offset, channel C
    int32_t  chD_hi_offs;       //!< Front end DC offset, channel D

    int32_t  chA_low_offs;      //!< Front end DC offset, channel A
    int32_t  chB_low_offs;      //!< Front end DC offset, channel B
    int32_t  chC_low_offs;      //!< Front end DC offset, channel C
    int32_t  chD_low_offs;      //!< Front end DC offset, channel D

    uint32_t chA_hi_aa;         //!< Filter equalization coefficients AA for High mode, channel A
    uint32_t chA_hi_bb;         //!< Filter equalization coefficients BB for High mode, channel A
    uint32_t chA_hi_pp;         //!< Filter equalization coefficients PP for High mode, channel A
    uint32_t chA_hi_kk;         //!< Filter equalization coefficients KK for High mode, channel A

    uint32_t chA_low_aa;        //!< Filter equalization coefficients AA for Low mode, channel A
    uint32_t chA_low_bb;        //!< Filter equalization coefficients BB for Low mode, channel A
    uint32_t chA_low_pp;        //!< Filter equalization coefficients PP for Low mode, channel A
    uint32_t chA_low_kk;        //!< Filter equalization coefficients KK for Low mode, channel A

    uint32_t chB_hi_aa;         //!< Filter equalization coefficients AA for High mode, channel B
    uint32_t chB_hi_bb;         //!< Filter equalization coefficients BB for High mode, channel B
    uint32_t chB_hi_pp;         //!< Filter equalization coefficients PP for High mode, channel B
    uint32_t chB_hi_kk;         //!< Filter equalization coefficients KK for High mode, channel B

    uint32_t chB_low_aa;        //!< Filter equalization coefficients AA for Low mode, channel B
    uint32_t chB_low_bb;        //!< Filter equalization coefficients BB for Low mode, channel B
    uint32_t chB_low_pp;        //!< Filter equalization coefficients PP for Low mode, channel B
    uint32_t chB_low_kk;        //!< Filter equalization coefficients KK for Low mode, channel B

    uint32_t chC_hi_aa;         //!< Filter equalization coefficients AA for High mode, channel C
    uint32_t chC_hi_bb;         //!< Filter equalization coefficients BB for High mode, channel C
    uint32_t chC_hi_pp;         //!< Filter equalization coefficients PP for High mode, channel C
    uint32_t chC_hi_kk;         //!< Filter equalization coefficients KK for High mode, channel C

    uint32_t chC_low_aa;        //!< Filter equalization coefficients AA for Low mode, channel C
    uint32_t chC_low_bb;        //!< Filter equalization coefficients BB for Low mode, channel C
    uint32_t chC_low_pp;        //!< Filter equalization coefficients PP for Low mode, channel C
    uint32_t chC_low_kk;        //!< Filter equalization coefficients KK for Low mode, channel C

    uint32_t chD_hi_aa;         //!< Filter equalization coefficients AA for High mode, channel D
    uint32_t chD_hi_bb;         //!< Filter equalization coefficients BB for High mode, channel D
    uint32_t chD_hi_pp;         //!< Filter equalization coefficients PP for High mode, channel D
    uint32_t chD_hi_kk;         //!< Filter equalization coefficients KK for High mode, channel D

    uint32_t chD_low_aa;        //!< Filter equalization coefficients AA for Low mode, channel D
    uint32_t chD_low_bb;        //!< Filter equalization coefficients BB for Low mode, channel D
    uint32_t chD_low_pp;        //!< Filter equalization coefficients PP for Low mode, channel D
    uint32_t chD_low_kk;        //!< Filter equalization coefficients KK for Low mode, channel D

} rp_calib_params_v2_t;


/**
 * Calibration parameters, stored in the EEPROM device
 * used in 250-12 models
 */
typedef struct {
    char dataStructureId;
    char wpCheck;
    char reserved[6];

    uint32_t gen_ch1_g_1;
    uint32_t gen_ch2_g_1;
    int32_t  gen_ch1_off_1;
    int32_t  gen_ch2_off_1;

    uint32_t gen_ch1_g_5;
    uint32_t gen_ch2_g_5;
    int32_t  gen_ch1_off_5;
    int32_t  gen_ch2_off_5;

    uint32_t osc_ch1_g_1_ac;
    uint32_t osc_ch2_g_1_ac;
    int32_t  osc_ch1_off_1_ac;
    int32_t  osc_ch2_off_1_ac;

    uint32_t osc_ch1_g_1_dc; // HIGH
    uint32_t osc_ch2_g_1_dc;
    int32_t  osc_ch1_off_1_dc;
    int32_t  osc_ch2_off_1_dc;

    uint32_t osc_ch1_g_20_ac; // LOW
    uint32_t osc_ch2_g_20_ac;
    int32_t  osc_ch1_off_20_ac;
    int32_t  osc_ch2_off_20_ac;

    uint32_t osc_ch1_g_20_dc;
    uint32_t osc_ch2_g_20_dc;
    int32_t  osc_ch1_off_20_dc;
    int32_t  osc_ch2_off_20_dc;
} rp_calib_params_v3_t;


#endif