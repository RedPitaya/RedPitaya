/**
 * @brief Red Pitaya Oscilloscope Calibration Module.
 *
 * @author Ulrich Habel (DF4IAH) <espero7757@gmx.net>
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
int rp_write_calib_params(rp_calib_params_t *calib_params)
{
    FILE   *fp;
    size_t  size;

    /* sanity check */
    if(calib_params == NULL) {
        fprintf(stderr, "rp_write_calib_params(): input structure "
                "not initialized\n");
        return -1;
    }

    /* open eeprom device */
    fp=fopen(eeprom_device, "rw+");
    if(fp == NULL) {
        fprintf(stderr, "rp_write_calib_params(): Can not open EEPROM device: "
                " %s\n", strerror(errno));
	fclose(fp);
       return -1;
    }

    /* ...and seek to the appropriate storage offset */
    if(fseek(fp, eeprom_calib_off, SEEK_SET) < 0) {
        fclose(fp);
        fprintf(stderr, "rp_write_calib_params(): fseek() failed: %s\n", 
                strerror(errno));
        return -1;
    }

    /* write data to eeprom component from specified buffer */
    size=fwrite(calib_params, sizeof(char), sizeof(rp_calib_params_t), fp);
    if(size != sizeof(rp_calib_params_t)) {
        fclose(fp);
        fprintf(stderr, "rp_write_calib_params(): fwrite() failed, "
                "returned bytes: %d (should be :%d)\n", size, 
                sizeof(rp_calib_params_t));
        return -1;
    }
    fclose(fp);

    return 0;
}


/*----------------------------------------------------------------------------*/
int rp_default_calib_params(rp_calib_params_t *calib_params)
{
	// ADC
    calib_params->fe_ch1_fs_g_hi        =  28101971;	/*  0.006543000 [V] @ 32 bit */				/* one step = 232.83064365e-12 V */
    calib_params->fe_ch2_fs_g_hi        =  28101971;	/*  0.006543000 [V] @ 32 bit */				/* one step = 232.83064365e-12 V */
    calib_params->fe_ch1_fs_g_lo        = 625682246;	/*  0.145678000 [V] @ 32 bit */				/* one step = 232.83064365e-12 V */
    calib_params->fe_ch2_fs_g_lo        = 625682246;	/*  0.145678000 [V] @ 32 bit */				/* one step = 232.83064365e-12 V */
    calib_params->fe_ch1_dc_offs        =       585;	/*  0.023362 HI [V] @ 14 bit */				/* treated as signed value for 14 bit ADC value */
    calib_params->fe_ch2_dc_offs        =       585;	/*  0.520152 LO [V] @ 14 bit */				/* treated as signed value for 14 bit ADC value */

    // DAC
    calib_params->be_ch1_fs             =  42949673;	/*  0.010000000 [V] @ 32 bit */				/* one step = 232.83064365e-12 V */
    calib_params->be_ch2_fs             =  42949673;	/*  0.010000000 [V] @ 32 bit */				/* one step = 232.83064365e-12 V */
    calib_params->be_ch1_dc_offs        =     16044;	/* -0.010376 DAC[V] @ 15 bit */				/* treated as unsigned mid-range value for 15 bit DAC value - assumption: Ref-Voltage = 1.000 V */
    calib_params->be_ch2_dc_offs        =     16044;	/* -0.010376 DAC[V] @ 15 bit */				/* treated as unsigned mid-range value for 15 bit DAC value - assumption: Ref-Voltage = 1.000 V */

    // internals
    calib_params->base_osc125mhz_realhz =   125e+6f;	/* 125 000 000 [Hz] of the DAC clock */

    return 0;
}

/*----------------------------------------------------------------------------*/
float rp_calib_calc_max_v(uint32_t fe_gain_fs, int probe_att)
{
    int probe_att_fact = (probe_att > 0) ?  10 : 1;

    return probe_att_fact * 100.0f * (fe_gain_fs / ((float) (1ULL << 32)));
}
