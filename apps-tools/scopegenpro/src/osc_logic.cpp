#include <DataManager.h>
#include <CustomParameters.h>
#include <math.h>
#include <unistd.h>
#include <mutex>

#include "osc_logic.h"
#include "rp_hw-profiles.h"
#include "common.h"
#include "main.h"

#include "math_logic.h"
#include "common/rp_formatter.h"

#define MAX_FREQ getMaxFreqRate()
#define MAX_TRIGGER_LEVEL getMaxTriggerLevel()

enum request_mode{
    NONE = 0x0,
    WAV  = 0x1,
    TDMS = 0x2,
    CSV  = 0x3
};

static const uint8_t g_adc_channels = getADCChannels();
bool g_need_update_sig_gen = false;
std::mutex g_need_update_sig_gen_mtx;

CFloatParameter     inTimeOffset("OSC_TIME_OFFSET", CBaseParameter::RW, 0, 0, -100000, 100000,CONFIG_VAR);
CDoubleParameter    inTimeScale("OSC_TIME_SCALE", CBaseParameter::RW, 1, 0,0,100000000,CONFIG_VAR);
CIntParameter       inViewStartPos("OSC_VIEW_START_POS", CBaseParameter::RO, 0, 0, 0, 16384,CONFIG_VAR);
CIntParameter       inViewEndPos("OSC_VIEW_END_POS", CBaseParameter::RO, 0, 0, 0, 16384,CONFIG_VAR);
CIntParameter       adc_count("ADC_COUNT", CBaseParameter::RO, getADCChannels(), 0, 0, 4,0);
CIntParameter       adc_rate("ADC_RATE", CBaseParameter::RO, getADCRate(), 0, getADCRate(), getADCRate(),0);
CIntParameter       osc_per_sec("OSC_PER_SEC", CBaseParameter::RW, 0 , 0, 0, 1000000,0);


/***************************************************************************************
*                                     OSCILLOSCOPE                                     *
****************************************************************************************/

/* --------------------------------  OUT SIGNALS  ------------------------------ */

CFloatBase64Signal  outCh[MAX_ADC_CHANNELS]   = INIT("ch","", 0, 0.0f);

/* ------------------------------- DATA PARAMETERS ------------------------------ */
// CIntParameter       dataSize("OSC_DATA_SIZE", CBaseParameter::RW, CH_SIGNAL_SIZE_DEFAULT, 0, 1, 16*1024);
CFloatParameter     viewPortion("OSC_VIEV_PART", CBaseParameter::RO, 0.1, 0, 0, 1);

// This is decimation not sample rate
CIntParameter       samplingRate("OSC_SAMPL_RATE", CBaseParameter::RW, RP_DEC_1, 0, RP_DEC_1, RP_DEC_65536);

/* --------------------------------  OUT PARAMETERS  ------------------------------ */
CBooleanParameter   inShow[MAX_ADC_CHANNELS] = INIT("CH","_SHOW", CBaseParameter::RW, false, 0,CONFIG_VAR);
CStringParameter    inName[MAX_ADC_CHANNELS] = INIT("IN","_CHANNEL_NAME_INPUT", CBaseParameter::RW, "", 0,CONFIG_VAR);


CBooleanParameter   inReset("OSC_RST", CBaseParameter::RW, false, 0);
CBooleanParameter   inRun("OSC_RUN", CBaseParameter::RW, false, 0);
CBooleanParameter   inAutoscale("OSC_AUTOSCALE", CBaseParameter::RW, false, 0);
CBooleanParameter   inSingle("OSC_SINGLE", CBaseParameter::RW, false, 0);

// 2500 - 5 step by 1000V max
CFloatParameter     inOffset[MAX_ADC_CHANNELS] = INIT("GPOS_OFFSET_CH","", CBaseParameter::RW, 0, 0, -250000, 250000,CONFIG_VAR);
CFloatParameter     inScale[MAX_ADC_CHANNELS] = INIT("GPOS_SCALE_CH","", CBaseParameter::RW, 1, 0, 0.00005, 50000,CONFIG_VAR);
CFloatParameter     inOffsetZero[MAX_ADC_CHANNELS] = INIT("GPOS_OFFSET_ZERO_CH","", CBaseParameter::RW, 0, 0, -250000, 250000,CONFIG_VAR);
CBooleanParameter   inInvShow[MAX_ADC_CHANNELS] = INIT("GPOS_INVERTED_CH","", CBaseParameter::RW, false, 0,CONFIG_VAR);

CFloatParameter     inProbe[MAX_ADC_CHANNELS] = INIT("OSC_CH","_PROBE", CBaseParameter::RW, 1, 0, 0, 1000,CONFIG_VAR);


CIntParameter       inGain[MAX_ADC_CHANNELS] = INIT("OSC_CH","_IN_GAIN", CBaseParameter::RW, RP_LOW, 0, 0, 1,CONFIG_VAR);
CIntParameter       inAC_DC[MAX_ADC_CHANNELS] = INIT("OSC_CH","_IN_AC_DC", CBaseParameter::RW, RP_DC, 0, 0, 1,CONFIG_VAR);

CIntParameter       inSmoothMode[MAX_ADC_CHANNELS] = INIT("OSC_CH","_SMOOTH", CBaseParameter::RW, RP_DC, 0, 0, 3,CONFIG_VAR);


/* --------------------------------  TRIGGER PARAMETERS --------------------------- */
CFloatParameter     inTriggLevel("OSC_TRIG_LEVEL", CBaseParameter::RW, 0, 0, -20, 20,CONFIG_VAR);
CFloatParameter     inTriggLimit("OSC_TRIG_LIMIT", CBaseParameter::RO, 0, 0, -20, 20,CONFIG_VAR);
CIntParameter       inTrigSweep("OSC_TRIG_SWEEP", CBaseParameter::RW, RPAPP_OSC_TRIG_AUTO, 0, RPAPP_OSC_TRIG_AUTO, RPAPP_OSC_TRIG_SINGLE,CONFIG_VAR);
CIntParameter       inTrigSource("OSC_TRIG_SOURCE", CBaseParameter::RW, RPAPP_OSC_TRIG_SRC_CH1, 0, RPAPP_OSC_TRIG_SRC_CH1, RPAPP_OSC_TRIG_SRC_EXTERNAL,CONFIG_VAR);
CIntParameter       inTrigSlope("OSC_TRIG_SLOPE", CBaseParameter::RW, RPAPP_OSC_TRIG_SLOPE_PE, 0, RPAPP_OSC_TRIG_SLOPE_NE, RPAPP_OSC_TRIG_SLOPE_PE,CONFIG_VAR);
CFloatParameter     inTrigHyst("OSC_TRIG_HYST", CBaseParameter::RW, 0.005, 0, 0, 1.0,CONFIG_VAR);
CBooleanParameter   inTrigLevel("OSC_TRIG_LIMIT_IS_PRESENT", CBaseParameter::RO, rp_HPGetIsExternalTriggerLevelPresentOrDefault(), 0);

CIntParameter triggerInfo("OSC_TRIG_INFO", CBaseParameter::RW, 0, 0, 0, 3);

CIntParameter pllControlEnable ("EXT_CLOCK_ENABLE", CBaseParameter::RW, 0, 0, 0, 1,rp_HPGetIsPLLControlEnableOrDefault() ? CONFIG_VAR : 0);
CIntParameter pllControlLocked ("EXT_CLOCK_LOCKED", CBaseParameter::RO, 0, 0, 0, 1);

CStringParameter 	download_file("DOWNLOAD_FILE", CBaseParameter::RW, "", 0);
CBooleanParameter   request_data("REQUEST_DATA", CBaseParameter::RW,false,0);
CIntParameter       request_format ("REQUEST_FORMAT", CBaseParameter::RW, 1, 0, 1, 3,CONFIG_VAR);
CBooleanParameter   request_normalize("REQUEST_NORMALIZE", CBaseParameter::RW,true,0,CONFIG_VAR);
CBooleanParameter   request_view("REQUEST_VIEW", CBaseParameter::RW,true,0,CONFIG_VAR);

CFloatParameter ext_trigger_level ("OSC_EXT_TRIG_LEVEL", CBaseParameter::RW, 0, 0, 0, 0,CONFIG_VAR);

CIntParameter       bufferRequest("OSC_BUFFER_REQUEST", CBaseParameter::RW, 0, 0, -1, 1);
CIntParameter       bufferSelected("OSC_BUFFER_CURRENT", CBaseParameter::RW, 0, 0, (MAX_BUFFERS-1) * -1, 0);


auto initExtTriggerLimits() -> void{
    if (rp_HPGetIsExternalTriggerLevelPresentOrDefault()){
        auto level = rp_HPGetIsExternalTriggerFullScalePresentOrDefault();
        auto is_sign = rp_HPGetIsExternalTriggerIsSignedOrDefault();
        ext_trigger_level.SetMin(is_sign ? -level : 0);
        ext_trigger_level.SetMax(level);
    }
}

auto initOscAfterLoad() -> void{
    if (rp_HPGetFastADCIsAC_DCOrDefault()){
        for(auto ch = 0u; ch < g_adc_channels ; ch++){
            rp_AcqSetAC_DC((rp_channel_t)ch,inAC_DC[ch].Value() == 0 ? RP_AC:RP_DC);
        }
    }

    if (rp_HPGetIsPLLControlEnableOrDefault()){
        rp_SetPllControlEnable(pllControlEnable.Value());
    }

    if (rp_HPGetFastADCIsLV_HVOrDefault()){
        for(auto ch = 0u; ch < g_adc_channels ; ch++){
            rp_AcqSetGain((rp_channel_t)ch, inGain[ch].Value() == 0 ? RP_LOW : RP_HIGH);
        }
    }
}

auto initOscBeforeLoadConfig() -> void{
    for(int i = 0; i < g_adc_channels; i++){
        auto ch = (rp_channel_t)i;
        inName[ch].Value() = std::string("IN") + std::to_string(i+1);
    }
}

auto updateTriggerLimit(bool force) -> void {
    // Checking trigger limitation
    float trigg_limit = 0;
    bool  is_signed = true;
	auto t_channel = (rpApp_osc_trig_source_t) inTrigSource.Value();
    switch(t_channel){
        case RPAPP_OSC_TRIG_SRC_CH1:
            rp_AcqGetGainV(RP_CH_1, &trigg_limit);
            trigg_limit = trigg_limit * inProbe[RP_CH_1].Value();
        break;
        case RPAPP_OSC_TRIG_SRC_CH2:
            rp_AcqGetGainV(RP_CH_2, &trigg_limit);
            trigg_limit = trigg_limit * inProbe[RP_CH_2].Value();
        break;
        case RPAPP_OSC_TRIG_SRC_CH3:
            if (g_adc_channels >= 3){
                rp_AcqGetGainV(RP_CH_3, &trigg_limit);
                trigg_limit = trigg_limit * inProbe[RP_CH_3].Value();
            }
        break;
        case RPAPP_OSC_TRIG_SRC_CH4:
            if (g_adc_channels >= 4){
                rp_AcqGetGainV(RP_CH_4, &trigg_limit);
                trigg_limit = trigg_limit * inProbe[RP_CH_4].Value();
            }
        break;
        case RPAPP_OSC_TRIG_SRC_EXTERNAL:
            trigg_limit = rp_HPGetIsExternalTriggerFullScalePresentOrDefault();
            is_signed = rp_HPGetIsExternalTriggerIsSignedOrDefault();
        break;
        default:
            ERROR("Unknown trigger source: %d",t_channel);
            rp_AcqGetGainV(RP_CH_1, &trigg_limit);
            trigg_limit = trigg_limit;
    }

    if (trigg_limit != inTriggLimit.Value() || force){
        inTriggLimit.SetMin(is_signed ? -trigg_limit : 0);
        inTriggLimit.SetMax(trigg_limit);
        inTriggLimit.SendValue(trigg_limit);
        // Need update trigger value
        float trigg_level;
        auto trig_invert = false;
        if (t_channel != RPAPP_OSC_TRIG_SRC_EXTERNAL){
            trig_invert = inInvShow[t_channel].Value();
        }
        rpApp_OscGetTriggerLevel(&trigg_level);
        inTriggLevel.SetMin(is_signed ? -trigg_limit : 0);
        inTriggLevel.SetMax(trigg_limit);
    	inTriggLevel.SendValue(trig_invert ? -trigg_level : trigg_level);
    }
}

auto updateOscParametersToWEB() -> void{
    bool running;
    rpApp_OscIsRunning(&running);
    if (inRun.Value() != running)
        inRun.SendValue(running);

    if (rp_HPGetIsPLLControlEnableOrDefault()){
        bool pll_control_enable = false;
        if (rp_GetPllControlEnable(&pll_control_enable) == RP_OK){
            if (pllControlEnable.Value() != pll_control_enable){
                pllControlEnable.SendValue(pll_control_enable);
            }
        }
        bool pll_control_locked = false;
        if (rp_GetPllControlLocked(&pll_control_locked) == RP_OK){
            if (pllControlLocked.Value() != pll_control_locked){
                pllControlLocked.SendValue(pll_control_locked);
            }
        }
    }

    // Update current trigger state on WEB
    rpApp_osc_trig_sweep_t mode;
	rpApp_OscGetTriggerSweep(&mode);
    uint8_t trig_state = 0;
	if (!running)
		trig_state = 0;
	else if (mode == RPAPP_OSC_TRIG_AUTO)
		trig_state = 1;
	else if (rpApp_OscIsTriggered() && mode != RPAPP_OSC_TRIG_AUTO)
		trig_state = 2;
	else if (!rpApp_OscIsTriggered() && mode != RPAPP_OSC_TRIG_AUTO)
		trig_state = 3;
    if (triggerInfo.Value() != trig_state){
        triggerInfo.SendValue(trig_state);
    }


    // Update view part ratio for trigger indicator

    float portion;
    rpApp_OscGetViewPart(&portion);
    if (viewPortion.Value() != portion){
        viewPortion.SendValue(portion);
    }

    updateTriggerLimit(false);


    // Send current decimation
    rp_acq_decimation_t sampling_rate;
    rp_AcqGetDecimation(&sampling_rate);
    if (samplingRate.Value() != sampling_rate){
        samplingRate.SendValue(sampling_rate);
    }

    float value;
    rpApp_OscGetTimeOffset(&value);
    if (inTimeOffset.Value() != value){
        inTimeOffset.SendValue(value);
        inTimeScale.Update();
        viewPortion.Update();
    }

    float ext_t_level;
    rpApp_OscGetExtTriggerLevel(&ext_t_level);
    if (ext_trigger_level.Value() != ext_t_level){
        ext_trigger_level.SendValue(ext_t_level);
    }

    double dvalue;
    for(int i = 0; i < g_adc_channels; i++){
        dvalue = 0;
        rpApp_OscGetAmplitudeScale((rpApp_osc_source)i, &dvalue);
        if (inScale[i].Value() != dvalue){
            inScale[i].SendValue(dvalue);
        }
    }

    if (inAutoscale.NewValue()){
        bool as = false;
        rpApp_OscGetAutoScale(&as);
    	inAutoscale.SendValue(as);
    }else{
        uint32_t start, end;
		rpApp_OscGetViewLimits(&start, &end);
        if (start != inViewStartPos.Value()){
            inViewStartPos.SendValue(start);
        }

        if (end != inViewEndPos.Value()){
            inViewEndPos.SendValue(end);
        }
    }

    rpApp_OscGetTimeScale(&value);
    if (value * 1000 != inTimeScale.Value()){
        inTimeScale.SendValue(value * 1000);
    }


    float fvalue = 0;
    rp_AcqGetTriggerHyst(&fvalue);
    if (inTrigHyst.Value() != fvalue){
        inTrigHyst.SendValue(fvalue);
    }


    for(int i = 0; i < g_adc_channels; i++){
        double dvalue = 0;
	    rpApp_OscGetAmplitudeOffset((rpApp_osc_source)i, &dvalue);
        if (dvalue > inOffset[i].GetMax()){
            dvalue = inOffset[i].GetMax();
            rpApp_OscSetAmplitudeOffset((rpApp_osc_source)i, dvalue);
        }

        if (dvalue < inOffset[i].GetMin()){
            dvalue = inOffset[i].GetMin();
            rpApp_OscSetAmplitudeOffset((rpApp_osc_source)i, dvalue);
        }

        if (inOffset[i].Value() != dvalue){
            inOffset[i].SendValue(dvalue);
        }
    }

    uint32_t count;
    rpApp_OscGetOscPerSec(&count);
    if (osc_per_sec.Value() != count){
        osc_per_sec.SendValue(count);
    }

    int32_t currentBuffer;
    rpApp_OscBufferCurrent(&currentBuffer);
    if (bufferSelected.Value() != currentBuffer){
        bufferSelected.SendValue(currentBuffer);
    }
}

auto getOscRunState() -> bool{
    bool running;
    rpApp_OscIsRunning(&running);
    return running;
}

auto isAutoScale() -> bool{
    return inAutoscale.NewValue();
}

auto getNeedUpdateSigGen() -> bool{
    std::lock_guard<std::mutex> lock(g_need_update_sig_gen_mtx);
    auto ret = g_need_update_sig_gen;
    g_need_update_sig_gen = false;
    return ret;
}

auto updateOscSignal() -> void{
    for(int i = 0; i < g_adc_channels; i++){
        if (inShow[i].Value()) {
            if (outCh[i].GetSize() != CH_SIGNAL_SIZE_DEFAULT)
                outCh[i].Resize(CH_SIGNAL_SIZE_DEFAULT);
            rpApp_OscGetViewData((rpApp_osc_source)i, &outCh[i][0], (uint32_t) CH_SIGNAL_SIZE_DEFAULT);
        } else {
            outCh[i].Resize(0);
        }
    }
    rpApp_OscRefreshViewData();
}

auto requestFile() -> void {
    download_file.Update();

    if (request_normalize.IsNewValue()){
        request_normalize.Update();
    }

    if (request_view.IsNewValue()){
        request_view.Update();
    }

    if (request_format.IsNewValue()){
        request_format.Update();
        TRACE("Export format %d",request_format.Value());
    }

    if (request_data.IsNewValue()){
        request_data.Update();
        request_data.Value() = false;

        request_mode mode = (request_mode)request_format.Value();
        rp_acq_decimation_t sampling_rate;
        rp_AcqGetDecimation(&sampling_rate);
        auto rate = getADCRate() / (uint32_t)sampling_rate;
        auto f_mode = rp_formatter_api::rp_mode_t::RP_F_WAV;
        auto is_view = request_view.Value()? RPAPP_VIEW_EXPORT : RPAPP_RAW_EXPORT;
        auto is_normal = request_normalize.Value();
        auto suffix = std::string("error");
        auto file_format = mode & 0xF;
        if (file_format == WAV){
            suffix = ".wav";
            is_normal = true;
            f_mode = rp_formatter_api::rp_mode_t::RP_F_WAV;
        }

        if (file_format == CSV){
            suffix = ".csv";
            f_mode = rp_formatter_api::rp_mode_t::RP_F_CSV;
        }

        if (file_format == TDMS){
            suffix = ".tdms";
            f_mode = rp_formatter_api::rp_mode_t::RP_F_TDMS;
        }

        rp_formatter_api::CFormatter formatter(f_mode,rate);

        float *buff[g_adc_channels + 1];

        for(int i = 0; i <= g_adc_channels; i++){
            buff[i] = NULL;
        }

        uint8_t allChannels = 0;

        for(int i = 0; i < g_adc_channels; i++){
            if (inShow[i].Value()) {
                uint32_t size = is_view ? CH_SIGNAL_SIZE_DEFAULT: ADC_BUFFER_SIZE;
                buff[i] = new float[size];
                if (rpApp_OscGetExportedData((rpApp_osc_source)i,is_view,is_normal,buff[i],&size) == RP_OK){
                    formatter.setChannel((rp_formatter_api::rp_channel_t)i, buff[i],size,std::string("Channel_") + std::to_string(i+1));
                    allChannels++;
                }
            }
        }

        if (isMathShow()){
            uint32_t size = is_view ? CH_SIGNAL_SIZE_DEFAULT: ADC_BUFFER_SIZE;
            buff[g_adc_channels] = new float[size];
            if (rpApp_OscGetExportedData(RPAPP_OSC_SOUR_MATH,is_view,is_normal,buff[g_adc_channels],&size) == RP_OK){
                formatter.setChannel((rp_formatter_api::rp_channel_t)allChannels, buff[g_adc_channels],size,std::string("Math"));
                allChannels++;
            }
        }

        std::string filename = std::string("scopegen_data") + suffix;
        formatter.openFile("/tmp/scopegenpro/" + filename);
        if (formatter.isOpenFile()){
            formatter.writeToFile();
            formatter.closeFile();
            download_file.Value() = filename;
        }else{
            download_file.Value() = std::string("error");
        }

        for(int i = 0; i <= g_adc_channels; i++){
            delete[] buff[i];
        }
    }
}



auto updateOscParams(bool force) -> void{

    bool requestSendForTimeCursor = false;
    bool requestSendTriggerLevel = false;


    if (rp_HPGetIsPLLControlEnableOrDefault()){
        if(IS_NEW(pllControlEnable) || force) {
            if (rp_SetPllControlEnable(pllControlEnable.NewValue()) == RP_OK)
                pllControlEnable.Update();
        }
    }

    requestFile();

/* ------ UPDATE OSCILLOSCOPE LOCAL PARAMETERS ------*/

    for(int i = 0; i <= g_adc_channels; i++){
        if (IS_NEW(inShow[i]) || force)
            inShow[i].Update();
    }


/* ------ SEND OSCILLOSCOPE PARAMETERS TO API ------*/
    IF_VALUE_CHANGED_BOOL(inRun, rpApp_OscRun(), rpApp_OscStop())

    if (inReset.NewValue()) {
        rpApp_OscReset();
        inReset.Update();
        inReset.Value() = false;
    }

    if (inSingle.NewValue()) {
        rpApp_OscSingle();
        inSingle.Update();
        inSingle.Value() = false;
        rpApp_osc_trig_sweep_t sweep;
        rpApp_OscGetTriggerSweep(&sweep);
        inTrigSweep.Value() = sweep;
    }

    if (bufferRequest.NewValue() != 0){
        if (bufferRequest.NewValue() == 1){
            rpApp_OscBufferSelectNext();
        }
        if (bufferRequest.NewValue() == -1){
            rpApp_OscBufferSelectPrev();
        }
        bufferRequest.Value() = 0;
    }

	inAutoscale.Update();

    if (inAutoscale.Value()) {
        if (rp_HPGetIsAttenuatorControllerPresentOrDefault()){
            for(int i = 0; i < g_adc_channels; i++){
                rpApp_OscSetInputGain((rp_channel_t)i, (rpApp_osc_in_gain_t)RPAPP_OSC_IN_GAIN_HV);
                inGain[i].Update();
                inGain[i].SendValue(RPAPP_OSC_IN_GAIN_HV);
            }
            sleep(1);
        }
        rpApp_OscAutoScale();
        return;
    }



    IF_VALUE_CHANGED_FORCE(inTrigHyst, rp_AcqSetTriggerHyst(inTrigHyst.NewValue()),force)

    if (IS_NEW(inTimeScale) || force){
        if (rpApp_OscSetTimeScale(inTimeScale.NewValue() / 1000.0) == RP_OK){
            inTimeScale.Update();
            std::lock_guard lock(g_need_update_sig_gen_mtx);
            g_need_update_sig_gen = true;
        }
    }

    if (inTimeOffset.Value() != inTimeOffset.NewValue() || force){
        if (rpApp_OscSetTimeOffset(inTimeOffset.NewValue()) == RP_OK){
            inTimeOffset.Update();
            requestSendForTimeCursor = true;
        }
    }


    int trig_source_new = inTrigSource.NewValue();
    bool update_trig_level = inTrigSource.Value() != trig_source_new;
    bool trig_inversion_changed = false;
    bool trig_invert = false;

    // Checking the signal inversion
    for(int i = 0; i < g_adc_channels; i++){

        if (IS_NEW(inOffset[i]) || force){
            if (rpApp_OscSetAmplitudeOffset((rpApp_osc_source)i,  inOffset[i].NewValue()) == RP_OK){
                inOffset[i].Update();
            }
        }

        if (IS_NEW(inScale[i]) || force) {
            if (rpApp_OscSetAmplitudeScale((rpApp_osc_source)i,  inScale[i].NewValue()) == RP_OK){
                inScale[i].Update();
                if (rp_HPGetIsAttenuatorControllerPresentOrDefault()){
                    // AUTO select gain on autoscale
                    int val = inScale[i].Value() > 0.1 ? RPAPP_OSC_IN_GAIN_HV : RPAPP_OSC_IN_GAIN_LV;
                    rpApp_osc_in_gain_t cur_val;
                    if (rpApp_OscGetInputGain((rp_channel_t)i,&cur_val) == RP_OK){
                        if (val != cur_val && !force){
                            if (rpApp_OscSetInputGain((rp_channel_t)i, (rpApp_osc_in_gain_t)val) == RP_OK)
                                inGain[i].SendValue(val);
                        }
                    }
                }
            }
        }

        if (trig_source_new == i){
            trig_invert = inInvShow[i].NewValue();
            trig_inversion_changed = inInvShow[i].Value() != trig_invert;
        }
        bool needRecalcOffsetFromZero = inProbe[i].IsNewValue();
        IF_VALUE_CHANGED_FORCE(inProbe[i], rpApp_OscSetProbeAtt((rp_channel_t)i, inProbe[i].NewValue()),force)

        IF_VALUE_CHANGED_FORCE(inSmoothMode[i], rpApp_OscSetSmoothMode((rp_channel_t)i, (rpApp_osc_interpolationMode)inSmoothMode[i].NewValue()),force)

        if (rp_HPGetFastADCIsAC_DCOrDefault()){
            IF_VALUE_CHANGED_FORCE(inAC_DC[i], rp_AcqSetAC_DC((rp_channel_t)i, inAC_DC[i].NewValue() == 0 ? RP_AC:RP_DC),force)
        }

        if (rp_HPGetFastADCIsLV_HVOrDefault()){
            IF_VALUE_CHANGED_FORCE(inGain[i], rpApp_OscSetInputGain((rp_channel_t)i, (rpApp_osc_in_gain_t)inGain[i].NewValue()),force)
        }

        auto trim = [](const std::string & source) {
            std::string s(source);
            s.erase(0,s.find_first_not_of(" \n\r\t"));
            s.erase(s.find_last_not_of(" \n\r\t")+1);
            return s;
        };

        if (IS_NEW(inName[i]) || force) {
            auto nv = trim(inName[i].NewValue());
            auto needResend = nv != inName[i].NewValue();
            if (nv == ""){
                auto str = inName[i].Value();
                inName[i].Update();
                inName[i].SendValue(str);
            }else{
                inName[i].Update();
                inName[i].Value() = nv;
                if (needResend){
                    inName[i].SendValue(nv);
                }
            }
        }

        if (IS_NEW(inOffsetZero[i]) || force || needRecalcOffsetFromZero){

            float att = 1;
            if (rpApp_OscGetProbeAtt((rp_channel_t)i,&att) == RP_OK){

                float value = inOffsetZero[i].NewValue() / att;
                if (rp_AcqSetOffset((rp_channel_t)i,value) == RP_OK){
                    inOffsetZero[i].Update();
                }
            }
        }
    }

    IF_VALUE_CHANGED_FORCE(inTrigSweep, rpApp_OscSetTriggerSweep((rpApp_osc_trig_sweep_t) inTrigSweep.NewValue()),force)

    if (inTrigSource.Value()!= inTrigSource.NewValue() || force){
        int res =  rpApp_OscSetTriggerSource((rpApp_osc_trig_source_t)inTrigSource.NewValue());
        if (res == RP_OK) {
            inTrigSource.Update(); // Used in pair with inTriggLevel
            inTriggLevel.Update();
            requestSendTriggerLevel = true;

        }
    }
    // Trigger level must be settled after trigger source
    if (trig_inversion_changed || IS_NEW(inTriggLevel) || force) {
        if (rpApp_OscSetTriggerLevel(trig_invert ? -inTriggLevel.NewValue() : inTriggLevel.NewValue()) == RP_OK) {
            inTriggLevel.Update(); // Used in pair with inTrigSource
            inTrigSource.Update();
            requestSendTriggerLevel = true;
        }
    }

    if (IS_NEW(ext_trigger_level) || force) {
        if (rpApp_OscSetExtTriggerLevel(ext_trigger_level.NewValue())){
            ext_trigger_level.Update();
        }
    }

    if (trig_inversion_changed || IS_NEW(inTrigSlope) || force) {
        rpApp_osc_trig_slope_t slope = static_cast<rpApp_osc_trig_slope_t>(inTrigSlope.NewValue());

        if (trig_invert) {
            slope = (slope == RPAPP_OSC_TRIG_SLOPE_PE) ? RPAPP_OSC_TRIG_SLOPE_NE : RPAPP_OSC_TRIG_SLOPE_PE;
        }
        if (rpApp_OscSetTriggerSlope(slope) == RP_OK) {
            inTrigSlope.Update();
        }
    }

    for(int i = 0; i < g_adc_channels; i++){
        IF_VALUE_CHANGED_FORCE(inInvShow[i], rpApp_OscSetInverted((rpApp_osc_source)i, inInvShow[i].NewValue()),force)
    }

    // IF_VALUE_CHANGED_FORCE(mathOperation, rpApp_OscSetMathOperation((rpApp_osc_math_oper_t) mathOperation.NewValue()),force)


    if (update_trig_level){
        // Update the level
		float trigg_level;
		rpApp_OscGetTriggerLevel(&trigg_level);
		inTriggLevel.Value() = trig_invert ? -trigg_level : trigg_level;
		inTriggLevel.Update(); // Used in pait with inTrigSource
        inTrigSource.Update();

        // Update the slope
        rpApp_osc_trig_slope_t slope = RPAPP_OSC_TRIG_SLOPE_PE;
        rpApp_OscGetTriggerSlope(&slope);

        if (trig_invert) {
            slope = (slope == RPAPP_OSC_TRIG_SLOPE_PE) ? RPAPP_OSC_TRIG_SLOPE_NE : RPAPP_OSC_TRIG_SLOPE_PE;
        }

        inTrigSlope.Value() = slope;
        inTrigSlope.Update();
        requestSendTriggerLevel = true;
	}

    if (requestSendForTimeCursor){
        inTimeScale.SendValue(inTimeScale.Value());
        viewPortion.SendValue(viewPortion.Value());
    }

    if (requestSendTriggerLevel){
        inTrigSource.SendValue(inTrigSource.Value());
        for(int i = 0; i < g_adc_channels; i++){
            inOffset[i].SendValue(inOffset[i].Value());
            inScale[i].SendValue(inScale[i].Value());
        }
    }

}

auto getOSCTimeScale() -> float{
    float tscale = inTimeScale.Value() / 1000;
    return tscale;
}
