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

#ifndef __WATERFALL_H
#define __WATERFALL_H

#define RP_SPECTR_WF_LIN 100
/* In fact Columns are re-calculated based on possible decimation */
#define RP_SPECTR_WF_COL 640

#define RP_SPECTR_WF_AVG_FILT 20
#define RP_SPECTR_WF_SPEC_MAX 160
#define RP_SPECTR_WF_SPEC_NOI 80
#define RP_SPECTR_WF_MAP_MAX  64
#define RP_SPECTR_WF_MAP_NOI  20

#include "jpeglib.h"

/*** Main Warerfall module calls ****/
int rp_spectr_wf_init(void);
int rp_spectr_wf_clean(void);

/* Reset the main map structure */
int rp_spectr_wf_clean_map(void);

/* Processes the input signal and put it to the map which is builded from 
 * multiple acquisitions.
 * Input signal length = c_dsp_sig_len (output from FFT) */
int rp_spectr_wf_calc(double *cha_in, double *chb_in);


/* Build the waterfall diagram out of the collected acquisitions and stores it */
int rp_spectr_wf_save_jpeg(const char *wf_file1, const char *wf_file2);

/*** Internal steps used in the processing ***/
/* Convolution:
 * Input length = c_dsp_sig_len 
 * Output length = c_dsp_sig_len + RP_SPECTR_WF_AVG_FILT - 1
 */
int rp_spectr_wf_conv(double *cha_in, double *chb_in,
                      double **cha_out, double **chb_out);

/* Decimation & moving to linear scale
 * Input sig. length = c_dsp_sig_len
 * Output signal = RP_SPECTR_WF_COL */
int rp_spectr_wf_dec_map(double *cha_in, double *chb_in,
                         int **cha_out, int **chb_out);

/* Adds new acquisition to the waterfall map 
 * Input sig length = RP_SPECTR_WF_COL 
 */
int rp_spectr_wf_add_to_map(int *cha_in, int *chb_in);

/* Creates RGB image in internal structures, used to dump JPEG or BMP
 * Input signal length = RP_SPECTR_WF_COL * RP_SPECTR_WF_LIN 
 * Output signal length = RP_SPECTR_WF_COL * RP_SPECTR_WF_LIN * 3 (RGB) 
 */
int rp_spectr_wf_create_rgb(int *data_in, JSAMPLE **data_out);

/* Compress image and store it, 
 * Input signal is of size RP_SPECTR_WF_COL * RP_SPECTR_WF_LIN *3 
 */
int rp_spectr_wf_comp_jpeg(JSAMPLE *data_in, const char *file_out);

#endif //__WATERFALL_H
