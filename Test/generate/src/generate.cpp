#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <vector>

#include "common/rp_arb.h"
#include "generate.h"

std::vector<std::string> g_arbList;

uint8_t getChannels() {
    uint8_t c = 0;
    if (rp_HPGetFastDACChannelsCount(&c) != RP_HP_OK) {
        fprintf(stderr, "[Error] Can't get fast DAC channels count\n");
    }
    return c;
}

uint32_t getMaxSpeed() {
    uint32_t c = 0;
    if (rp_HPGetBaseFastDACSpeedHz(&c) != RP_HP_OK) {
        fprintf(stderr, "[Error] Can't get fast DAC speed\n");
    }
    return c;
}

float fullScale() {

    float fs = 0;
    if (rp_HPGetFastDACOutFullScale(RP_CH_1, &fs) != RP_HP_OK) {
        fprintf(stderr, "[Error:generate_writeData] Can't get fast DAC out full scale\n");
        return RP_NOTS;
    }

    return fs;
}

bool getIsLoad50Ohm() {
    bool c = 0;
    if (rp_HPGetIsDAC50OhmMode(&c) != RP_HP_OK) {
        fprintf(stderr, "[Error] Can't get fast DAC load mode\n");
    }
    return c;
}

bool getIsGainX5() {
    bool c = 0;
    if (rp_HPGetIsGainDACx5(&c) != RP_HP_OK) {
        fprintf(stderr, "[Error] Can't get fast DAC gain\n");
    }
    return c;
}

auto loadARBList() -> void {
    uint32_t c = 0;
    rp_ARBInit();
    std::string list;
    if (!rp_ARBGetCount(&c)) {
        for (uint32_t i = 0; i < c; i++) {
            std::string name;
            if (!rp_ARBGetName(i, &name)) {
                bool is_valid;
                if (!rp_ARBIsValid(name, &is_valid)) {
                    if (is_valid)
                        g_arbList.push_back(name);
                }
            }
        }
    }
}

auto getARBList() -> std::vector<std::string> {
    return g_arbList;
}

/** Signal generator main */
int gen(config_t& conf) {
    rp_channel_t ch = (rp_channel_t)conf.ch;
    if (conf.regDebug) {
        rp_EnableDebugReg();
    }

    rp_InitReset(false);

    if (rp_CalibInit() != RP_HW_CALIB_OK) {
        fprintf(stderr, "Error init calibration\n");
        return -1;
    }

    rp_GenSetLoadMode(ch, conf.load);
    rp_GenOutDisable(ch);

    uint8_t channels = 0;
    if (rp_HPGetFastADCChannelsCount(&channels) != RP_HP_OK) {
        fprintf(stderr, "[Error:getRawBuffer] Can't get fast ADC channels count\n");
        return -1;
    }

    rp_calib_params_t calib;
    if (conf.calib) {
        calib = rp_GetCalibrationSettings();
    } else {
        calib = rp_GetDefaultCalibrationSettings();
    }

    rp_CalibrationSetParams(calib);
    rp_GenOffset(ch, 0);
    rp_GenAmp(ch, conf.amp / 2.0);
    rp_GenFreq(ch, conf.freq);
    rp_GenTriggerSource(ch, RP_GEN_TRIG_SRC_INTERNAL);

    if (conf.arb == "") {
        if (conf.type == RP_WAVEFORM_SINE) {
            rp_GenWaveform(ch, RP_WAVEFORM_SINE);
        }

        if (conf.type == RP_WAVEFORM_SQUARE) {
            rp_GenWaveform(ch, RP_WAVEFORM_SQUARE);
        }

        if (conf.type == RP_WAVEFORM_TRIANGLE) {
            rp_GenWaveform(ch, RP_WAVEFORM_TRIANGLE);
        }

        if (conf.type == RP_WAVEFORM_DC) {
            rp_GenWaveform(ch, RP_WAVEFORM_DC);
        }

        if (conf.type == RP_WAVEFORM_NOISE) {
            rp_GenWaveform(ch, RP_WAVEFORM_NOISE);
        }

        if (conf.type == RP_WAVEFORM_SWEEP) {
            rp_GenSweepDir(ch, RP_GEN_SWEEP_DIR_UP_DOWN);
            rp_GenSweepMode(ch, RP_GEN_SWEEP_MODE_LOG);
            rp_GenSweepStartFreq(ch, conf.freq);
            rp_GenSweepEndFreq(ch, conf.end_freq);
            rp_GenWaveform(ch, RP_WAVEFORM_SWEEP);
        }
    } else {
        float data[DAC_BUFFER_SIZE];
        uint32_t s;
        rp_ARBGetSignalByName(conf.arb, data, &s);
        if (rp_GenArbWaveform(ch, data, s) != RP_OK) {
            fprintf(stderr, "Error load ARB waveform\n");
        }
        rp_GenWaveform(ch, RP_WAVEFORM_ARBITRARY);
    }

    if (rp_HPGetIsGainDACx5OrDefault()) {
        rp_GenSetGainOut(ch, conf.gain);
    }

    if (conf.freq != 0) {
        rp_GenOutEnable(ch);
        rp_GenSynchronise();
    }
    rp_Release();
    return 0;
}