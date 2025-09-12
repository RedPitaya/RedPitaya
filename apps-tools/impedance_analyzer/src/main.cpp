
#include "main.h"

#include <complex.h>
#include <fcntl.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/syslog.h>
#include <unistd.h>
#include <limits>

#include <algorithm>
#include <mutex>
#include <thread>
#include <vector>

#include "bodeApp.h"
#include "common/version.h"

#include "lcrApp.h"
#include "main.h"
#include "rpApp.h"
#include "rp_hw-profiles.h"
#include "rp_hw_calib.h"
#include "settings.h"

#include "common/profiler.h"
#include "web/rp_client.h"

#define NAN_VALUE std::numeric_limits<float>::min()
#define CHECK_NAN_INF(X)                \
    if (std::isnan(X) || std::isinf(X)) \
        X = NAN_VALUE;
#define NAN_INF(X) std::isnan(X) || std::isinf(X)
#define MAX_STEPS 3000

enum { IA_NONE = 0, IA_START = 1, IA_RESET_CONFIG_SETTINGS = 2, IA_RESET_CONFIG_SETTINGS_DONE = 3, IA_START_DONE = 4, IA_START_PROCESS = 5 } ia_status_t;

enum {
    IA_FREQ = 0,
    IA_Z = 1,
    IA_PHASE = 2,
    IA_Y_s = 3,
    IA_NEG_PHASE = 4,
    IA_R_s = 5,
    IA_R_p = 6,
    IA_X_s = 7,
    IA_G_p = 8,
    IA_B_p = 9,
    IA_C_s = 10,
    IA_C_p = 11,
    IA_L_s = 12,
    IA_L_p = 13,
    IA_Q = 14,
    IA_D = 15,
    IA_P_P = 16
} ia_signal_t;

// Control parameters
CIntParameter ia_status("IA_STATUS", CBaseParameter::RW, IA_NONE, 0, 0, 100);

//Parameters
CIntParameter ia_start_freq("IA_START_FREQ", CBaseParameter::RW, 1000, 0, 1, getMaxADC(), CONFIG_VAR);
CIntParameter ia_end_freq("IA_END_FREQ", CBaseParameter::RW, 10000, 0, 2, getMaxADC(), CONFIG_VAR);
CIntParameter ia_steps("IA_STEPS", CBaseParameter::RW, 25, 0, 1, MAX_STEPS, CONFIG_VAR);

CIntParameter ia_averaging("IA_AVERAGING", CBaseParameter::RW, 1, 0, 1, 10, CONFIG_VAR);
CFloatParameter ia_shunt("IA_SHUNT", CBaseParameter::RW, 1, 0, 1, 10e6, CONFIG_VAR);
CIntParameter ia_lcr_shunt("IA_LCR_SHUNT", CBaseParameter::RW, RP_LCR_S_10, 0, RP_LCR_S_10, RP_LCR_S_1M, CONFIG_VAR);

CFloatParameter ia_amplitude("IA_AMPLITUDE", CBaseParameter::RW, 0.5, 0, 0, 1, CONFIG_VAR);
CFloatParameter ia_dc_bias("IA_DC_BIAS", CBaseParameter::RW, 0, 0, -1, 1, CONFIG_VAR);
CIntParameter ia_y_axis("IA_Y_AXIS", CBaseParameter::RW, IA_Z, 0, IA_Z, IA_P_P, CONFIG_VAR);
CBooleanParameter ia_scale("IA_SCALE", CBaseParameter::RW, false, 0, CONFIG_VAR);
CIntParameter ia_scale_plot("IA_X_SCALE", CBaseParameter::RW, 0, 0, 0, 3, CONFIG_VAR);

// Status parameters
CIntParameter max_adc("RP_MAX_ADC", CBaseParameter::RO, getMaxADC(), 0, getMaxADC(), getMaxADC());
CStringParameter redpitaya_model("RP_MODEL_STR", CBaseParameter::RO, getModelS(), 0);
CFloatParameter ia_current_freq("IA_CURRENT_FREQ", CBaseParameter::RW, 1, 0, 0, getMaxADC());
CIntParameter ia_current_step("IA_CURRENT_STEP", CBaseParameter::RW, 1, 0, 1, MAX_STEPS);
CBooleanParameter lcr_ext("IA_LCR_EXT", CBaseParameter::RW, false, 0);

// Cursors
CBooleanParameter cur_x1_enable("IA_CURSOR_X1_ENABLE", CBaseParameter::RW, false, 0, CONFIG_VAR);
CBooleanParameter cur_x2_enable("IA_CURSOR_X2_ENABLE", CBaseParameter::RW, false, 0, CONFIG_VAR);
CBooleanParameter cur_y1_enable("IA_CURSOR_Y1_ENABLE", CBaseParameter::RW, false, 0, CONFIG_VAR);
CBooleanParameter cur_y2_enable("IA_CURSOR_Y2_ENABLE", CBaseParameter::RW, false, 0, CONFIG_VAR);

CFloatParameter cur_x1("IA_CURSOR_X1", CBaseParameter::RW, 0.25, 0, 0, 1, CONFIG_VAR);
CFloatParameter cur_x2("IA_CURSOR_X2", CBaseParameter::RW, 0.75, 0, 0, 1, CONFIG_VAR);
CFloatParameter cur_y1("IA_CURSOR_Y1", CBaseParameter::RW, 0.25, 0, 0, 1, CONFIG_VAR);
CFloatParameter cur_y2("IA_CURSOR_Y2", CBaseParameter::RW, 0.75, 0, 0, 1, CONFIG_VAR);

//Singals
CFloatSignal ia_signal[IA_P_P + 1] = {
    {"IA_SIGNAL_FREQ", CH_SIGNAL_SIZE_DEFAULT, 0.0f}, {"IA_SIGNAL_Z", CH_SIGNAL_SIZE_DEFAULT, 0.0f},         {"IA_SIGNAL_PHASE", CH_SIGNAL_SIZE_DEFAULT, 0.0f},
    {"IA_SIGNAL_Y", CH_SIGNAL_SIZE_DEFAULT, 0.0f},    {"IA_SIGNAL_NEG_PHASE", CH_SIGNAL_SIZE_DEFAULT, 0.0f}, {"IA_SIGNAL_R_s", CH_SIGNAL_SIZE_DEFAULT, 0.0f},
    {"IA_SIGNAL_R_p", CH_SIGNAL_SIZE_DEFAULT, 0.0f},  {"IA_SIGNAL_X_s", CH_SIGNAL_SIZE_DEFAULT, 0.0f},       {"IA_SIGNAL_G_p", CH_SIGNAL_SIZE_DEFAULT, 0.0f},
    {"IA_SIGNAL_B_p", CH_SIGNAL_SIZE_DEFAULT, 0.0f},  {"IA_SIGNAL_C_s", CH_SIGNAL_SIZE_DEFAULT, 0.0f},       {"IA_SIGNAL_C_p", CH_SIGNAL_SIZE_DEFAULT, 0.0f},
    {"IA_SIGNAL_L_s", CH_SIGNAL_SIZE_DEFAULT, 0.0f},  {"IA_SIGNAL_L_p", CH_SIGNAL_SIZE_DEFAULT, 0.0f},       {"IA_SIGNAL_Q", CH_SIGNAL_SIZE_DEFAULT, 0.0f},
    {"IA_SIGNAL_D", CH_SIGNAL_SIZE_DEFAULT, 0.0f},    {"IA_SIGNAL_P_p", CH_SIGNAL_SIZE_DEFAULT, 0.0f}};

std::vector<float> signals_array[IA_P_P + 1];

std::thread* g_thread = NULL;
std::mutex g_signalMutex;
bool g_exit_flag;
double g_lastCheckExt = 0;

void threadLoop();

auto getModel() -> rp_HPeModels_t {
    rp_HPeModels_t c = STEM_125_14_v1_0;
    if (rp_HPGetModel(&c) != RP_HP_OK) {
        ERROR_LOG("Can't get board model");
    }
    return c;
}

auto getModelS() -> std::string {
    rp_HPeModels_t c = STEM_125_14_v1_0;
    if (rp_HPGetModel(&c) != RP_HP_OK) {
        ERROR_LOG("Can't get board model");
    }

    switch (c) {
        case STEM_125_10_v1_0:
        case STEM_125_14_v1_0:
        case STEM_125_14_v1_1:
        case STEM_125_14_LN_v1_1:
        case STEM_125_14_LN_BO_v1_1:
        case STEM_125_14_LN_CE1_v1_1:
        case STEM_125_14_LN_CE2_v1_1:
        case STEM_125_14_Z7020_v1_0:
        case STEM_125_14_Z7020_LN_v1_1:
        case STEM_125_14_v2_0:
        case STEM_125_14_Pro_v2_0:
        case STEM_125_14_Z7020_Pro_v1_0:
        case STEM_125_14_Z7020_Pro_v2_0:
        case STEM_125_14_Z7020_Ind_v2_0:
        case STEM_125_14_Z7020_LL_v1_1:
        case STEM_125_14_Z7020_LL_v1_2:
        case STEM_65_16_Z7020_LL_v1_1:
        case STEM_125_14_Z7020_TI_v1_3:
        case STEM_65_16_Z7020_TI_v1_3:
            return "Z10";

        case STEM_122_16SDR_v1_0:
        case STEM_122_16SDR_v1_1:
            return "Z20";

        case STEM_125_14_Z7020_4IN_v1_0:
        case STEM_125_14_Z7020_4IN_v1_2:
        case STEM_125_14_Z7020_4IN_v1_3:
        case STEM_125_14_Z7020_4IN_BO_v1_3:
            return "Z10";

        case STEM_250_12_v1_0:
        case STEM_250_12_v1_1:
        case STEM_250_12_v1_2:
        case STEM_250_12_v1_2a:
        case STEM_250_12_v1_2b:
            return "Z20_250_12";
        case STEM_250_12_120:
            return "Z20_250_12_120";

        default:
            ERROR_LOG("Can't get board model");
            exit(-1);
    }
    return "Z10";
}

auto getMaxADC() -> uint32_t {
    rp_HPeModels_t c = STEM_125_14_v1_0;
    int dev = 0;
    if (rp_HPGetModel(&c) != RP_HP_OK) {
        ERROR_LOG("Can't get board model");
    }

    switch (c) {
        case STEM_125_10_v1_0:
        case STEM_125_14_v1_0:
        case STEM_125_14_v1_1:
        case STEM_125_14_LN_v1_1:
        case STEM_125_14_LN_BO_v1_1:
        case STEM_125_14_LN_CE1_v1_1:
        case STEM_125_14_LN_CE2_v1_1:
        case STEM_125_14_Z7020_v1_0:
        case STEM_125_14_Z7020_LN_v1_1:
        case STEM_125_14_v2_0:
        case STEM_125_14_Pro_v2_0:
        case STEM_125_14_Z7020_Pro_v1_0:
        case STEM_125_14_Z7020_Pro_v2_0:
        case STEM_125_14_Z7020_Ind_v2_0:
        case STEM_125_14_Z7020_LL_v1_1:
        case STEM_125_14_Z7020_LL_v1_2:
        case STEM_125_14_Z7020_TI_v1_3:
            dev = 2;
            break;

        case STEM_122_16SDR_v1_0:
        case STEM_122_16SDR_v1_1:
            dev = 2;
            break;

        case STEM_125_14_Z7020_4IN_v1_0:
        case STEM_125_14_Z7020_4IN_v1_2:
        case STEM_125_14_Z7020_4IN_v1_3:
        case STEM_125_14_Z7020_4IN_BO_v1_3:
            dev = 2;
            break;

        case STEM_250_12_v1_0:
        case STEM_250_12_v1_1:
        case STEM_250_12_v1_2:
        case STEM_250_12_v1_2a:
        case STEM_250_12_v1_2b:
            dev = 4;
            break;
        case STEM_250_12_120:
            dev = 4;
            break;
        case STEM_65_16_Z7020_LL_v1_1:
        case STEM_65_16_Z7020_TI_v1_3:
            dev = 1;
            break;
        default:
            ERROR_LOG("Can't get board model");
            exit(-1);
    }
    uint32_t adc = rp_HPGetBaseFastADCSpeedHzOrDefault();
    return adc / dev;
}

auto setLCRExtState(bool state) -> void {
    if (lcr_ext.Value() != state) {
        lcr_ext.SendValue(state);
        if (state) {
            lcrApp_LcrSetShuntMode(RP_LCR_S_EXTENSION);
            lcrApp_LcrSetShunt((lcr_shunt_t)ia_lcr_shunt.Value());
        } else {
            lcrApp_LcrSetShuntMode(RP_LCR_S_CUSTOM);
        }
    }
}

// Return value in milliseconds 1.0 = 1ms
auto getClock() -> double {
    struct timespec tp;
    clock_gettime(CLOCK_REALTIME, &tp);
    return ((double)tp.tv_sec * 1000.f) + ((double)tp.tv_nsec / 1000000.f);
}

//Application description
const char* rp_app_desc(void) {
    return (const char*)"Red Pitaya impedance analyser application.\n";
}

//Application init
int rp_app_init(void) {
    fprintf(stderr, "Loading impedance analyser version %s-%s.\n", VERSION_STR, REVISION_STR);
    CDataManager::GetInstance()->SetParamInterval(100);
    CDataManager::GetInstance()->SetSignalInterval(100);

    rp_WC_Init();

    lcrApp_lcrInit();
    rp_AcqSetAC_DC(RP_CH_1, RP_DC);
    rp_AcqSetAC_DC(RP_CH_2, RP_DC);
    updateParametersByConfig();
    lcrApp_LcrRun();
    lcrApp_LcrSetShuntIsAuto(false);
    if (lcrApp_LcrSetShunt((lcr_shunt_t)ia_lcr_shunt.Value()) == RP_LCR_OK) {
        lcrApp_LcrSetShuntMode(RP_LCR_S_EXTENSION);
        lcr_ext.SendValue(true);
    } else {
        lcrApp_LcrSetShuntMode(RP_LCR_S_CUSTOM);
        lcr_ext.SendValue(false);
    }

    for (int i = 0; i <= IA_P_P; i++) {
        signals_array[i].reserve(MAX_STEPS);
    }

    g_thread = new std::thread(threadLoop);
    return 0;
}

//Application exit
int rp_app_exit(void) {
    g_exit_flag = true;
    if (g_thread != NULL) {
        if (g_thread->joinable())
            g_thread->join();
    }
    lcrApp_LcrRelease();
    fprintf(stderr, "Unloading bode analyser version %s-%s.\n", VERSION_STR, REVISION_STR);
    return 0;
}

//Set parameters
int rp_set_params(rp_app_params_t*, int) {
    return 0;
}

//Get parameters
int rp_get_params(rp_app_params_t**) {
    return 0;
}

//Get signals
int rp_get_signals(float***, int*, int*) {
    return 0;
}

//Update signals
void UpdateSignals(void) {
    std::lock_guard<std::mutex> lock(g_signalMutex);
    for (int i = 0; i <= IA_P_P; i++) {
        ia_signal[i].Set(signals_array[i]);
    }
}

//Update parameters
void UpdateParams(void) {
    //Start frequency update
    if (ia_start_freq.IsNewValue()) {
        ia_start_freq.Update();
    }

    //End frequency update
    if (ia_end_freq.IsNewValue()) {
        ia_end_freq.Update();
    }

    //Steps update
    if (ia_steps.IsNewValue()) {
        ia_steps.Update();
    }

    //Averaging update
    if (ia_averaging.IsNewValue()) {
        ia_averaging.Update();
    }

    //Amplitude update
    if (ia_amplitude.IsNewValue()) {
        ia_amplitude.Update();
    }

    //DC bias update
    if (ia_dc_bias.IsNewValue()) {
        ia_dc_bias.Update();
    }

    if (ia_scale.IsNewValue()) {
        ia_scale.Update();
    }

    if (ia_scale_plot.IsNewValue()) {
        ia_scale_plot.Update();
    }

    if (ia_shunt.IsNewValue()) {
        ia_shunt.Update();
        lcrApp_LcrSetCustomShunt(ia_shunt.Value());
    }

    if (ia_y_axis.IsNewValue()) {
        ia_y_axis.Update();
    }

    if (ia_lcr_shunt.IsNewValue()) {
        auto curValue = ia_lcr_shunt.Value();
        TRACE("Set LCR shunt %d", ia_lcr_shunt.NewValue());
        if (lcrApp_LcrSetShunt((lcr_shunt_t)ia_lcr_shunt.NewValue()) == RP_LCR_OK) {
            ia_lcr_shunt.Update();
        } else {
            ia_lcr_shunt.SendValue(curValue);
        }
    }

    //Measure start update
    if (ia_status.IsNewValue()) {
        ia_status.Update();
    }

    if (cur_x1.IsNewValue()) {
        cur_x1.Update();
        cur_x1.SendValue(cur_x1.Value());
    }

    if (cur_x2.IsNewValue()) {
        cur_x2.Update();
        cur_x2.SendValue(cur_x2.Value());
    }

    if (cur_y1.IsNewValue()) {
        cur_y1.Update();
        cur_y1.SendValue(cur_y1.Value());
    }

    if (cur_y2.IsNewValue()) {
        cur_y2.Update();
        cur_y2.SendValue(cur_y2.Value());
    }

    if (cur_x1_enable.IsNewValue()) {
        cur_x1_enable.Update();
        cur_x1_enable.SendValue(cur_x1_enable.Value());
    }

    if (cur_x2_enable.IsNewValue()) {
        cur_x2_enable.Update();
        cur_x2_enable.SendValue(cur_x2_enable.Value());
    }

    if (cur_y1_enable.IsNewValue()) {
        cur_y1_enable.Update();
        cur_y1_enable.SendValue(cur_y1_enable.Value());
    }

    if (cur_y2_enable.IsNewValue()) {
        cur_y2_enable.Update();
        cur_y2_enable.SendValue(cur_y2_enable.Value());
    }
    auto curT = getClock();
    if ((curT - g_lastCheckExt) > 2000) {
        auto state = lcrApp_LcrCheckExtensionModuleConnection(true) == 0;
        g_lastCheckExt = curT;
        setLCRExtState(state);
    }
}

void PostUpdateSignals() {}

void OnNewParams(void) {
    if (ia_status.IsNewValue()) {
        if (ia_status.NewValue() == IA_RESET_CONFIG_SETTINGS) {
            TRACE("Delete config");
            deleteConfig(getHomeDirectory() + "/.config/redpitaya/apps/impedance_analyzer_" + std::to_string((int)getModel()) + "/config.json");
            ia_status.Update();
            ia_status.SendValue(IA_RESET_CONFIG_SETTINGS_DONE);
            return;
        }
    }

    bool config_changed = isChanged();

    //Update parameters
    UpdateParams();

    if (config_changed) {
        configSet(getHomeDirectory() + "/.config/redpitaya/apps/impedance_analyzer_" + std::to_string((int)getModel()), "config.json");
    }
}

void OnNewSignals(void) {
    UpdateSignals();
}

void updateParametersByConfig() {
    configGet(getHomeDirectory() + "/.config/redpitaya/apps/impedance_analyzer_" + std::to_string((int)getModel()) + "/config.json");
    CDataManager::GetInstance()->SendAllParams();
}

void threadLoop() {
    profiler::resetAll();
    g_exit_flag = false;

    int cur_step = 0;
    int avaraging = 0;
    float start_freq = 0;
    float end_freq = 0;
    int steps = 0;
    float gen_ampl = 0;
    float dc_bias = 0;

    while (!g_exit_flag) {
        usleep(1000);  // 1 ms

        int status = ia_status.Value();

        if (status == IA_START || status == IA_START_PROCESS) {
            if (status == IA_START) {
                std::lock_guard<std::mutex> lock(g_signalMutex);
                for (int i = IA_FREQ; i <= IA_P_P; i++) {
                    signals_array[i].clear();
                }
                cur_step = 0;

                avaraging = ia_averaging.Value();
                start_freq = ia_start_freq.Value();
                end_freq = ia_end_freq.Value();
                steps = ia_steps.Value();
                gen_ampl = ia_amplitude.Value();
                dc_bias = ia_dc_bias.Value();

                if (lcr_ext.Value()) {
                    lcrApp_LcrSetShuntMode(RP_LCR_S_EXTENSION);
                    lcrApp_LcrSetShunt((lcr_shunt_t)ia_lcr_shunt.Value());
                } else {
                    lcrApp_LcrSetShuntMode(RP_LCR_S_CUSTOM);
                    lcrApp_LcrSetCustomShunt(ia_shunt.Value());
                }
                lcrApp_GenRun();
                ia_status.SendValue(IA_START_PROCESS);
            }

            if (cur_step < steps) {
                profiler::resetAll();
                uint32_t current_freq = 0.;
                float freq_step = 0;
                // float next_freq = 0.;

                if (ia_scale.NewValue()) {
                    // Log
                    auto a = log10f(start_freq);
                    auto b = log10f(end_freq);
                    auto c = (b - a) / ((float)steps - 1);

                    current_freq = pow(10.f, c * cur_step + a);
                    // next_freq = pow(10.f, c * (cur_step + 1) + a);
                } else {
                    // Linear
                    freq_step = (end_freq - start_freq) / ((float)steps - 1);
                    current_freq = start_freq + freq_step * cur_step;
                    // next_freq = start_freq + freq_step * (cur_step - 1);
                }
                lcrApp_LcrSetPause(true);
                // lcrApp_Set
                lcrApp_LcrSetFrequency(current_freq);
                lcrApp_LcrSetAmplitude(gen_ampl);
                lcrApp_LcrSetOffset(dc_bias);
                lcrApp_GenSetSettings();
                usleep(1000);
                lcrApp_LcrSetPause(false);
                lcr_main_data_t data;
                lcr_main_data_t res;
                memset(&data, 0, sizeof(lcr_main_data_t));
                memset(&res, 0, sizeof(lcr_main_data_t));
                int maxParam = 24;
                double* res_arr = reinterpret_cast<double*>(&res);
                double* data_arr = reinterpret_cast<double*>(&data);

                for (int i = 0; i < avaraging; ++i) {

                    do {
                        usleep(1000);
                        lcrApp_LcrCopyParams(&data);
                        if (g_exit_flag) {
                            return;
                        }
                    } while ((uint32_t)data.lcr_freq != current_freq);

                    res.lcr_freq = data.lcr_freq;
                    res.lcr_P_p_amp = data.lcr_P_p_amp;

                    for (int z = 1; z <= maxParam; z++) {
                        res_arr[z] = NAN_INF(data_arr[z]) ? data_arr[z] : res_arr[z] + data_arr[z];
                    }
                }
                lcrApp_LcrSetPause(true);
                for (int z = 1; z <= maxParam; z++) {
                    res_arr[z] = NAN_INF(res_arr[z]) ? NAN_VALUE : ((res_arr[z] * 1000000000000.0) / (double)avaraging);
                }
                cur_step++;
                ia_current_step.SendValue(cur_step);
                ia_current_freq.SendValue(current_freq);

                std::lock_guard<std::mutex> lock(g_signalMutex);
                if (std::find(begin(signals_array[IA_FREQ]), end(signals_array[IA_FREQ]), res.lcr_freq) == end(signals_array[IA_FREQ])) {
                    signals_array[IA_FREQ].push_back(res.lcr_freq);
                    signals_array[IA_Z].push_back(res.lcr_amplitude);
                    signals_array[IA_PHASE].push_back(res.lcr_phase);
                    signals_array[IA_Y_s].push_back(res.lcr_Y_abs);
                    signals_array[IA_NEG_PHASE].push_back(res.lcr_Phase_Y);
                    signals_array[IA_R_s].push_back(res.lcr_R_s);
                    signals_array[IA_R_p].push_back(res.lcr_R_p);
                    signals_array[IA_X_s].push_back(res.lcr_X_s);
                    signals_array[IA_G_p].push_back(res.lcr_G_p);
                    signals_array[IA_B_p].push_back(res.lcr_B_p);
                    signals_array[IA_C_s].push_back(res.lcr_C_s);
                    signals_array[IA_C_p].push_back(res.lcr_C_p);
                    signals_array[IA_L_s].push_back(res.lcr_L_s);
                    signals_array[IA_L_p].push_back(res.lcr_L_p);
                    signals_array[IA_Q].push_back(res.lcr_Q);
                    signals_array[IA_D].push_back(res.lcr_D);
                    signals_array[IA_P_P].push_back(res.lcr_P_p_amp);
                }
            } else {
                ia_status.SendValue(IA_START_DONE);
            }
        }
    }
}
