/**
 * $Id$
 *
 * @brief Red Pitaya Waterfall diagram.
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
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <errno.h>

#include "jpeglib.h"

#include "waterfall.h"
#include "dsp.h"
#include "wf_colmap.h"

/* TODO: More appripriate names */
float g_mm = 0.0;
float g_qq = 0.0;

/* How many samples is skipped after convolution */
const int c_skip_after_conv = 10;

/* Decimation step - without skipped firsy c_skip_after_conf samples */
int   g_dec_wat_step = 0;

float *rp_wf_avg_filter = NULL;

/* Results of the filtering/convolution, size:
 *   c_dsp_sig_len + RP_SPECTR_AVG_FILT - 1 
 */
int g_conv_len;
double *rp_wf_cha_cnv = NULL;
double *rp_wf_chb_cnv = NULL;

int g_spectr_wf_col;

/* Result of decimation & mapping - the length of RP_SPECTR_WF_COL */
int *rp_wf_cha_dec_map = NULL;
int *rp_wf_chb_dec_map = NULL;

/* Maps which is builded from multiple acquisitions, size:
 * RP_SPECTR_WF_COL * RP_SPECTR_WF_LIN 
 */
int *rp_wf_cha_cont_map = NULL;
int *rp_wf_chb_cont_map = NULL;
int  rp_wf_cont_map_idx = -1;

/* The following structures are of R,G,B order data 
 * Size is RP_SPECTR_WF_COL * RP_SPECTR_WF_LIN * 3 (for RGB) 
 */
JSAMPLE *rp_wf_cha_wat = NULL;
JSAMPLE *rp_wf_chb_wat = NULL;

int rp_spectr_wf_init(void)
{
    int i;

    /* Just to be sure... */
    rp_spectr_wf_clean();

    g_mm = 
        ((double)RP_SPECTR_WF_MAP_MAX-RP_SPECTR_WF_MAP_NOI)/
        (double)(RP_SPECTR_WF_SPEC_MAX-RP_SPECTR_WF_SPEC_NOI);

    g_qq = (double)RP_SPECTR_WF_MAP_MAX - g_mm * RP_SPECTR_WF_SPEC_MAX;

    rp_wf_avg_filter = (float *)malloc(RP_SPECTR_WF_AVG_FILT * sizeof(float));
    if(!rp_wf_avg_filter) {
        fprintf(stderr, "rp_spectr_wf_init() can not allocate memory\n");
        return -1;
    }

    for(i = 0; i < RP_SPECTR_WF_AVG_FILT; i++)
        rp_wf_avg_filter[i] = 1.0;

    g_conv_len = c_dsp_sig_len + RP_SPECTR_WF_AVG_FILT - 1;

    rp_wf_cha_cnv = (double *)malloc(g_conv_len * sizeof(double));
    rp_wf_chb_cnv = (double *)malloc(g_conv_len * sizeof(double));

    if(!rp_wf_cha_cnv || !rp_wf_chb_cnv) {
        fprintf(stderr, "rp_spectr_wf_init() can not allocate memory\n");
        rp_spectr_wf_clean();
        return -1;
    }

    g_dec_wat_step = 
        ceil((g_conv_len-c_skip_after_conv) / (double)RP_SPECTR_WF_COL);

    g_spectr_wf_col = round((g_conv_len-c_skip_after_conv) / g_dec_wat_step);

    rp_wf_cha_dec_map = (int *)malloc(g_spectr_wf_col * sizeof(int));
    rp_wf_chb_dec_map = (int *)malloc(g_spectr_wf_col * sizeof(int));
    if(!rp_wf_cha_dec_map || !rp_wf_chb_dec_map) {
        fprintf(stderr, "rp_spectr_wf_init() can not allocate memory\n");
        rp_spectr_wf_clean();
        return -1;
    }

    rp_wf_cha_cont_map = (int *)malloc(RP_SPECTR_WF_LIN * g_spectr_wf_col
                                       * sizeof(int));
    rp_wf_chb_cont_map = (int *)malloc(RP_SPECTR_WF_LIN * g_spectr_wf_col
                                       * sizeof(int));
    if(!rp_wf_cha_cont_map || !rp_wf_chb_cont_map) {
        fprintf(stderr, "rp_spectr_wf_init() can not allocate memory\n");
        rp_spectr_wf_clean();
        return -1;
    }
    rp_spectr_wf_clean_map();
    
    /* Initialize the rp_wf_cha_wat & chb_wat structures which will be used
     * to build a picture - x3 is for R,G,B
     */
    rp_wf_cha_wat = (JSAMPLE *)malloc(RP_SPECTR_WF_LIN * g_spectr_wf_col *
                                      3 * sizeof(JSAMPLE));
    rp_wf_chb_wat = (JSAMPLE *)malloc(RP_SPECTR_WF_LIN * g_spectr_wf_col *
                                      3 * sizeof(JSAMPLE));
    if(!rp_wf_cha_wat || !rp_wf_chb_wat) {
        fprintf(stderr, "rp_spectr_wf_init() can not allocate memory\n");
        return -1;
    }

    return 0;
}

int rp_spectr_wf_clean(void)
{
    if(rp_wf_avg_filter) {
        free(rp_wf_avg_filter);
        rp_wf_avg_filter = NULL;
    }
    if(rp_wf_cha_cnv) {
        free(rp_wf_cha_cnv);
        rp_wf_cha_cnv = NULL;
    }
    if(rp_wf_chb_cnv) {
        free(rp_wf_chb_cnv);
        rp_wf_chb_cnv = NULL;
    }
    if(rp_wf_cha_dec_map) {
        free(rp_wf_cha_dec_map);
        rp_wf_cha_dec_map = NULL;
    }
    if(rp_wf_chb_dec_map) {
        free(rp_wf_chb_dec_map);
        rp_wf_chb_dec_map = NULL;
    }
    if(rp_wf_cha_cont_map) {
        free(rp_wf_cha_cont_map);
        rp_wf_cha_cont_map = NULL;
    }
    if(rp_wf_chb_cont_map) {
        free(rp_wf_chb_cont_map);
        rp_wf_chb_cont_map = NULL;
    }
    if(rp_wf_cha_wat) {
        free(rp_wf_cha_wat);
        rp_wf_cha_wat = NULL;
    }
    if(rp_wf_chb_wat) {
        free(rp_wf_chb_wat);
        rp_wf_chb_wat = NULL;
    }
    return 0;
}

int rp_spectr_wf_clean_map(void)
{
    if(!rp_wf_cha_cont_map || !rp_wf_chb_cont_map) {
        fprintf(stderr, "rp_spectr_wf_clean_map() not initialized!\n");
        return -1;
    }

    memset(rp_wf_cha_cont_map, 0, 
           RP_SPECTR_WF_LIN * g_spectr_wf_col * sizeof(int));
    memset(rp_wf_chb_cont_map, 0, 
           RP_SPECTR_WF_LIN * g_spectr_wf_col * sizeof(int));
    rp_wf_cont_map_idx = RP_SPECTR_WF_LIN - 1; /* start with the last line */

    return 0;
}

int rp_spectr_wf_calc(double *cha_in, double *chb_in)
{
    if(!cha_in || !chb_in) {
        fprintf(stderr, "rp_spectr_wf_calc(): input signals not initialized\n");
        return -1;
    }
    if(!rp_wf_cha_cnv || !rp_wf_cha_cnv) {
        fprintf(stderr, "rp_spectr_wf_calc(): internals not initialized\n");
        return -1;
    }

    if(rp_spectr_wf_conv(cha_in, chb_in, &rp_wf_cha_cnv, &rp_wf_chb_cnv) < 0) {
        fprintf(stderr, "rp_spectr_wf_calc(): rp_spectr_wf_conv() failed\n");
        return -1;
    }

    if(rp_spectr_wf_dec_map(rp_wf_cha_cnv, rp_wf_chb_cnv, 
                            &rp_wf_cha_dec_map, &rp_wf_chb_dec_map) < 0) {
        fprintf(stderr, "rp_spectr_wf_calc(): rp_spectr_wf_dec_map() failed\n");
        return -1;
    }

    if(rp_spectr_wf_add_to_map(rp_wf_cha_dec_map, rp_wf_chb_dec_map) < 0) {
        fprintf(stderr, 
                "rp_spectr_wf_calc(): rp_spectr_wf_add_to_map() failed\n");
        return -1;
    }

    return 0;
}

int rp_spectr_wf_save_jpeg(const char *wf_cha_file, const char *wf_chb_file) 
{
    if(!rp_wf_cha_cont_map || !rp_wf_chb_cont_map || 
       !rp_wf_cha_wat || !rp_wf_chb_wat) {
        fprintf(stderr, "rp_spectr_wf_save_jpeg(): not initialized\n");
        return -1;
    }
    
    if(rp_spectr_wf_create_rgb(rp_wf_cha_cont_map, &rp_wf_cha_wat) < 0) {
        fprintf(stderr, "rp_spectr_wf_save_jpeg(): rp_spectr_wf_create_rgb() "
                " failed\n");
        return -1;
    }

    if(rp_spectr_wf_create_rgb(rp_wf_chb_cont_map, &rp_wf_chb_wat) < 0) {
        fprintf(stderr, "rp_spectr_wf_save_jpeg(): rp_spectr_wf_create_rgb() "
                " failed\n");
        return -1;
    }

    if(rp_spectr_wf_comp_jpeg(rp_wf_cha_wat, wf_cha_file) < 0) {
        fprintf(stderr, "rp_spectr_wf_save_jpeg(): rp_spectr_wf_comp_jpeg() "
                " failed\n");
        return -1;
    }

    if(rp_spectr_wf_comp_jpeg(rp_wf_chb_wat, wf_chb_file) < 0) {
        fprintf(stderr, "rp_spectr_wf_save_jpeg(): rp_spectr_wf_comp_jpeg() "
                " failed\n");
        return -1;
    }

    return 0;
}

/* Signal lengths:
 *  - input: c_dsp_sig_len
 *  - avg. filter: RP_SPECTR_WF_AVG_FILT
 *  - output: c_dsp_sig_len + RP_SPECTR_WF_AVG_FILT - 1 
 */
int rp_spectr_wf_conv(double *cha_in, double *chb_in,
                      double **cha_out, double **chb_out)
{
    double *cha_o = *cha_out;
    double *chb_o = *chb_out;
    int n;

    if(!cha_in || !chb_in || !cha_o || !chb_o || !rp_wf_avg_filter) {
        fprintf(stderr, "rp_spectr_wf_conv() not initialized\n");
        return -1;
    }

    for(n = 0; n < g_conv_len; n++) {
        int kmin, kmax, k;

        cha_o[n] = chb_o[n] = 0;

        kmin = (n >= RP_SPECTR_WF_AVG_FILT - 1) ? 
            n - (RP_SPECTR_WF_AVG_FILT - 1) : 0;
        kmax = (n < c_dsp_sig_len - 1) ? n : c_dsp_sig_len - 1;

        for(k = kmin; k <= kmax; k++) {
            cha_o[n] += cha_in[k] * rp_wf_avg_filter[n - k];
            chb_o[n] += chb_in[k] * rp_wf_avg_filter[n - k];
        }
    }

    return 0;
}

/* Input sig. length = c_dsp_sig_len
 * Output signal = RP_SPECTR_WF_COL */
inline int __rp_spectr_wf_limit(double a)
{
    a = a > 64 ? 64 : a;
    a = a <  1 ?  1 : a;
    return (int)a;
}

int rp_spectr_wf_dec_map(double *cha_in, double *chb_in,
                         int **cha_out, int **chb_out)
{
    int *cha_o = *cha_out;
    int *chb_o = *chb_out;
    int i, o;
    if(!cha_in || !chb_in || !cha_o || !chb_o) {
        fprintf(stderr, "rp_spectr_wf_dec() not initialized\n");
        return -1;
    }

    /* decimate */
    for(i = c_skip_after_conv, o = 0; o < g_spectr_wf_col; 
        o++, i+=g_dec_wat_step) {
        double cha_s, chb_s;
        if(i >= c_dsp_sig_len) {
            continue;
        }
        /* put to linear scale */
        cha_s = 20*log10(cha_in[i]);
        chb_s = 20*log10(chb_in[i]);

        /* prepare map */
        cha_o[o] = __rp_spectr_wf_limit(round(cha_s * g_mm + g_qq));
        chb_o[o] = __rp_spectr_wf_limit(round(chb_s * g_mm + g_qq));
    }

    return 0;
}

int rp_spectr_wf_add_to_map(int *cha_in, int *chb_in)
{
    int start_idx;
    if(!cha_in || !chb_in || !rp_wf_cha_cont_map || !rp_wf_chb_cont_map ||
       (rp_wf_cont_map_idx == -1)) {
        fprintf(stderr, "rp_spectr_wf_add_to_map() not initialized\n");
        return -1;
    }

    start_idx = (rp_wf_cont_map_idx * g_spectr_wf_col);

    memcpy(&rp_wf_cha_cont_map[start_idx], &cha_in[0], 
           g_spectr_wf_col * sizeof(int));
    memcpy(&rp_wf_chb_cont_map[start_idx], &chb_in[0], 
           g_spectr_wf_col * sizeof(int));
    
    /* Decrease the rp_wf_cont_map_idx and wraps it if necessary but be sure to 
     * wrap it if necessary 
     * TODO: Should we clean last map or override it?
     */
    if(--rp_wf_cont_map_idx < 0)
        rp_wf_cont_map_idx = RP_SPECTR_WF_LIN - 1;

    return 0;
}

int rp_spectr_wf_create_rgb(int *data_in, JSAMPLE **data_out)
{
    JSAMPLE *data_o = *data_out;
    int i, j;
    int start_map_idx = (rp_wf_cont_map_idx+1) * g_spectr_wf_col - 1;

    if(!data_in || !data_o) {
        fprintf(stderr, "rp_spectr_wf_create_rgb() not initialized\n");
        return -1;
    }

    /* Data out is of format R, G, B, R, G, B ... R, G, B */
    for(i = (g_spectr_wf_col * RP_SPECTR_WF_LIN * 3 - 1), j = start_map_idx; 
        i >= 2; i-=3, j--) {
        int colmap_idx;
        if(j < 0) {
            j = g_spectr_wf_col * RP_SPECTR_WF_LIN - 1;
        }
        colmap_idx = data_in[j];

        data_o[i-2] = rp_wf_colmap[colmap_idx][0];
        data_o[i-1] = rp_wf_colmap[colmap_idx][1];
        data_o[i-0] = rp_wf_colmap[colmap_idx][2];
    }

    return 0;
}

int rp_spectr_wf_comp_jpeg(JSAMPLE *data_in, const char *file_str)
{
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr       jerr;
    FILE                       *out_file;
    JSAMPROW                    row_pointer[1];
    int                         row_stride;

    if(!data_in) {
        fprintf(stderr, "rp_spectr_wf_comp_jpeg() not initialized\n");
        return -1;
    }

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);

    if((out_file = fopen(file_str, "wb")) == NULL) {
        fprintf(stderr, "rp_spectr_wf_comp_jpeg() can not open file (%s): %s\n", 
                file_str, strerror(errno));
        return -1;
    }
    jpeg_stdio_dest(&cinfo, out_file);

    cinfo.image_width      = g_spectr_wf_col;
    cinfo.image_height     = RP_SPECTR_WF_LIN;
    cinfo.input_components = 3;
    cinfo.in_color_space   = JCS_RGB;

    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, 95, TRUE);
    jpeg_start_compress(&cinfo, TRUE);

    row_stride = cinfo.image_width * 3;
    while(cinfo.next_scanline < cinfo.image_height) {
        row_pointer[0] = &data_in[cinfo.next_scanline * row_stride];
        (void)jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    jpeg_finish_compress(&cinfo);

    fclose(out_file);

    jpeg_destroy_compress(&cinfo);

    sync();

    return 0;
}
