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

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <unistd.h>
#include <iostream>
#include <vector>

#include "api250-12/rp-spi.h"
#include "common/version.h"
#include "options.h"
#include "rp.h"
#include "rp_hw_calib.h"

/** Program name */
const char* g_argv0 = NULL;

rp_channel_trigger_t getTrigChByTrigSource(rp_acq_trig_src_t src) {

    switch (src) {
        case RP_TRIG_SRC_NOW:
            return RP_T_CH_1;

        case RP_TRIG_SRC_CHA_PE:
        case RP_TRIG_SRC_CHA_NE:
            return RP_T_CH_1;

        case RP_TRIG_SRC_CHB_PE:
        case RP_TRIG_SRC_CHB_NE:
            return RP_T_CH_2;

        case RP_TRIG_SRC_CHC_PE:
        case RP_TRIG_SRC_CHC_NE:
            return RP_T_CH_3;

        case RP_TRIG_SRC_CHD_PE:
        case RP_TRIG_SRC_CHD_NE:
            return RP_T_CH_4;

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
int main(int argc, char* argv[]) {

    if (!rp_HPGetFastADCIsSplitTriggerOrDefault()) {
        fprintf(stderr, "Split trigger mode not supported\n");
        exit(0);
    }

    g_argv0 = argv[0];
    auto option = parse(argc, argv);

    if (option.error || option.showHelp || option.showVersion) {
        usage(option);
        return -1;
    }

    if (option.enableDebug) {
        rp_EnableDebugReg();
    }

    if (rp_InitReset(option.reset_hk) != RP_OK) {
        fprintf(stderr, "Error init rp api\n");
        return -1;
    }

    rp_AcqSetSplitTrigger(true);

    if (option.dataSize > ADC_BUFFER_SIZE) {
        fprintf(stderr, "[Error] Data size must be less than %i\n", ADC_BUFFER_SIZE);
        usage(option);
        return -1;
    }

    if (rp_CalibInit() != RP_HW_CALIB_OK) {
        fprintf(stderr, "Error init calibration\n");
        return -1;
    }

    uint8_t channels = 0;
    if (rp_HPGetFastADCChannelsCount(&channels) != RP_HP_OK) {
        fprintf(stderr, "[Error:getRawBuffer] Can't get fast ADC channels count\n");
        return -1;
    }

    rp_calib_params_t calib;
    if (!option.disableCalibration) {
        calib = rp_GetCalibrationSettings();
    } else {
        calib = rp_GetDefaultUniCalibrationSettings();
    }
    if (rp_HPGetFastADCIsFilterPresentOrDefault()) {
        if (!option.enableEqualization) {
            for (int i = 0; i < calib.fast_adc_count_1_1; ++i) {
                calib.fast_adc_filter_1_1[i].aa = 0;
                calib.fast_adc_filter_1_1[i].bb = 0;
            }

            for (int i = 0; i < calib.fast_adc_count_1_20; ++i) {
                calib.fast_adc_filter_1_20[i].aa = 0;
                calib.fast_adc_filter_1_20[i].bb = 0;
            }
        }

        if (!option.enableShaping) {

            for (int i = 0; i < calib.fast_adc_count_1_1; ++i) {
                calib.fast_adc_filter_1_1[i].pp = 0;
                calib.fast_adc_filter_1_1[i].kk = 0xffffff;
            }

            for (int i = 0; i < calib.fast_adc_count_1_20; ++i) {
                calib.fast_adc_filter_1_20[i].pp = 0;
                calib.fast_adc_filter_1_20[i].kk = 0xffffff;
            }
        }
    }

    for (int i = 0; i < channels; i++) {
        rp_AcqSetBypassFilter((rp_channel_t)i, option.bypassFilter);
    }

    rp_CalibrationSetParams(calib);

    for (int i = 0; i < channels; i++) {
        if (rp_HPGetIsCalibInFPGAOrDefault()) {
            rp_AcqSetCalibInFPGA((rp_channel_t)i);
        }

        if (rp_HPGetFastADCIsAC_DCOrDefault()) {
            rp_AcqSetAC_DC((rp_channel_t)i, option.ac_dc_mode[i]);
        }
        rp_AcqSetDecimationFactorCh((rp_channel_t)i, option.decimation);
        rp_AcqSetGain((rp_channel_t)i, option.attenuator_mode[i]);
        rp_AcqSetTriggerDelayDirectCh((rp_channel_t)i, option.dataSize);
    }

    if (option.is_ext_trig_lev) {
        rp_SetExternalTriggerLevel(option.trigger_level_ext);
    }

    for (int i = 0; i < channels; i++) {
        if (option.trigger_mode[i] != RP_TRIG_SRC_DISABLED) {
            rp_AcqSetTriggerLevel((rp_channel_trigger_t)i, option.trigger_level[i]);
        }
    }

    for (int i = 0; i < channels; i++) {
        if (option.trigger_mode[i] != RP_TRIG_SRC_DISABLED) {
            rp_AcqStartCh((rp_channel_t)i);
        }
    }

    for (int i = 0; i < channels; i++) {
        if (option.trigger_mode[i] != RP_TRIG_SRC_DISABLED) {
            rp_AcqSetTriggerSrcCh((rp_channel_t)i, option.trigger_mode[i]);
        }
    }

    for (int i = 0; i < channels; i++) {
        if (option.trigger_mode[i] != RP_TRIG_SRC_DISABLED) {
            bool fillState = false;
            rp_acq_trig_state_t trig_state = RP_TRIG_STATE_WAITING;

            while (1) {
                rp_AcqGetTriggerStateCh((rp_channel_t)i, &trig_state);
                if (trig_state == RP_TRIG_STATE_TRIGGERED) {
                    break;
                } else {
                    usleep(1);
                }
            }

            while (!fillState) {
                rp_AcqGetBufferFillStateCh((rp_channel_t)i, &fillState);
            }
            rp_AcqStopCh((rp_channel_t)i);
        }
    }

    int start_ch = 0;
    int end_ch = channels - 1;
    auto size = MAX(ADC_BUFFER_SIZE, option.dataSize);
    if (option.showInVolt) {
        std::vector<float> buffers[4];
        for (auto i = start_ch; i <= end_ch; i++) {
            if (option.trigger_mode[i] != RP_TRIG_SRC_DISABLED) {
                uint32_t pos = 0;
                uint32_t acq_u_size = option.dataSize;
                rp_AcqGetWritePointerAtTrigCh((rp_channel_t)i, &pos);
                buffers[i].resize(size);
                pos = (pos + option.offset + ADC_BUFFER_SIZE) % ADC_BUFFER_SIZE;
                auto ch = (rp_channel_t)i;
                rp_AcqGetDataV(ch, pos, &acq_u_size, buffers[i].data());
            }
        }

        for (uint32_t i = 0; i < option.dataSize; i++) {
            bool printSeparator = false;
            for (auto j = start_ch; j <= end_ch; j++) {
                if (option.trigger_mode[j] != RP_TRIG_SRC_DISABLED) {
                    if (printSeparator) {
                        fprintf(stdout, " ");
                    }
                    fprintf(stdout, "%f", buffers[j][i]);
                    printSeparator = true;
                }
            }
            fprintf(stdout, "\n");
        }

    } else {
        std::vector<int16_t> buffers[4];
        for (auto i = start_ch; i <= end_ch; i++) {
            if (option.trigger_mode[i] != RP_TRIG_SRC_DISABLED) {
                uint32_t pos = 0;
                uint32_t acq_u_size = option.dataSize;
                rp_AcqGetWritePointerAtTrigCh((rp_channel_t)i, &pos);
                buffers[i].resize(size);
                pos = (pos + option.offset + ADC_BUFFER_SIZE) % ADC_BUFFER_SIZE;
                auto ch = (rp_channel_t)i;
                if (option.disableCalibration)
                    rp_AcqGetDataRaw(ch, pos, &acq_u_size, buffers[i].data());
                else
                    rp_AcqGetDataRawWithCalib(ch, pos, &acq_u_size, buffers[i].data());
            }
        }
        const char* format_str = (option.showInHex == false) ? "%7d" : "0x%08X";
        for (uint32_t i = 0; i < option.dataSize; i++) {
            bool printSeparator = false;
            for (auto j = start_ch; j <= end_ch; j++) {
                if (option.trigger_mode[j] != RP_TRIG_SRC_DISABLED) {
                    if (printSeparator) {
                        fprintf(stdout, " ");
                    }
                    fprintf(stdout, format_str, buffers[j][i]);
                    printSeparator = true;
                }
            }
            fprintf(stdout, "\n");
        }
    }

    if (rp_Release() != RP_OK) {
        fprintf(stderr, "Error release rp api\n");
        return -1;
    }
    return 0;
}
