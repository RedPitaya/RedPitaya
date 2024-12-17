#ifndef __OPTIONS_H
#define __OPTIONS_H

#include "rp.h"
#include "rp_hw-profiles.h"
#include <getopt.h>

typedef enum {
    RP_125_14,
    RP_250_12,
    RP_125_14_4CH
} models_t;

uint8_t getChannels();
bool getIsExtTrigLevel();
bool getIsSplitTriggers();
bool getIsACDC();
auto parseTrigger(char *value,int channels) -> int;

struct Options {

    uint32_t      dataSize;
    uint32_t      decimation;

    int channels;
    bool is_ext_trig_lev;
    bool is_ac_dc;

    std::vector<option> options;
    std::string usage;
    std::string opts;

    rp_pinState_t attenuator_mode[4];

    rp_acq_ac_dc_mode_t ac_dc_mode[2];

    float trigger_level[4];
    float trigger_level_ext = 0;
    int offset = 0;
    rp_acq_trig_src_t    trigger_mode[4];

    bool                 showVersion;
    bool                 showHelp;
    bool                 showInHex;
    bool                 showInVolt;
    bool                 enableDebug;
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
            trigger_mode[i] = RP_TRIG_SRC_DISABLED;
            trigger_level[i] = 0;
        }

        trigger_level_ext = 0;

        showVersion = false;
        showHelp = false;
        showInHex = false;
        showInVolt = false;
        enableDebug = false;
        disableCalibration = false;
        enableEqualization = false;
        enableShaping = false;
        error = true;
        reset_hk = false;
        enableAXI = false;
    };
};

auto usage(Options &opt) -> void;
auto parse(int argc, char* argv[]) -> Options;


#endif