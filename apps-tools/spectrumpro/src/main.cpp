#include "main.h"
#include "common.h"
#include "common/profiler.h"
#include "common/rp_sweep.h"
#include "generator.h"
#include "math/rp_dsp.h"
#include "neon_asm.h"
#include "rp.h"
#include "rp_math.h"
#include "settings.h"
#include "web/rp_client.h"
#include "web/rp_websocket.h"

#define DATA_TCP_PORT 9900

static uint8_t g_adc_count = getADCChannels();

CStringParameter redpitaya_model_str("RP_MODEL_STR", CBaseParameter::RO, getModelName(), 0);
CIntParameter redpitaya_model("RP_MODEL", CBaseParameter::RO, (int)getModel(), 0, 0, 300);
CIntParameter redpitaya_adc_count("ADC_COUNT", CBaseParameter::RO, getADCChannels(), 0, 0, 4);

CBooleanParameter isFilter("SPEC_IS_FILTER", CBaseParameter::RO, rp_HPGetFastADCIsFilterPresentOrDefault(), 0);
CBooleanParameter isAC_DC("SPEC_IS_AC_DC", CBaseParameter::RO, rp_HPGetFastADCIsAC_DCOrDefault(), 0);
CBooleanParameter isHV_LV("SPEC_IS_HV_LV", CBaseParameter::RO, rp_HPGetFastADCIsLV_HVOrDefault(), 0);

CInt32BinarySignal signal_mode("signal_mode", 1, 0.0f);
CFloatBinarySignal s_xaxis("ch_xaxis", CH_SIGNAL_DATA, 0.0f);
CFloatBinarySignal s_xaxis_full("ch_xaxis_full", CH_SIGNAL_DATA, 0.0f);

// CFloatSignal s_waterfall[ADC_CHANNELS]  = INIT("ch","_waterfall",CH_SIGNAL_DATA, 0.0f);
CFloatBinarySignal s_view[MAX_ADC_CHANNELS] = INIT("ch", "_view", CH_SIGNAL_DATA, 0.0f);
CFloatBinarySignal s_view_min[MAX_ADC_CHANNELS] = INIT("ch", "_view_min", CH_SIGNAL_DATA, 0.0f);
CFloatBinarySignal s_view_max[MAX_ADC_CHANNELS] = INIT("ch", "_view_max", CH_SIGNAL_DATA, 0.0f);

CFloatBinarySignal s_full[MAX_ADC_CHANNELS] = INIT("ch", "_full", CH_SIGNAL_DATA, 0.0f);
CFloatBinarySignal s_min_full[MAX_ADC_CHANNELS] = INIT("ch", "_min_full", CH_SIGNAL_DATA, 0.0f);
CFloatBinarySignal s_max_full[MAX_ADC_CHANNELS] = INIT("ch", "_max_full", CH_SIGNAL_DATA, 0.0f);

CIntParameter view_port_width("view_port_width", CBaseParameter::RW, 256, 0, 256, 4096);
CFloatParameter view_port_start("view_port_start", CBaseParameter::RW, 0, 0, 0, MAX_FREQ);
CFloatParameter view_port_end("view_port_end", CBaseParameter::RW, MAX_FREQ, 0, 0, MAX_FREQ);

CIntParameter freq_unit("freq_unit", CBaseParameter::RW, 2, 0, 0, 2, CONFIG_VAR);
CIntParameter y_axis_mode("y_axis_mode", CBaseParameter::RW, 0, 0, 0, 6,
                          CONFIG_VAR);  // 0 -dBm mode ; 1 - Volt mode ; 2 -dBu mode; 3 -dBV mode; 4 -dBuV mode; 5 - mW; 6 - dBW
CIntParameter adc_freq("ADC_FREQ", CBaseParameter::RO, 0, 0, 0, getADCRate());
CIntParameter rbw("RBW", CBaseParameter::RO, 0, 0, 0, MAX_FREQ);
CFloatParameter impedance("DBU_IMP_FUNC", CBaseParameter::RW, 50, 0, 0.1, 1000, CONFIG_VAR);

CFloatParameter xmin("xmin", CBaseParameter::RW, 0, 0, 0, MAX_FREQ, CONFIG_VAR);
CFloatParameter xmax("xmax", CBaseParameter::RW, MAX_FREQ, 0, 0, MAX_FREQ, CONFIG_VAR);

CIntParameter windowMode("SPEC_WINDOW_MODE", CBaseParameter::RW, rp_dsp_api::HAMMING, 0, 0, 6, CONFIG_VAR);
CIntParameter bufferSize("SPEC_BUFFER_SIZE", CBaseParameter::RW, rpApp_SpecGetADCBufferSize(), 0, 256, 16384, CONFIG_VAR);
CBooleanParameter cutDC("SPEC_CUT_DC", CBaseParameter::RW, (bool)rpApp_SpecGetRemoveDC(), 0, CONFIG_VAR);
CBooleanParameter requestFullData("requestFullData", CBaseParameter::RW, false, 0);

/* WEB GUI Buttons */
CBooleanParameter inReset("SPEC_RST", CBaseParameter::RW, false, 0);
CBooleanParameter inRun("SPEC_RUN", CBaseParameter::RW, true, 0);
CBooleanParameter inAutoscale("SPEC_AUTOSCALE", CBaseParameter::RW, false, 0);
CBooleanParameter inSingle("SPEC_SINGLE", CBaseParameter::RW, false, 0);
CIntParameter xAxisLogMode("xAxisLogMode", CBaseParameter::RW, 0, 0, 0, 1, CONFIG_VAR);

CBooleanParameter inShow[MAX_ADC_CHANNELS] = INIT("CH", "_SHOW", CBaseParameter::RW, false, 0, CONFIG_VAR);
CBooleanParameter inFreeze[MAX_ADC_CHANNELS] = INIT("CH", "_FREEZE", CBaseParameter::RW, false, 0, CONFIG_VAR);
CBooleanParameter inShowMin[MAX_ADC_CHANNELS] = INIT("CH", "_SHOW_MIN", CBaseParameter::RW, false, 0, CONFIG_VAR);
CBooleanParameter inShowMax[MAX_ADC_CHANNELS] = INIT("CH", "_SHOW_MAX", CBaseParameter::RW, false, 0, CONFIG_VAR);
CIntParameter inGain[MAX_ADC_CHANNELS] = INIT("CH", "_IN_GAIN", CBaseParameter::RW, 0, 0, 0, 1, CONFIG_VAR);
CIntParameter inProbe[MAX_ADC_CHANNELS] = INIT("CH", "_PROBE", CBaseParameter::RW, 1, 0, 1, 1000, CONFIG_VAR);
CIntParameter inFilter[MAX_ADC_CHANNELS] = INIT("CH", "_IN_FILTER", CBaseParameter::RW, 1, 0, 0, 1, CONFIG_VAR);
CIntParameter inAC_DC[MAX_ADC_CHANNELS] = INIT("CH", "_IN_AC_DC", CBaseParameter::RW, 0, 0, 0, 1, CONFIG_VAR);
CFloatParameter peak_freq[MAX_ADC_CHANNELS] = INIT("peak", "_freq", CBaseParameter::RW, -1, 0, -1, 1e10f);
CFloatParameter peak_power[MAX_ADC_CHANNELS] = INIT("peak", "_power", CBaseParameter::RW, 0, 0, -10000000, +10000000);

/* --------------------------------  CURSORS  ------------------------------ */
CBooleanParameter cursorx[CURSORS_COUNT] = INIT2("SPEC_CURSOR_X", "", CBaseParameter::RW, false, 0, CONFIG_VAR);
CBooleanParameter cursory[CURSORS_COUNT] = INIT2("SPEC_CURSOR_Y", "", CBaseParameter::RW, false, 0, CONFIG_VAR);

CFloatParameter cursorV[CURSORS_COUNT] = INIT2("SPEC_CUR", "_V", CBaseParameter::RW, 0.25, 0, 0, 1, CONFIG_VAR);
CFloatParameter cursorT[CURSORS_COUNT] = INIT2("SPEC_CUR", "_T", CBaseParameter::RW, 0.25, 0, 0, 1, CONFIG_VAR);

CBooleanParameter pllControlEnable("EXT_CLOCK_ENABLE", CBaseParameter::RW, 0, 0, CONFIG_VAR);
CIntParameter pllControlLocked("EXT_CLOCK_LOCKED", CBaseParameter::RW, 0, 0, 0, 1);

/////////////////////////////

CIntParameter controlSettings("CONTROL_CONFIG_SETTINGS", CBaseParameter::RW, 0, 0, 0, 10);
CStringParameter fileSettings("FILE_SATTINGS", CBaseParameter::RW, "", 0);
CStringParameter listFileSettings("LIST_FILE_SATTINGS", CBaseParameter::RW, "", 0);

const std::vector<std::string> g_savedParams = {"CH1_IN_GAIN",
                                                "CH2_IN_GAIN",
                                                "CH3_IN_GAIN",
                                                "CH4_IN_GAIN",
                                                "CH1_IN_FILTER",
                                                "CH2_IN_FILTER",
                                                "CH3_IN_FILTER",
                                                "CH4_IN_FILTER",
                                                "CH1_IN_AC_DC",
                                                "CH2_IN_AC_DC",
                                                "CH3_IN_AC_DC",
                                                "CH4_IN_AC_DC"};

static float* data[MAX_ADC_CHANNELS + 1];
static float data_freeze[MAX_ADC_CHANNELS][CH_SIGNAL_DATA];
static float data_min[MAX_ADC_CHANNELS][CH_SIGNAL_DATA];
static float data_max[MAX_ADC_CHANNELS][CH_SIGNAL_DATA];
static int g_old_signalSize = 0;

rp_websocket::CWEBServer data_server;

std::mutex g_data_mutex;
std::vector<int> g_indexArray;

void updateParametersByConfig();
void resetAllMinMax();

void UpdateParams(void) {
    inRun.Update();

    setSweepRun(inRun.Value());

    auto adc = rpApp_SpecGetADCFreq();
    auto buf = rpApp_SpecGetADCBufferSize();
    if (adc_freq.Value() != adc)
        adc_freq.SendValue(adc);

    if (adc / buf != rbw.Value())
        rbw.SendValue(adc / buf);

    if (inRun.Value()) {
        for (auto i = 0u; i < g_adc_count; i++) {
            if (inShow[i].Value()) {
                float value = 0;
                rpApp_SpecGetPeakFreq((rp_channel_t)i, &value);
                if (peak_freq[i].Value() != value) {
                    peak_freq[i].SendValue(value);
                }
                rpApp_SpecGetPeakPower((rp_channel_t)i, &value);
                if (peak_power[i].Value() != value) {
                    peak_power[i].SendValue(value);
                }
            } else {
                if (peak_freq[i].Value() != -1) {
                    peak_freq[i].SendValue(-1);
                }
                if (peak_power[i].Value() != -200) {
                    peak_power[i].SendValue(-200);
                }
            }
        }
    }

    updateGen();
}

void resetMinMax(int ch, int mode) {
    std::lock_guard lock(g_data_mutex);
    auto size = CH_SIGNAL_DATA;
    for (int i = 0; i < size; ++i) {
        if (mode == 0)
            data_min[ch][i] = std::numeric_limits<float>::max();
        else
            data_max[ch][i] = std::numeric_limits<float>::lowest();
    }
}

void updateMin(int ch, float* data, int size) {
    if (size > CH_SIGNAL_DATA)
        size = CH_SIGNAL_DATA;
    for (int i = 0; i < size; ++i) {
        if (data_min[ch][i] > data[i]) {
            data_min[ch][i] = data[i];
        }
    }
}

void updateMax(int ch, float* data, int size) {
    if (size > CH_SIGNAL_DATA)
        size = CH_SIGNAL_DATA;
    for (int i = 0; i < size; ++i) {
        if (data_max[ch][i] < data[i]) {
            data_max[ch][i] = data[i];
        }
    }
}

void copyData(CFloatBinarySignal& dest, float* src, int size) {
    if (dest.GetSize() != size)
        dest.Resize(size);
    memcpy_neon(&dest[0], src, size * sizeof(float));
}

void UpdateBinarySignals(void) {
    inRun.Update();
    if (inRun.Value() == false) {
        return;
    }
    size_t signal_size = 0;
    rpApp_SpecGetViewSize(&signal_size);
    bool isShow = true;
    for (auto i = 0u; i < g_adc_count; i++) {
        isShow |= inShow[i].Value();
    }

    int ret = rpApp_SpecGetViewData(data, signal_size);
    if (ret != 0) {
        return;
    }

    auto mode = rp_dsp_api::mode_t::DBM;
    rpApp_SpecGetMode(&mode);
    signal_mode[0] = mode;

    for (auto i = 0u; i < g_adc_count; i++) {
        if (inFreeze[i].Value() && g_old_signalSize == signal_size) {
            memcpy_neon(data[i + 1], data_freeze[i], signal_size * sizeof(float));
        } else {
            memcpy_neon(data_freeze[i], data[i + 1], signal_size * sizeof(float));
        }
    }

    if (g_old_signalSize != signal_size || inReset.Value()) {
        resetAllMinMax();
    }

    std::lock_guard lock(g_data_mutex);

    int width = view_port_width.Value();
    if (width > signal_size)
        width = signal_size;

    // Resize X axis
    int i_start = 0;
    int i_stop = signal_size;
    for (int i = 0; i < signal_size; ++i) {
        if (view_port_start.Value() < data[0][i]) {
            break;
        }
        i_start = i;
    }
    if (i_start > 0)
        i_start--;

    for (int i = signal_size - 1; i >= 0; --i) {
        if (view_port_end.Value() > data[0][i]) {
            break;
        }
        i_stop = i;
    }
    if (i_stop < signal_size - 1)
        i_stop++;

    int i_start_w = 0;
    int i_stop_w = signal_size;
    for (int i = 0; i < signal_size; ++i) {
        if (xmin.Value() < data[0][i]) {
            break;
        }
        i_start_w = i;
    }
    for (int i = signal_size - 1; i >= 0; --i) {
        if (xmax.Value() > data[0][i]) {
            break;
        }
        i_stop_w = i;
    }

    prepareIndexArray(&g_indexArray, i_start, i_stop, width, xAxisLogMode.Value());

    decimateData(s_xaxis, data[0], i_start, i_stop, width, xAxisLogMode.Value(), g_indexArray.data());
    // End resize

    if (requestFullData.Value()) {
        requestFullData.Value() = false;
        if (isShow) {
            copyData(s_xaxis_full, data[0], signal_size);
            for (auto ch = 0u; ch < g_adc_count; ch++) {
                if (inShow[ch].Value()) {
                    if (inFreeze[ch].Value())
                        copyData(s_full[ch], data_freeze[ch], signal_size);
                    else
                        copyData(s_full[ch], data[ch + 1], signal_size);

                    if (inShowMin[ch].Value()) {
                        copyData(s_min_full[ch], data_min[ch], signal_size);
                    } else {
                        s_min_full[ch].Resize(0);
                    }
                    if (inShowMax[ch].Value()) {
                        copyData(s_max_full[ch], data_max[ch], signal_size);
                    } else {
                        s_max_full[ch].Resize(0);
                    }
                } else {
                    s_full[ch].Resize(0);
                    s_min_full[ch].Resize(0);
                    s_max_full[ch].Resize(0);
                }
            }
        } else {
            s_xaxis_full.Resize(0);
            for (auto ch = 0u; ch < g_adc_count; ch++) {
                s_full[ch].Resize(0);
                s_min_full[ch].Resize(0);
                s_max_full[ch].Resize(0);
            }
        }
    } else {
        s_xaxis_full.Resize(0);
        for (auto ch = 0u; ch < g_adc_count; ch++) {
            s_full[ch].Resize(0);
            s_min_full[ch].Resize(0);
            s_max_full[ch].Resize(0);
        }
    }

    for (auto ch = 0u; ch < g_adc_count; ch++) {
        updateMin(ch, data[ch + 1], signal_size);
        updateMax(ch, data[ch + 1], signal_size);

        if (inShow[ch].Value()) {
            decimateData(s_view[ch], data[ch + 1], i_start, i_stop, width, xAxisLogMode.Value(), g_indexArray.data());
            if (inShowMin[ch].Value()) {
                decimateData(s_view_min[ch], data_min[ch], i_start, i_stop, width, xAxisLogMode.Value(), g_indexArray.data());
            } else {
                s_view_min[ch].Resize(0);
            }
            if (inShowMax[ch].Value()) {
                decimateData(s_view_max[ch], data_max[ch], i_start, i_stop, width, xAxisLogMode.Value(), g_indexArray.data());
            } else {
                s_view_max[ch].Resize(0);
            }
        } else {
            s_view[ch].Resize(0);
            s_view_min[ch].Resize(0);
            s_view_max[ch].Resize(0);
        }
    }

    inReset.Value() = false;
    g_old_signalSize = signal_size;
}

void resetAllMinMax() {
    for (auto ch = 0u; ch < g_adc_count; ch++) {
        if (inShowMin[ch].Value())
            resetMinMax(ch, 0);
        if (inShowMax[ch].Value())
            resetMinMax(ch, 1);
    }
}

void OnNewParams(void) {

    if (controlSettings.IsNewValue()) {
        if (controlSettings.NewValue() == controlSettings::REQUEST_RESET) {
            deleteConfig();
            configSetWithList(g_savedParams);
            controlSettings.Update();
            controlSettings.SendValue(controlSettings::RESET_DONE);
            return;
        }

        if (controlSettings.NewValue() == controlSettings::SAVE) {
            controlSettings.Update();
            fileSettings.Update();
            configSet();
            saveCurrentSettingToStore(fileSettings.Value());
            controlSettings.SendValue(controlSettings::NONE);
            listFileSettings.SendValue(getListOfSettingsInStore());
        }

        if (controlSettings.NewValue() == controlSettings::LOAD) {
            controlSettings.Update();
            fileSettings.Update();
            loadSettingsFromStore(fileSettings.Value());
            configGet();
            controlSettings.SendValue(controlSettings::LOAD_DONE);
        }

        if (controlSettings.NewValue() == controlSettings::DELETE) {
            controlSettings.Update();
            fileSettings.Update();
            deleteStoredConfig(fileSettings.Value());
            controlSettings.SendValue(controlSettings::NONE);
            listFileSettings.SendValue(getListOfSettingsInStore());
        }
    }

    bool config_changed = isChanged();

    if (rp_HPIsFastDAC_PresentOrDefault())
        UpdateGeneratorParameters(false);

    for (auto ch = 0u; ch < g_adc_count; ch++) {
        if (inShow[ch].IsNewValue()) {
            inShow[ch].Update();
            inShow[ch].SendValue(inShow[ch].Value());
            RESEND(inShow[ch])
        }
    }

    if (inReset.IsNewValue()) {
        inReset.Update();
    }

    if (xmin.IsNewValue() || xmax.IsNewValue() || freq_unit.IsNewValue()) {
        freq_unit.Update();
        xmin.Update();
        xmax.Update();

        if (xmax.Value() <= xmin.Value()) {
            xmin.Value() = xmax.Value() * 0.95;
        }
        rpApp_SpecSetFreqRange(xmin.Value(), xmax.Value());
        resetAllMinMax();
    }

    for (auto ch = 0u; ch < g_adc_count; ch++) {
        if (inShowMin[ch].IsNewValue()) {
            inShowMin[ch].Update();
            if (inShowMin[ch].Value()) {
                if (!inFreeze[ch].Value())
                    resetMinMax(ch, 0);
            }
            RESEND(inShowMin[ch])
        }

        if (inShowMax[ch].IsNewValue()) {
            inShowMax[ch].Update();
            if (inShowMax[ch].Value()) {
                if (!inFreeze[ch].Value())
                    resetMinMax(ch, 1);
            }
            RESEND(inShowMax[ch])
        }

        if (inFreeze[ch].IsNewValue()) {
            inFreeze[ch].Update();
            RESEND(inFreeze[ch])
        }
    }

    for (auto i = 0u; i < CURSORS_COUNT; i++) {
        if (cursory[i].IsNewValue()) {
            cursory[i].Update();
            cursory[i].SendValue(cursory[i].Value());
            cursorV[i].SendValue(cursorV[i].Value());
        }
        if (cursorV[i].IsNewValue()) {
            cursorV[i].Update();
            cursorV[i].SendValue(cursorV[i].Value());
        }

        if (cursorx[i].IsNewValue()) {
            cursorx[i].Update();
            cursorx[i].SendValue(cursorx[i].Value());
            cursorT[i].SendValue(cursorT[i].Value());
        }

        if (cursorT[i].IsNewValue()) {
            cursorT[i].Update();
            cursorT[i].SendValue(cursorT[i].Value());
        }
    }

    if (requestFullData.IsNewValue()) {
        requestFullData.Update();
    }

    if (xAxisLogMode.IsNewValue()) {
        xAxisLogMode.Update();
        xAxisLogMode.SendValue(xAxisLogMode.Value());
    }

    if (view_port_width.IsNewValue()) {
        view_port_width.Update();
    }

    if (view_port_start.IsNewValue()) {
        view_port_start.Update();
    }

    if (view_port_end.IsNewValue()) {
        view_port_end.Update();
    }

    if (windowMode.IsNewValue()) {
        if (rpApp_SpecSetWindow((rp_dsp_api::window_mode_t)windowMode.NewValue()) == RP_OK) {
            windowMode.Update();
            resetAllMinMax();
        }
    }

    if (impedance.IsNewValue()) {
        if (rpApp_SpecSetImpedance(impedance.NewValue()) == RP_OK)
            impedance.Update();
    }

    if (y_axis_mode.IsNewValue()) {
        if (rpApp_SpecSetMode((rp_dsp_api::mode_t)y_axis_mode.NewValue()) == RP_OK) {
            y_axis_mode.Update();
            resetAllMinMax();
        }
    }

    if (bufferSize.IsNewValue()) {
        if (rpApp_SpecSetADCBufferSize(bufferSize.NewValue()) == RP_OK) {
            bufferSize.Update();
            resetAllMinMax();
        }
    }

    if (cutDC.IsNewValue()) {
        if (rpApp_SpecSetRemoveDC(cutDC.NewValue()) == RP_OK) {
            cutDC.Update();
            RESEND(cutDC)
        }
    }

    if (rp_HPGetFastADCIsAC_DCOrDefault()) {
        for (auto ch = 0u; ch < g_adc_count; ch++) {
            if (inAC_DC[ch].IsNewValue()) {
                if (rp_AcqSetAC_DC((rp_channel_t)ch, inAC_DC[ch].NewValue() == 0 ? RP_AC : RP_DC) == RP_OK) {
                    inAC_DC[ch].Update();
                    RESEND(inAC_DC[ch])
                }
            }
        }
    }

    if (rp_HPGetFastADCIsFilterPresentOrDefault()) {
        for (auto ch = 0u; ch < g_adc_count; ch++) {
            if (inFilter[ch].IsNewValue()) {
                if (rp_AcqSetBypassFilter((rp_channel_t)ch, inFilter[ch].NewValue() == 0) == RP_OK) {
                    inFilter[ch].Update();
                    RESEND(inFilter[ch])
                }
            }
        }
    }

    if (rp_HPGetIsPLLControlEnableOrDefault()) {
        if (pllControlEnable.IsNewValue()) {
            if (rp_SetPllControlEnable(pllControlEnable.NewValue()) == RP_OK) {
                pllControlEnable.Update();
                RESEND(pllControlEnable)
            }
        }

        bool pll_control_locked = false;
        if (rp_GetPllControlLocked(&pll_control_locked) == RP_OK) {
            if (pllControlLocked.Value() != pll_control_locked)
                pllControlLocked.SendValue(pll_control_locked);
        }
    }

    if (rp_HPGetFastADCIsLV_HVOrDefault()) {
        for (auto ch = 0u; ch < g_adc_count; ch++) {
            if (inGain[ch].IsNewValue()) {
                if (rp_AcqSetGain((rp_channel_t)ch, inGain[ch].NewValue() == 0 ? RP_LOW : RP_HIGH) == 0) {
                    inGain[ch].Update();
                    rpApp_SpecReset();
                    RESEND(inGain[ch])
                }
            }
        }
    }

    for (auto ch = 0u; ch < g_adc_count; ch++) {
        if (inProbe[ch].IsNewValue()) {
            if (rpApp_SpecSetProbe((rp_channel_t)ch, inProbe[ch].NewValue()) == RP_OK) {
                inProbe[ch].Update();
                rpApp_SpecReset();
                RESEND(inProbe[ch])
            }
        }
    }
    // Save the configuration file
    if (config_changed) {
        configSet();
    }
}

extern "C" int rp_app_init(void) {
    fprintf(stderr, "Loading spectrum version %s-%s.\n", VERSION_STR, REVISION_STR);

    for (auto ch = 0u; ch < MAX_ADC_CHANNELS + 1; ch++)
        data[ch] = new float[CH_SIGNAL_DATA];

    g_indexArray.reserve(CH_SIGNAL_DATA);

    setHomeSettingsPath("/.config/redpitaya/apps/spectrumpro_" + std::to_string((int)getModel()) + "/");

#ifdef ZIP_DISABLED
    CDataManager::GetInstance()->SetEnableParamsGZip(false);
    CDataManager::GetInstance()->SetEnableSignalsGZip(false);
    CDataManager::GetInstance()->SetEnableBinarySignalsGZip(false);
#endif

    CDataManager::GetInstance()->SetParamInterval(50);
    CDataManager::GetInstance()->SetSignalInterval(10);

    appGenInit();
    rp_WC_Init();

    rp_Init();
    rp_Reset();

    rpApp_SpecRun();

    for (auto ch = 0u; ch < g_adc_count; ch++) {
        if (rp_HPGetFastADCIsLV_HVOrDefault()) {
            rp_AcqSetGain((rp_channel_t)ch, RP_LOW);
        }
        if (rp_HPGetFastADCIsAC_DCOrDefault())
            rp_AcqSetAC_DC((rp_channel_t)ch, RP_AC);
        if (rp_HPGetFastDACIsTempProtectionOrDefault()) {
            rp_SetEnableTempProtection((rp_channel_t)ch, true);
        }
        if (rp_HPGetIsGainDACx5OrDefault()) {
            rp_GenSetGainOut((rp_channel_t)ch, RP_GAIN_1X);
        }
    }

    listFileSettings.Value() = getListOfSettingsInStore();
    updateParametersByConfig();

    data_server.startServerBinaray(DATA_TCP_PORT);

    return 0;
}

extern "C" int rp_app_exit(void) {
    fprintf(stderr, "Unloading spectrum version %s-%s.\n", VERSION_STR, REVISION_STR);
    rpApp_SpecStop();
    if (rp_HPGetIsPLLControlEnableOrDefault())
        rp_SetPllControlEnable(false);
    appGenExit();
    return 0;
}

extern "C" const char* rp_app_desc(void) {
    return (const char*)"Red Pitaya spectrometer application.\n";
}

void PostUpdateBinarySignals(void) {}

void updateParametersByConfig() {
    configGet();

    if (rp_HPGetFastADCIsAC_DCOrDefault()) {
        for (auto ch = 0u; ch < g_adc_count; ch++) {
            rp_AcqSetAC_DC((rp_channel_t)ch, inAC_DC[ch].Value() == 0 ? RP_AC : RP_DC);
        }
    }

    if (rp_HPGetFastADCIsFilterPresentOrDefault()) {
        for (auto ch = 0u; ch < g_adc_count; ch++) {
            rp_AcqSetBypassFilter((rp_channel_t)ch, inFilter[ch].Value() == 0);
        }
    }

    if (rp_HPGetFastADCIsLV_HVOrDefault()) {
        for (auto ch = 0u; ch < g_adc_count; ch++) {
            rp_AcqSetGain((rp_channel_t)ch, inGain[ch].Value() == 0 ? RP_LOW : RP_HIGH);
        }
    }

    for (auto ch = 0u; ch < g_adc_count; ch++) {
        rpApp_SpecSetProbe((rp_channel_t)ch, inProbe[ch].Value());
    }

    if (rp_HPIsFastDAC_PresentOrDefault()) {
        UpdateGeneratorParameters(true);
    }

    if (rp_HPGetIsPLLControlEnableOrDefault()) {
        rp_SetPllControlEnable(pllControlEnable.Value());
    }

    rpApp_SpecSetImpedance(impedance.Value());
    rpApp_SpecSetWindow((rp_dsp_api::window_mode_t)windowMode.Value());
    rpApp_SpecSetRemoveDC(cutDC.Value());
    rpApp_SpecSetADCBufferSize(bufferSize.Value());

    CDataManager::GetInstance()->SendAllParams();
}
