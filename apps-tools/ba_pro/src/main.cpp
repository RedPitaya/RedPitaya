#include "main.h"

#include <complex.h>
#include <fcntl.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/syslog.h>
#include <unistd.h>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <exception>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "bodeApp.h"
#include "common/rp_log.h"
#include "common/version.h"

#include "main.h"
#include "rp_hw-profiles.h"
#include "rp_hw_calib.h"
#include "settings.h"

#include "math/rp_math.h"
#include "rpApp.h"
#include "web/rp_client.h"

/***************************************************************************************
*                                     BODE ANALYSER                                    *
***************************************************************************************/

enum {
    BA_NONE = 0,
    BA_START = 1,
    BA_START_CALIB = 2,
    BA_RESET_CALIB = 3,
    BA_RESET_CONFIG_SETTINGS = 4,
    BA_RESET_CONFIG_SETTINGS_DONE = 5,
    BA_START_DONE = 6,
    BA_START_CALIB_DONE = 7,
    BA_START_PROCESS = 8,
    BA_START_CALIB_PROCESS = 9
} ba_status_t;

/***************************************************************************************
*  Hardware profile cache.
*  Filled exactly once from rp_app_init(). Every rp_HPGet*() call used to happen during
*  static initialisation of this shared object, i.e. on dlopen(), before the framework
*  was up. Now nothing touches the EEPROM until the app is actually started.
***************************************************************************************/

namespace {

struct HWProfile {
    rp_HPeModels_t model = STEM_125_14_v1_0;
    std::string modelStr = "Z10";
    bool genBias = true;
    bool isLV_HV = false;
    bool isAC_DC = false;
    int freqMin = 1;
    int freqMax = 1;
    uint32_t maxADC = 1;
    float ampDef = 0.f;
    float ampMax = 0.f;
};

HWProfile g_hw;
bool g_hwLoaded = false;

auto modelToStr(rp_HPeModels_t model) -> const char*;
auto modelToAmpDef(rp_HPeModels_t model) -> float;
auto modelToAmpMax(rp_HPeModels_t model) -> float;
auto modelToGenBias(rp_HPeModels_t model) -> bool;

}  // namespace

/***************************************************************************************
*  Parameters.
*  Held in a single heap object so that construction (and CDataManager registration)
*  happens inside rp_app_init(), after the hardware profile is known.
*  Access through P(): P().ba_status.Value()
***************************************************************************************/

struct CParams {

    // Control parameters
    CIntParameter ba_status;

    //Parameters
    CIntParameter ba_start_freq;
    CIntParameter ba_end_freq;
    CIntParameter ba_steps;
    CIntParameter ba_periods_number;
    CIntParameter ba_averaging;
    CFloatParameter ba_amplitude;
    CFloatParameter ba_dc_bias;
    CBooleanParameter isDCBias;
    CFloatParameter ba_gain_min;
    CFloatParameter ba_gain_max;
    CFloatParameter ba_phase_min;
    CFloatParameter ba_phase_max;
    CBooleanParameter ba_scale;
    CIntParameter ba_x_scale;
    CBooleanParameter ba_auto_scale;
    CFloatParameter ba_input_threshold;
    CBooleanParameter ba_show_all;
    CIntParameter ba_logic_mode;
    CIntParameter inGain;
    CBooleanParameter isGain;
    CIntParameter inAC_DC;
    CIntParameter inProbe;

    // Status parameters
    CStringParameter redpitaya_model;
    CFloatParameter ba_current_freq;
    CIntParameter ba_current_step;
    CBooleanParameter ba_calibrate_enable;

    //Singals
    CIntBase64Signal ba_bad_signal;
    CFloatBase64Signal ba_signal_1;
    CFloatBase64Signal ba_signal_2;
    CIntBase64Signal ba_signal_parameters;
    CBooleanParameter ba_cur_x1;
    CBooleanParameter ba_cur_x2;
    CBooleanParameter ba_cur_y1;
    CBooleanParameter ba_cur_y2;
    CBooleanParameter ba_cur_z1;
    CBooleanParameter ba_cur_z2;
    CFloatParameter ba_cur_x1_pos;
    CFloatParameter ba_cur_x2_pos;
    CFloatParameter ba_cur_y1_pos;
    CFloatParameter ba_cur_y2_pos;
    CFloatParameter ba_cur_z1_pos;
    CFloatParameter ba_cur_z2_pos;

    CParams()
        : ba_status("BA_STATUS", CBaseParameter::RW, 0, 0, 0, 100),
          ba_start_freq("BA_START_FREQ", CBaseParameter::RW, std::max<int>(1000, outFreqMin()), 0, outFreqMin(), getMaxADC(), CONFIG_VAR),
          ba_end_freq("BA_END_FREQ", CBaseParameter::RW, getMaxADC(), 0, outFreqMin(), getMaxADC(), CONFIG_VAR),
          ba_steps("BA_STEPS", CBaseParameter::RW, 25, 0, 2, CH_SIGNAL_SIZE_DEFAULT, CONFIG_VAR),
          ba_periods_number("BA_PERIODS_NUMBER", CBaseParameter::RW, 8, 0, 1, 8, CONFIG_VAR),
          ba_averaging("BA_AVERAGING", CBaseParameter::RW, 1, 0, 1, 10, CONFIG_VAR),
          ba_amplitude("BA_AMPLITUDE", CBaseParameter::RW, outAmpDef(), 0, 0, outAmpMax(), CONFIG_VAR),
          ba_dc_bias("BA_DC_BIAS", CBaseParameter::RW, 0, 0, -outAmpMax(), outAmpMax(), CONFIG_VAR),
          isDCBias("BA_IS_DC_BIAS", CBaseParameter::RO, isGenBias(), 0),
          ba_gain_min("BA_GAIN_MIN", CBaseParameter::RW, -30, 0, -100, 100, CONFIG_VAR),
          ba_gain_max("BA_GAIN_MAX", CBaseParameter::RW, 10, 0, -100, 100, CONFIG_VAR),
          ba_phase_min("BA_PHASE_MIN", CBaseParameter::RW, -90, 0, -90, 90, CONFIG_VAR),
          ba_phase_max("BA_PHASE_MAX", CBaseParameter::RW, 90, 0, -90, 90, CONFIG_VAR),
          ba_scale("BA_SCALE", CBaseParameter::RW, true, 0, CONFIG_VAR),
          ba_x_scale("BA_X_SCALE", CBaseParameter::RW, 0, 0, 0, 3, CONFIG_VAR),
          ba_auto_scale("BA_AUTO_SCALE", CBaseParameter::RW, true, 0, CONFIG_VAR),
          ba_input_threshold("BA_INPUT_THRESHOLD", CBaseParameter::RW, 0.001, 0, 0, 1, CONFIG_VAR),
          ba_show_all("BA_SHOW_ALL", CBaseParameter::RW, true, 0, CONFIG_VAR),
          ba_logic_mode("BA_LOGIC_MODE", CBaseParameter::RW, 0, 0, 0, 10, CONFIG_VAR),
          inGain("BA_IN_GAIN", CBaseParameter::RW, RP_LOW, 0, 0, 1, CONFIG_VAR),
          isGain("BA_IS_GAIN", CBaseParameter::RO, isLV_HV(), 0),
          inAC_DC("BA_IN_AC_DC", CBaseParameter::RW, RP_DC, 0, 0, 1, CONFIG_VAR),
          inProbe("BA_PROBE", CBaseParameter::RW, 1, 0, 0, 2000, CONFIG_VAR),
          redpitaya_model("RP_MODEL_STR", CBaseParameter::RO, getModelS(), 0),
          ba_current_freq("BA_CURRENT_FREQ", CBaseParameter::RW, 1, 0, 0, getMaxADC()),
          ba_current_step("BA_CURRENT_STEP", CBaseParameter::RW, 1, 0, 1, getMaxADC()),
          ba_calibrate_enable("BA_CALIBRATE_ENABLE", CBaseParameter::RW, false, 0),
          ba_bad_signal("BA_BAD_SIGNAL", CH_SIGNAL_SIZE_DEFAULT, 0),
          ba_signal_1("BA_SIGNAL_1", CH_SIGNAL_SIZE_DEFAULT, 0.0f),
          ba_signal_2("BA_SIGNAL_2", CH_SIGNAL_SIZE_DEFAULT, 0.0f),
          ba_signal_parameters("BA_SIGNAL_PARAMETERS", 4, 0),
          ba_cur_x1("BA_CURSOR_X1", CBaseParameter::RW, false, 0, CONFIG_VAR),
          ba_cur_x2("BA_CURSOR_X2", CBaseParameter::RW, false, 0, CONFIG_VAR),
          ba_cur_y1("BA_CURSOR_Y1", CBaseParameter::RW, false, 0, CONFIG_VAR),
          ba_cur_y2("BA_CURSOR_Y2", CBaseParameter::RW, false, 0, CONFIG_VAR),
          ba_cur_z1("BA_CURSOR_Z1", CBaseParameter::RW, false, 0, CONFIG_VAR),
          ba_cur_z2("BA_CURSOR_Z2", CBaseParameter::RW, false, 0, CONFIG_VAR),
          ba_cur_x1_pos("BA_CURSOR_X1_POS", CBaseParameter::RW, 0.333, 0, 0, 1, CONFIG_VAR),
          ba_cur_x2_pos("BA_CURSOR_X2_POS", CBaseParameter::RW, 0.666, 0, 0, 1, CONFIG_VAR),
          ba_cur_y1_pos("BA_CURSOR_Y1_POS", CBaseParameter::RW, 0.333, 0, 0, 1, CONFIG_VAR),
          ba_cur_y2_pos("BA_CURSOR_Y2_POS", CBaseParameter::RW, 0.666, 0, 0, 1, CONFIG_VAR),
          ba_cur_z1_pos("BA_CURSOR_Z1_POS", CBaseParameter::RW, 0.333, 0, 0, 1, CONFIG_VAR),
          ba_cur_z2_pos("BA_CURSOR_Z2_POS", CBaseParameter::RW, 0.666, 0, 0, 1, CONFIG_VAR) {}
};

static std::unique_ptr<CParams> g_params;
static bool g_initialised = false;

/* Never call before rp_app_init() has returned 0. */
static inline CParams& P() {
    return *g_params;
}

static std::vector<float> signal;
static std::vector<float> phase;
static std::vector<int> bad_signal;
static std::vector<int> signal_parameters;

static std::vector<float> signalView;
static std::vector<float> phaseView;
static std::vector<int> bad_signalView;
static std::vector<int> signal_parametersView;

std::thread* g_thread = NULL;
std::mutex g_signalMutex;
std::atomic<bool> g_exit_flag{false};
std::atomic<bool> g_request_show{false};

void threadLoop();

namespace {

auto modelToStr(rp_HPeModels_t model) -> const char* {

    switch (model) {
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
        case STEM_125_14_BO_v2_0:
        case STEM_125_14_Pro_v2_0:
        case STEM_125_14_Pro_BO_v2_0:
        case STEM_125_14_Z7020_Pro_v1_0:
        case STEM_125_14_Z7020_Pro_v2_0:
        case STEM_125_14_Z7020_Pro_BO_v2_0:
        case STEM_125_14_Z7020_Ind_v2_0:
        case STEM_125_14_Z7020_LL_v1_1:
        case STEM_65_16_Z7020_LL_v1_1:
        case STEM_125_14_Z7020_LL_v1_2:
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
            break;
    }
    return nullptr;
}

auto modelToGenBias(rp_HPeModels_t model) -> bool {

    switch (model) {
        case STEM_122_16SDR_v1_0:
        case STEM_122_16SDR_v1_1:
            return false;
        default:;
    }
    return true;
}

auto modelToAmpDef(rp_HPeModels_t model) -> float {
    switch (model) {
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
        case STEM_125_14_BO_v2_0:
        case STEM_125_14_Pro_v2_0:
        case STEM_125_14_Pro_BO_v2_0:
        case STEM_125_14_Z7020_Pro_v1_0:
        case STEM_125_14_Z7020_Pro_v2_0:
        case STEM_125_14_Z7020_Pro_BO_v2_0:
        case STEM_125_14_Z7020_Ind_v2_0:
        case STEM_125_14_Z7020_LL_v1_1:
        case STEM_125_14_Z7020_LL_v1_2:
        case STEM_65_16_Z7020_LL_v1_1:
        case STEM_65_16_Z7020_TI_v1_3:
        case STEM_125_14_Z7020_TI_v1_3:
            return 0.9;
        case STEM_122_16SDR_v1_0:
        case STEM_122_16SDR_v1_1:
            return 0.4;
        case STEM_125_14_Z7020_4IN_v1_0:
        case STEM_125_14_Z7020_4IN_v1_2:
        case STEM_125_14_Z7020_4IN_v1_3:
        case STEM_125_14_Z7020_4IN_BO_v1_3:
            return 0.9;
        case STEM_250_12_v1_0:
        case STEM_250_12_v1_1:
        case STEM_250_12_v1_2:
        case STEM_250_12_v1_2a:
        case STEM_250_12_v1_2b:
        case STEM_250_12_120:
            return 0.9;
        default: {
            ERROR_LOG("Unknown model: %d.", model);
            return 0;
        }
    }
}

auto modelToAmpMax(rp_HPeModels_t model) -> float {
    switch (model) {
        case STEM_125_10_v1_0:
        case STEM_125_14_v1_0:
        case STEM_125_14_v1_1:
        case STEM_125_14_LN_v1_1:
        case STEM_125_14_LN_BO_v1_1:
        case STEM_125_14_LN_CE1_v1_1:
        case STEM_125_14_LN_CE2_v1_1:
        case STEM_125_14_Z7020_v1_0:
        case STEM_125_14_Z7020_LN_v1_1:
            return 1;
        case STEM_125_14_v2_0:
        case STEM_125_14_BO_v2_0:
        case STEM_125_14_Pro_v2_0:
        case STEM_125_14_Pro_BO_v2_0:
        case STEM_125_14_Z7020_Pro_v1_0:
        case STEM_125_14_Z7020_Pro_v2_0:
        case STEM_125_14_Z7020_Pro_BO_v2_0:
        case STEM_125_14_Z7020_Ind_v2_0:
            return 2;
        case STEM_125_14_Z7020_LL_v1_1:
        case STEM_125_14_Z7020_LL_v1_2:
        case STEM_65_16_Z7020_LL_v1_1:
        case STEM_65_16_Z7020_TI_v1_3:
        case STEM_125_14_Z7020_TI_v1_3:
            return 2;
        case STEM_122_16SDR_v1_0:
        case STEM_122_16SDR_v1_1:
            return 0.5;
        case STEM_125_14_Z7020_4IN_v1_0:
        case STEM_125_14_Z7020_4IN_v1_2:
        case STEM_125_14_Z7020_4IN_v1_3:
        case STEM_125_14_Z7020_4IN_BO_v1_3:
            return 1;
        case STEM_250_12_v1_0:
        case STEM_250_12_v1_1:
        case STEM_250_12_v1_2:
        case STEM_250_12_v1_2a:
        case STEM_250_12_v1_2b:
        case STEM_250_12_120:
            return 10.0;
        default: {
            ERROR_LOG("Unknown model: %d.", model);
            return 0;
        }
    }
}

}  // namespace

/* Reads the whole hardware profile in one go. Called once, from rp_app_init(). */
static int loadHWProfile() {
    if (g_hwLoaded) {
        return RP_OK;
    }

    rp_HPeModels_t model = STEM_125_14_v1_0;
    if (rp_HPGetModel(&model) != RP_HP_OK) {
        ERROR_LOG("Can't get board model");
        return RP_EOOR;
    }

    const char* name = modelToStr(model);
    if (name == nullptr) {
        ERROR_LOG("Unsupported board model: %d", model);
        return RP_EOOR;
    }

    g_hw.model = model;
    g_hw.modelStr = name;
    g_hw.genBias = modelToGenBias(model);
    g_hw.ampDef = modelToAmpDef(model);
    g_hw.ampMax = modelToAmpMax(model);
    g_hw.isLV_HV = rp_HPGetFastADCIsLV_HVOrDefault();
    g_hw.isAC_DC = rp_HPGetFastADCIsAC_DCOrDefault();
    g_hw.freqMin = rp_HPGetGenMinSpeedHzOrDefault();
    g_hw.freqMax = rp_HPGetGenMaxSpeedHzOrDefault();

    uint32_t lpf = 0;
    if (rp_HPGetFastADCMaxLowPassFilterHz(&lpf) != RP_HP_OK) {
        ERROR_LOG("Can't get ADC low-pass filter value");
        return RP_EOOR;
    }
    g_hw.maxADC = std::min<uint32_t>(lpf, (uint32_t)g_hw.freqMax);

    g_hwLoaded = true;
    return RP_OK;
}

/* Cheap accessors -- no hardware access, safe to call from anywhere after init. */
auto getModel() -> rp_HPeModels_t {
    return g_hw.model;
}
auto getModelS() -> std::string {
    return g_hw.modelStr;
}
auto isGenBias() -> bool {
    return g_hw.genBias;
}
auto isLV_HV() -> bool {
    return g_hw.isLV_HV;
}
auto isAC_DC() -> bool {
    return g_hw.isAC_DC;
}
auto outFreqMin() -> int {
    return g_hw.freqMin;
}
auto outFreqMax() -> int {
    return g_hw.freqMax;
}
auto outAmpDef() -> float {
    return g_hw.ampDef;
}
auto outAmpMax() -> float {
    return g_hw.ampMax;
}
auto getMaxADC() -> uint32_t {
    return g_hw.maxADC;
}

//Application description
const char* rp_app_desc(void) {
    return (const char*)"Red Pitaya Bode analyser application.\n";
}

//Application init
int rp_app_init(void) {
    fprintf(stderr, "Loading bode analyser version %s-%s.\n", VERSION_STR, REVISION_STR);
    signal.reserve(CH_SIGNAL_SIZE_DEFAULT);
    phase.reserve(CH_SIGNAL_SIZE_DEFAULT);
    bad_signal.reserve(CH_SIGNAL_SIZE_DEFAULT);
    signal_parameters.reserve(CH_SIGNAL_SIZE_DEFAULT);

#ifdef ZIP_DISABLED
    CDataManager::GetInstance()->SetEnableParamsGZip(false);
    CDataManager::GetInstance()->SetEnableSignalsGZip(false);
    CDataManager::GetInstance()->SetEnableBinarySignalsGZip(false);
#endif
    CDataManager::GetInstance()->SetParamInterval(50);
    CDataManager::GetInstance()->SetSignalInterval(50);

    rp_Init();
    rp_AcqSetAC_DC(RP_CH_1, RP_DC);
    rp_AcqSetAC_DC(RP_CH_2, RP_DC);

    // Hardware profile first: the parameter bounds below depend on it.
    if (loadHWProfile() != RP_OK) {
        ERROR_LOG("Failed to read the hardware profile, aborting init");
        rp_Release();
        return -1;
    }

    // Parameters are created here, not at load time. This is what registers them
    // with CDataManager, so nothing may touch P() before this point.
    // Reset first, so that a second rp_app_init() on an already-loaded .so cannot
    // briefly have two sets of identically named parameters registered at once.
    g_params.reset();
    try {
        g_params = std::make_unique<CParams>();
    } catch (const std::exception& e) {
        ERROR_LOG("Failed to create parameters: %s", e.what());
        rp_Release();
        return -1;
    } catch (...) {
        ERROR_LOG("Failed to create parameters");
        rp_Release();
        return -1;
    }

    rpApp_BaInit();
    rpApp_BaReadCalibration();
    updateParametersByConfig();

    rp_WC_Init();
    g_exit_flag = false;
    g_initialised = true;
    g_thread = new std::thread(threadLoop);
    return 0;
}

//Application exit
int rp_app_exit(void) {
    // rp_app_init() may have bailed out early; it already released what it took.
    // Without this the loader calling exit after a failed init would double-release.
    if (!g_initialised) {
        return 0;
    }
    g_initialised = false;

    g_exit_flag = true;
    if (g_thread) {
        g_thread->join();
        delete g_thread;
        g_thread = nullptr;
    }
    rp_Release();
    rpApp_BaRelease();

    // g_params is deliberately NOT destroyed here. CDataManager stores raw
    // CBaseParameter* and it is not visible from this translation unit whether
    // ~CBaseParameter deregisters them. Leaving the object alive until the .so is
    // unloaded reproduces the original lifetime of the globals exactly and cannot
    // dangle. If ~CBaseParameter does deregister, add g_params.reset() here.

    fprintf(stderr, "Unloading bode analyser version %s-%s.\n", VERSION_STR, REVISION_STR);
    return 0;
}

//Update signals
void UpdateSignals(void) {
    if (!g_params) {
        return;
    }
    if (g_request_show) {
        std::lock_guard lock(g_signalMutex);
        P().ba_signal_1.Set(signal);
        P().ba_signal_2.Set(phase);
        P().ba_bad_signal.Set(bad_signal);
        P().ba_signal_parameters.Set(signal_parameters);
        g_request_show = false;
    }
}
void UpdateParams(void) {}

//Update parameters
void UpdateParamsFromWeb(void) {
    if (!g_params) {
        return;
    }
    //Measure start update
    if (P().ba_status.IsNewValue()) {
        P().ba_status.Update();
    }

    //Start frequency update
    if (P().ba_start_freq.IsNewValue()) {
        P().ba_start_freq.Update();
    }

    //End frequency update
    if (P().ba_end_freq.IsNewValue()) {
        P().ba_end_freq.Update();
    }

    //Steps update
    if (P().ba_steps.IsNewValue()) {
        P().ba_steps.Update();
    }

    //Periods number update
    if (P().ba_periods_number.IsNewValue()) {
        P().ba_periods_number.Update();
    }

    //Averaging update
    if (P().ba_averaging.IsNewValue()) {
        P().ba_averaging.Update();
    }

    //Amplitude update
    if (P().ba_amplitude.IsNewValue()) {
        P().ba_amplitude.Update();
    }

    //DC bias update
    if (P().ba_dc_bias.IsNewValue()) {
        P().ba_dc_bias.Update();
    }

    //Gain min update
    if (P().ba_gain_min.IsNewValue()) {
        P().ba_gain_min.Update();
    }

    //Gain max update
    if (P().ba_gain_max.IsNewValue()) {
        P().ba_gain_max.Update();
    }

    //Phase min update
    if (P().ba_phase_min.IsNewValue()) {
        P().ba_phase_min.Update();
    }

    //Phase max update
    if (P().ba_phase_max.IsNewValue()) {
        P().ba_phase_max.Update();
    }

    //Scale update
    if (P().ba_scale.IsNewValue()) {
        P().ba_scale.Update();
    }

    if (P().ba_x_scale.IsNewValue()) {
        P().ba_x_scale.Update();
    }

    if (P().ba_logic_mode.IsNewValue()) {
        P().ba_logic_mode.Update();
    }

    if (P().inProbe.IsNewValue()) {
        P().inProbe.Update();
    }

    if (P().inAC_DC.IsNewValue()) {
        P().inAC_DC.Update();
    }

    if (P().inGain.IsNewValue()) {
        P().inGain.Update();
    }

    //Scale update
    if (IS_NEW(P().ba_input_threshold)) {
        P().ba_input_threshold.Update();
    }

    if (IS_NEW(P().ba_auto_scale)) {
        P().ba_auto_scale.Update();
    }

    if (IS_NEW(P().ba_show_all)) {
        P().ba_show_all.Update();
    }

    auto is_calib = rpApp_BaGetCalibStatus();
    if (P().ba_calibrate_enable.Value() != is_calib) {
        P().ba_calibrate_enable.SendValue(is_calib);
    }

    if (IS_NEW(P().ba_cur_x1)) {
        P().ba_cur_x1.Update();
    }

    if (IS_NEW(P().ba_cur_x2)) {
        P().ba_cur_x2.Update();
    }

    if (IS_NEW(P().ba_cur_y1)) {
        P().ba_cur_y1.Update();
    }

    if (IS_NEW(P().ba_cur_y2)) {
        P().ba_cur_y2.Update();
    }

    if (IS_NEW(P().ba_cur_z1)) {
        P().ba_cur_z1.Update();
    }

    if (IS_NEW(P().ba_cur_z2)) {
        P().ba_cur_z2.Update();
    }

    if (IS_NEW(P().ba_cur_x1_pos)) {
        P().ba_cur_x1_pos.Update();
    }

    if (IS_NEW(P().ba_cur_x2_pos)) {
        P().ba_cur_x2_pos.Update();
    }

    if (IS_NEW(P().ba_cur_y1_pos)) {
        P().ba_cur_y1_pos.Update();
    }

    if (IS_NEW(P().ba_cur_y2_pos)) {
        P().ba_cur_y2_pos.Update();
    }

    if (IS_NEW(P().ba_cur_z1_pos)) {
        P().ba_cur_z1_pos.Update();
    }

    if (IS_NEW(P().ba_cur_z2_pos)) {
        P().ba_cur_z2_pos.Update();
    }
}

void bode_ResetCalib() {
    std::lock_guard<std::mutex> lock(g_signalMutex);
    rpApp_BaResetCalibration();
    rpApp_BaReadCalibration();
    TRACE_SHORT("Calibration reseted");
}

void PostUpdateSignals() {}

void OnNewParams(void) {
    if (!g_params) {
        return;
    }

    if (P().ba_status.IsNewValue()) {
        if (P().ba_status.NewValue() == BA_RESET_CONFIG_SETTINGS) {
            TRACE_SHORT("Delete config");
            deleteConfig(getHomeDirectory() + "/.config/redpitaya/apps/ba_pro_" + std::to_string((int)getModel()) + "/config.json");
            P().ba_status.Update();
            P().ba_status.SendValue(BA_RESET_CONFIG_SETTINGS_DONE);
            return;
        }
    }

    bool config_changed = isChanged();

    //Update parameters
    UpdateParamsFromWeb();

    if (P().ba_status.Value() == BA_RESET_CALIB) {
        bode_ResetCalib();
        P().ba_status.SendValue(0);
    }

    if (config_changed) {
        configSet(getHomeDirectory() + "/.config/redpitaya/apps/ba_pro_" + std::to_string((int)getModel()), "config.json");
    }
}

void OnNewSignals(void) {
    UpdateSignals();
}

void updateParametersByConfig() {
    if (!g_params) {
        return;
    }
    configGet(getHomeDirectory() + "/.config/redpitaya/apps/ba_pro_" + std::to_string((int)getModel()) + "/config.json");
    CDataManager::GetInstance()->SendAllParams();
}

void threadLoop() {
    rp_ba_buffer_t buffer(ADC_BUFFER_SIZE);
    int cur_step = 0;

    int avaraging = 0;

    float start_freq = 0;
    float end_freq = 0;
    float steps = 0;
    float threshold = 0;
    float per_number = 0;
    float gen_ampl = 0;
    float dc_bias = 0;
    float probe = 0;
    rp_ba_logic_t logic_mode = RP_BA_LOGIC_TRAP;

    while (!g_exit_flag) {
        usleep(100);

        int status = P().ba_status.Value();
        // user start calibration
        if (status == BA_START_CALIB || status == BA_START_CALIB_PROCESS) {
            if (status == BA_START_CALIB) {
                bode_ResetCalib();
                std::lock_guard lock(g_signalMutex);
                signal.clear();
                phase.clear();
                bad_signal.clear();
                signal_parameters.clear();
                cur_step = 0;

                avaraging = 1;
                // Points below the generator minimum are never measured, so they only slow calibration down
                start_freq = std::max<float>(100, outFreqMin());
                end_freq = getMaxADC();
                steps = 500;
                threshold = P().ba_input_threshold.Value();
                logic_mode = (rp_ba_logic_t)P().ba_logic_mode.Value();
                per_number = P().ba_periods_number.Value();
                gen_ampl = P().ba_amplitude.Value();
                dc_bias = P().ba_dc_bias.Value();
                probe = P().inProbe.Value();
                signal_parameters.push_back(start_freq);
                signal_parameters.push_back(end_freq);
                signal_parameters.push_back(steps);
                P().ba_status.SendValue(BA_START_CALIB_PROCESS);
                if (isLV_HV()) {
                    rp_AcqSetGain(RP_CH_1, P().inGain.Value() != 0 ? RP_HIGH : RP_LOW);
                    rp_AcqSetGain(RP_CH_2, P().inGain.Value() != 0 ? RP_HIGH : RP_LOW);
                }

                if (isAC_DC()) {
                    rp_AcqSetAC_DC(RP_CH_1, P().inAC_DC.Value() == 1 ? RP_DC : RP_AC);
                    rp_AcqSetAC_DC(RP_CH_2, P().inAC_DC.Value() == 1 ? RP_DC : RP_AC);
                }
                g_request_show = true;
            }

            if (cur_step < steps) {

                float amplitude = 0, phase_out = 0;
                float current_freq = 0.;
                float freq_step = 0;
                bool low_signal = false;

                if (P().ba_scale.NewValue()) {
                    // Log
                    auto a = log10f(start_freq);
                    auto b = log10f(end_freq);
                    auto c = (b - a) / (steps - 1);

                    current_freq = pow(10.f, c * cur_step + a);
                } else {
                    // Linear
                    freq_step = (end_freq - start_freq) / (steps - 1);
                    current_freq = start_freq + freq_step * cur_step;
                }

                for (int i = 0; i < avaraging; ++i) {
                    float ampl_step = 0;
                    float phase_step = 0;
                    rpApp_BaSafeThreadAcqPrepare();
                    auto ret = rpApp_BaGetAmplPhase(logic_mode, gen_ampl, dc_bias, per_number, buffer, &ampl_step, &phase_step, current_freq, probe, threshold);
                    if (ret == RP_EOOR) {  // isnan && isinf
                        low_signal = true;
                        ampl_step = 0;
                        phase_step = 0;
                    }
                    if (ret == RP_EIPV) {
                        low_signal = true;
                    }

                    amplitude += ampl_step;
                    phase_out += phase_step;
                }

                amplitude /= (int)avaraging;
                phase_out /= (int)avaraging;

                cur_step++;
                P().ba_current_step.SendValue(cur_step);
                P().ba_current_freq.SendValue(current_freq);

                std::lock_guard lock(g_signalMutex);
                rpApp_BaWriteCalib(current_freq, amplitude, phase_out);
                signal.push_back(rpApp_BaCalibGain(current_freq, amplitude));
                phase.push_back(rpApp_BaCalibPhase(current_freq, phase_out));

                if (low_signal) {
                    bad_signal.push_back(1);
                } else {
                    bad_signal.push_back(0);
                }
                g_request_show = true;
            } else {
                rpApp_BaReadCalibration();
                P().ba_calibrate_enable.SendValue(rpApp_BaGetCalibStatus());
                P().ba_status.SendValue(BA_START_CALIB_DONE);
            }
        }

        if (status == BA_START || status == BA_START_PROCESS) {
            if (status == BA_START) {
                std::lock_guard lock(g_signalMutex);
                signal.clear();
                phase.clear();
                bad_signal.clear();
                signal_parameters.clear();
                cur_step = 0;

                avaraging = P().ba_averaging.Value();
                start_freq = P().ba_start_freq.Value();
                end_freq = P().ba_end_freq.Value();
                steps = P().ba_steps.Value();
                threshold = P().ba_input_threshold.Value();
                logic_mode = (rp_ba_logic_t)P().ba_logic_mode.Value();
                per_number = P().ba_periods_number.Value();
                gen_ampl = P().ba_amplitude.Value();
                dc_bias = P().ba_dc_bias.Value();
                probe = P().inProbe.Value();

                signal_parameters.push_back(start_freq);
                signal_parameters.push_back(end_freq);
                signal_parameters.push_back(steps);

                if (isLV_HV()) {
                    rp_AcqSetGain(RP_CH_1, P().inGain.Value() != 0 ? RP_HIGH : RP_LOW);
                    rp_AcqSetGain(RP_CH_2, P().inGain.Value() != 0 ? RP_HIGH : RP_LOW);
                }

                if (isAC_DC()) {
                    rp_AcqSetAC_DC(RP_CH_1, P().inAC_DC.Value() == 1 ? RP_DC : RP_AC);
                    rp_AcqSetAC_DC(RP_CH_2, P().inAC_DC.Value() == 1 ? RP_DC : RP_AC);
                }

                P().ba_status.SendValue(BA_START_PROCESS);
                TRACE_SHORT("start_freq %f", start_freq);
                TRACE_SHORT("end_freq %f", end_freq);
                TRACE_SHORT("steps %f", steps);
                g_request_show = true;
            }

            if (cur_step < (int)steps) {

                float amplitude = 0, phase_out = 0;
                float current_freq = 0.;
                float freq_step = 0;
                bool low_signal = false;

                if (P().ba_scale.NewValue()) {
                    // Log
                    auto a = log10f(start_freq);
                    auto b = log10f(end_freq);
                    auto c = (b - a) / (steps - 1);

                    current_freq = pow(10.f, c * cur_step + a);
                } else {
                    // Linear
                    freq_step = (end_freq - start_freq) / (steps - 1);
                    current_freq = start_freq + freq_step * cur_step;
                }

                for (int i = 0; i < avaraging; ++i) {
                    float ampl_step = 0;
                    float phase_step = 0;
                    rpApp_BaSafeThreadAcqPrepare();
                    auto ret = rpApp_BaGetAmplPhase(logic_mode, gen_ampl, dc_bias, per_number, buffer, &ampl_step, &phase_step, current_freq, probe, threshold);
                    if (ret == RP_EOOR) {  // isnan && isinf
                        low_signal = true;
                        ampl_step = 0;
                        phase_step = 0;
                    }

                    if (ret == RP_EIPV) {
                        low_signal = true;
                    }

                    amplitude += ampl_step;
                    phase_out += phase_step;
                }

                amplitude /= (int)avaraging;
                phase_out /= (int)avaraging;

                cur_step++;
                P().ba_current_step.SendValue(cur_step);
                P().ba_current_freq.SendValue(current_freq);

                std::lock_guard lock(g_signalMutex);

                signal.push_back(rpApp_BaCalibGain(current_freq, amplitude));
                phase.push_back(rpApp_BaCalibPhase(current_freq, phase_out));

                if (low_signal) {
                    bad_signal.push_back(1);
                } else {
                    bad_signal.push_back(0);
                }
                g_request_show = true;
            } else {
                P().ba_status.SendValue(BA_START_DONE);
            }
        }
    }
}