/**
 *
 * @brief Red Pitaya simple signal acquisition utility.
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
#include <getopt.h>
#include <string.h>
#include <sys/param.h>
#include <iostream>

#include "rp.h"
#include "version.h"
#include "options.h"

#ifdef Z20_250_12
#include "api250-12/rp-spi.h"
#endif

/** Program name */
const char *g_argv0 = NULL;


rp_channel_trigger_t getTrigChByTrigSource(rp_acq_trig_src_t src){

    switch(src){
        case RP_TRIG_SRC_NOW:
            return RP_T_CH_1;
           
        case RP_TRIG_SRC_CHA_PE:
        case RP_TRIG_SRC_CHA_NE:
            return RP_T_CH_1;
    
        case RP_TRIG_SRC_CHB_PE:
        case RP_TRIG_SRC_CHB_NE:
            return RP_T_CH_2;

#ifdef Z20_125_4CH
        case RP_TRIG_SRC_CHC_PE:
        case RP_TRIG_SRC_CHC_NE:
            return RP_T_CH_3;

        case RP_TRIG_SRC_CHD_PE:
        case RP_TRIG_SRC_CHD_NE:
            return RP_T_CH_4;
#endif

        case RP_TRIG_SRC_EXT_PE:
        case RP_TRIG_SRC_EXT_NE:
            return RP_T_CH_EXT;
        default:
            return RP_T_CH_1;
    }
}


typedef struct {
    uint32_t aa;
    uint32_t bb;
    uint32_t pp;
    uint32_t kk;
} ecu_shape_filter_t;

/** Acquire utility main */
int main(int argc, char *argv[])
{
    
    g_argv0 = argv[0];
    auto option = parse(argc,argv);

    if (option.error || option.showHelp || option.showVersion){
        usage(g_argv0);
        return -1;
    }



#ifdef Z20_250_12
    if (!option.disableReset) {
        rp_spi_fpga::rp_spi_load_via_fpga("/opt/redpitaya/lib/configs/AD9613BCPZ-250.xml");
    }
#endif


    if (rp_InitReset(option.reset_hk) != RP_OK){
        fprintf(stderr,"Error init rp api\n");
        return -1;
    }

    auto calib = rp_GetCalibrationSettings();
    if (option.disableCalibration){
        calib = rp_GetDefaultCalibrationSettings();
    }

#if defined Z10 || defined Z20_125
    if (!option.enableEqualization){
        calib.hi_filter_aa_ch1 = 0;
        calib.hi_filter_bb_ch1 = 0;
        calib.hi_filter_aa_ch2 = 0;
        calib.hi_filter_bb_ch2 = 0;
        calib.low_filter_aa_ch1 = 0;
        calib.low_filter_bb_ch1 = 0;
        calib.low_filter_aa_ch2 = 0;
        calib.low_filter_bb_ch2 = 0;
    }

    if (!option.enableShaping){
        calib.hi_filter_pp_ch1 = 0;
        calib.hi_filter_kk_ch1 = 0xffffff;
        calib.hi_filter_pp_ch2 = 0;
        calib.hi_filter_kk_ch2 = 0xffffff;
        calib.low_filter_pp_ch1 = 0;
        calib.low_filter_kk_ch1 = 0xffffff;
        calib.low_filter_pp_ch2 = 0;
        calib.low_filter_kk_ch2 = 0xffffff;
    }
#endif

#if defined Z20_125_4CH
    if (!option.enableEqualization){
        calib.chA_hi_aa = 0;
        calib.chA_hi_bb = 0;
        calib.chB_hi_aa = 0;
        calib.chB_hi_bb = 0;
        calib.chC_hi_aa = 0;
        calib.chC_hi_bb = 0;
        calib.chD_hi_aa = 0;
        calib.chD_hi_bb = 0;

        calib.chA_low_aa = 0;
        calib.chA_low_bb = 0;
        calib.chB_low_aa = 0;
        calib.chB_low_bb = 0;
        calib.chC_low_aa = 0;
        calib.chC_low_bb = 0;
        calib.chD_low_aa = 0;
        calib.chD_low_bb = 0;
    }

    if (!option.enableShaping){
        calib.chA_hi_pp = 0;
        calib.chA_hi_kk = 0xffffff;
        calib.chB_hi_pp = 0;
        calib.chB_hi_kk = 0xffffff;
        calib.chC_hi_pp = 0;
        calib.chC_hi_kk = 0xffffff;
        calib.chD_hi_pp = 0;
        calib.chD_hi_kk = 0xffffff;

        calib.chA_low_pp = 0;
        calib.chA_low_kk = 0xffffff;
        calib.chB_low_pp = 0;
        calib.chB_low_kk = 0xffffff;
        calib.chC_low_pp = 0;
        calib.chC_low_kk = 0xffffff;
        calib.chD_low_pp = 0;
        calib.chD_low_kk = 0xffffff;
    }
#endif

    rp_CalibrationSetParams(calib);
    
    

#ifdef Z20_250_12
    rp_AcqSetAC_DC(RP_CH_1,option.ac_dc_mode[RP_CH_1]);
    rp_AcqSetAC_DC(RP_CH_2,option.ac_dc_mode[RP_CH_2]);
#endif


    rp_AcqSetGain(RP_CH_1,option.attenuator_mode[RP_CH_1]);
    rp_AcqSetGain(RP_CH_2,option.attenuator_mode[RP_CH_2]);
#ifdef Z20_125_4CH
    rp_AcqSetGain(RP_CH_3,option.attenuator_mode[RP_CH_3]);
    rp_AcqSetGain(RP_CH_4,option.attenuator_mode[RP_CH_4]);
#endif

    rp_AcqSetDecimationFactor(option.decimation);
    
    // Default trigger delay 0 and equal ADC_BUUFER / 2
	rp_AcqSetTriggerDelay(-ADC_BUFFER_SIZE / 2.0 + option.dataSize);
    rp_AcqSetTriggerLevel(getTrigChByTrigSource(option.trigger_mode),option.trigger_level);

	rp_AcqStart();
	rp_AcqSetTriggerSrc(option.trigger_mode);

	bool fillState = false;
	rp_acq_trig_state_t trig_state = RP_TRIG_STATE_WAITING;

	while(1) {
		rp_AcqGetTriggerState(&trig_state);
		if (trig_state == RP_TRIG_STATE_TRIGGERED) {
			break;
		} else {
			usleep(1);
		}
	}

	while(!fillState){
		rp_AcqGetBufferFillState(&fillState);
	}
	rp_AcqStop();

  	uint32_t pos = 0;
    uint32_t acq_u_size = option.dataSize;
	rp_AcqGetWritePointerAtTrig(&pos);
	
    int start_ch = (int)RP_CH_1;
#ifdef Z20_125_4CH    
    int end_ch = (int)RP_CH_4;
#else
    int end_ch = (int)RP_CH_2;
#endif

    if (option.showInVolt){      
        float buffers[CHANNELS][ADC_BUFFER_SIZE];
        for(auto i = start_ch; i <= end_ch; i++){
            auto ch = (rp_channel_t)i;
            rp_AcqGetDataV(ch,pos, &acq_u_size, buffers[i]);

        }
        for(uint32_t i = 0; i< option.dataSize; i++){
            bool printSeparator = false;
            for(auto j = start_ch; j <= end_ch; j++){
                if (printSeparator){
                    fprintf(stdout," ");
                }
                fprintf(stdout,"%f",buffers[j][i]);
                printSeparator = true;
            }
            fprintf(stdout,"\n");
        }

    }else{
        int16_t buffers[CHANNELS][ADC_BUFFER_SIZE];
        
        for(auto i = start_ch; i <= end_ch; i++){
            auto ch = (rp_channel_t)i;
            rp_AcqGetDataRaw(ch,pos, &acq_u_size, buffers[i]);
        }
        
        const char *format_str = (option.showInHex == false) ? "%7d" : "0x%08X";
        for(uint32_t i = 0; i< option.dataSize; i++){
            bool printSeparator = false;
            for(auto j = start_ch; j <= end_ch; j++){
                if (printSeparator){
                    fprintf(stdout," ");
                }
                fprintf(stdout,format_str,buffers[j][i]);
                printSeparator = true;
            }
            fprintf(stdout,"\n");
        }
    }    

    if (rp_Release() != RP_OK){
        fprintf(stderr,"Error release rp api\n");
        return -1;
    }
    return 0;
}
