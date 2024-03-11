#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <vector>
#include <string.h>
#include <unistd.h>

#include "generate.h"
#include "rp_arb.h"

std::vector<std::string> g_arbList;

uint8_t getChannels(){
    uint8_t c = 0;
    if (rp_HPGetFastDACChannelsCount(&c) != RP_HP_OK){
        fprintf(stderr,"[Error] Can't get fast DAC channels count\n");
    }
    return c;
}

uint32_t getMaxSpeed(){
    uint32_t c = 0;
    if (rp_HPGetBaseFastDACSpeedHz(&c) != RP_HP_OK){
        fprintf(stderr,"[Error] Can't get fast DAC speed\n");
    }
    return c;
}

float fullScale(){

    float fs = 0;
    if (rp_HPGetHWDACFullScale(&fs) != RP_HP_OK){
        fprintf(stderr,"[Error:generate_writeData] Can't get fast DAC full scale\n");
        return RP_NOTS;
    }

    float c = 0;
    if (rp_HPGetFastDACGain(0,&c) != RP_HP_OK){
        fprintf(stderr,"[Error] Can't get fast DAC full scale\n");
    }
    return c * fs;
}

models_t getModel(){
    rp_HPeModels_t c = STEM_125_14_v1_0;
    if (rp_HPGetModel(&c) != RP_HP_OK){
        fprintf(stderr,"[Error] Can't get board model\n");
    }

    switch (c)
    {
        case STEM_125_10_v1_0:
        case STEM_125_14_v1_0:
        case STEM_125_14_v1_1:
        case STEM_125_14_LN_v1_1:
        case STEM_125_14_Z7020_v1_0:
        case STEM_125_14_Z7020_LN_v1_1:
            return RP_125_14;

        case STEM_122_16SDR_v1_0:
        case STEM_122_16SDR_v1_1:
            return RP_125_14;

        case STEM_125_14_Z7020_4IN_v1_0:
        case STEM_125_14_Z7020_4IN_v1_2:
        case STEM_125_14_Z7020_4IN_v1_3:
            return RP_125_14_4CH;

        case STEM_250_12_v1_0:
        case STEM_250_12_v1_1:
        case STEM_250_12_v1_2:
        case STEM_250_12_v1_2a:
        case STEM_250_12_120:
            return RP_250_12;
        default:
            fprintf(stderr,"[Error] Can't get board model\n");
            exit(-1);
    }
    return RP_125_14;
}

auto loadARBList() -> void{
    uint32_t c = 0;
    rp_ARBInit();
    std::string list;
    if (!rp_ARBGetCount(&c)){
        for(uint32_t i = 0; i < c; i++){
            std::string name;
            if (!rp_ARBGetName(i,&name)){
                bool is_valid;
                if (!rp_ARBIsValid(name,&is_valid)){
                    if (is_valid)
                        g_arbList.push_back(name);
                }
            }
        }
    }
}

auto getARBList() -> std::vector<std::string>{
    return g_arbList;
}


/** Signal generator main */
int gen(config_t &conf)
{
    rp_channel_t ch = (rp_channel_t)conf.ch;
    if (conf.regDebug){
        rp_EnableDebugReg();
    }

    rp_InitReset(false);

    if (rp_CalibInit() != RP_HW_CALIB_OK){
        fprintf(stderr,"Error init calibration\n");
        return -1;
    }

    rp_GenOutDisable(ch);

    uint8_t channels = 0;
    if (rp_HPGetFastADCChannelsCount(&channels) != RP_HP_OK){
        fprintf(stderr,"[Error:getRawBuffer] Can't get fast ADC channels count\n");
        return -1;
    }

    rp_calib_params_t calib;
    if (conf.calib){
        calib = rp_GetCalibrationSettings();
    }else{
        calib = rp_GetDefaultCalibrationSettings();
    }

    rp_CalibrationSetParams(calib);
    rp_GenOffset(ch,0);
    rp_GenAmp(ch,conf.amp / 2.0);
    rp_GenFreq(ch,conf.freq);
    rp_GenTriggerSource(ch,RP_GEN_TRIG_SRC_INTERNAL);

    if (conf.arb == ""){
        if (conf.type == RP_WAVEFORM_SINE){
            rp_GenWaveform(ch, RP_WAVEFORM_SINE);
        }

        if (conf.type == RP_WAVEFORM_SQUARE){
            rp_GenWaveform(ch, RP_WAVEFORM_SQUARE);
        }

        if (conf.type == RP_WAVEFORM_TRIANGLE){
            rp_GenWaveform(ch, RP_WAVEFORM_TRIANGLE);
        }

        if (conf.type == RP_WAVEFORM_DC){
            rp_GenWaveform(ch, RP_WAVEFORM_DC);
        }

        if (conf.type == RP_WAVEFORM_SWEEP){
            rp_GenSweepDir(ch,RP_GEN_SWEEP_DIR_UP_DOWN);
            rp_GenSweepMode(ch,RP_GEN_SWEEP_MODE_LOG);
            rp_GenSweepStartFreq(ch,conf.freq);
            rp_GenSweepEndFreq(ch,conf.end_freq);
            rp_GenWaveform(ch, RP_WAVEFORM_SWEEP);
        }
    }else{
        float data[DAC_BUFFER_SIZE];
        uint32_t s;
        rp_ARBGetSignalByName(conf.arb,data,&s);
        if (rp_GenArbWaveform(ch,data,s)!= RP_OK){
            fprintf(stderr,"Error load ARB waveform\n");
        }
        rp_GenWaveform(ch, RP_WAVEFORM_ARBITRARY);
    }

    if (rp_HPGetIsGainDACx5OrDefault()){
        rp_GenSetGainOut(ch,conf.gain);
    }

    if (conf.freq != 0) {
        rp_GenOutEnable(ch);
        rp_GenSynchronise();
    }
    rp_Release();
    return 0;
}