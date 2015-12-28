/**
 * @brief Red Pitaya RadioBox main module.
 *
 * @author Ulrich Habel (DF4IAH) <espero7757@gmx.net>
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
//#include <math.h>
#include <errno.h>
#include <pthread.h>
//#include <sys/types.h>
#include <sys/mman.h>

#include "version.h"
#include "worker.h"
#include "fpga.h"
#include "cb_http.h"
#include "cb_ws.h"

#include "main.h"


#ifndef VERSION
# define VERSION "(not set)"
#endif
#ifndef REVISION
# define REVISION "(not set)"
#endif


/** @brief calibration data layout within the EEPROM device */
rp_calib_params_t               g_rp_main_calib_params;

/** @brief Holds last received transport frame index number and flag 0x80 for processing data */
unsigned char                   g_transport_pktIdx = 0;

/** @brief The system XADC memory file descriptor used to mmap() the FPGA space. */
int                             g_fpga_sys_xadc_mem_fd = -1;
/** @brief The system XADC memory layout of the FPGA registers. */
fpga_sys_xadc_reg_mem_t*        g_fpga_sys_xadc_reg_mem = NULL;

#if 0
/** @brief The system GPIO for LEDs memory file descriptor used to mmap() the FPGA space. */
int                             g_fpga_sys_gpio_leds_mem_fd = -1;
/** @brief The system GPIO for LEDs memory layout of the FPGA registers. */
fpga_sys_gpio_reg_mem_t*        g_fpga_sys_gpio_leds_reg_mem = NULL;
#endif

/** @brief HouseKeeping memory file descriptor used to mmap() the FPGA space */
int                             g_fpga_hk_mem_fd = -1;
/** @brief HouseKeeping memory layout of the FPGA registers */
fpga_hk_reg_mem_t*              g_fpga_hk_reg_mem = NULL;

/** @brief RadioBox memory file descriptor used to mmap() the FPGA space */
int                             g_fpga_rb_mem_fd = -1;
/** @brief RadioBox memory layout of the FPGA registers */
fpga_rb_reg_mem_t*              g_fpga_rb_reg_mem = NULL;

/** @brief Describes app. parameters with some info/limitations in high definition - compare initial values with: fpga_rb.fpga_rb_enable() */
const rb_app_params_t g_rb_default_params[RB_PARAMS_NUM + 1] = {
    { /* Running mode - transport_pktIdx 1 */
        "rb_run",              0.0,   1, 0, 0.0,       1.0  },

    { /* CAR_OSC modulation source selector - transport_pktIdx 1
       * ( 0: none (CW mode),
       *   1: RF Input 1,
       *   2: RF Input 2,
       *   4: EXT AI0,
       *   5: EXT AI1,
       *   6: EXT AI2,
       *   7: EXT AI3,
       *  15: MOD_OSC
       * )
       **/
        "car_osc_modsrc_s",    0.0,   1,  0, 0.0,     15.0  },

    { /* CAR_OSC modulation type selector (0: AM, 1: FM, 2: PM) - transport_pktIdx 1 */
        "car_osc_modtyp_s",    0.0,   1,  0, 0.0,      2.0  },


    { /* RBLED CON_SRC_PNT - transport_pktIdx 2 */
        "rbled_csp_s",         0.0,   1,  0, 0.0,     63.0  },

    { /* RFOUT1 CON_SRC_PNT - transport_pktIdx 2 */
        "rfout1_csp_s",        0.0,   1,  0, 0.0,     63.0  },

    { /* RFOUT2 CON_SRC_PNT - transport_pktIdx 2 */
        "rfout2_csp_s",        0.0,   1,  0, 0.0,     63.0  },


    { /* CAR_OSC frequency (Hz) - transport_pktIdx 3 */
        "car_osc_qrg_f",       0.0,   1,  0, 0.0,  62.5e+6  },

    { /* MOD_OSC frequency (Hz) - transport_pktIdx 3 */
        "mod_osc_qrg_f",       0.0,   1,  0, 0.0,  62.5e+6  },


    { /* AMP_RF amplitude (mV) - transport_pktIdx 4 */
        "amp_rf_gain_f",       0.0,   1,  0, 0.0,   2047.0  },

    { /* MOD_OSC magnitude (AM:%, FM:Hz, PM:°) - transport_pktIdx 4 */
        "mod_osc_mag_f",       0.0,   1,  0, 0.0,     1e+6  },


    { /* MUX in (Mic in) slider ranges from 0% to 100% - transport_pktIdx 5 */
        "muxin_gain_f",        0.0,   1,  0, 0.0,    100.0  },


    { /* has to be last entry */
        NULL,                  0.0,  -1, -1, 0.0,      0.0  }
};

/** @brief CallBack copy of params to inform the worker */
rp_app_params_t*                g_rp_cb_in_params = NULL;
/** @brief Holds mutex to access on parameters from outside to the worker thread */
pthread_mutex_t                 g_rp_cb_in_params_mutex = PTHREAD_MUTEX_INITIALIZER;

/** @brief Current copy of params of the worker thread */
rb_app_params_t*                g_rb_info_worker_params = NULL;
/** @brief Holds mutex to access parameters from the worker thread to any other context */
pthread_mutex_t                 g_rb_info_worker_params_mutex = PTHREAD_MUTEX_INITIALIZER;


/** @brief params initialized */
int                             g_params_init_done = 0;  /* @see worker.c */


/** @brief name of the param element for the packet counter */
const char TRANSPORT_pktIdx[]      = "pktIdx";

const int  IEEE754_DOUBLE_EXP_BIAS = 1023;
const int  IEEE754_DOUBLE_EXP_BITS = 12;
const int  IEEE754_DOUBLE_MNT_BITS = 52;
const int  RB_CELL_MNT_BITS        = 20;

const char CAST_NAME_EXT_SE[]      = "SE_";
const char CAST_NAME_EXT_HI[]      = "HI_";
const char CAST_NAME_EXT_MI[]      = "MI_";
const char CAST_NAME_EXT_LO[]      = "LO_";
const int  CAST_NAME_EXT_LEN       = 3;


/*----------------------------------------------------------------------------------*/
int is_quad(const char* name)
{
    char* ptr = strrchr(name, '_');
    if (ptr && !strcmp("_f", ptr)) {
        return 1;
    }
    return 0;
}


/*----------------------------------------------------------------------------------*/
double cast_4xbf_to_1xdouble(float f_se, float f_hi, float f_mi, float f_lo)
{
    unsigned long long ull = 0ULL;
    double*            dp  = (void*) &ull;

    if (!f_se && !f_hi && !f_mi && !f_lo) {
        //fprintf(stderr, "INFO cast_4xbf_to_1xdouble (zero) - out(d=%lf) <-- in(f_se=%f, f_hi=%f, f_mi=%f, f_lo=%f)\n", 0.0, f_se, f_hi, f_mi, f_lo);
        return 0.0;
    }

    // avoid under-rounding when casting to integer
    uint64_t i_se = (uint64_t) (0.5f + f_se);
    uint64_t i_hi = (uint64_t) (0.5f + f_hi);
    uint64_t i_mi = (uint64_t) (0.5f + f_mi);
    uint64_t i_lo = (uint64_t) (0.5f + f_lo);

    /* unsigned long long interpretation */
    ull   = (i_hi & 0x00fffffULL);  // 20 bits

    ull <<= RB_CELL_MNT_BITS;
    ull  |= (i_mi & 0x00fffffULL);  // 20 bits

    ull <<= IEEE754_DOUBLE_MNT_BITS - (RB_CELL_MNT_BITS << 1);
    ull  |= (i_lo & 0x0000fffULL);  // 12 bits

    ull  |= (i_se & 0x0000fffULL) <<  IEEE754_DOUBLE_MNT_BITS;

    /* double interpretation */
    //fprintf(stderr, "INFO cast_4xbf_to_1xdouble (val)  - out(d=%lf) <-- in(f_se=%f, f_hi=%f, f_mi=%f, f_lo=%f)\n", *dp, f_se, f_hi, f_mi, f_lo);
    return *dp;
}

/*----------------------------------------------------------------------------------*/
int cast_1xdouble_to_4xbf(float* f_se, float* f_hi, float* f_mi, float* f_lo, double d)
{
    unsigned long long ull = 0;
    double*            dp  = (void*) &ull;

    if (!f_se || !f_hi || !f_mi || !f_lo) {
        return -1;
    }

    if (d == 0.0) {
        /* use unnormalized zero instead */
        *f_se = 0.0f;
        *f_hi = 0.0f;
        *f_mi = 0.0f;
        *f_lo = 0.0f;
        //fprintf(stderr, "INFO cast_1xdouble_to_4xbf (zero) - out(f_se=%f, f_hi=%f, f_mi=%f, f_lo=%f) <-- in(d=%lf)\n", *f_se, *f_hi, *f_mi, *f_lo, d);
        return 0;
    }

    /* double interpretation */
    *dp = d;

    /* unsigned long long interpretation */
    uint64_t ui = ull;

    *f_lo   = ui & 0x00000fffULL;

    ui    >>= IEEE754_DOUBLE_MNT_BITS - (RB_CELL_MNT_BITS << 1);
    *f_mi   = ui & 0x000fffffULL;  // 12 bits

    ui    >>= RB_CELL_MNT_BITS;
    *f_hi   = ui & 0x000fffffULL;  // 20 bits

    ui    >>= RB_CELL_MNT_BITS;
    *f_se   = ui & 0x00000fffULL;  // 20 bits

    //fprintf(stderr, "INFO cast_1xdouble_to_4xbf (val)  - out(f_se=%f, f_hi=%f, f_mi=%f, f_lo=%f) <-- in(d=%lf)\n", *f_se, *f_hi, *f_mi, *f_lo, d);
    return 0;
}


/*----------------------------------------------------------------------------------*/
const char* rp_app_desc(void)
{
    return (const char *)"RedPitaya RadioBox application by DF4IAH and DD8UU.\n";
}


/*----------------------------------------------------------------------------------*/
int rp_create_traces(float** a_traces[TRACE_NUM])
{
    float** trc;

    fprintf(stderr, "rp_create_traces: BEGIN\n");

    trc = malloc(TRACE_NUM * sizeof(float*));
    if (!trc) {
        return -1;
    }

    // First step - void all pointers in case of early return
    int i;
    for (i = 0; i < TRACE_NUM; i++)
        trc[i] = NULL;

    // Set-up for each trace
    for (i = 0; i < TRACE_NUM; i++) {
        trc[i] = (float*) malloc(TRACE_LENGTH * sizeof(float));
        if (!(trc[i])) {
            // de-allocate all memory as much as already taken
            rp_free_traces(a_traces);
            return -2;
        }
        // wipe-out all random data
        memset(trc[i], 0, TRACE_LENGTH * sizeof(float));
    }

    // Success - assign to in/out parameter
    *a_traces = trc;
    fprintf(stderr, "rp_create_traces: END\n\n");

    return 0;
}

/*----------------------------------------------------------------------------------*/
void rp_free_traces(float** a_traces[TRACE_NUM])
{
    fprintf(stderr, "rp_free_traces: BEGIN\n");

    if (!a_traces) {
        return;
    }

    float** trc = *a_traces;

    if (trc) {
        int i;
        for (i = 0; i < TRACE_NUM; i++) {
            if (trc[i]) {
                free(trc[i]);
                trc[i] = NULL;
            }
        }
        free(trc);
    }
    *a_traces = NULL;

    fprintf(stderr, "rp_free_traces: END\n\n");
}


/*----------------------------------------------------------------------------------*/
int rp_find_parms_index(const rp_app_params_t* src, const char* name)
{
    if (!src || !name) {
        fprintf(stderr, "ERROR find_parms_index - Bad function arguments received.\n");
        return -2;
    }

    int i = 0;
    while (src[i].name) {
        if (!strcmp(src[i].name, name)) {
            return i;
        }
        ++i;
    }
    return -1;
}

/*----------------------------------------------------------------------------------*/
int rb_find_parms_index(const rb_app_params_t* src, const char* name)
{
    if (!src || !name) {
        fprintf(stderr, "ERROR find_parms_index - Bad function arguments received.\n");
        return -2;
    }

    int i = 0;
    while (src[i].name) {
        if (!strcmp(src[i].name, name)) {
            return i;
        }
        ++i;
    }
    return -1;
}


/*----------------------------------------------------------------------------------*/
void rp2rb_params_value_copy(rb_app_params_t* dst_line, const rp_app_params_t src_line_se, const rp_app_params_t src_line_hi, const rp_app_params_t src_line_mi, const rp_app_params_t src_line_lo)
{
    dst_line->value          = cast_4xbf_to_1xdouble(src_line_se.value, src_line_hi.value, src_line_mi.value, src_line_lo.value);

    dst_line->fpga_update    = src_line_lo.fpga_update;
    dst_line->read_only      = src_line_lo.read_only;

//  dst_line->min_val        = cast_3xbf_to_1xdouble(src_line_se.min_val, src_line_hi.min_val, src_line_mi.min_val, src_line_lo.min_val);
//  dst_line->max_val        = cast_3xbf_to_1xdouble(src_line_se.max_val, src_line_hi.max_val, src_line_mi.max_val, src_line_lo.max_val);
}

/*----------------------------------------------------------------------------------*/
void rb2rp_params_value_copy(rp_app_params_t* dst_line_se, rp_app_params_t* dst_line_hi, rp_app_params_t* dst_line_mi, rp_app_params_t* dst_line_lo, const rb_app_params_t src_line)
{
    //fprintf(stderr, "DEBUG rb2rp_params_value_copy - before value: %lf\n", src_line.value);
    cast_1xdouble_to_4xbf(&(dst_line_se->value), &(dst_line_hi->value), &(dst_line_mi->value), &(dst_line_lo->value), src_line.value);

    dst_line_se->fpga_update = 0;
    dst_line_hi->fpga_update = 0;
    dst_line_mi->fpga_update = 0;
    dst_line_lo->fpga_update = src_line.fpga_update;

    dst_line_se->read_only = 0;
    dst_line_hi->read_only = 0;
    dst_line_mi->read_only = 0;
    dst_line_lo->read_only = src_line.read_only;

    //fprintf(stderr, "DEBUG rb2rp_params_value_copy - before min_val: %lf\n", src_line.min_val);
    cast_1xdouble_to_4xbf(&(dst_line_se->min_val), &(dst_line_hi->min_val), &(dst_line_mi->min_val), &(dst_line_lo->min_val), src_line.min_val);
    //fprintf(stderr, "DEBUG rb2rp_params_value_copy - before max_val: %lf\n", src_line.max_val);
    cast_1xdouble_to_4xbf(&(dst_line_se->max_val), &(dst_line_hi->max_val), &(dst_line_mi->max_val), &(dst_line_lo->max_val), src_line.max_val);
    //fprintf(stderr, "DEBUG rb2rp_params_value_copy - done.\n");
}


/*----------------------------------------------------------------------------------*/
int rp_copy_params(rp_app_params_t** dst, const rp_app_params_t src[], int len, int do_copy_all_attr)
{
    const rp_app_params_t* s = src;
    int l_num_params = 0;

    /* check arguments */
    if (!s || !dst) {
        fprintf(stderr, "ERROR rp_copy_params - Internal error, the destination Application parameters variable is not set.\n");
        return -1;
    }

    /* check if destination buffer is allocated already */
    rp_app_params_t* p_dst = *dst;
    if (p_dst) {
        //fprintf(stderr, "DEBUG rp_copy_params - dst exists - updating into dst vector.\n");
        /* destination buffer exists */
        int i;
        for (i = 0; s[i].name; i++) {
            l_num_params++;
            //fprintf(stderr, "DEBUG rp_copy_params - processing name = %s\n", s[i].name);
            /* process each parameter entry of the list */

            if (!strcmp(p_dst[i].name, s[i].name)) {  // direct mapping found - just copy the value
                //fprintf(stderr, "DEBUG rp_copy_params - direct mapping used\n");
                p_dst[i].value = s[i].value;
                if (s[i].fpga_update & 0x80) {
                    p_dst[i].fpga_update |=  0x80;  // transfer FPGA update marker in case it is present
                } else {
                    p_dst[i].fpga_update &= ~0x80;  // or remove it when it is not set
                }

                if (do_copy_all_attr) {  // if default parameters are taken, use all attributes
                    p_dst[i].fpga_update    = s[i].fpga_update;
                    p_dst[i].read_only      = s[i].read_only;
                    p_dst[i].min_val        = s[i].min_val;
                    p_dst[i].max_val        = s[i].max_val;
                }

            } else {
                //fprintf(stderr, "DEBUG rp_copy_params - iterative searching ...\n");
                int j;
                for (j = 0; p_dst[j].name; j++) {  // scanning the complete list
                    if (j == i) {  // do a short-cut here
                        continue;
                    }

                    if (!strcmp(p_dst[j].name, s[i].name)) {
                        p_dst[j].value = s[i].value;
                        if (s[i].fpga_update & 0x80) {
                            p_dst[j].fpga_update |=  0x80;  // transfer FPGA update marker in case it is present
                        } else {
                            p_dst[j].fpga_update &= ~0x80;  // or remove it when it is not set
                        }

                        if (do_copy_all_attr) {  // if default parameters are taken, use all attributes
                            p_dst[j].fpga_update    = s[i].fpga_update;  // copy FPGA update marker in case it is present
                            p_dst[j].read_only      = s[i].read_only;
                            p_dst[j].min_val        = s[i].min_val;
                            p_dst[j].max_val        = s[i].max_val;
                        }
                        break;
                    }
                }  // for (; p_dst[j].name ;)
            }  // if () else
        }  // for ()

    } else {
        /* destination buffer has to be allocated, create a new parameter list */

        if (len >= 0) {
            l_num_params = len;

        } else {
            /* retrieve the number of source parameters */
            for (l_num_params = 0; s[l_num_params].name; l_num_params++) { }
        }

        /* allocate array of parameter entries, parameter names must be allocated separately */
        p_dst = (rp_app_params_t*) malloc(sizeof(rp_app_params_t) * (l_num_params + 1));
        if (!p_dst) {
            fprintf(stderr, "ERROR rp_copy_params - memory problem, the destination buffer could not be allocated (1).\n");
            return -3;
        }
        /* prepare a copy for built-in attributes. Strings have to be handled on their own way */
        memcpy(p_dst, s, (l_num_params + 1) * sizeof(rp_app_params_t));

        /* allocate memory and copy character strings for params names */
        int i;
        for (i = 0; s[i].name; i++) {
            int slen = strlen(s[i].name);
            p_dst[i].name = (char*) malloc(slen + 1);  // old pointer to name does not belong to us and has to be discarded
            if (!(p_dst[i].name)) {
                fprintf(stderr, "ERROR rp_copy_params - memory problem, the destination buffer could not be allocated (2).\n");
                return -4;
            }
            strncpy(p_dst[i].name, s[i].name, slen);
            p_dst[i].name[slen] = '\0';
        }

        /* mark last one as final entry */
        p_dst[l_num_params].name = NULL;
        p_dst[l_num_params].value = -1;
    }
    *dst = p_dst;

    return l_num_params;
}

/*----------------------------------------------------------------------------------*/
int rb_copy_params(rb_app_params_t** dst, const rb_app_params_t src[], int len, int do_copy_all_attr)
{
    const rb_app_params_t* s = src;
    int l_num_params = 0;

    /* check arguments */
    if (!dst) {
        fprintf(stderr, "ERROR rb_copy_params - Internal error, the destination Application parameters variable is not set.\n");
        return -1;
    }
    if (!s) {
        //fprintf(stderr, "INFO rb_copy_params - no source parameter list given, taking default parameters instead.\n");
        s = g_rb_default_params;
    }

    /* check if destination buffer is allocated already */
    rb_app_params_t* p_dst = *dst;
    if (p_dst) {
        //fprintf(stderr, "DEBUG rb_copy_params - dst exists - updating into dst vector.\n");
        /* destination buffer exists */
        int i, j;
        for (i = 0, j = 0; s[i].name; i++) {
            l_num_params++;
            //fprintf(stderr, "DEBUG rb_copy_params - processing name = %s\n", s[i].name);
            /* process each parameter entry of the list */

            j = rb_find_parms_index(p_dst, s[i].name);
            if (j >= 0) {
                //fprintf(stderr, "DEBUG rb_copy_params - fill in\n");
                p_dst[j].value = s[i].value;
                if (s[i].fpga_update & 0x80) {
                    p_dst[j].fpga_update |=  0x80;  // transfer FPGA update marker in case it is present
                } else {
                    p_dst[j].fpga_update &= ~0x80;  // or remove it when it is not set
                }

                if (do_copy_all_attr) {  // if default parameters are taken, use all attributes
                    p_dst[j].fpga_update    = s[i].fpga_update;
                    p_dst[j].read_only      = s[i].read_only;
                    p_dst[j].min_val        = s[i].min_val;
                    p_dst[j].max_val        = s[i].max_val;
                }
            }  // if (j >= 0)
        }  // for ()

    } else {
        /* destination buffer has to be allocated, create a new parameter list */

        if (len >= 0) {
            l_num_params = len;

        } else {
            /* retrieve the number of source parameters */
            for (l_num_params = 0; s[l_num_params].name; l_num_params++) { }
        }

        /* allocate array of parameter entries, parameter names must be allocated separately */
        p_dst = (rb_app_params_t*) malloc(sizeof(rb_app_params_t) * (l_num_params + 1));
        if (!p_dst) {
            fprintf(stderr, "ERROR rb_copy_params - memory problem, the destination buffer could not be allocated (1).\n");
            return -3;
        }

        /* allocate memory and copy character strings for params names */
        int i;
        for (i = 0; s[i].name; i++) {
            const int slen = strlen(s[i].name);

            p_dst[i].name = (char*) malloc(slen + 1);  // old pointer to name does not belong to us and has to be discarded
            if (!(p_dst[i].name)) {
                fprintf(stderr, "ERROR rb_copy_params - memory problem, the destination buffer could not be allocated (2).\n");
                return -4;
            }
            strncpy(p_dst[i].name, s[i].name, slen);
            p_dst[i].name[slen] = '\0';

            p_dst[i].value          = s[i].value;
            p_dst[i].fpga_update    = s[i].fpga_update;
            if (do_copy_all_attr) {                                                                     // if default parameters are taken, use all attributes
                p_dst[i].fpga_update    = s[i].fpga_update;
                p_dst[i].read_only      = s[i].read_only;
                p_dst[i].min_val        = s[i].min_val;
                p_dst[i].max_val        = s[i].max_val;
            }
        }

        /* mark last one as final entry */
        p_dst[l_num_params].name = NULL;
        p_dst[l_num_params].value = -1;
    }
    *dst = p_dst;

    return l_num_params;
}


/*----------------------------------------------------------------------------------*/
int rp_copy_params_rb2rp(rp_app_params_t** dst, const rb_app_params_t src[])
{
    int l_num_single_params = 0;
    int l_num_quad_params = 0;

    /* check arguments */
    if (!dst) {
        fprintf(stderr, "ERROR rp_copy_params_rb2rp - Internal error, the destination Application parameters vector variable is not set.\n");
        return -1;
    }
    if (!src) {
        fprintf(stderr, "ERROR rp_copy_params_rb2rp - Internal error, the source Application parameters vector variable is not set.\n");
        return -2;
    }

    /* check if destination buffer is allocated already */
    rp_app_params_t* p_dst = *dst;
    if (p_dst) {
        //fprintf(stderr, "DEBUG rp_copy_params_rb2rp: freeing ...\n");
        rp_free_params(dst);
    }

    /* destination buffer has to be allocated, create a new parameter list */
    {
        /* get the number entries */
        {
            int i;
            for (i = 0; src[i].name; i++) {
                //fprintf(stderr, "DEBUG rp_copy_params_rb2rp - get_the_number_entries - name=%s\n", src[i].name);
                if (is_quad(src[i].name)) {
                    l_num_quad_params++;
                } else {
                    l_num_single_params++;
                }
            }
            //fprintf(stderr, "DEBUG rp_copy_params_rb2rp - num_single_params=%d, num_quad_params=%d\n", l_num_single_params, l_num_quad_params);
        }

        /* allocate array of parameter entries, parameter names must be allocated separately */
        p_dst = (rp_app_params_t*) malloc(sizeof(rp_app_params_t) * (1 + l_num_single_params + (l_num_quad_params << 2)));
        if (!p_dst) {
            fprintf(stderr, "ERROR rp_copy_params_rb2rp - memory problem, the destination buffer failed to be allocated (1).\n");
            return -3;
        }

        /* allocate memory and copy character strings for params names */
        int i, j;
        for (i = 0, j = 0; src[i].name; i++) {
            const int slen = strlen(src[i].name);
            char found = 0;

            /* limit transfer volume to a part of all param entries, @see main.g_rb_default_params (abt. line 70) for a full list */
            switch (g_transport_pktIdx & 0x7f) {
            case 1:
                if (!strcmp("rb_run",           src[i].name) ||
                    !strcmp("car_osc_modsrc_s", src[i].name) ||
                    !strcmp("car_osc_modtyp_s", src[i].name)) {
                    found = 1;
                }
                break;

            case 2:
                if (!strcmp("rbled_csp_s",      src[i].name) ||
                    !strcmp("rfout1_csp_s",     src[i].name) ||
                    !strcmp("rfout2_csp_s",     src[i].name)) {
                    found = 1;
                }
                break;

            case 3:
                if (!strcmp("car_osc_qrg_f",    src[i].name) ||
                    !strcmp("mod_osc_qrg_f",    src[i].name)) {
                    found = 1;
                }
                break;

            case 4:
                if (!strcmp("amp_rf_gain_f",    src[i].name) ||
                    !strcmp("mod_osc_mag_f",    src[i].name)) {
                    found = 1;
                }
                break;

            case 5:
                if (!strcmp("muxin_gain_f",     src[i].name)) {
                    found = 1;
                }
                break;

            default:
                /* no limitation of output data */
                   found = 1;
                break;
            }

            if (!found) {
                if (is_quad(src[i].name)) {
                    l_num_quad_params--;
                } else {
                    l_num_single_params--;
                }
                //fprintf(stderr, "DEBUG rp_copy_params_rb2rp - Limit transfer - for pktIdx = %d  no  name = %s - num_single_params = %d, num_quad_params = %d - continue\n", g_transport_pktIdx, src[i].name, l_num_single_params, l_num_quad_params);
                continue;
            }

            if (is_quad(src[i].name)) {                                                                 // float parameter --> QUAD encoder
                int j_se = j++;
                int j_hi = j++;
                int j_mi = j++;
                int j_lo = j++;

                p_dst[j_se].name = (char*) malloc(CAST_NAME_EXT_LEN + slen + 1);
                p_dst[j_hi].name = (char*) malloc(CAST_NAME_EXT_LEN + slen + 1);
                p_dst[j_mi].name = (char*) malloc(CAST_NAME_EXT_LEN + slen + 1);
                p_dst[j_lo].name = (char*) malloc(CAST_NAME_EXT_LEN + slen + 1);
                if (!(p_dst[j_se].name) || !(p_dst[j_hi].name) || !(p_dst[j_mi].name) || !(p_dst[j_lo].name)) {
                    fprintf(stderr, "ERROR rp_copy_params_rb2rp - memory problem, the destination buffers failed to be allocated (2).\n");
                    return -3;
                }

                strncpy( p_dst[j_se].name,   CAST_NAME_EXT_SE, CAST_NAME_EXT_LEN);
                strncpy((p_dst[j_se].name) + CAST_NAME_EXT_LEN, src[i].name, slen);
                p_dst[j_se].name[CAST_NAME_EXT_LEN + slen] = '\0';

                strncpy( p_dst[j_hi].name,   CAST_NAME_EXT_HI, CAST_NAME_EXT_LEN);
                strncpy((p_dst[j_hi].name) + CAST_NAME_EXT_LEN, src[i].name, slen);
                p_dst[j_hi].name[CAST_NAME_EXT_LEN + slen] = '\0';

                strncpy( p_dst[j_mi].name,   CAST_NAME_EXT_MI, CAST_NAME_EXT_LEN);
                strncpy((p_dst[j_mi].name) + CAST_NAME_EXT_LEN, src[i].name, slen);
                p_dst[j_mi].name[CAST_NAME_EXT_LEN + slen] = '\0';

                strncpy( p_dst[j_lo].name,   CAST_NAME_EXT_LO, CAST_NAME_EXT_LEN);
                strncpy((p_dst[j_lo].name) + CAST_NAME_EXT_LEN, src[i].name, slen);
                p_dst[j_lo].name[CAST_NAME_EXT_LEN + slen] = '\0';

                rb2rp_params_value_copy(&(p_dst[j_se]), &(p_dst[j_hi]), &(p_dst[j_mi]), &(p_dst[j_lo]), src[i]);
                fprintf(stderr, "INFO rp_copy_params_rb2rp - out[%d, %d, %d, %d] copied from in[%d] - name = %s, val = %lf\n", j_se, j_hi, j_mi, j_lo, i, src[i].name, src[i].value);

            } else {                                                                                    // SINGLE parameter
                p_dst[j].name = (char*) malloc(slen + 1);
                if (!(p_dst[j].name)) {
                    fprintf(stderr, "ERROR rp_copy_params_rb2rp - memory problem, the destination buffers failed to be allocated (3).\n");
                    return -3;
                }

                strncpy((p_dst[j].name), src[i].name, slen);
                p_dst[j].name[slen] = '\0';

                p_dst[j].value       = src[i].value;
                p_dst[j].fpga_update = src[i].fpga_update;
                p_dst[j].read_only   = src[i].read_only;
                p_dst[j].min_val     = src[i].min_val;
                p_dst[j].max_val     = src[i].max_val;
                fprintf(stderr, "INFO rp_copy_params_rb2rp - out[%d] copied from in[%d] - name = %s, val = %lf\n", j, i, src[i].name, src[i].value);

                j++;
            }  // else SINGLE
        }  // for ()

        /* mark last one as final entry */
        p_dst[l_num_single_params + (l_num_quad_params << 2)].name = NULL;
        p_dst[l_num_single_params + (l_num_quad_params << 2)].value = -1;
    }
    *dst = p_dst;

    return l_num_single_params + (l_num_quad_params << 2);
}

/*----------------------------------------------------------------------------------*/
 int rp_copy_params_rp2rb(rb_app_params_t** dst, const rp_app_params_t src[])
{
    int l_num_single_params = 0;
    int l_num_quad_params = 0;

    /* check arguments */

    if (!dst) {
        fprintf(stderr, "ERROR rp_copy_params_rp2rb - Internal error, the destination Application parameters vector variable is not set.\n");
        return -1;
    }
    if (!src) {
        fprintf(stderr, "ERROR rp_copy_params_rp2rb - Internal error, the source Application parameters vector variable is not set.\n");
        return -2;
    }

    /* get the number entries */
    {
        int i;
        for (i = 0; src[i].name; i++) {
            //fprintf(stderr, "DEBUG rp_copy_params_rp2rb - get_the_number_entries - name=%s\n", src[i].name);
            if (is_quad(src[i].name)) {
                /* count LO_yyy entries */
                if (!strncmp(CAST_NAME_EXT_LO, src[i].name, CAST_NAME_EXT_LEN)) {
                    l_num_quad_params++;
                }
            } else {
                l_num_single_params++;
            }
        }  // for ()
        //fprintf(stderr, "DEBUG rp_copy_params_rp2rb - num_single_params=%d, num_quad_params=%d\n", l_num_single_params, l_num_quad_params);
    }

    /* check if destination buffer is allocated already */
    rb_app_params_t* p_dst = *dst;
    if (p_dst) {
        //fprintf(stderr, "DEBUG rp_copy_params_rp2rb = dst vector is valid\n");

        int i, j;
        for (i = 0, j = 0; src[i].name; i++) {
            const int slen = strlen(src[i].name);

            if (!strncmp(CAST_NAME_EXT_SE, src[i].name, CAST_NAME_EXT_LEN) ||
                !strncmp(CAST_NAME_EXT_HI, src[i].name, CAST_NAME_EXT_LEN) ||
                !strncmp(CAST_NAME_EXT_MI, src[i].name, CAST_NAME_EXT_LEN)) {
                continue;  // skip all SE_, HI_ and MI_ params
            }

            if (!strcmp(TRANSPORT_pktIdx, src[i].name)) {
                l_num_single_params--;
                //fprintf(stderr, "DEBUG rp_copy_params_rp2rb - disregard this entry (1) - name = %s - num_single_params = %d, num_quad_params = %d\n", src[i].name, l_num_single_params, l_num_quad_params);
                continue;
            }

            if (!strncmp(CAST_NAME_EXT_LO, src[i].name, CAST_NAME_EXT_LEN)) {                           // QUAD elements - since here: LO_xxx
                char name_se[256];
                char name_hi[256];
                char name_mi[256];

                /* prepare name variants */
                {
                    strncpy(name_se, CAST_NAME_EXT_SE, CAST_NAME_EXT_LEN);
                    strncpy(name_se + CAST_NAME_EXT_LEN, CAST_NAME_EXT_LEN + src[i].name, slen);
                    name_se[CAST_NAME_EXT_LEN + slen] = '\0';

                    strncpy(name_hi, CAST_NAME_EXT_HI, CAST_NAME_EXT_LEN);
                    strncpy(name_hi + CAST_NAME_EXT_LEN, CAST_NAME_EXT_LEN + src[i].name, slen);
                    name_hi[CAST_NAME_EXT_LEN + slen] = '\0';

                    strncpy(name_mi, CAST_NAME_EXT_MI, CAST_NAME_EXT_LEN);
                    strncpy(name_mi + CAST_NAME_EXT_LEN, CAST_NAME_EXT_LEN + src[i].name, slen);
                    name_mi[CAST_NAME_EXT_LEN + slen] = '\0';
                }

                /* find all quad elements */
                int i_se = rp_find_parms_index(src, name_se);
                int i_hi = rp_find_parms_index(src, name_hi);
                int i_mi = rp_find_parms_index(src, name_mi);
                int i_lo = i;
                if (i_se < 0 || i_hi < 0 || i_mi < 0) {
                    continue;  // no quad found, ignore uncomplete entries
                }

                j = rb_find_parms_index(p_dst, src[i].name + CAST_NAME_EXT_LEN);  // the extension is stripped away before the compare
                if (j < 0) {
                    // discard new entry if not already known in target vector
                    fprintf(stderr, "WARNING rp_copy_params_rp2rb (1) - input element of vector is unknown - name = %s\n", src[i].name);
                    continue;
                }

                rp2rb_params_value_copy(&(p_dst[j]), src[i_se], src[i_hi], src[i_mi], src[i_lo]);
                fprintf(stderr, "INFO rp_copy_params_rp2rb - out[%d] copied from in[%d, %d, %d, %d] - name = %s, val = %lf\n", j, i_se, i_hi, i_mi, i_lo, src[i_lo].name + CAST_NAME_EXT_LEN, p_dst[j].value);

            } else {                                                                                    // SINGLE element
                j = rb_find_parms_index(p_dst, src[i].name);
                if (j < 0) {
                    // discard new entry if not already known in target vector
                    fprintf(stderr, "WARNING rp_copy_params_rp2rb (2) - input element of vector is unknown - name = %s\n", src[i].name);
                    continue;
                }

                p_dst[j].value       = src[i].value;
                p_dst[j].fpga_update = src[i].fpga_update;
                p_dst[j].read_only   = src[i].read_only;
                p_dst[j].min_val     = src[i].min_val;
                p_dst[j].max_val     = src[i].max_val;
                fprintf(stderr, "INFO rp_copy_params_rp2rb - out[%d] copied from in[%d] - name = %s, val = %lf\n", j, i, src[i].name, src[i].value);
            }
        }  // for ()

    } else {
        //fprintf(stderr, "DEBUG rp_copy_params_rp2rb = creating new dst vector\n");
        /* destination buffer has to be allocated, create a new parameter list */

        /* allocate array of parameter entries, parameter names must be allocated separately */
        p_dst = (rb_app_params_t*) malloc(sizeof(rb_app_params_t) * (1 + l_num_single_params + (l_num_quad_params << 2)));
        if (!p_dst) {
            fprintf(stderr, "ERROR rp_copy_params_rp2rb - memory problem, the destination buffer failed to be allocated (1).\n");
            return -3;
        }

        /* allocate memory and copy character strings for params names */
        int i, j;
        for (i = 0, j = 0; src[i].name; i++) {
            if (!strncmp(CAST_NAME_EXT_SE, src[i].name, CAST_NAME_EXT_LEN) ||
                !strncmp(CAST_NAME_EXT_HI, src[i].name, CAST_NAME_EXT_LEN) ||
                !strncmp(CAST_NAME_EXT_MI, src[i].name, CAST_NAME_EXT_LEN)) {
                //fprintf(stderr, "DEBUG rp_copy_params_rp2rb - skip this name=%s\n", src[i].name);
                continue;  // skip all SE_, HI_ and MI_ params
            }

            if (!strncmp(CAST_NAME_EXT_LO, src[i].name, CAST_NAME_EXT_LEN)) {                           // QUAD elements - since here: LO_xxx
                const int slen = strlen(src[i].name) - CAST_NAME_EXT_LEN;
                char name_se[256];
                char name_hi[256];
                char name_mi[256];

                //fprintf(stderr, "DEBUG rp_copy_params_rp2rb - QUAD - name=%s\n", src[i].name);
                /* prepare name variants */
                {
                    strncpy(name_se, CAST_NAME_EXT_SE, CAST_NAME_EXT_LEN);
                    strncpy(name_se + CAST_NAME_EXT_LEN, src[i].name + CAST_NAME_EXT_LEN, slen);
                    name_se[CAST_NAME_EXT_LEN + slen] = '\0';

                    strncpy(name_hi, CAST_NAME_EXT_HI, CAST_NAME_EXT_LEN);
                    strncpy(name_hi + CAST_NAME_EXT_LEN, src[i].name + CAST_NAME_EXT_LEN, slen);
                    name_hi[CAST_NAME_EXT_LEN + slen] = '\0';

                    strncpy(name_mi, CAST_NAME_EXT_MI, CAST_NAME_EXT_LEN);
                    strncpy(name_mi + CAST_NAME_EXT_LEN, src[i].name + CAST_NAME_EXT_LEN, slen);
                    name_mi[CAST_NAME_EXT_LEN + slen] = '\0';
                }

                /* find all quad elements */
                int i_se = rp_find_parms_index(src, name_se);
                int i_hi = rp_find_parms_index(src, name_hi);
                int i_mi = rp_find_parms_index(src, name_mi);
                int i_lo = i;
                if (i_se < 0 || i_hi < 0 || i_mi < 0) {
                    continue;
                }

                /* create for each valid "rp" input vector tuple a "rb" output vector entry */
                p_dst[j].name = (char*) malloc(slen + 1);
                if (!p_dst[j].name) {
                    fprintf(stderr, "ERROR rp_copy_params_rp2rb - memory problem, the destination buffers failed to be allocated (2).\n");
                    return -3;
                }

                strncpy(p_dst[j].name, src[i_lo].name + CAST_NAME_EXT_LEN, slen);                       // yyy <-- LO_yyy
                p_dst[j].name[slen] = '\0';

                rp2rb_params_value_copy(&(p_dst[j]), src[i_se], src[i_hi], src[i_mi], src[i_lo]);
                fprintf(stderr, "INFO rp_copy_params_rp2rb - out[%d] copied from in[%d,%d,%d,%d] - name = %s, val = %lf\n", j, i_se, i_hi, i_mi, i_lo, src[i_lo].name + CAST_NAME_EXT_LEN, p_dst[j].value);

                j++;

            } else {                                                                                    // SINGLE element
                const int slen = strlen(src[i].name);

                //fprintf(stderr, "DEBUG rp_copy_params_rp2rb - SINGLE - name=%s\n", src[i].name);

                if (!strcmp(TRANSPORT_pktIdx, src[i].name)) {
                    l_num_single_params--;
                    //fprintf(stderr, "DEBUG rp_copy_params_rp2rb - disregard this entry (1) - name = %s - num_single_params = %d, num_quad_params = %d\n", src[i].name, l_num_single_params, l_num_quad_params);
                    continue;
                }

                p_dst[j].name = (char*) malloc(slen + 1);
                if (!(p_dst[j].name)) {
                    fprintf(stderr, "ERROR rp_copy_params_rp2rb - memory problem, the destination buffer could not be allocated (2).\n");
                    return -3;
                }
                strncpy(p_dst[j].name, src[i].name, slen);
                p_dst[j].name[slen] = '\0';

                p_dst[j].value       = src[i].value;
                p_dst[j].fpga_update = src[i].fpga_update;
                p_dst[j].read_only   = src[i].read_only;
                p_dst[j].min_val     = src[i].min_val;
                p_dst[j].max_val     = src[i].max_val;
                fprintf(stderr, "INFO rp_copy_params_rp2rb - out[%d] copied from in[%d] - name = %s, val = %lf\n", j, i, src[i].name, src[i].value);

                j++;
            }
        }  // for ()
    }  // if () else

    /* mark last one as final entry */
    p_dst[l_num_single_params + l_num_quad_params].name  = NULL;
    p_dst[l_num_single_params + l_num_quad_params].value = -1;

    *dst = p_dst;

    return l_num_single_params + l_num_quad_params;
}


/*----------------------------------------------------------------------------------*/
int print_rb_params(rb_app_params_t* params)
{
    if (!params) {
        return -1;
    }

    int i;
    for (i = 0; params[i].name; i++) {
        fprintf(stderr, "INFO print_rb_params: params[%d].name = %s - value = %lf\n", i, params[i].name, params[i].value);
    }

    return 0;
}


/*----------------------------------------------------------------------------------*/
int rp_free_params(rp_app_params_t** params)
{
    if (!params) {
        return -1;
    }

    /* free params structure */
    if (*params) {
        rp_app_params_t* p = *params;

        int i;
        for (i = 0; p[i].name; i++) {
            free(p[i].name);
            p[i].name = NULL;
        }

        free(*params);
        *params = NULL;
    }
    return 0;
}

/*----------------------------------------------------------------------------------*/
int rb_free_params(rb_app_params_t** params)
{
    if (!params) {
        return -1;
    }

    /* free params structure */
    if (*params) {
        rb_app_params_t* p = *params;

        int i;
        for (i = 0; p[i].name; i++) {
            free(p[i].name);
            p[i].name = NULL;
        }

        free(p);
        *params = NULL;
    }
    return 0;
}
