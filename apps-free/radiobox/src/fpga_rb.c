/**
 * @brief Red Pitaya FPGA Interface for the RadioBox sub-module.
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
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "main.h"
#include "calib.h"
#include "fpga.h"
#include "cb_http.h"


/** @brief calibration data layout within the EEPROM device */
extern rp_calib_params_t    g_rp_main_calib_params;

/** @brief CallBack copy of params from the worker when requested */
extern rb_app_params_t*     g_rb_info_worker_params;
/** @brief Holds mutex to access on parameters from the worker thread to any other context */
extern pthread_mutex_t      g_rb_info_worker_params_mutex;

/** @brief The RadioBox memory file descriptor used to mmap() the FPGA space. */
extern int                  g_fpga_rb_mem_fd;
/** @brief The RadioBox memory layout of the FPGA registers. */
extern fpga_rb_reg_mem_t*   g_fpga_rb_reg_mem;


/*----------------------------------------------------------------------------*/
int fpga_rb_init(void)
{
    fprintf(stderr, "fpga_rb_init: BEGIN\n");

    /* make sure all previous data is vanished */
    fpga_rb_exit();

    /* init the RadioBox FPGA sub-module access */
	if (fpga_mmap_area(&g_fpga_rb_mem_fd, (void**) &g_fpga_rb_reg_mem, FPGA_RB_BASE_ADDR, FPGA_RB_BASE_SIZE)) {
        fprintf(stderr, "ERROR - fpga_rb_init: g_fpga_rb_reg_mem - mmap() failed: %s\n", strerror(errno));
		fpga_exit();
		return -1;
	}
    fprintf(stderr, "DEBUG fpga_rb_init: g_fpga_rb_reg_mem - having access pointer.\n");

    // enable RadioBox sub-module
    fpga_rb_enable(1);

    fprintf(stderr, "fpga_rb_init: END\n");
    return 0;
}

/*----------------------------------------------------------------------------*/
int fpga_rb_exit(void)
{
    fprintf(stderr, "fpga_rb_exit: BEGIN\n");

    /* disable RadioBox sub-module */
    fpga_rb_enable(0);

    /* unmap the RadioBox sub-module */
	if (fpga_munmap_area(&g_fpga_rb_mem_fd, (void**) &g_fpga_rb_reg_mem, FPGA_RB_BASE_ADDR, FPGA_RB_BASE_SIZE)) {
		fprintf(stderr, "ERROR - fpga_rb_exit: g_fpga_rb_reg_mem - munmap() failed: %s\n", strerror(errno));
	}

	fprintf(stderr, "fpga_rb_exit: END\n");
    return 0;
}

/*----------------------------------------------------------------------------*/
void fpga_rb_enable(int enable)
{
    if (!g_fpga_rb_reg_mem) {
        return;
    }

    //fprintf(stderr, "fpga_rb_enable(%d): BEGIN\n", enable);

    if (enable) {
        // enable RadioBox
        g_fpga_rb_reg_mem->ctrl        = 0x00000001;    // enable RB sub-module
        fpga_rb_reset();

        g_fpga_rb_reg_mem->led_ctrl    = 0x00000000;    // disable RB LEDs
        g_fpga_rb_reg_mem->muxin_gain  = 0x00000100;    // open Mic gain 1:1 (FS = 2Vpp)

    } else {
        //fprintf(stderr, "fpga_rb_enable: turning off RB LEDs\n");
        g_fpga_rb_reg_mem->led_ctrl    = 0x00000000;    // disable RB LEDs

        // disable RadioBox
        //fprintf(stderr, "fpga_rb_enable: disabling RB sub-module\n");
        g_fpga_rb_reg_mem->ctrl        = 0x00000000;    // disable RB sub-module
    }

    //fprintf(stderr, "fpga_rb_enable(%d): END\n", enable);
}

/*----------------------------------------------------------------------------*/
void fpga_rb_reset(void)
{
    if (!g_fpga_rb_reg_mem) {
        return;
    }

    // send resync to OSC1 and OSC2
    g_fpga_rb_reg_mem->ctrl = 0x00001011;

    // send resync and reset to OSC1 and OSC2
    g_fpga_rb_reg_mem->ctrl = 0x00001017;

    // send resync to OSC1 and OSC2
    g_fpga_rb_reg_mem->ctrl = 0x00001011;

    // run mode of both oscillators
    g_fpga_rb_reg_mem->ctrl = 0x00000001;
}


/*----------------------------------------------------------------------------*/
int fpga_rb_update_all_params(rb_app_params_t* p)
{
    int    loc_rb_run     = 0;
    int    loc_modsrc     = 0;
    int    loc_modtyp     = 0;
    int    loc_led_ctrl   = 0;
    double loc_osc_car_qrg   = 0.0;
    double loc_osc_mod_qrg   = 0.0;
    double loc_osc_car_amp   = 0.0;
    double loc_osc_mod_mag   = 0.0;
    double loc_muxin_gain = 0.0;

    //fprintf(stderr, "fpga_rb_update_all_params: BEGIN\n");

    if (!g_fpga_rb_reg_mem || !p) {
        //fprintf(stderr, "ERROR - fpga_rb_update_all_params: bad parameter (p=%p) or not init'ed(g=%p)\n", p, g_fpga_rb_reg_mem);
        return -1;
    }

    int idx;
    for (idx = 0; p[idx].name; idx++) {
        if (!(p[idx].name)) {
            break;  // end of list
        }

        if (!(p[idx].fpga_update & 0x80)) {  // MARKer set?
            fprintf(stderr, "DEBUG - fpga_rb_update_all_params: skipped not modified parameter (name=%s)\n", p[idx].name);
            continue;  // this value is not marked to update the FPGA
        }
        fprintf(stderr, "DEBUG - fpga_rb_update_all_params: this parameter has to update the FPGA (name=%s)\n", p[idx].name);

        /* Remove the marker */
        p[idx].fpga_update &= ~0x80;

        /* Get current parameters from the worker */
        {
            //fprintf(stderr, "INFO - fpga_rb_update_all_params: waiting for cb_out_params ...\n");
            pthread_mutex_lock(&g_rb_info_worker_params_mutex);
            if (g_rb_info_worker_params) {
                //print_rb_params(rb_info_worker_params);
                loc_rb_run      = (int) g_rb_info_worker_params[RB_RUN].value;
                loc_modsrc      = (int) g_rb_info_worker_params[RB_OSC_CAR_MODSRC].value;
                loc_modtyp      = (int) g_rb_info_worker_params[RB_OSC_CAR_MODTYP].value;
                loc_led_ctrl    = (int) g_rb_info_worker_params[RB_LED_CTRL].value;
                loc_osc_car_qrg = g_rb_info_worker_params[RB_OSC_CAR_QRG].value;
                loc_osc_mod_qrg = g_rb_info_worker_params[RB_OSC_MOD_QRG].value;
                loc_osc_car_amp = g_rb_info_worker_params[RB_OSC_CAR_AMP].value;
                loc_osc_mod_mag = g_rb_info_worker_params[RB_OSC_MOD_MAG].value;
                loc_muxin_gain  = g_rb_info_worker_params[RB_MUXIN_GAIN].value;
            }
            pthread_mutex_unlock(&g_rb_info_worker_params_mutex);
            //fprintf(stderr, "INFO - fpga_rb_update_all_params: ... done\n");
        }

        /* Since here process on each known parameter accordingly */

        if (!strcmp("rb_run", p[idx].name)) {
            fprintf(stderr, "INFO - fpga_rb_update_all_params: #got rb_run = %d\n", (int) (p[idx].value));
            fpga_rb_enable((int) (p[idx].value));
            fpga_rb_set_ctrl((int) p[idx].value, loc_modsrc, loc_modtyp, loc_led_ctrl, loc_osc_car_qrg, loc_osc_mod_qrg, loc_osc_car_amp, loc_osc_mod_mag, loc_muxin_gain);

        } else if (!strcmp("osc1_modsrc_s", p[idx].name)) {
            fprintf(stderr, "INFO - fpga_rb_update_all_params: #got osc1_modsrc_s = %d\n", (int) (p[idx].value));
            fpga_rb_set_ctrl(loc_rb_run, (int) (p[idx].value), loc_modtyp, loc_led_ctrl, loc_osc_car_qrg, loc_osc_mod_qrg, loc_osc_car_amp, loc_osc_mod_mag, loc_muxin_gain);

        } else if (!strcmp("osc1_modtyp_s", p[idx].name)) {
            fprintf(stderr, "INFO - fpga_rb_update_all_params: #got osc1_modtyp_s = %d\n", (int) (p[idx].value));
            fpga_rb_set_ctrl(loc_rb_run, loc_modsrc, (int) (p[idx].value), loc_led_ctrl, loc_osc_car_qrg, loc_osc_mod_qrg, loc_osc_car_amp, loc_osc_mod_mag, loc_muxin_gain);

        } else if (!strcmp("loc_led_ctrl_s", p[idx].name)) {
            fprintf(stderr, "INFO - fpga_rb_update_all_params: #got led_ctrl_s = %d\n", (int) (p[idx].value));
            fpga_rb_set_ctrl(loc_rb_run, loc_modsrc, loc_modtyp, (int) (p[idx].value), loc_osc_car_qrg, loc_osc_mod_qrg, loc_osc_car_amp, loc_osc_mod_mag, loc_muxin_gain);

        } else if (!strcmp("osc1_qrg_f", p[idx].name)) {
            fprintf(stderr, "INFO - fpga_rb_update_all_params: #got osc1_qrg_f = %lf\n", p[idx].value);
            fpga_rb_set_ctrl(loc_rb_run, loc_modsrc, loc_modtyp, loc_led_ctrl, p[idx].value, loc_osc_mod_qrg, loc_osc_car_amp, loc_osc_mod_mag, loc_muxin_gain);

        } else if (!strcmp("osc2_qrg_f", p[idx].name)) {
            fprintf(stderr, "INFO - fpga_rb_update_all_params: #got osc2_qrg_f = %lf\n", p[idx].value);
            fpga_rb_set_ctrl(loc_rb_run, loc_modsrc, loc_modtyp, loc_led_ctrl, loc_osc_car_qrg, p[idx].value, loc_osc_car_amp, loc_osc_mod_mag, loc_muxin_gain);

        } else if (!strcmp("osc1_amp_f", p[idx].name)) {
            fprintf(stderr, "INFO - fpga_rb_update_all_params: #got osc1_amp_f = %lf\n", p[idx].value);
            fpga_rb_set_ctrl(loc_rb_run, loc_modsrc, loc_modtyp, loc_led_ctrl, loc_osc_car_qrg, loc_osc_mod_qrg, p[idx].value, loc_osc_mod_mag, loc_muxin_gain);

        } else if (!strcmp("osc2_mag_f", p[idx].name)) {
            fprintf(stderr, "INFO - fpga_rb_update_all_params: #got osc2_mag_f = %lf\n", p[idx].value);
            fpga_rb_set_ctrl(loc_rb_run, loc_modsrc, loc_modtyp, loc_led_ctrl, loc_osc_car_qrg, loc_osc_mod_qrg, loc_osc_car_amp, p[idx].value, loc_muxin_gain);

        } else if (!strcmp("muxin_gain_f", p[idx].name)) {
            fprintf(stderr, "INFO - fpga_rb_update_all_params: #got muxin_gain_f = %lf\n", p[idx].value);
            fpga_rb_set_ctrl(loc_rb_run, loc_modsrc, loc_modtyp, loc_led_ctrl, loc_osc_car_qrg, loc_osc_mod_qrg, loc_osc_car_amp, loc_osc_mod_mag, p[idx].value);
        }  // else if ()

    }  // for ()

    //fprintf(stderr, "fpga_rb_update_all_params: END\n");
    return 0;
}


/*----------------------------------------------------------------------------*/
void fpga_rb_set_ctrl(int rb_run, int modsrc, int modtyp, int led_ctrl, double osc1_qrg, double osc2_qrg, double osc1_amp, double osc2_mag, double muxin_gain)
{
    //fprintf(stderr, "INFO - fpga_rb_set_ctrl: rb_run=%d, modsrc=%d, modtyp=%d, osc1_qrg=%lf, osc2_qrg=%lf, osc1_amp=%lf, osc2_mag=%lf\n",
    //        rb_run, modsrc, modtyp, osc1_qrg, osc2_qrg, osc1_amp, osc2_mag);

    g_fpga_rb_reg_mem->led_ctrl = led_ctrl;

    if (rb_run) {
        fpga_rb_set_osc_car_mod_none_am_pm(osc1_qrg);                                                      // OSC1 frequency
        fpga_rb_set_osc_mod_mod_am_fm_pm(osc2_qrg);                                                        // OSC2 frequency
        fpga_rb_set_osc_car_mixer_mod_none_fm_pm(osc1_amp);                                                // OSC1 mixer
        fpga_rb_set_muxin_gain(muxin_gain);                                                             // MUXIN gain setting

        switch (modtyp) {

        case RB_MODTYP_AM: {
            fprintf(stderr, "INFO - fpga_rb_set_ctrl: setting FPGA for AM\n");

            g_fpga_rb_reg_mem->ctrl &= ~0x00000060;                                                     // control: turn off all other streams
            g_fpga_rb_reg_mem->ctrl |=  0x00000080;                                                     // control: AM by OSC1 mixer amplitude streaming
            fpga_rb_set_osc_mod_mixer_mod_am(osc1_amp, osc2_mag);                                          // AM by streaming in amplitude
        }
        break;

        case RB_MODTYP_FM: {
            fprintf(stderr, "INFO - fpga_rb_set_ctrl: setting FPGA for FM\n");

            g_fpga_rb_reg_mem->ctrl &= ~0x000000c0;                                                     // control: turn off all other streams
            g_fpga_rb_reg_mem->ctrl |=  0x00000020;                                                     // control: FM by OSC1 increment streaming
            fpga_rb_set_osc_mod_mixer_mod_fm(osc1_qrg, osc2_mag);                                          // FM by streaming in DDS increment
        }
        break;

        case RB_MODTYP_PM: {
            fprintf(stderr, "INFO - fpga_rb_set_ctrl: setting FPGA for PM\n");

            g_fpga_rb_reg_mem->ctrl &= ~0x000000a0;                                                     // control: turn off all other streams
            g_fpga_rb_reg_mem->ctrl |=  0x00000040;                                                     // control: PM by OSC1 offset streaming
            fpga_rb_set_osc_mod_mixer_mod_pm(osc1_qrg, osc2_mag);                                          // PM by streaming in DDS phase offset
        }
        break;

        }


        switch (modsrc) {

        default:
        case RB_MODSRC_NONE: {
            fprintf(stderr, "INFO - fpga_rb_set_ctrl: setting FPGA modsrc to (none)\n");

            g_fpga_rb_reg_mem->ctrl &= ~0x000000e0;                                                     // control: turn off all streams into OSC1 and OSC1 mixer
            g_fpga_rb_reg_mem->muxin_src = 0x00000000;
        }
        break;

        case RB_MODSRC_OSC_MOD: {
            fprintf(stderr, "INFO - fpga_rb_set_ctrl: setting FPGA modsrc to (none)\n");

            g_fpga_rb_reg_mem->muxin_src = 0x00000000;
        }
        break;

        case RB_MODSRC_RF_IN1: {
            fprintf(stderr, "INFO - fpga_rb_set_ctrl: setting FPGA modsrc to RF_inp_1\n");

            g_fpga_rb_reg_mem->muxin_src = 0x00000020;                                                  // source ID: 32
        }
        break;

        case RB_MODSRC_RF_IN2: {
            fprintf(stderr, "INFO - fpga_rb_set_ctrl: setting FPGA modsrc to RF_inp_2\n");

            g_fpga_rb_reg_mem->muxin_src = 0x00000021;                                                  // source ID: 33
        }
        break;

        case RB_MODSRC_EXP_AI0: {
            fprintf(stderr, "INFO - fpga_rb_set_ctrl: setting FPGA modsrc to AI0\n");

            g_fpga_rb_reg_mem->muxin_src = 0x00000010;                                                  // source ID: 16
        }
        break;

        case RB_MODSRC_EXP_AI1: {
            fprintf(stderr, "INFO - fpga_rb_set_ctrl: setting FPGA modsrc to AI1\n");

            g_fpga_rb_reg_mem->muxin_src = 0x00000018;                                                  // source ID: 24
        }
        break;

        case RB_MODSRC_EXP_AI2: {
            fprintf(stderr, "INFO - fpga_rb_set_ctrl: setting FPGA modsrc to AI2\n");

            g_fpga_rb_reg_mem->muxin_src = 0x00000011;                                                  // source ID: 17
        }
        break;

        case RB_MODSRC_EXP_AI3: {
            fprintf(stderr, "INFO - fpga_rb_set_ctrl: setting FPGA modsrc to AI3\n");

            g_fpga_rb_reg_mem->muxin_src = 0x00000019;                                                  // source ID: 25
        }
        break;

#if 0
        case RB_MODSRC_VP_VN: {
            g_fpga_rb_reg_mem->muxin_src = 0x00000003;                                                  // source ID: 3
        }
        break;
#endif

        }  // switch ()

    } else {
        fpga_rb_set_osc_car_mod_none_am_pm(0.0);                                                           // OSC1 frequency
        fpga_rb_set_osc_mod_mod_am_fm_pm(0.0);                                                             // OSC2 frequency
        fpga_rb_set_osc_car_mixer_mod_none_fm_pm(0.0);                                                     // OSC1 mixer
        fpga_rb_set_osc_mod_mixer_mod_fm(0.0f, 0.0);                                                       // OSC2 mixer

        g_fpga_rb_reg_mem->ctrl &= ~0x000000e0;                                                         // control: turn off all streams into OSC1 and OSC1 mixer
    }
}

/*----------------------------------------------------------------------------*/
void fpga_rb_set_osc_car_mod_none_am_pm(double osc1_qrg)
{
    double qrg1 = 0.5 + (1ULL << 48) * (osc1_qrg / g_rp_main_calib_params.base_osc125mhz_realhz);

    //fprintf(stderr, "INFO - fpga_rb_set_osc1_mod_none_am_pm: qrg1=%lf (INC steps)  <--  in(osc1_qrg=%lf)\n",
    //        qrg1, osc1_qrg);

    g_fpga_rb_reg_mem->osc_car_inc_lo = (uint32_t) (((uint64_t) qrg1) & 0xffffffff);
    g_fpga_rb_reg_mem->osc_car_inc_hi = (uint32_t) (((uint64_t) qrg1) >> 32);
    g_fpga_rb_reg_mem->osc_car_ofs_lo = 0;                                                              // no carrier phase offset
    g_fpga_rb_reg_mem->osc_car_ofs_hi = 0;                                                              // no carrier phase offset
}

/*----------------------------------------------------------------------------*/
void fpga_rb_set_osc_car_mixer_mod_none_fm_pm(double osc1_amp)
{
//    double gain1 = 0.5 + ((double) (1ULL << 31)) * (osc1_amp / 2048.0);                                 // TODO: DAC amplitude correction goes into here
//    double ofs1  = 0.0f;                                                                                // TODO: DAC offset correction goes into here
//  double ofs1  = (1ULL << 46);                                                                          // TODO: DAC offset correction goes into here

    //fprintf(stderr, "INFO - fpga_rb_set_osc1_mixer_mod_none_fm_pm: in(gain1=%lf, ofs=%lf)\n", gain1, ofs1);

// TODO new settings
    /*
    g_fpga_rb_reg_mem->osc1_mix_gain = (uint32_t) (((uint64_t) gain1) & 0xffffffff);
    g_fpga_rb_reg_mem->osc1_mix_ofs_lo = (uint32_t) (((uint64_t) ofs1)  & 0xffffffff);
    g_fpga_rb_reg_mem->osc1_mix_ofs_hi = (uint32_t) (((uint64_t) ofs1) >> 32);
    */
}

/*----------------------------------------------------------------------------*/
void fpga_rb_set_osc_mod_mod_am_fm_pm(double osc2_qrg)
{
    double qrg2 = 0.5 + ((double) (1ULL << 48)) * (osc2_qrg / g_rp_main_calib_params.base_osc125mhz_realhz);

    //fprintf(stderr, "INFO - fpga_rb_set_osc2_mod_am_fm_pm: in(qrg2=%lf)\n", qrg2);

    g_fpga_rb_reg_mem->osc_mod_inc_lo = (uint32_t) (((uint64_t) qrg2) & 0xffffffff);
    g_fpga_rb_reg_mem->osc_mod_inc_hi = (uint32_t) (((uint64_t) qrg2) >> 32);
    g_fpga_rb_reg_mem->osc_mod_ofs_lo = 0;                                                              // no carrier phase offset
    g_fpga_rb_reg_mem->osc_mod_ofs_hi = 0;                                                              // no carrier phase offset
}

/*----------------------------------------------------------------------------*/
void fpga_rb_set_osc_mod_mixer_mod_am(double osc1_amp, double osc2_mag)
{
    double gain2 = 0.5 +  ((double) (1ULL << 30))                           * (osc2_mag / 100.0)  * (osc1_amp / 2048.0);
    double ofs2  = 0.5 + (((double) (1ULL << 47)) - ((double) (1ULL << 46)) * (osc2_mag / 100.0)) * (osc1_amp / 2048.0);

    //fprintf(stderr, "INFO - fpga_rb_set_osc2_mixer_mod_am: in(gain2=%lf, ofs2=%lf)\n", gain2, ofs2);

    g_fpga_rb_reg_mem->osc_mod_mix_gain   = (uint32_t) (((uint64_t) gain2) & 0xffffffff);
    g_fpga_rb_reg_mem->osc_mod_mix_ofs_lo = (uint32_t) (((uint64_t) ofs2)  & 0xffffffff);
    g_fpga_rb_reg_mem->osc_mod_mix_ofs_hi = (uint32_t) (((uint64_t) ofs2) >> 32);
}

/*----------------------------------------------------------------------------*/
void fpga_rb_set_osc_mod_mixer_mod_fm(double osc1_qrg, double osc2_mag)
{
    double gain2 = 0.5 + ((double) (1ULL << 32)) * (osc2_mag / g_rp_main_calib_params.base_osc125mhz_realhz);
    double ofs2  = 0.5 + ((double) (1ULL << 48)) * (osc1_qrg / g_rp_main_calib_params.base_osc125mhz_realhz);

    //fprintf(stderr, "INFO - fpga_rb_set_osc2_mixer_mod_fm: in(gain2=%lf, ofs2=%lf)\n", gain2, ofs2);

    g_fpga_rb_reg_mem->osc_mod_mix_gain   = (uint32_t) (((uint64_t) gain2) & 0xffffffff);
    g_fpga_rb_reg_mem->osc_mod_mix_ofs_lo = (uint32_t) (((uint64_t) ofs2)  & 0xffffffff);
    g_fpga_rb_reg_mem->osc_mod_mix_ofs_hi = (uint32_t) (((uint64_t) ofs2) >> 32);
}

/*----------------------------------------------------------------------------*/
void fpga_rb_set_osc_mod_mixer_mod_pm(double osc1_qrg, double osc2_mag)
{
    double gain2 = 0.5 + ((double) (1ULL << 31)) * (osc2_mag / 360.0);

    //fprintf(stderr, "INFO - fpga_rb_set_osc2_mixer_mod_pm: osc1_qrg=%lf, osc2_mag=%lf\n", osc1_qrg, osc2_mag);

    g_fpga_rb_reg_mem->osc_mod_mix_gain   = (uint32_t) (((uint64_t) gain2) & 0xffffffff);
    g_fpga_rb_reg_mem->osc_mod_mix_ofs_lo = (uint32_t) 0;
    g_fpga_rb_reg_mem->osc_mod_mix_ofs_hi = (uint32_t) 0;
}

/*----------------------------------------------------------------------------*/
void fpga_rb_set_muxin_gain(double muxin_gain)
{
    double p;

    if (muxin_gain > 100.0) {
        muxin_gain = 100.0;
    }

    if (muxin_gain <= 0.0) {
        g_fpga_rb_reg_mem->muxin_gain = 0;

    } else if (muxin_gain <= 80.0) {  // 0% .. 80%
        p = muxin_gain * ((double) 0xffff) / 80.0;
        g_fpga_rb_reg_mem->muxin_gain = (((uint32_t) p) & 0xffff);

    } else {  // 80%+ .. 100%: set the logarithmic amplifier
        p  = (muxin_gain - 80.0) * (7.0 / 20.0);
        uint32_t bitfield = ((int) p) * 0b111;
        g_fpga_rb_reg_mem->muxin_gain = (bitfield | 0xffff);  // open mixer completely and activate booster
    }
}


#if 0
/*----------------------------------------------------------------------------*/
/**
 * @brief Reads value from the specific RadioBox sub-module register
 *
 * @param[in] rb_reg_ofs  offset value for the RadioBox base address to be written to.
 *
 * @retval  value of the specified register.
 */
uint32_t fpga_rb_read_register(unsigned int rb_reg_ofs)
{
    fprintf(stderr, "fpga_rb_read_register: BEGIN\n");
    if (!g_fpga_rb_reg_mem) {
        return -1;
    }

    uint32_t value = *((uint32_t*) ((void*) g_fpga_rb_reg_mem) + rb_reg_ofs);
    fprintf(stderr, "fpga_rb_read_register: ofs=0x%06x --> read=0x%08x\n", rb_reg_ofs, value);
    fprintf(stderr, "fpga_rb_read_register: END\n");
    return value;
}

/*----------------------------------------------------------------------------*/
/**
 * @brief Writes value to the specific RadioBox sub-module register
 *
 * @param[in] rb_reg_ofs  offset value for the RadioBox base address to be written to.
 * @param[in] value  value that is written to the specified register.
 *
 * @retval  0 Success
 * @retval -1 Failure, error message is output on standard error device
 */
int fpga_rb_write_register(unsigned int rb_reg_ofs, uint32_t value)
{
    fprintf(stderr, "fpga_rb_write_register: BEGIN\n");

    if (!g_fpga_rb_reg_mem) {
        return -1;
    }

    //fprintf(stderr, "INFO fpga_rb_write_register: Compare LED access: %p, calced=%p\n", &(g_fpga_rb_reg_mem->led_ctrl), ((void*) g_fpga_rb_reg_mem) + rb_reg_ofs);

    fprintf(stderr, "fpga_rb_write_register: ofs=0x%06x <-- write=0x%08x\n", rb_reg_ofs, value);
    *((uint32_t*) ((void*) g_fpga_rb_reg_mem) + rb_reg_ofs) = value;

    fprintf(stderr, "fpga_rb_write_register: END\n");
    return 0;
}
#endif
