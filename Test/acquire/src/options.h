#ifndef __OPTIONS_H
#define __OPTIONS_H

#include "rp.h"
#include "rp_hw-profiles.h"

typedef enum {
    RP_125_14,
    RP_250_12,
    RP_125_14_4CH
} models_t;

models_t getModel();
uint8_t getChannels();

struct Options {

    uint32_t      dataSize;
    uint32_t      decimation;

    rp_pinState_t attenuator_mode[4];

    rp_acq_ac_dc_mode_t ac_dc_mode[4];

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
    bool                 enableAXI;


    Options(){
        dataSize = ADC_BUFFER_SIZE;
        decimation = RP_DEC_1;

        for(int i = 0; i < getChannels(); i++){
            attenuator_mode[i] = RP_LOW;
            ac_dc_mode[i] = RP_AC;
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
        enableAXI = false;
    };
};

auto usage(char const *progName) -> void;
auto parse(int argc, char* argv[]) -> Options;


#endif