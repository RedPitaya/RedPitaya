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

        g_fpga_rb_reg_mem->src_con_pnt = 0x001C0000;    // disable RB LEDs, set RFOUT1 to AMP_RF output and RFOUT2 to silence
        g_fpga_rb_reg_mem->muxin_gain  = 0x00007FFF;    // open Mic gain 1:1 (FS = 2Vpp) = 80 % Mic gain setting

        g_fpga_rb_reg_mem->amp_rf_gain = 0x00000C80;    // open RF output at -10 dBm (= 200 mVpp @ 50 Ohm)
        g_fpga_rb_reg_mem->amp_rf_ofs  = 0;             // no corrections done

    } else {
        //fprintf(stderr, "fpga_rb_enable: turning off RB LEDs\n");
        g_fpga_rb_reg_mem->src_con_pnt = 0x00000000;    // disable RB LEDs, RFOUT1 and RFOUT2
        g_fpga_rb_reg_mem->muxin_gain  = 0x00000000;    // shut Mic input

        g_fpga_rb_reg_mem->amp_rf_gain = 0;             // no output

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

    /* control: turn off all streams into CAR_OSC, MOD_OSC and QMIX_CAR Q for SSB */
    g_fpga_rb_reg_mem->ctrl &= ~0x00106060;

    /* reset all registers of the MOD_OSC to get fixed phase of 0 deg */
    g_fpga_rb_reg_mem->mod_osc_inc_hi = 0;
    g_fpga_rb_reg_mem->mod_osc_inc_lo = 0;
    g_fpga_rb_reg_mem->mod_osc_ofs_hi = 0;
    g_fpga_rb_reg_mem->mod_osc_ofs_hi = 0;

    /* reset all registers of the CAR_OSC to get fixed phase of 0 deg */
    g_fpga_rb_reg_mem->car_osc_inc_hi = 0;
    g_fpga_rb_reg_mem->car_osc_inc_lo = 0;
    g_fpga_rb_reg_mem->car_osc_ofs_hi = 0;
    g_fpga_rb_reg_mem->car_osc_ofs_hi = 0;


    /* send resync to CAR_OSC and MOD_OSC to zero phase registers */
    g_fpga_rb_reg_mem->ctrl = 0x00001011;

    /* send resync and reset to CAR_OSC and MOD_OSC */
    g_fpga_rb_reg_mem->ctrl = 0x00001017;

    /* send resync to CAR_OSC and MOD_OSC */
    g_fpga_rb_reg_mem->ctrl = 0x00001011;

    /* run mode of both oscillators */
    g_fpga_rb_reg_mem->ctrl = 0x00000001;
}


/*----------------------------------------------------------------------------*/
int fpga_rb_update_all_params(rb_app_params_t* p)
{
    int    loc_rb_run      = 0;
    int    loc_modsrc      = 0;
    int    loc_modtyp      = 0;
    int    loc_led_csp     = 0;
    int    loc_rfout1_csp  = 0;
    int    loc_rfout2_csp  = 0;
    double loc_car_osc_qrg = 0.0;
    double loc_mod_osc_qrg = 0.0;
    double loc_amp_rf_gain = 0.0;
    double loc_mod_osc_mag = 0.0;
    double loc_muxin_gain  = 0.0;

    //fprintf(stderr, "fpga_rb_update_all_params: BEGIN\n");

    if (!g_fpga_rb_reg_mem || !p) {
        fprintf(stderr, "ERROR - fpga_rb_update_all_params: bad parameter (p=%p) or not init'ed(g=%p)\n", p, g_fpga_rb_reg_mem);
        return -1;
    }

    /* Get current parameters from the worker */
    {
        //fprintf(stderr, "INFO - fpga_rb_update_all_params: waiting for cb_out_params ...\n");
        pthread_mutex_lock(&g_rb_info_worker_params_mutex);
        if (g_rb_info_worker_params) {
            //print_rb_params(rb_info_worker_params);
            loc_rb_run      = (int) g_rb_info_worker_params[RB_RUN].value;
            loc_modsrc      = (int) g_rb_info_worker_params[RB_CAR_OSC_MODSRC].value;
            loc_modtyp      = (int) g_rb_info_worker_params[RB_CAR_OSC_MODTYP].value;
            loc_led_csp     = (int) g_rb_info_worker_params[RB_LED_CON_SRC_PNT].value;
            loc_rfout1_csp  = (int) g_rb_info_worker_params[RB_RFOUT1_CON_SRC_PNT].value;
            loc_rfout2_csp  = (int) g_rb_info_worker_params[RB_RFOUT2_CON_SRC_PNT].value;
            loc_car_osc_qrg = g_rb_info_worker_params[RB_CAR_OSC_QRG].value;
            loc_mod_osc_qrg = g_rb_info_worker_params[RB_MOD_OSC_QRG].value;
            loc_amp_rf_gain = g_rb_info_worker_params[RB_AMP_RF_GAIN].value;
            loc_mod_osc_mag = g_rb_info_worker_params[RB_MOD_OSC_MAG].value;
            loc_muxin_gain  = g_rb_info_worker_params[RB_MUXIN_GAIN].value;
        }
        pthread_mutex_unlock(&g_rb_info_worker_params_mutex);
        //fprintf(stderr, "INFO - fpga_rb_update_all_params: ... done\n");
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

        /* Since here process on each known parameter accordingly */
        if (!strcmp("rb_run", p[idx].name)) {
            fprintf(stderr, "INFO - fpga_rb_update_all_params: #got rb_run = %d\n", (int) (p[idx].value));
            loc_rb_run = ((int) (p[idx].value));

        } else if (!strcmp("car_osc_modsrc_s", p[idx].name)) {
            fprintf(stderr, "INFO - fpga_rb_update_all_params: #got car_osc_modsrc_s = %d\n", (int) (p[idx].value));
            loc_modsrc = ((int) (p[idx].value));

        } else if (!strcmp("car_osc_modtyp_s", p[idx].name)) {
            fprintf(stderr, "INFO - fpga_rb_update_all_params: #got car_osc_modtyp_s = %d\n", (int) (p[idx].value));
            loc_modtyp = ((int) (p[idx].value));


        } else if (!strcmp("rbled_csp_s", p[idx].name)) {
            fprintf(stderr, "INFO - fpga_rb_update_all_params: #got rbled_csp_s = %d\n", (int) (p[idx].value));
            loc_led_csp = ((int) (p[idx].value));

        } else if (!strcmp("rfout1_csp_s", p[idx].name)) {
            fprintf(stderr, "INFO - fpga_rb_update_all_params: #got rfout1_csp_s = %d\n", (int) (p[idx].value));
            loc_rfout1_csp = ((int) (p[idx].value));

        } else if (!strcmp("rfout2_csp_s", p[idx].name)) {
            fprintf(stderr, "INFO - fpga_rb_update_all_params: #got rfout2_csp_s = %d\n", (int) (p[idx].value));
            loc_rfout2_csp = ((int) (p[idx].value));


        } else if (!strcmp("car_osc_qrg_f", p[idx].name)) {
            fprintf(stderr, "INFO - fpga_rb_update_all_params: #got car_osc_qrg_f = %lf\n", p[idx].value);
            loc_car_osc_qrg = p[idx].value;

        } else if (!strcmp("mod_osc_qrg_f", p[idx].name)) {
            fprintf(stderr, "INFO - fpga_rb_update_all_params: #got mod_osc_qrg_f = %lf\n", p[idx].value);
            loc_mod_osc_qrg = p[idx].value;


        } else if (!strcmp("amp_rf_gain_f", p[idx].name)) {
            fprintf(stderr, "INFO - fpga_rb_update_all_params: #got amp_rf_gain_f = %lf\n", p[idx].value);
            loc_amp_rf_gain = p[idx].value;

        } else if (!strcmp("mod_osc_mag_f", p[idx].name)) {
            fprintf(stderr, "INFO - fpga_rb_update_all_params: #got mod_osc_mag_f = %lf\n", p[idx].value);
            loc_mod_osc_mag = p[idx].value;


        } else if (!strcmp("muxin_gain_f", p[idx].name)) {
            fprintf(stderr, "INFO - fpga_rb_update_all_params: #got muxin_gain_f = %lf\n", p[idx].value);
            loc_muxin_gain = p[idx].value;
        }  // else if ()
    }  // for ()

    /* set the new values */
    {
        fpga_rb_enable(loc_rb_run);
        if (loc_rb_run) {
            fpga_rb_set_ctrl(
                    loc_rb_run,
                    loc_modsrc,
                    loc_modtyp,
                    ((loc_rfout2_csp << 0x18) & 0x3f000000) | ((loc_rfout1_csp << 0x10) & 0x003f0000) | (loc_led_csp & 0x0000003f),
                    loc_car_osc_qrg,
                    loc_mod_osc_qrg,
                    loc_amp_rf_gain,
                    loc_mod_osc_mag,
                    loc_muxin_gain);
        }
    }

    //fprintf(stderr, "fpga_rb_update_all_params: END\n");
    return 0;
}


/*----------------------------------------------------------------------------*/
void fpga_rb_set_ctrl(int rb_run, int modsrc, int modtyp, int src_con_pnt, double car_osc_qrg, double mod_osc_qrg, double amp_rf_gain, double mod_osc_mag, double muxin_gain)
{
    const int ssb_weaver_osc_qrg = 1700.0;

    //fprintf(stderr, "INFO - fpga_rb_set_ctrl: rb_run=%d, modsrc=%d, modtyp=%d, src_con_pnt=%d, car_osc_qrg=%lf, mod_osc_qrg=%lf, amp_rf_gain=%lf, mod_osc_mag=%lf, muxin_gain=%lf\n",
    //        rb_run, modsrc, modtyp, src_con_pnt, car_osc_qrg, mod_osc_qrg, amp_rf_gain, mod_osc_mag, muxin_gain);

    if ((g_fpga_rb_reg_mem->src_con_pnt) != src_con_pnt) {
        fprintf(stderr, "INFO - fpga_rb_set_ctrl: setting src_con_pnt to new value = %d\n", src_con_pnt);
        g_fpga_rb_reg_mem->src_con_pnt = src_con_pnt;
    }

    if (rb_run) {
        fpga_rb_reset();
        fpga_rb_set_amp_rf_gain_ofs__4mod_all(amp_rf_gain, 0.0);                                        // AMP_RF gain setting [mV] is global and not modulation dependent


        switch (modsrc) {

        default:
        case RB_MODSRC_NONE: {
            fprintf(stderr, "INFO - fpga_rb_set_ctrl: setting FPGA modsrc to (none)\n");

            fpga_rb_set_muxin_gain(0.0);                                                                // MUXIN gain setting
            g_fpga_rb_reg_mem->muxin_src = 0x00000000;
            fpga_rb_set_car_osc_qrg__4mod_cw_ssb_am_pm(car_osc_qrg);                                    // CAR_OSC frequency
            fpga_rb_set_mod_qmix_gain_ofs__4mod_cw_ssbweaver_am(0.0, 1);                                // CW operation
        }
        break;

        case RB_MODSRC_MOD_OSC: {
            fprintf(stderr, "INFO - fpga_rb_set_ctrl: setting FPGA modsrc to MOD_OSC\n");

            fpga_rb_set_muxin_gain(0.0);                                                                // MUXIN gain setting
            g_fpga_rb_reg_mem->muxin_src = 0x00000000;
        }
        break;

        case RB_MODSRC_RF_IN1: {
            fprintf(stderr, "INFO - fpga_rb_set_ctrl: setting FPGA modsrc to RF_inp_1\n");

            fpga_rb_set_muxin_gain(muxin_gain);                                                         // MUXIN gain setting
            g_fpga_rb_reg_mem->muxin_src = 0x00000020;                                                  // source ID: 32
        }
        break;

        case RB_MODSRC_RF_IN2: {
            fprintf(stderr, "INFO - fpga_rb_set_ctrl: setting FPGA modsrc to RF_inp_2\n");

            fpga_rb_set_muxin_gain(muxin_gain);                                                         // MUXIN gain setting
            g_fpga_rb_reg_mem->muxin_src = 0x00000021;                                                  // source ID: 33
        }
        break;

        case RB_MODSRC_EXP_AI0: {
            fprintf(stderr, "INFO - fpga_rb_set_ctrl: setting FPGA modsrc to AI0\n");

            fpga_rb_set_muxin_gain(muxin_gain);                                                         // MUXIN gain setting
            g_fpga_rb_reg_mem->muxin_src = 0x00000010;                                                  // source ID: 16
        }
        break;

        case RB_MODSRC_EXP_AI1: {
            fprintf(stderr, "INFO - fpga_rb_set_ctrl: setting FPGA modsrc to AI1\n");

            fpga_rb_set_muxin_gain(muxin_gain);                                                         // MUXIN gain setting
            g_fpga_rb_reg_mem->muxin_src = 0x00000018;                                                  // source ID: 24
        }
        break;

        case RB_MODSRC_EXP_AI2: {
            fprintf(stderr, "INFO - fpga_rb_set_ctrl: setting FPGA modsrc to AI2\n");

            fpga_rb_set_muxin_gain(muxin_gain);                                                         // MUXIN gain setting
            g_fpga_rb_reg_mem->muxin_src = 0x00000011;                                                  // source ID: 17
        }
        break;

        case RB_MODSRC_EXP_AI3: {
            fprintf(stderr, "INFO - fpga_rb_set_ctrl: setting FPGA modsrc to AI3\n");

            fpga_rb_set_muxin_gain(muxin_gain);                                                         // MUXIN gain setting
            g_fpga_rb_reg_mem->muxin_src = 0x00000019;                                                  // source ID: 25
        }
        break;

#if 0
        case RB_MODSRC_VP_VN: {
            fpga_rb_set_muxin_gain(muxin_gain);                                                         // MUXIN gain setting
            g_fpga_rb_reg_mem->muxin_src = 0x00000003;                                                  // source ID: 3
        }
        break;
#endif

        }  // switch (modsrc)


        if (modsrc != RB_MODSRC_NONE) {
            switch (modtyp) {

            case RB_MODTYP_USB: {
                fprintf(stderr, "INFO - fpga_rb_set_ctrl: setting FPGA for USB\n");

                g_fpga_rb_reg_mem->ctrl |=  0x00100000;                                                 // control: enabling QMIX_CAR Q path in the AMP_RF
                fpga_rb_set_car_osc_qrg__4mod_cw_ssb_am_pm(car_osc_qrg + ssb_weaver_osc_qrg);           // CAR_OSC frequency with ssb_weaver_osc_qrg correction
                fpga_rb_set_mod_osc_qrg__4mod_ssbweaver_am_fm_pm(+ssb_weaver_osc_qrg);                  // MOD_OSC weaver method mixer LO frequency
                fpga_rb_set_mod_qmix_gain_ofs__4mod_cw_ssbweaver_am(mod_osc_mag, 0);                    // SSB operation has no carrier
            }
            break;

            case RB_MODTYP_LSB: {
                fprintf(stderr, "INFO - fpga_rb_set_ctrl: setting FPGA for LSB\n");

                g_fpga_rb_reg_mem->ctrl |=  0x00100000;                                                 // control: enabling QMIX_CAR Q path in the AMP_RF
                fpga_rb_set_car_osc_qrg__4mod_cw_ssb_am_pm(car_osc_qrg - ssb_weaver_osc_qrg);           // CAR_OSC frequency with ssb_weaver_osc_qrg correction
                fpga_rb_set_mod_osc_qrg__4mod_ssbweaver_am_fm_pm(-ssb_weaver_osc_qrg);                  // MOD_OSC weaver method mixer LO frequency
                fpga_rb_set_mod_qmix_gain_ofs__4mod_cw_ssbweaver_am(mod_osc_mag, 0);                    // SSB operation has no carrier
            }
            break;

            case RB_MODTYP_AM: {
                fprintf(stderr, "INFO - fpga_rb_set_ctrl: setting FPGA for AM\n");

                fpga_rb_set_car_osc_qrg__4mod_cw_ssb_am_pm(car_osc_qrg);                                // CAR_OSC frequency
                if (modsrc == RB_MODSRC_MOD_OSC) {
                    fpga_rb_set_mod_osc_qrg__4mod_ssbweaver_am_fm_pm(mod_osc_qrg);                      // MOD_OSC frequency
                }
                fpga_rb_set_mod_qmix_gain_ofs__4mod_cw_ssbweaver_am(mod_osc_mag, 1);                    // AM by streaming in amplitude
            }
            break;

            case RB_MODTYP_FM: {
                fprintf(stderr, "INFO - fpga_rb_set_ctrl: setting FPGA for FM\n");

                g_fpga_rb_reg_mem->ctrl |=  0x00000020;                                                 // control: FM by CAR_OSC increment streaming
                if (modsrc == RB_MODSRC_MOD_OSC) {
                    fpga_rb_set_mod_osc_qrg__4mod_ssbweaver_am_fm_pm(mod_osc_qrg);                      // MOD_OSC frequency
                }
                fpga_rb_set_mod_qmix_gain_ofs__4mod_fm(car_osc_qrg, mod_osc_mag);                       // FM by streaming in DDS increment
            }
            break;

            case RB_MODTYP_PM: {
                fprintf(stderr, "INFO - fpga_rb_set_ctrl: setting FPGA for PM\n");

                g_fpga_rb_reg_mem->ctrl |=  0x00000040;                                                 // control: PM by CAR_OSC offset streaming
                fpga_rb_set_car_osc_qrg__4mod_cw_ssb_am_pm(car_osc_qrg);                                // CAR_OSC frequency
                if (modsrc == RB_MODSRC_MOD_OSC) {
                    fpga_rb_set_mod_osc_qrg__4mod_ssbweaver_am_fm_pm(mod_osc_qrg);                      // MOD_OSC frequency
                }
                fpga_rb_set_mod_qmix_gain_ofs__4mod_pm(car_osc_qrg, mod_osc_mag);                       // PM by streaming in DDS phase offset
            }
            break;

            }  // switch (modtyp)
        }  // if (modsrc != RB_MODSRC_NONE)

    } else {  // else if (rb_run)
        fpga_rb_set_amp_rf_gain_ofs__4mod_all(0.0, 0.0);                                                // AMP_RF gain/offset control
        fpga_rb_set_car_osc_qrg__4mod_cw_ssb_am_pm(0.0);                                                // CAR_OSC frequency
        fpga_rb_set_mod_osc_qrg__4mod_ssbweaver_am_fm_pm(0.0);                                          // MOD_OSC frequency
        fpga_rb_set_mod_qmix_gain_ofs__4mod_fm(0.0f, 0.0);                                              // MOD_QMIX gain/offset control
        fpga_rb_reset();                                                                                // control: turn off all streams into CAR_OSC and CAR_OSC mixer
    }
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
        //fprintf(stderr, "INFO - fpga_rb_set_muxin_gain: ZERO   muxin_gain=%lf --> bitfield=0x%08x\n", muxin_gain, g_fpga_rb_reg_mem->muxin_gain);

    } else if (muxin_gain < 80.0) {  // 0% .. 80%-
        uint32_t bitfield = (uint32_t) (0.5 + (muxin_gain * ((double) 0x7fff) / 80.0));
        g_fpga_rb_reg_mem->muxin_gain = (0x7fff & bitfield);  // 16 bit gain value and no booster shift bits
        //fprintf(stderr, "INFO - fpga_rb_set_muxin_gain: NORMAL muxin_gain=%lf --> bitfield=0x%08x\n", muxin_gain, g_fpga_rb_reg_mem->muxin_gain);

    } else {  // 80% .. 100%: set the logarithmic amplifier
        p  = (muxin_gain - 80.0) * (7.0 / 20.0);
        uint32_t bitfield = (uint32_t) (0.5 + p);
        g_fpga_rb_reg_mem->muxin_gain = ((bitfield << 16) | 0x7fff);  // open mixer completely and activate booster
        //fprintf(stderr, "INFO - fpga_rb_set_muxin_gain: BOOST  muxin_gain=%lf --> bitfield=0x%08x\n", muxin_gain, g_fpga_rb_reg_mem->muxin_gain);
    }
}

/*----------------------------------------------------------------------------*/
void fpga_rb_set_mod_osc_qrg__4mod_ssbweaver_am_fm_pm(double mod_osc_qrg)
{
    double qrg = ((double) (1ULL << 48)) * (mod_osc_qrg / g_rp_main_calib_params.base_osc125mhz_realhz);
    if (qrg > 0.0) {
        qrg += 0.5;
    } else if (qrg < 0.0) {
        qrg -= -0.5;
    }

    int64_t bitfield = qrg;
    uint32_t bf_hi = (uint32_t) (bitfield >> 32);
    uint32_t bf_lo = (uint32_t) (bitfield & 0xffffffff);

    fprintf(stderr, "INFO - fpga_rb_set_mod_osc_qrg__4mod_ssbweaver_am_fm_pm: (qrg=%lf, HI=0x%08x, LO=0x%08x) <-- in(mod_osc_qrg=%lf)\n",
            qrg,
            bf_hi,
            bf_lo,
            mod_osc_qrg);

    g_fpga_rb_reg_mem->mod_osc_inc_lo = bf_lo;
    g_fpga_rb_reg_mem->mod_osc_inc_hi = bf_hi;
    g_fpga_rb_reg_mem->mod_osc_ofs_lo = 0UL;                                                            // no carrier phase offset
    g_fpga_rb_reg_mem->mod_osc_ofs_hi = 0UL;                                                            // no carrier phase offset
}

/*----------------------------------------------------------------------------*/
void fpga_rb_set_mod_qmix_gain_ofs__4mod_cw_ssbweaver_am(double mod_qmix_grade, int isOffset)
{
    double gain, ofs;

    if (isOffset) {  // CW and AM modulation: reduced modulation by 1/2 and added offset to reach the maximum at the modulation peaks
        gain = ((double) 0x3fff) * (mod_qmix_grade / 100.0);
        ofs  = ((double) ((1ULL << 47) - 1)) - (((double) ((1ULL << 46) - 1)) * (mod_qmix_grade / 100.0));

    } else {  // SSB modulation: no offset but full modulation
        gain = ((double) 0x7fff) * (mod_qmix_grade / 100.0);
        ofs  = 0.0;
    }

    fprintf(stderr, "INFO - fpga_rb_set_mod_qmix_gain_ofs__4mod_cw_am: (gain=%lf, ofs=%lf) <-- in(mod_qmix_grade=%lf)\n",
            gain, ofs, mod_qmix_grade);

    g_fpga_rb_reg_mem->mod_qmix_gain   = ((uint32_t) gain) & 0x7fff;
    g_fpga_rb_reg_mem->mod_qmix_ofs_lo = (uint32_t) (((uint64_t) ofs)  & 0xffffffff);                   // CW, and AM have carrier enabled,
    g_fpga_rb_reg_mem->mod_qmix_ofs_hi = (uint32_t) (((uint64_t) ofs) >> 32);                           // SSB is zero symmetric w/o a carrier
}

/*----------------------------------------------------------------------------*/
void fpga_rb_set_mod_qmix_gain_ofs__4mod_fm(double car_osc_qrg, double mod_osc_mag)
{
    double gain = 0.5 + ((double) 0x7fff) * ((1UL << 14) * mod_osc_mag / g_rp_main_calib_params.base_osc125mhz_realhz);
    double ofs  = 0.5 + ((double) (1ULL << 48)) * (car_osc_qrg / g_rp_main_calib_params.base_osc125mhz_realhz);
    fprintf(stderr, "INFO - fpga_rb_set_mod_qmix_gain_ofs__4mod_fm: (gain=%lf, ofs=%lf) <-- in(car_osc_qrg=%lf, mod_osc_mag=%lf)\n",
            gain, ofs, car_osc_qrg, mod_osc_mag);

    g_fpga_rb_reg_mem->mod_qmix_gain   = ((uint32_t) gain) & 0x7fff;                                    // FM deviation
    g_fpga_rb_reg_mem->mod_qmix_ofs_lo = (uint32_t) (((uint64_t) ofs)  & 0xffffffff);                   // FM carrier frequency
    g_fpga_rb_reg_mem->mod_qmix_ofs_hi = (uint32_t) (((uint64_t) ofs) >> 32);
}

/*----------------------------------------------------------------------------*/
void fpga_rb_set_mod_qmix_gain_ofs__4mod_pm(double car_osc_qrg, double mod_osc_mag)
{
    double gain = 0.5 + ((double) 0x7fff) * (mod_osc_mag / 360.0);
    fprintf(stderr, "INFO - fpga_rb_set_mod_osc_mixer_mod_pm: car_osc_qrg=%lf, mod_osc_mag=%lf\n",
            car_osc_qrg, mod_osc_mag);

    g_fpga_rb_reg_mem->mod_qmix_gain   = ((uint32_t) gain) & 0x7fff;                                    // PM phase magnitude
    g_fpga_rb_reg_mem->mod_qmix_ofs_lo = 0UL;                                                           // PM based on zero phase w/o modulation
    g_fpga_rb_reg_mem->mod_qmix_ofs_hi = 0UL;
}

/*----------------------------------------------------------------------------*/
void fpga_rb_set_car_osc_qrg__4mod_cw_ssb_am_pm(double car_osc_qrg)
{
    double qrg = ((double) (1ULL << 48)) * (car_osc_qrg / g_rp_main_calib_params.base_osc125mhz_realhz);
    if (qrg > 0.0) {
        qrg += 0.5;
    } else if (qrg < 0.0) {
        qrg -= -0.5;
    }

    int64_t bitfield = qrg;
    uint32_t bf_hi = (uint32_t) (bitfield >> 32);
    uint32_t bf_lo = (uint32_t) (bitfield & 0xffffffff);

    fprintf(stderr, "INFO - fpga_rb_set_car_osc_qrg__4mod_none_ssb_am_pm: (qrg=%lf, HI=0x%08x, LO=0x%08x) <-- in(car_osc_qrg=%lf)\n",
            qrg,
            bf_hi,
            bf_lo,
            car_osc_qrg);

    g_fpga_rb_reg_mem->car_osc_inc_lo = bf_lo;
    g_fpga_rb_reg_mem->car_osc_inc_hi = bf_hi;
    g_fpga_rb_reg_mem->car_osc_ofs_lo = 0UL;                                                            // no carrier phase offset
    g_fpga_rb_reg_mem->car_osc_ofs_hi = 0UL;                                                            // no carrier phase offset
}

/*----------------------------------------------------------------------------*/
void fpga_rb_set_amp_rf_gain_ofs__4mod_all(double amp_rf_gain, double amp_rf_ofs)
{
    double gain = ((double) 0x7fff) * (amp_rf_gain / 2048.0);
    double ofs  = ((double) 0x7fff) * (amp_rf_ofs  / 2048.0);

    fprintf(stderr, "INFO - fpga_rb_set_amp_rf_gain_ofs__4mod_all: (gain=%lf, ofs=%lf) <-- in(amp_rf_gain=%lf, amp_rf_ofs=%lf)\n",
            gain, ofs, amp_rf_gain, amp_rf_ofs);

    g_fpga_rb_reg_mem->amp_rf_gain = ((uint32_t) gain) & 0xffff;
    g_fpga_rb_reg_mem->amp_rf_ofs  = ((uint32_t) ofs)  & 0xffff;
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

    //fprintf(stderr, "INFO fpga_rb_write_register: Compare LED access: %p, calced=%p\n", &(g_fpga_rb_reg_mem->src_con_pnt), ((void*) g_fpga_rb_reg_mem) + rb_reg_ofs);

    fprintf(stderr, "fpga_rb_write_register: ofs=0x%06x <-- write=0x%08x\n", rb_reg_ofs, value);
    *((uint32_t*) ((void*) g_fpga_rb_reg_mem) + rb_reg_ofs) = value;

    fprintf(stderr, "fpga_rb_write_register: END\n");
    return 0;
}
#endif
