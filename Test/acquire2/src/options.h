#ifndef __OPTIONS_H
#define __OPTIONS_H

#include "rp.h"

#if defined Z10 || defined Z20 || defined Z20_125 || defined Z20_250_12
#define CHANNELS 2
#endif

#if defined Z20_125_4CH
#define CHANNELS 4
#endif

struct Options {

    uint32_t      dataSize;   
    uint32_t      decimation;

    rp_pinState_t attenuator_mode[CHANNELS];

#if defined Z20_250_12
    rp_acq_ac_dc_mode_t ac_dc_mode[CHANNELS];
#endif

    float trigger_level;
    rp_acq_trig_src_t    trigger_mode;

    bool                 showVersion;
    bool                 showHelp;
    bool                 showInHex;
    bool                 showInVolt;
    bool                 disableReset;
    bool                 disableCalibration;
    bool                 enableEqualization;
    bool                 enableShaping;
    bool                 error;
    bool                 reset_hk;


    Options(){
        dataSize = ADC_BUFFER_SIZE;
        decimation = RP_DEC_1;
        for(int i = 0; i < CHANNELS; i++){
            attenuator_mode[i] = RP_LOW;
#if defined Z20_250_12
            ac_dc_mode[i] = RP_AC;
#endif
        }
        
        trigger_level = 0;
        trigger_mode = RP_TRIG_SRC_NOW;

        showVersion = false;
        showHelp = false;
        showInHex = false;
        showInVolt = false;
        disableReset = false;
        disableCalibration = false;
        enableEqualization = false;
        enableShaping = false;
        error = true;
        reset_hk = false;
    };
};

auto usage(char const *progName) -> void;
auto parse(int argc, char* argv[]) -> Options;


#endif