/**
 *
 * @brief Red Pitaya filter calib utility.
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
#include <iomanip>
#include <iostream>
#include <vector>

#include "calib/calib.h"
#include "calib/calib_man.h"
#include "calib/filter_logic.h"
#include "calib/filter_logicNch.h"
#include "common/version.h"
#include "options.h"
#include "rp.h"

using namespace options;

std::atomic_bool g_stopApp = false;

static const char app_params[] = {""};
static const std::vector<options::ParamConfig> params = {
    {"auto", "auto", 'a', RP_OPT_PARAM_MISSING, RP_OPT_INT, {0, 0}, 0, "Automatic filter calibration using internal generator."},
    {"auto_ext", "auto_ext", 'e', RP_OPT_PARAM_MISSING, RP_OPT_INT, {0, 0}, 0, "Automatic filter calibration using external generator (PWM signal of 1kHz 1.8 Vpp)."},
    {"initK", "initK", 'i', RP_OPT_PARAM_REQUIRED, RP_OPT_INT, {0x001FFFFF, 0x00FFFFFF}, 0x00dFFFFF, "Sets the value for the KK parameter. The default value is 0xdFFFFF."},
    {"write", "write", 'w', RP_OPT_PARAM_MISSING, RP_OPT_INT, {0, 0}, 0, "Write new parameters to eeprom."},
    {"help", "help", 'h', RP_OPT_PARAM_MISSING, RP_OPT_INT, {0, 0}, 0, "Print this message."}};

static const uint8_t g_adcChannes = rp_HPGetFastADCChannelsCountOrDefault();
static void termSignalHandler(int) {
    g_stopApp = true;
}

static void installTermSignalHandler() {
    signal(SIGINT, termSignalHandler);
    signal(SIGTERM, termSignalHandler);
}

void progress(const std::string& prefix, uint64_t dnow, uint64_t dtotal) {
    auto draw = [](const std::string& prefix, double progress, uint64_t dnow, uint64_t dtotal) {
        const int barWidth = 50;
        std::cout << "\033[2K";
        std::cout << "\r";
        std::cout << prefix.c_str();
        std::cout << "[";
        int pos = static_cast<int>(barWidth * progress);
        for (int i = 0; i < barWidth; ++i) {
            if (i < pos)
                std::cout << "=";
            else if (i == pos)
                std::cout << ">";
            else
                std::cout << " ";
        }
        std::cout << "] ";
        std::cout << std::fixed << std::setprecision(1) << (progress * 100.0) << "% ";
        std::cout << "(" << dnow << "/" << dtotal << ")";
        std::cout.flush();
    };

    double progress = static_cast<double>(dnow) / dtotal;

    draw(prefix, progress, dnow, dtotal);
    if (dnow == dtotal) {
        std::cout << std::endl;
    }
};

/** Acquire utility main */
int main(int argc, char* argv[]) {

    if (!rp_HPGetFastADCIsFilterPresentOrDefault()) {
        fprintf(stderr, "The current board does not have the ability to calibrate the filter.\n");
        return -1;
    }
    auto option = parse(params, argc, argv);

    if (option.error) {
        options::usage(argv[0], app_params);
        options::printParams(params);
        return -1;
    }

    if (option.isParam("help")) {
        options::usage(argv[0], app_params);
        options::printParams(params);
        return 0;
    }

    installTermSignalHandler();

    auto isAutoInt = option.isParam("auto");
    if (isAutoInt || option.isParam("auto_ext")) {
        std::string info_msg = "";
        rp_Init();
        auto acq = rp_calib::COscilloscope::Create(8);
        auto calib_man = rp_calib::CCalibMan::Create(acq);
        auto filter_logicNch = rp_calib::CFilter_logicNch::Create(calib_man);
        acq->start();
        calib_man->initSq(8);
        calib_man->setModeLV_HV(RP_LOW);
        filter_logicNch->init();
        if (isAutoInt) {
            printf("Start internal generator\n");
            calib_man->setOffset(RP_CH_1, 0);
            calib_man->setFreq(RP_CH_1, 1000);
            calib_man->setAmp(RP_CH_1, 0.9);
            calib_man->setGenType(RP_CH_1, (int)RP_WAVEFORM_SQUARE);
            calib_man->enableGen(RP_CH_1, true);
        }
        printf("Starting data capture and filter initialization");
        acq->startAutoFilterNCh(8);
        if (option.isParam("initK")) {
            for (int i = 0; i < g_adcChannes; i++) {
                auto kk = option.getInt("initK");
                if (kk != nullptr) {
                    uint32_t kk32 = (uint32_t)*kk;
                    calib_man->setDisableFilter((rp_channel_t)i, kk32);  // Disable filter in calib manager
                } else {
                    ERROR_LOG("Can't get KK value")
                    filter_logicNch = nullptr;
                    acq->stop();
                    rp_Release();
                }
            }
        } else {
            for (int i = 0; i < g_adcChannes; i++) {
                calib_man->setDisableFilter((rp_channel_t)i, 0x00dFFFFF);  // Disable filter in calib manager
            }
        }
        calib_man->updateCalib();
        for (int i = 0; i < g_adcChannes; i++) {
            acq->updateAcqFilter((rp_channel_t)i);  // Set filter in FPGA
        }
        printf(" [OK]\n");

        printf("Filter values before calibration\n");

        std::vector<double> values;
        values.resize(g_adcChannes);
        rp_calib::COscilloscope::DataPassAutoFilterSync data;
        for (uint8_t n = 0; n < 10; n++) {
            std::this_thread::sleep_for(std::chrono::microseconds(100000));
            data = acq->getDataAutoFilterSync();
            for (int i = 0; i < g_adcChannes; i++) {
                auto v = (data.valueCH[i].calib_value_raw + data.valueCH[i].deviation);
                values[i] += v;
            }
        }
        for (int i = 0; i < g_adcChannes; i++) {
            printf("\tChannel %d = %f [AA=0x%X BB=0x%X PP=0x%X KK=0x%X]\n", i + 1, values[i] / 10.0, data.valueCH[i].f_aa, data.valueCH[i].f_bb, data.valueCH[i].f_pp,
                   data.valueCH[i].f_kk);
        }

        while (1) {
            while (filter_logicNch->setCalibParameters() != -1) {
                auto dp = acq->getDataAutoFilterSync();
                filter_logicNch->setCalculatedValue(dp);
            }
            filter_logicNch->removeHalfCalib();
            if (filter_logicNch->nextSetupCalibParameters() == -1) {
                break;
            }
            progress("AA/BB Calibration ", filter_logicNch->calcProgress(), 100);

            if (g_stopApp) {
                filter_logicNch = nullptr;
                acq->stop();
                rp_Release();
                printf("\n");
                return -1;
            }
        }

        progress("AA/BB Calibration ", 100, 100);

        for (int ch = 0; ch < g_adcChannes; ch++) {
            printf("[Channel %d] Amplitude Calibration (PP)\n", ch + 1);
            filter_logicNch->setGoodCalibParameterCh((rp_channel_t)ch);
            std::this_thread::sleep_for(std::chrono::microseconds(1000000));
            auto volt_ref = 0.9;
            while (1) {
                auto d = acq->getDataAutoFilterSync();
                if (filter_logicNch->calibPPCh((rp_channel_t)ch, d, volt_ref) != 0)
                    break;
                if (g_stopApp) {
                    filter_logicNch = nullptr;
                    acq->stop();
                    rp_Release();
                    return -1;
                }
            }
            printf("[Channel %d] Amplitude Calibration (PP) [DONE]\n", ch + 1);
        }

        printf("Filter values after calibration\n");

        for (int i = 0; i < g_adcChannes; i++) {
            values[i] = 0;
        }

        for (uint8_t n = 0; n < 10; n++) {
            std::this_thread::sleep_for(std::chrono::microseconds(100000));
            data = acq->getDataAutoFilterSync();
            for (int i = 0; i < g_adcChannes; i++) {
                auto v = (data.valueCH[i].calib_value_raw + data.valueCH[i].deviation);
                values[i] += v;
            }
        }
        for (int i = 0; i < g_adcChannes; i++) {
            printf("\tChannel %d = %f [AA=0x%X BB=0x%X PP=0x%X KK=0x%X]\n", i + 1, values[i] / 10.0, data.valueCH[i].f_aa, data.valueCH[i].f_bb, data.valueCH[i].f_pp,
                   data.valueCH[i].f_kk);
        }
        if (option.isParam("write")) {
            calib_man->writeCalib();
            printf("Writing parameters to eeprom\n");
        }
        filter_logicNch = nullptr;
        acq->stop();
        rp_Release();
        return 0;
    } else {
        options::usage(argv[0], app_params);
        options::printParams(params);
        return -1;
    }

    return 0;
}
