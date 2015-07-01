/**
 * $Id: calib.c 881 2013-12-16 05:37:34Z rp_jmenart $
 *
 * @brief Red Pitaya Oscilloscope Calibration Module.
 *
 * @Author Jure Menart <juremenart@gmail.com>
 *         
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "calib.h"

const char eeprom_device[]="/sys/bus/i2c/devices/0-0050/eeprom";
const int  eeprom_calib_off=0x0008;


/*----------------------------------------------------------------------------*/
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
int rp_read_calib_params(rp_calib_params_t *calib_params)
{
    FILE   *fp;
    size_t  size;

    /* sanity check */
    if(calib_params == NULL) {
        fprintf(stderr, "rp_read_calib_params(): input structure "
                "not initialized\n");
        return -1;
    }

    /* open eeprom device */
    fp=fopen(eeprom_device, "r");
    if(fp == NULL) {
        fprintf(stderr, "rp_read_calib_params(): Can not open EEPROM device: "
                " %s\n", strerror(errno));
       return -1;
    }

    /* ...and seek to the appropriate storage offset */
    if(fseek(fp, eeprom_calib_off, SEEK_SET) < 0) {
        fclose(fp);
        fprintf(stderr, "rp_read_calib_params(): fseek() failed: %s\n", 
                strerror(errno));
        return -1;
    }

    /* read data from eeprom component and store it to the specified buffer */
    size=fread(calib_params, sizeof(char), sizeof(rp_calib_params_t), fp);
    if(size != sizeof(rp_calib_params_t)) {
        fclose(fp);
        fprintf(stderr, "rp_read_calib_params(): fread() failed, "
                "returned bytes: %d (should be :%d)\n", size, 
                sizeof(rp_calib_params_t));
        return -1;
    }
    fclose(fp);

    return 0;
}


/*----------------------------------------------------------------------------*/
/**
 * Initialize calibration parameters to default values.
 *
 * @param[out] calib_params  Pointer to target buffer to be initialized.
 * @retval 0 Success, could never fail.
 */

int rp_default_calib_params(rp_calib_params_t *calib_params)
{
    calib_params->fe_ch1_fs_g_hi = 28101971; /* 0.6543 [V] */
    calib_params->fe_ch2_fs_g_hi = 28101971; /* 0.6543 [V] */
    calib_params->fe_ch1_fs_g_lo = 625682246; /* 14.56 [V] */
    calib_params->fe_ch2_fs_g_lo = 625682246; /* 14.56 [V] */
    calib_params->fe_ch1_dc_offs = 585;
    calib_params->fe_ch2_dc_offs = 585;
    calib_params->be_ch1_fs = 42949673; /* 1 [V] */
    calib_params->be_ch2_fs = 42949673; /* 1 [V] */
    calib_params->be_ch1_dc_offs = 0x3eac;
    calib_params->be_ch2_dc_offs = 0x3eac;

    return 0;
}
