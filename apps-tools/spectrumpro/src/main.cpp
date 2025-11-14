#include "main.h"
#include <array>
#include "common.h"
#include "common/profiler.h"
#include "common/rp_sweep.h"
#include "generator.h"
#include "math/rp_dsp.h"
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

CInt32BinarySignal data_settings("settings", 16, 0);
CCustomBinarySignal<rp_dsp_api::cdsp_data_t> s_view[MAX_ADC_CHANNELS] = INIT("ch", "_view", 0, 0.0f);
CCustomBinarySignal<rp_dsp_api::cdsp_data_t> peak_freq("peak_freq", MAX_ADC_CHANNELS* COUNT_DSP_MODE, 0.0f);
CCustomBinarySignal<rp_dsp_api::cdsp_data_t> peak_power("peak_power", MAX_ADC_CHANNELS* COUNT_DSP_MODE, 0.0f);
CCustomBinarySignal<rp_dsp_api::cdsp_data_t> s_xaxis("ch_xaxis", 0, 0.0f);

// Parameters for the graph display window
CIntParameter xmin("xmin", CBaseParameter::RW, 0, 0, 0, MAX_FREQ, CONFIG_VAR);
CIntParameter xmax("xmax", CBaseParameter::RW, MAX_FREQ, 0, 2000, MAX_FREQ, CONFIG_VAR);

CIntParameter y_axis_mode("y_axis_mode", CBaseParameter::RW, rp_dsp_api::DBM, 0, rp_dsp_api::DBM, rp_dsp_api::DBW, CONFIG_VAR);
CIntParameter adc_freq("ADC_FREQ", CBaseParameter::RO, 0, 0, 0, getADCRate());
CIntParameter rbw("RBW", CBaseParameter::RO, 0, 0, 0, MAX_FREQ);
CFloatParameter impedance("DBU_IMP_FUNC", CBaseParameter::RW, 50, 0, 0.1, 1000, CONFIG_VAR);

CIntParameter windowMode("SPEC_WINDOW_MODE", CBaseParameter::RW, rp_dsp_api::HAMMING, 0, 0, 6, CONFIG_VAR);
CIntParameter bufferSize("SPEC_BUFFER_SIZE", CBaseParameter::RW, rpApp_SpecGetADCBufferSize(), 0, 256, 16384, CONFIG_VAR);
CBooleanParameter cutDC("SPEC_CUT_DC", CBaseParameter::RW, (bool)rpApp_SpecGetRemoveDC(), 0, CONFIG_VAR);
CBooleanParameter requestFullData("requestFullData", CBaseParameter::RW, false, 0);

/* WEB GUI Buttons */
CBooleanParameter inRun("SPEC_RUN", CBaseParameter::RW, true, 0);
CIntParameter xAxisLogMode("xAxisLogMode", CBaseParameter::RW, 0, 0, 0, 3, CONFIG_VAR);

CBooleanParameter inShow[MAX_ADC_CHANNELS] = INIT("CH", "_SHOW", CBaseParameter::RW, false, 0, CONFIG_VAR);
CBooleanParameter inShowMin[MAX_ADC_CHANNELS] = INIT("CH", "_SHOW_MIN", CBaseParameter::RW, false, 0, CONFIG_VAR);
CBooleanParameter inShowMax[MAX_ADC_CHANNELS] = INIT("CH", "_SHOW_MAX", CBaseParameter::RW, false, 0, CONFIG_VAR);
CIntParameter inGain[MAX_ADC_CHANNELS] = INIT("CH", "_IN_GAIN", CBaseParameter::RW, 0, 0, 0, 1, CONFIG_VAR);
CIntParameter inProbe[MAX_ADC_CHANNELS] = INIT("CH", "_PROBE", CBaseParameter::RW, 1, 0, 1, 1000, CONFIG_VAR);
CIntParameter inFilter[MAX_ADC_CHANNELS] = INIT("CH", "_IN_FILTER", CBaseParameter::RW, 1, 0, 0, 1, CONFIG_VAR);
CIntParameter inAC_DC[MAX_ADC_CHANNELS] = INIT("CH", "_IN_AC_DC", CBaseParameter::RW, 0, 0, 0, 1, CONFIG_VAR);

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

rp_websocket::CWEBServer data_server;

void UpdateParams(void) {
    inRun.Update();

    setSweepRun(inRun.Value());

    auto adc = rpApp_SpecGetADCFreq();
    auto buf = rpApp_SpecGetADCBufferSize();
    if (adc_freq.Value() != adc)
        adc_freq.SendValue(adc);

    if (adc / buf != rbw.Value())
        rbw.SendValue(adc / buf);

    updateGen();
}

void UpdateBinarySignals(void) {

    inRun.Update();
    if (inRun.Value() == false) {
        return;
    }

    auto mode = y_axis_mode.Value();

    const rp_dsp_api::rp_dsp_result_t* data = nullptr;
    rpApp_SpecLockData();
    rpApp_SpecGetViewData(&data);
    if (data == NULL) {
        rpApp_SpecUnlockData();
        return;
    }

    data_settings[0] = mode;
    data_settings[1] = data->m_maxFreq;
    data_settings[2] = data->m_data_size;
    if ((size_t)s_xaxis.GetSize() != data->m_data_size) {
        s_xaxis.Resize(data->m_data_size);
    }
    memcpy(s_xaxis.GetDataPtr()->data(), data->m_freq_vector.data(), s_xaxis.GetSize() * sizeof(rp_dsp_api::cdsp_data_t));
    s_xaxis.ForceSend();
    for (auto ch = 0u; ch < g_adc_count; ch++) {
        if (inShow[ch].Value()) {
            if ((size_t)s_view[ch].GetSize() != data->m_data_size) {
                s_view[ch].Resize(data->m_data_size);
            }
            memcpy(s_view[ch].GetDataPtr()->data(), data->m_result[mode][ch].data(), s_view[ch].GetSize() * sizeof(rp_dsp_api::cdsp_data_t));
            s_view[ch].ForceSend();
        } else {
            if (s_view[ch].GetSize())
                s_view[ch].Resize(0);
        }
    }

    for (auto ch = 0u; ch < g_adc_count; ch++) {
        for (uint8_t mode = MIN_DSP_MODE; mode < COUNT_DSP_MODE; mode++) {
            peak_freq[mode + ch * COUNT_DSP_MODE] = data->m_peak_freq[mode][ch];
            peak_power[mode + ch * COUNT_DSP_MODE] = data->m_peak_power[mode][ch];
        }
    }
    peak_freq.NeedSend();
    peak_power.NeedSend();

    rpApp_SpecUnlockData();
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
            rpApp_SpecSetEnable((rp_channel_t)ch, inShow[ch].Value());
            RESEND(inShow[ch])
        }
    }

    if (xmin.IsNewValue() || xmax.IsNewValue()) {
        xmin.Update();
        xmax.Update();
        if (xmax.Value() <= xmin.Value()) {
            xmin.Value() = xmax.Value() * 0.8;
        }
        xmin.SendValue(xmin.Value());
        xmax.SendValue(xmax.Value());
        rpApp_SpecSetFreqMax(xmax.Value());
    }

    for (auto ch = 0u; ch < g_adc_count; ch++) {
        if (inShowMin[ch].IsNewValue()) {
            inShowMin[ch].Update();
            RESEND(inShowMin[ch])
        }

        if (inShowMax[ch].IsNewValue()) {
            inShowMax[ch].Update();
            RESEND(inShowMax[ch])
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

    if (windowMode.IsNewValue()) {
        if (rpApp_SpecSetWindow((rp_dsp_api::window_mode_t)windowMode.NewValue()) == RP_OK) {
            windowMode.Update();
        }
    }

    if (impedance.IsNewValue()) {
        if (rpApp_SpecSetImpedance(impedance.NewValue()) == RP_OK)
            impedance.Update();
    }

    if (y_axis_mode.IsNewValue()) {
        y_axis_mode.Update();
    }

    if (bufferSize.IsNewValue()) {
        if (rpApp_SpecSetADCBufferSize(bufferSize.NewValue()) == RP_OK) {
            bufferSize.Update();
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

    setHomeSettingsPath("/.config/redpitaya/apps/spectrumpro_" + std::to_string((int)getModel()) + "/");

#ifdef ZIP_DISABLED
    CDataManager::GetInstance()->SetEnableParamsGZip(false);
    CDataManager::GetInstance()->SetEnableSignalsGZip(false);
    CDataManager::GetInstance()->SetEnableBinarySignalsGZip(false);
#endif

    CDataManager::GetInstance()->SetParamInterval(50);
    CDataManager::GetInstance()->SetSignalInterval(15);

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
        rpApp_SpecSetEnable((rp_channel_t)ch, inShow[ch].Value());
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
    rpApp_SpecSetFreqMax(xmax.Value());

    CDataManager::GetInstance()->SendAllParams();
}
