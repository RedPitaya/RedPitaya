/** @file generate.c
 *
 * $Id: generate.c 882 2013-12-16 12:46:01Z crt.valentincic $
 *
 * @brief Red Pitaya simple signal/function generator with pre-defined signal types.
 * @author Jure Menart <juremenart@gmail.com>
 * @copyright Red Pitaya  http://www.redpitaya.com
 */

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "generate.h"
#include "fpga_awg.h"

/**
 * GENERAL DESCRIPTION:
 *
 * The code below performs a function of a signal generator, which produces
 * a a signal of user-selectable predefined Signal shape
 * [Sine, Square, Triangle], Amplitude and Frequency on a selected Channel:
 *
 *
 *                   /-----\
 *   Signal shape -->|     | -->[data]--+-->[FPGA buf 1]--><DAC 1>
 *   Amplitude ----->| AWG |            |
 *   Frequency ----->|     |             -->[FPGA buf 2]--><DAC 2>
 *                   \-----/            ^
 *                                      |
 *   Channel ---------------------------+ 
 *
 *
 * This is achieved by first parsing the four parameters defining the 
 * signal properties from the command line, followed by synthesizing the 
 * signal in data[] buffer @ 125 MHz sample rate within the
 * generate_signal() function, depending on the Signal shape, Amplitude
 * and Frequency parameters. The data[] buffer is then transferred
 * to the specific FPGA buffer, defined by the Channel parameter -
 * within the write_signal_fpga() function.
 * As an alternative a shape of output signal can be through the file system
 * by applying values into gen_waveform_file file.
 * The FPGA logic continuously sends the data from both FPGA buffers to the
 * corresponding DACs @ 125 MHz, which in turn produces the synthesized
 * signal on Red Pitaya SMA output connectors labeled DAC1 & DAC2.
 *
 */

/** Maximal signal amplitude [Vpp]. */
const double c_max_amplitude = 2.0;

/** Temporary data buffer, used during signal shape calculation. */
int32_t ch1_data[AWG_SIG_LEN];
int32_t ch2_data[AWG_SIG_LEN];

/** Pointer to externally defined calibration parameters. */
rp_calib_params_t *gen_calib_params = NULL;

/** Maximal Signal Voltage on DAC outputs on channel A. It is expressed in [V],
 * and calculated from apparent Back End Full Scale calibration parameter.
 */
float ch1_max_dac_v;

/** Maximal Signal Voltage on DAC outputs on channel B. It is expressed in [V],
 * and calculated from apparent Back End Full Scale calibration parameter.
 */
float ch2_max_dac_v;

/** Predefined File name, used for definition of arbitrary Signal Shape. */
const char *gen_waveform_file1="/tmp/gen_ch1.csv";
const char *gen_waveform_file2="/tmp/gen_ch2.csv";


/*----------------------------------------------------------------------------------*/
/**
 * Synthesize a desired signal.
 *
 * Generates/synthesizes a signal, based on three predefined signal
 * types/shapes, signal amplitude & frequency. The data[] vector of 
 * samples at 125 MHz is generated to be re-played by the FPGA AWG module.
 *
 * @param[in]  ampl           Signal amplitude [Vpp].
 * @param[in]  freq           Signal frequency [Hz].
 * @param[in]  calib_dc_offs  Calibrated Instrument DC offset
 * @param[in]  max_dac_v      Maximum DAC voltage in [V]
 * @param[in]  user_dc_offs   User defined DC offset
 * @param[in]  type           Signal type/shape [Sine, Square, Triangle].
 * @param[in]  data           Returned synthesized AWG data vector.
 * @param[out] awg            Returned AWG parameters.
 */
void synthesize_signal(float ampl, float freq, int calib_dc_offs, int calib_fs,
                       float max_dac_v, float user_dc_offs, awg_signal_t type, 
                       int32_t *data, awg_param_t *awg) 
{
    uint32_t i;

    /* Various locally used constants - HW specific parameters */
    const int trans0 = 30;
    const int trans1 = 300;
    const float tt2 = 0.249;

    int trans = round(freq / 1e6 * ((float) trans1)); /* 300 samples at 1 MHz */
    int user_dc_off_cnt = 
        round((1<<(c_awg_fpga_dac_bits-1)) * user_dc_offs / max_dac_v);
    uint32_t amp; 

    awg->offsgain = ((calib_dc_offs+user_dc_off_cnt) << 16) | 0x2000;
    awg->step = round(65536.0 * freq/c_awg_smpl_freq * ((float) AWG_SIG_LEN));
    awg->wrap = round(65536 * AWG_SIG_LEN-1);


    
    
    //= (ampl) * (1<<(c_awg_fpga_dac_bits-2));
    //fpga_awg_calc_dac_max_v(calib_fs)
    
    amp= round(ampl/2/fpga_awg_calc_dac_max_v(calib_fs)* ((1<<(c_awg_fpga_dac_bits-1))-1) );
    
    if (amp >= (1<<(c_awg_fpga_dac_bits-1))) {
        /* Truncate to max value if needed */
        amp = (1<<(c_awg_fpga_dac_bits-1))-1;
    }

    if (trans <= 10) {
        trans = trans0;
    }

    /* Fill data[] with appropriate buffer samples */
    for(i = 0; i < AWG_SIG_LEN; i++) {
        /* Sine */
        if (type == eSignalSine) {
            data[i] = round(amp * cos(2*M_PI*(float)i/(float)AWG_SIG_LEN));
        }
 
        /* Square */
        if (type == eSignalSquare) {
            data[i] = round(amp * cos(2*M_PI*(float)i/(float)AWG_SIG_LEN));
            if (data[i] > 0)
                data[i] = amp;
            else 
                data[i] = -amp;

            /* Soft linear transitions */
            float mm, qq, xx, xm;
            float x1, x2, y1, y2;    

            xx = i;
            xm = AWG_SIG_LEN;
            mm = -2.0*(float)amp/(float)trans; 
            qq = (float)amp * (2 + xm/(2.0*(float)trans));

            x1 = xm * tt2;
            x2 = xm * tt2 + (float)trans;

            if ( (xx > x1) && (xx <= x2) ) {

                y1 = (float)amp;
                y2 = -(float)amp;

                mm = (y2 - y1) / (x2 - x1);
                qq = y1 - mm * x1;

                data[i] = round(mm * xx + qq); 
            }

            x1 = xm * 0.75;
            x2 = xm * 0.75 + trans;

            if ( (xx > x1) && (xx <= x2)) {  

                y1 = -(float)amp;
                y2 = (float)amp;

                mm = (y2 - y1) / (x2 - x1);
                qq = y1 - mm * x1;

                data[i] = round(mm * xx + qq); 
            }
        }

        /* Triangle */
        if (type == eSignalTriangle) {
            data[i] = 
                round(-1.0*(float)amp*
                     (acos(cos(2*M_PI*(float)i/(float)AWG_SIG_LEN))/M_PI*2-1));
        }

        /* TODO: Remove, not necessary in C/C++. */
        if(data[i] < 0)
            data[i] += (1 << 14);
    }
}


/*----------------------------------------------------------------------------------*/
/**
 * @brief Read Time Based Signal Definition from the file system
 *
 * The gen_waveform_file is a simple text file, constituted from lines of triple
 * data: time_base, channel A and channel B values. At most AWG_SIG_LEN lines are
 * parsed and apparent data are put to the specified buffers. It is expected the
 * specified buffers are large enough, no check is made within a function. After the
 * Signal Definition is read from a file, the final Signal Shape is calculated with
 * calculate_data() function.
 *
 * @param[out]  time_vect Time base vector
 * @param[out]  ch1_data  Channel A buffer
 * @param[out]  ch2_data  Channel B buffer
 * @retval      -1        Failure, error message is output on standard error
 * @retval      >0        Number of parsed lines
 */
int read_in_file(int chann,  float *ch_data)
{
    FILE *fi = NULL;
    int i, read_size, samples_read=0;
    
    /* open file */
    if (chann==1){
      fi = fopen(gen_waveform_file1, "r+");
      //fi = fopen(gen_waveform_file1, "r+");      
      if(fi == NULL) {
	       fprintf(stderr, "read_in_file(): Can not open input file (%s): %s\n", gen_waveform_file1, strerror(errno));
	return -1;
      }
    }
    else{
      fi = fopen(gen_waveform_file2, "r+");
      
      if(fi == NULL) { 
        fprintf(stderr, "read_in_file(): Can not open input file (%s): %s\n", gen_waveform_file2, strerror(errno));
	
	return -1;
	
      } 
    }
    
    /* parse at most AWG_SIG_LEN lines and save data to the specified buffers */
    for(i = 0; i < AWG_SIG_LEN; i++) {
      read_size = fscanf(fi, "%f \n", &ch_data[i]);
	    
	    if((read_size == EOF) || (read_size != 1)) {
           i -= 1;
	       break;
        }
    }
    samples_read = i + 1;

    if (samples_read>=AWG_SIG_LEN)
      samples_read=AWG_SIG_LEN-1;
     
    /* check for errors */
    if(i == 0) {
        fprintf(stderr, "read_in_file() cannot read in signal, wrong format?\n");
	    fclose(fi);
        return -1;
	}
    /* close a file */
    fclose(fi);

    /* and return the number of parsed lines */
    return samples_read;
}


/*----------------------------------------------------------------------------------*/
/**
 * @brief Calculate Signal Shape based on Time Based Signal definition

 * Function is intended to calculate the shape of utput Signal for the individual channel,
 * i.e. a function must be called separatelly for each signal.
 * Time Based Signal definition is taken from input parameters in_data. time_vect and
 * in_data_len arguments. The output signal shape is additionally parameterized with
 * amp, calib_dc_offs, max_dac_v and user_dc_offs arguments, which depict properties of
 * referenced channel. Calculated Signal Shape is returned in the specified out_data buffer.
 * Beside of shape calculation the apparent AWG settings for referenced channel are calculated.
 *
 * @param[in]  in_data        Array of signal values, applied at apparent time from Time Based vector
 * @param[in]  time_vect      Array, specifying Time Based values
 * @param[in]  in_data_len    Number of valid entries in the specified buffers
 * @param[in]  amp            Maximal amplitude of calculated Signal
 * @param[in]  calib_dc_offs  Calibrated DC offset, specified in ADC counts
 * @param[in]  max_dac_v      Maximal Voltage on DAC outputs, expressed in [V]
 * @param[in]  user_dc_offs   Configurable DC offset, expressed in [V]
 * @param[out] out_data       Output Signal Shape data buffer
 * @param[out] awg            Modified AWG settings with updated offsgain, step and wrap parameters

 * @retval -1  Failure, error message is output on standard error
 * @retval  0  Success
 */
int  calculate_data(float *in_data, int in_data_len, 
                    float amp, float freq, int calib_dc_offs, uint32_t calib_fs, float max_dac_v, 
                    float user_dc_offs, int32_t *out_data, awg_param_t *awg)
{
    float max_amp,min_amp;
    float k_norm;
    int i;

    /* calculate configurable DC offset, expressed in ADC counts */
    int user_dc_off_cnt = 
        round((1<<(c_awg_fpga_dac_bits-1)) * user_dc_offs / max_dac_v);

    /* partial check for validity of input parameters */
    if((in_data == NULL) || 
       (out_data == NULL) || (awg == NULL) || (in_data_len >= AWG_SIG_LEN)) {
        fprintf(stderr, "Internal error, the Time Based signal definition is not correctly specified.\n");
        return -1;
    }

    /* modify AWG settings  */
    awg->offsgain = ((calib_dc_offs+user_dc_off_cnt) << 16) | 0x2000;
    awg->step = round(65536 * freq/c_awg_smpl_freq * in_data_len); 
    awg->wrap = round(65536 * in_data_len-1);

    /* retrieve max amplitude of the specified Signal Definition, it is used for the normalization */
    max_amp = -1e30;
    min_amp = +1e30;
    
    for(i = 0; i < in_data_len; i++) {
        if((in_data[i]) > max_amp)
            max_amp = in_data[i];
	
        if((in_data[i]) < min_amp)
            min_amp = in_data[i];
	
    }
    /* calculate normalization factor */
    
    if (max_amp-min_amp==0)      
    {
      if (max_amp==0)
	
	k_norm=0;
      
      else
	
	k_norm=  (float)((1<<(c_awg_fpga_dac_bits-1))-1) * amp /(max_amp*2) /fpga_awg_calc_dac_max_v(calib_fs);
    } 
    else
      
      k_norm = (float)((1<<(c_awg_fpga_dac_bits-1))-1) * amp /(max_amp-min_amp) /fpga_awg_calc_dac_max_v(calib_fs);
    
    /* normalize Signal values */
    for(i = 0; i < in_data_len; i++) {
        out_data[i] = round(k_norm * (in_data[i]-(max_amp+min_amp)/2));
	
	// clipping
	if(out_data[i]>(1<<(c_awg_fpga_dac_bits-1))-1)
	  out_data[i]=(1<<(c_awg_fpga_dac_bits-1))-1;
	if(out_data[i]<(-(1<<(c_awg_fpga_dac_bits-1))))
	  out_data[i]=(-(1<<(c_awg_fpga_dac_bits-1)));
	
	
        if(out_data[i] < 0)
            out_data[i] += (1 << 14);
    }
    /* ...and pad it with zero */
    for(i = i+1; i < AWG_SIG_LEN; i++) {
        out_data[i] = 0;
    }
    
    return 0;
}

/*----------------------------------------------------------------------------------*/
/**
 * @brief Write synthesized data[] to FPGA buffer.
 *
 * @param[in] ch         Channel number [0, 1].
 * @param[in] mode       Trigger mode: 0 - continuous, 1 - single, 2 - external
 * @param[in] trigger    Trigger one pulse (if mode == single).
 * @param[in] data       AWG synthesized data to be written to FPGA.
 * @param[in] awg        AWG parameters to be written to FPGA.
 */
void write_data_fpga(uint32_t ch, int mode, int trigger, const int32_t *data,
                     const awg_param_t *awg, int wrap) 
{
    uint32_t i;
    int mode_mask = 0;
    uint32_t state_machine = g_awg_reg->state_machine_conf;

    switch(mode) {
    case 0: /* continuous */
      if (wrap==1)
      {
	mode_mask = 0x01;
	break;
      }
      else	  
      {
        mode_mask = 0x11;
        break;
      }
    case 1: /* single */
        if (trigger)
            mode_mask = 0x21;
        else
            mode_mask = 0x20;
        break;
    case 2: /* external */
        mode_mask = 0x22;
        break;
    }

    if(ch == 0) {
        /* Channel A */
        state_machine &= ~0xff;

        g_awg_reg->state_machine_conf = state_machine | 0xC0;
        g_awg_reg->cha_scale_off      = awg->offsgain;
        g_awg_reg->cha_count_wrap     = awg->wrap;
        g_awg_reg->cha_count_step     = awg->step;
        g_awg_reg->cha_start_off      = 0;

        for(i = 0; i < AWG_SIG_LEN; i++) {
            g_awg_cha_mem[i] = data[i];
        }

        g_awg_reg->state_machine_conf = state_machine | mode_mask;
    } else {
        /* Channel B */
        state_machine &= ~0xff0000;

        g_awg_reg->state_machine_conf = state_machine | 0xC00000;
        g_awg_reg->chb_scale_off      = awg->offsgain;
        g_awg_reg->chb_count_wrap     = awg->wrap;
        g_awg_reg->chb_count_step     = awg->step;
        g_awg_reg->chb_start_off      = 0;

        for(i = 0; i < AWG_SIG_LEN; i++) {
            g_awg_chb_mem[i] = data[i];
        }

        g_awg_reg->state_machine_conf = state_machine | (mode_mask<<16);
    }
}


/*----------------------------------------------------------------------------------*/
/** @brief Initialize specified Signal Shape data buffer and apparent AWG settings
 *
 * @param[in]  calib_dc_offs  calibration DC offset value
 * @param[out] data           Signal Shape data buffer
 * @param[out] awg            AWG parameters.
 */
void clear_signal(int calib_dc_offs, int32_t *data, awg_param_t *awg)
{
    int i;

    awg->offsgain = ((calib_dc_offs) << 16) | 0x2000;
    awg->step = 0;
    awg->wrap = 0;

    for(i = 0; i < AWG_SIG_LEN; i++)
        data[i]=0;
}


/*----------------------------------------------------------------------------------*/
/** @brief Initialize Arbitrary Signal Generator module
 *
 * A function is intended to be called within application initialization. It's purpose
 * is to remember a specified pointer to calibration parameters, to initialie
 * Arbitrary Waveform Generator module and to calculate maximal voltage, which can be
 * applied on DAC device on individual channel.
 *
 * @param[in]  calib_params  pointer to calibration parameters
 * @retval     -1 failure, error message is reported on standard error
 * @retval      0 successful initialization
 */

int generate_init(rp_calib_params_t *calib_params)
{
    gen_calib_params = calib_params;

    if(fpga_awg_init() < 0) {
        return -1;
    }

    ch1_max_dac_v = fpga_awg_calc_dac_max_v(gen_calib_params->be_ch1_fs);
    ch2_max_dac_v = fpga_awg_calc_dac_max_v(gen_calib_params->be_ch2_fs);
    return 0;
}


/*----------------------------------------------------------------------------------*/
/** @brief Cleanup Arbitrary Signal Generator module
 *
 * A function is intended to be called on application's termination. The main purpose
 * of this function is to release allocated resources...
 *
 * @retval      0 success, never fails.
 */
int generate_exit(void)
{
    fpga_awg_exit();

    return 0;
}

/*----------------------------------------------------------------------------------*/
/**
 * @brief Update Arbitrary Signal Generator module towards actual settings.
 *
 * A function is intended to be called whenever one of the following settings on each channel
 * is modified
 *    - enable
 *    - signal type
 *    - amplitude
 *    - frequency
 *    - DC offset
 *    - trigger mode
 *
 * @param[in] params  Pointer to overall configuration parameters
 * @retval -1 failure, error message is repoted on standard error device
 * @retval  0 succesful update
 */
int generate_update(rp_app_params_t *params)
{
    awg_param_t ch1_param, ch2_param;
    awg_signal_t ch1_type;
    awg_signal_t ch2_type;
    int ch1_enable = params[GEN_ENABLE_CH1].value;
    int ch2_enable = params[GEN_ENABLE_CH2].value;
    
    //float time_vect[AWG_SIG_LEN], ch1_amp[AWG_SIG_LEN], ch2_amp[AWG_SIG_LEN];
    
    float ch1_arb[AWG_SIG_LEN];
    float ch2_arb[AWG_SIG_LEN];
    
    int wrap;
    
    //int invalid_file=0;
    
    int in_smpl_len1 = 0;
    int in_smpl_len2 = 0;

    ch1_type = (awg_signal_t)params[GEN_SIG_TYPE_CH1].value;
    ch2_type = (awg_signal_t)params[GEN_SIG_TYPE_CH2].value;

 
    
    if((ch1_type == eSignalFile))
        if((in_smpl_len1 = read_in_file(1, ch1_arb)) < 0)
	{   // Invalid file
	    params[GEN_ENABLE_CH1].value=0;
	    params[GEN_SIG_TYPE_CH1].value=eSignalSine;
	    ch1_type = params[GEN_SIG_TYPE_CH1].value;
	    ch1_enable = params[GEN_ENABLE_CH1].value;
            //invalid_file=1;
	}
	    
    if((ch2_type == eSignalFile))
        if((in_smpl_len2 = read_in_file(2, ch2_arb)) < 0)
	{   // Invalid file
	    params[GEN_ENABLE_CH2].value=0;
	    params[GEN_SIG_TYPE_CH2].value=eSignalSine;
	    ch2_type = params[GEN_SIG_TYPE_CH2].value;
	    ch2_enable = params[GEN_ENABLE_CH2].value;
           // invalid_file=1;	
	}
	 
    /* Waveform from signal gets treated differently then others */
    if(ch1_enable > 0) {
        if(ch1_type < eSignalFile) {
            synthesize_signal(params[GEN_SIG_AMP_CH1].value,
                              params[GEN_SIG_FREQ_CH1].value,
                              gen_calib_params->be_ch1_dc_offs,
			       gen_calib_params->be_ch1_fs,
                              ch1_max_dac_v,
                              params[GEN_SIG_DCOFF_CH1].value,
                              ch1_type, ch1_data, &ch1_param);
			       wrap=0;  // whole buffer used
        } else {
            /* Signal file */
            calculate_data(ch1_arb,  in_smpl_len1,
                           params[GEN_SIG_AMP_CH1].value, params[GEN_SIG_FREQ_CH1].value,
                           gen_calib_params->be_ch1_dc_offs,
			    gen_calib_params->be_ch1_fs,
                           ch1_max_dac_v, params[GEN_SIG_DCOFF_CH1].value,
                           ch1_data, &ch1_param);
	    wrap=0;
	    if (in_smpl_len1<AWG_SIG_LEN)
	      wrap=1; // wrapping after (in_smpl_lenx) samples
        }
    } else {
        clear_signal(gen_calib_params->be_ch1_dc_offs, ch1_data, &ch1_param);
    }
    write_data_fpga(0, params[GEN_TRIG_MODE_CH1].value,
                    params[GEN_SINGLE_CH1].value,
                    ch1_data, &ch1_param, wrap);

    /* Waveform from signal gets treated differently then others */
    if(ch2_enable > 0) {
        if(ch2_type < eSignalFile) {
            synthesize_signal(params[GEN_SIG_AMP_CH2].value,
                              params[GEN_SIG_FREQ_CH2].value,
                              gen_calib_params->be_ch2_dc_offs,
			       gen_calib_params->be_ch2_fs,
                              ch2_max_dac_v,
                              params[GEN_SIG_DCOFF_CH2].value,
                              ch2_type, ch2_data, &ch2_param);
	                      wrap=0; // whole buffer used
        } else {
            /* Signal file */
            calculate_data(ch2_arb, in_smpl_len2,
                           params[GEN_SIG_AMP_CH2].value, params[GEN_SIG_FREQ_CH2].value,
			   gen_calib_params->be_ch2_dc_offs,
			   gen_calib_params->be_ch2_fs,
			   ch2_max_dac_v, params[GEN_SIG_DCOFF_CH2].value,
			   ch2_data, &ch2_param);
	    wrap=0;
	    if (in_smpl_len2<AWG_SIG_LEN)
	      wrap=1; // wrapping after (in_smpl_lenx) samples
        }
    } else {
        clear_signal(gen_calib_params->be_ch2_dc_offs, ch2_data, &ch2_param);
    }
    write_data_fpga(1, params[GEN_TRIG_MODE_CH2].value,
                    params[GEN_SINGLE_CH2].value,
                    ch2_data, &ch2_param, wrap);

    /* Always return singles to 0 */
    params[GEN_SINGLE_CH1].value = 0;
    params[GEN_SINGLE_CH2].value = 0;

    
    //if (invalid_file==1)
    //  return -1;  // Use this return value to notify the GUI user about invalid file. 
    
    return 0;
}

