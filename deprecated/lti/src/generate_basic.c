/**
 * 
 *
 * @brief Red Pitaya direct FPGA signal generator access
 *        
 *
 * @Author Jure Menart <juremenart@gmail.com>
 * @Author Dashpi <dashpi46@gmail.com>
 *         
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "generate_basic.h"
#include "fpga_awg.h"

/**
 * GENERAL DESCRIPTION:
 *
 * The code below provides direct access to signal generator FPGA logic.
 *
 *
 *  
 *   -->[data & control]--+-->[FPGA buf 1]--><DAC 1>
 *                        |
 *                        -->[FPGA buf 2]--><DAC 2>
 *    
 *   The code enables the access to the signal generator samples and the logic that defines the sample output rate.
 *  
 *
 */


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





/*----------------------------------------------------------------------------------*/
/**
 * @brief Get signal generator sample table pointers (data part)
 *
 */

int lti_fpga_get_gen_ptr(int **cha_signal, int **chb_signal)
{
    *cha_signal = (int *)g_awg_cha_mem;
    *chb_signal = (int *)g_awg_chb_mem;
    return 0;
}

/*----------------------------------------------------------------------------------*/
/**
 * @brief Direct fpga control (control part)
 *
 * @param[in] ch         Channel number [0, 1].
 * @param[in] param      Parameter: 0 - state(8 bits LSB), 1 - scale, 2 - offset, 3 - wrap, 4 - start_offs, 5 - step, 6 - state (32 bits - both channels simultaneously controlled)
 * @param[in] value      Sets the value to be written
 */
void dir_gen_fpga_set(uint32_t ch, int param, uint32_t value) 
{
  
   uint32_t state_machine = g_awg_reg->state_machine_conf;
   
   switch (param) {
     case 0:
       if (ch==0)
       {
	state_machine &= ~0xff;
        g_awg_reg->state_machine_conf = state_machine | (value & 0xff);
       }
       else
       {
	state_machine &= ~0xff0000;
        g_awg_reg->state_machine_conf = state_machine | (value & 0xff0000);	 
       }
     break; 
     
     case 1:
       if (ch==0)
       {
        g_awg_reg->cha_scale_off = (g_awg_reg->cha_scale_off & (~0x3fff)) | (value & 0x3fff);
       }
       else
       {
        g_awg_reg->chb_scale_off = (g_awg_reg->chb_scale_off & (~0x3fff)) | (value & 0x3fff); 
       }
     break;      
     
     case 2:
       if (ch==0)
       {
        g_awg_reg->cha_scale_off = (g_awg_reg->cha_scale_off & (0xffff)) | ((value<<16) & 0x3fff0000);
       }
       else
       {
        g_awg_reg->chb_scale_off = (g_awg_reg->chb_scale_off & (0xffff)) | ((value<<16) & 0x3fff0000); 
       }
     break;     
     
     case 3:
       if (ch==0)
       {
        g_awg_reg->cha_count_wrap =  (value & 0x3fffffff);
       }
       else
       {
        g_awg_reg->chb_count_wrap =  (value & 0x3fffffff);
       }
     break;          

     case 4:
       if (ch==0)
       {
        g_awg_reg->cha_start_off =  (value & 0x3fffffff);
       }
       else
       {
        g_awg_reg->chb_start_off =  (value & 0x3fffffff);
       }
     break;        

     case 5:
       if (ch==0)
       {
        g_awg_reg->cha_count_step =  (value & 0x3fffffff);
       }
       else
       {
        g_awg_reg->chb_count_step =  (value & 0x3fffffff);
       }
     break;   

     case 6:

        g_awg_reg->state_machine_conf =  value;
       
     break;            
     
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


