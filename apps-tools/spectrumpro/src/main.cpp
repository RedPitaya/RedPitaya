#include "main.h"
#include "settings.h"
#include "math/rp_dsp.h"
#include "rp.h"
#include "neon_asm.h"
#include "rp_math.h"
#include "common.h"
#include "common/rp_sweep.h"
#include "web/rp_client.h"

#define MAX_FREQ getMaxFreqRate()
#define LEVEL_AMPS_MAX outAmpMax()
#define LEVEL_AMPS_DEF outAmpDef()

#define CURSORS_COUNT 2

#define XSTR(s) STR(s)
#define STR(s) #s

#define INIT2(PREF,SUFF,args...) {{PREF "1" SUFF,args},{PREF "2" SUFF,args}}
#define INIT(PREF,SUFF,args...) {{PREF "1" SUFF,args},{PREF "2" SUFF,args},{PREF "3" SUFF,args},{PREF "4" SUFF,args}}
#define RESEND(X) X.SendValue(X.Value());

static uint8_t g_adc_count = getADCChannels();
static uint8_t g_dac_count = getDACChannels();

enum { CH_SIGNAL_DATA = (8*1024)};

enum controlSettings{
    NONE            =   0,
    REQUEST_RESET   =   1,
    RESET_DONE      =   2,
    REQUEST_LIST    =   3,
    SAVE            =   4,
    DELETE          =   5,
    LOAD            =   6,
    LOAD_DONE       =   7
};

CStringParameter redpitaya_model              ("RP_MODEL_STR", CBaseParameter::RO, getModelName(), 0);
CIntParameter    redpitaya_adc_count          ("ADC_COUNT", CBaseParameter::RO, getADCChannels(), 0, 0, 4);

CIntSignal   signal_mode                      ("signal_mode", 1, 0.0f);
CFloatBase64Signal s_xaxis                          ("ch_xaxis", CH_SIGNAL_DATA, 0.0f);
CFloatBase64Signal s_xaxis_full                     ("ch_xaxis_full", CH_SIGNAL_DATA, 0.0f);

// CFloatSignal s_waterfall[ADC_CHANNELS]  = INIT("ch","_waterfall",CH_SIGNAL_DATA, 0.0f);
CFloatBase64Signal s_view[MAX_ADC_CHANNELS]       = INIT("ch","_view", CH_SIGNAL_DATA, 0.0f);
CFloatBase64Signal s_view_min[MAX_ADC_CHANNELS]   = INIT("ch","_view_min", CH_SIGNAL_DATA, 0.0f);
CFloatBase64Signal s_view_max[MAX_ADC_CHANNELS]   = INIT("ch","_view_max", CH_SIGNAL_DATA, 0.0f);


CFloatBase64Signal s_full[MAX_ADC_CHANNELS]       = INIT("ch","_full", CH_SIGNAL_DATA, 0.0f);
CFloatBase64Signal s_min_full[MAX_ADC_CHANNELS]   = INIT("ch","_min_full", CH_SIGNAL_DATA, 0.0f);
CFloatBase64Signal s_max_full[MAX_ADC_CHANNELS]   = INIT("ch","_max_full", CH_SIGNAL_DATA, 0.0f);


CIntParameter   view_port_width     ("view_port_width",   CBaseParameter::RW, 256, 0, 256, 4096);
CFloatParameter view_port_start     ("view_port_start",   CBaseParameter::RW, 0, 0, 0, MAX_FREQ);
CFloatParameter view_port_end       ("view_port_end",     CBaseParameter::RW, MAX_FREQ, 0, 0, MAX_FREQ);

CIntParameter   freq_unit           ("freq_unit",    CBaseParameter::RWSA, 2, 0, 0, 2, CONFIG_VAR);
CIntParameter   y_axis_mode         ("y_axis_mode",  CBaseParameter::RW, 0, 0, 0, 6, CONFIG_VAR); // 0 -dBm mode ; 1 - Volt mode ; 2 -dBu mode; 3 -dBV mode; 4 -dBuV mode; 5 - mW; 6 - dBW
CIntParameter   adc_freq            ("ADC_FREQ",     CBaseParameter::RWSA, 0, 0, 0, getADCRate());
CIntParameter   rbw                 ("RBW",          CBaseParameter::RWSA, 0, 0, 0, MAX_FREQ);
CFloatParameter impedance           ("DBU_IMP_FUNC", CBaseParameter::RW, 50, 0, 0.1, 1000,CONFIG_VAR);

CFloatParameter xmin                ("xmin", CBaseParameter::RW, 0, 0, 0, MAX_FREQ,CONFIG_VAR);
CFloatParameter xmax                ("xmax", CBaseParameter::RW, MAX_FREQ, 0, 0, MAX_FREQ,CONFIG_VAR);

CIntParameter windowMode            ("SPEC_WINDOW_MODE", CBaseParameter::RW, rp_dsp_api::HAMMING  , 0, 0, 6,CONFIG_VAR);
CIntParameter bufferSize            ("SPEC_BUFFER_SIZE", CBaseParameter::RW, rpApp_SpecGetADCBufferSize(), 0, 256, 16384,CONFIG_VAR);
CBooleanParameter cutDC             ("SPEC_CUT_DC", CBaseParameter::RW, (bool)rpApp_SpecGetRemoveDC(), 0,CONFIG_VAR);
CBooleanParameter requestFullData   ("requestFullData", CBaseParameter::RW, false, 0);


/* WEB GUI Buttons */
CBooleanParameter inReset           ("SPEC_RST", CBaseParameter::RW, false, 0);
CBooleanParameter inRun             ("SPEC_RUN", CBaseParameter::RW, true, 0);
CBooleanParameter inAutoscale       ("SPEC_AUTOSCALE", CBaseParameter::RW, false, 0);
CBooleanParameter inSingle          ("SPEC_SINGLE", CBaseParameter::RW, false, 0);
CIntParameter     xAxisLogMode      ("xAxisLogMode", CBaseParameter::RW, 0, 0 , 0, 1,CONFIG_VAR) ;

CBooleanParameter inShow[MAX_ADC_CHANNELS]      = INIT("CH","_SHOW", CBaseParameter::RW, false, 0,CONFIG_VAR);
CBooleanParameter inFreeze[MAX_ADC_CHANNELS]    = INIT("CH","_FREEZE", CBaseParameter::RW, false, 0,CONFIG_VAR);
CBooleanParameter inShowMin[MAX_ADC_CHANNELS]   = INIT("CH","_SHOW_MIN", CBaseParameter::RW, false, 0,CONFIG_VAR);
CBooleanParameter inShowMax[MAX_ADC_CHANNELS]   = INIT("CH","_SHOW_MAX", CBaseParameter::RW, false, 0,CONFIG_VAR);
CIntParameter inGain[MAX_ADC_CHANNELS]          = INIT("CH","_IN_GAIN", CBaseParameter::RW, 0, 0, 0, 1, CONFIG_VAR);
CIntParameter inProbe[MAX_ADC_CHANNELS]         = INIT("CH","_PROBE", CBaseParameter::RW, 1, 0, 1, 1000, CONFIG_VAR);

CIntParameter inAC_DC[MAX_ADC_CHANNELS]         = INIT("CH","_IN_AC_DC", CBaseParameter::RW, 0, 0, 0, 1, CONFIG_VAR);
CFloatParameter peak_freq[MAX_ADC_CHANNELS]     = INIT("peak","_freq", CBaseParameter::ROSA, -1, 0, -1, +1e6f);
CFloatParameter peak_power[MAX_ADC_CHANNELS]    = INIT("peak","_power", CBaseParameter::ROSA, 0, 0, -10000000, +10000000);

/* --------------------------------  CURSORS  ------------------------------ */
CBooleanParameter cursorx[CURSORS_COUNT]     = INIT2("SPEC_CURSOR_X","", CBaseParameter::RW, false, 0,CONFIG_VAR);
CBooleanParameter cursory[CURSORS_COUNT]     = INIT2("SPEC_CURSOR_Y","", CBaseParameter::RW, false, 0,CONFIG_VAR);

CFloatParameter cursorV[CURSORS_COUNT]       = INIT2("SPEC_CUR","_V", CBaseParameter::RW, 0.25, 0, 0, 1,CONFIG_VAR);
CFloatParameter cursorT[CURSORS_COUNT]       = INIT2("SPEC_CUR","_T", CBaseParameter::RW, 0.25, 0, 0, 1,CONFIG_VAR);

// Generator
CBooleanParameter outState[MAX_DAC_CHANNELS]               = INIT2("OUTPUT","_STATE", CBaseParameter::RW, false, 0,CONFIG_VAR);
CFloatParameter outAmplitude[MAX_DAC_CHANNELS]             = INIT2("SOUR","_VOLT", CBaseParameter::RW, LEVEL_AMPS_DEF  , 0, 0, LEVEL_AMPS_MAX,CONFIG_VAR);
CFloatParameter outOffset[MAX_DAC_CHANNELS]                = INIT2("SOUR","_VOLT_OFFS", CBaseParameter::RW, 0, 0, -LEVEL_AMPS_MAX, LEVEL_AMPS_MAX,CONFIG_VAR);
CFloatParameter outFrequancy[MAX_DAC_CHANNELS]             = INIT2("SOUR","_FREQ_FIX", CBaseParameter::RW, 1000, 0, 1, MAX_FREQ,CONFIG_VAR);
CFloatParameter outPhase[MAX_DAC_CHANNELS]                 = INIT2("SOUR","_PHAS", CBaseParameter::RW, 0, 0, -360, 360,CONFIG_VAR);
CFloatParameter outDCYC[MAX_DAC_CHANNELS]                  = INIT2("SOUR","_DCYC", CBaseParameter::RW, 50, 0, 0, 100,CONFIG_VAR);
CFloatParameter outRiseTime[MAX_DAC_CHANNELS]              = INIT2("SOUR","_RISE", CBaseParameter::RW, 1, 0, 0.1, 1000,CONFIG_VAR);
CFloatParameter outFallTime[MAX_DAC_CHANNELS]              = INIT2("SOUR","_FALL", CBaseParameter::RW, 1, 0, 0.1, 1000,CONFIG_VAR);
CStringParameter outWaveform[MAX_DAC_CHANNELS]             = INIT2("SOUR","_FUNC", CBaseParameter::RW, "0", 0,CONFIG_VAR);

CStringParameter outARBList = CStringParameter("ARB_LIST", CBaseParameter::RW, loadARBList(), 0);

CIntParameter outGain[MAX_DAC_CHANNELS]                    = INIT2("CH","_OUT_GAIN", CBaseParameter::RW, 0, 0, 0, 1);
CIntParameter outTemperatureRuntime[MAX_DAC_CHANNELS]      = INIT2("SOUR","_TEMP_RUNTIME", CBaseParameter::RWSA, 0, 0, 0, 1);
CIntParameter outTemperatureLatched[MAX_DAC_CHANNELS]      = INIT2("SOUR","_TEMP_LATCHED", CBaseParameter::RWSA, 0, 0, 0, 1);
CIntParameter outImp[MAX_DAC_CHANNELS]                     = INIT2("SOUR","_IMPEDANCE", CBaseParameter::RW, 0, 0, 0, 1,isZModePresent() ? CONFIG_VAR : 0);

CBooleanParameter outImpExt("SOUR_IMPEDANCE_EXT", CBaseParameter::RO, isZModePresent(), 0);

CBooleanParameter pllControlEnable  ("EXT_CLOCK_ENABLE", CBaseParameter::RW, 0, 0,CONFIG_VAR);
CIntParameter pllControlLocked      ("EXT_CLOCK_LOCKED", CBaseParameter::ROSA, 0, 0, 0, 1);


/// SWEEP VARIABLES /////////
CFloatParameter outSweepStartFrequancy[2]   = INIT2("SOUR","_SWEEP_FREQ_START", CBaseParameter::RW, 1000, 0, 1, MAX_FREQ,CONFIG_VAR);
CFloatParameter outSweepEndFrequancy[2]     = INIT2("SOUR","_SWEEP_FREQ_END"  , CBaseParameter::RW, 10000, 0, 1, MAX_FREQ,CONFIG_VAR);
CIntParameter   outSweepMode[2]             = INIT2("SOUR","_SWEEP_MODE", CBaseParameter::RW, RP_GEN_SWEEP_MODE_LINEAR, 0, RP_GEN_SWEEP_MODE_LINEAR, RP_GEN_SWEEP_MODE_LOG,CONFIG_VAR);
CIntParameter   outSweepDir[2]              = INIT2("SOUR","_SWEEP_DIR", CBaseParameter::RW, RP_GEN_SWEEP_DIR_NORMAL, 0, RP_GEN_SWEEP_DIR_NORMAL, RP_GEN_SWEEP_DIR_UP_DOWN,CONFIG_VAR);
CIntParameter   outSweepTime[2]             = INIT2("SOUR","_SWEEP_TIME", CBaseParameter::RW, 1000000, 0, 1, 2000000000,CONFIG_VAR); // In microseconds
CBooleanParameter outSweepState[2]          = INIT2("SOUR","_SWEEP_STATE", CBaseParameter::RW, false, 0,CONFIG_VAR);

CBooleanParameter outSweepReset         ("SWEEP_RESET", CBaseParameter::RW, false, 0);

/////////////////////////////

CIntParameter controlSettings("CONTROL_CONFIG_SETTINGS", CBaseParameter::RW, 0, 0, 0, 10);
CStringParameter fileSettings("FILE_SATTINGS", CBaseParameter::RW, "", 0);
CStringParameter listFileSettings("LIST_FILE_SATTINGS", CBaseParameter::RW, "", 0);

const std::vector<std::string> g_savedParams = {"OSC_CH1_IN_GAIN","OSC_CH2_IN_GAIN","OSC_CH3_IN_GAIN","OSC_CH4_IN_GAIN",
                                                "OSC_CH1_IN_AC_DC","OSC_CH2_IN_AC_DC","OSC_CH3_IN_AC_DC","OSC_CH4_IN_AC_DC"};

rp_sweep_api::CSweepController *g_sweepController;

void updateGen(void);


static float*       data[MAX_ADC_CHANNELS + 1];
static float        data_freeze[MAX_ADC_CHANNELS][CH_SIGNAL_DATA];
static float        data_min[MAX_ADC_CHANNELS][CH_SIGNAL_DATA];
static float        data_max[MAX_ADC_CHANNELS][CH_SIGNAL_DATA];
static int          g_old_signalSize = 0;
std::mutex          g_data_mutex;

std::vector<int>    g_indexArray;

void updateParametersByConfig();
void resetAllMinMax();
void resetMinMax();

void UpdateParams(void)
{
  	inRun.Update();

    if (rp_HPIsFastDAC_PresentOrDefault()){
        g_sweepController->pause(!inRun.Value());
    }

	// if (inRun.Value() == false){
	// 	return;
    // }

    auto adc = rpApp_SpecGetADCFreq();
    auto buf = rpApp_SpecGetADCBufferSize();
    adc_freq.SendValue(adc);
    rbw.SendValue(adc/buf);

    for(auto i = 0u; i < g_adc_count; i++){
        if (inShow[i].Value()) {
            rpApp_SpecGetPeakFreq((rp_channel_t)i, &peak_freq[i].Value());
            rpApp_SpecGetPeakPower((rp_channel_t)i, &peak_power[i].Value());
        }else{
            peak_freq[i].Value()  = -1;
            peak_power[i].Value() = -200;
        }
    }


    if (rp_HPIsFastDAC_PresentOrDefault())
	    updateGen();

}

void resetMinMax(int ch,int mode){
    std::lock_guard lock(g_data_mutex);
    auto size = CH_SIGNAL_DATA;
    for(int i = 0; i < size; ++i){
        if (mode == 0)
            data_min[ch][i] = std::numeric_limits<float>::max();
        else
            data_max[ch][i] = std::numeric_limits<float>::lowest();
    }
}

void updateMin(int ch, float *data,int size){
    if (size > CH_SIGNAL_DATA) size = CH_SIGNAL_DATA;
    for(int i = 0 ; i < size ; ++i){
        if (data_min[ch][i] > data[i]){
            data_min[ch][i] = data[i];
        }
    }
}

void updateMax(int ch, float *data,int size){
    if (size > CH_SIGNAL_DATA) size = CH_SIGNAL_DATA;
    for(int i = 0 ; i < size ; ++i){
        if (data_max[ch][i] < data[i]){
            data_max[ch][i] = data[i];
        }
    }
}

float indexInLogSpace(int start,int stop, int value){
    if (value == 0) return value;
    double a = start;
    double b = stop;
    double x = log10f_neon(b/a)/(b-a);
    value = (log10f_neon((double)value/b)/x + b);
    return value;
}

float koffInLogSpace(float start,float stop, float value){
    if (value == 0) return value;
    double a = start;
    double b = stop;

    double x = log10f_neon(b/a)/(b-a);
    value = (b / pow(10, x * b) * pow(10, value * x));
    return value;
}

void prepareIndexArray(std::vector<int> *data, int start,int stop,int view_size,int log_mode){
    float koef = 1;
    float koefMax = 1;
    int pointsNumOrig = stop - start;
    int pointsNum = pointsNumOrig;
    view_size /= 2.0;
    if (pointsNum > view_size)
    {
        koefMax = (float)pointsNum / view_size;
        koef = (float)pointsNum / view_size;
    }
    data->resize(stop-start);
    // int *idx = new int[stop-start];
    for (size_t i = start, j = 0; i < stop; ++i, ++j)
    {
        int index = (j / koef);
        if (log_mode  == 1) {
            float z = stop - start;
            index = round(indexInLogSpace(1, view_size * 3 + 1, ((float)(j * view_size * 3)) / z + 1));
        }
        (*data)[i-start] = index;
    }
    // return idx;
}

void decimateDataMinMax(CFloatBase64Signal &dest, float *src,int start,int stop,int view_size,int log_mode,int *indexArray){

    auto timeNowP1 = std::chrono::system_clock::now();

    float koef = 1;
    float koefMax = 1;
    int pointsNumOrig = stop - start;
    int pointsNum = pointsNumOrig;
    view_size /= 2.0;
    if (pointsNum > view_size)
    {
        koefMax = (float)pointsNum / view_size;
        koef = (float)pointsNum / view_size;
    }
    float v_min = 0;
    float v_max = 0;
    std::vector<int> min;
    std::vector<int> max;
    min.reserve(CH_SIGNAL_DATA);
    max.reserve(CH_SIGNAL_DATA);
    int last_index = -1;
    for (size_t i = start, j = 0; i < stop; ++i, ++j)
    {
        int index = indexArray[i-start];

        if (index != last_index) {
            last_index = index;
            min.push_back(i);
            max.push_back(i);
            v_min = std::numeric_limits<float>::max();
            v_max = std::numeric_limits<float>::lowest();
        }
        if (v_max < src[i]) {
            v_max = src[i];
            max[max.size()-1] = i;
        }

        if (v_min > src[i]) {
            v_min = src[i];
            min[min.size()-1] = i;
        }
    }
    dest.Resize(min.size() + max.size());
    for(int i = 0, j =0 ; i < min.size() + max.size(); i+=2,j++){
        dest[i] = min[j] < max[j] ? src[min[j]] : src[max[j]];
        dest[i+1] = min[j] > max[j] ? src[min[j]] : src[max[j]];
    }
}

void decimateDataFirstN(CFloatBase64Signal &dest, float *src,int start,int stop,int view_size,int log_mode,int *indexArray){

    // auto timeNowP1 = std::chrono::system_clock::now();

    int pointsNumOrig = stop - start;
    int pointsNum = pointsNumOrig;
    if (pointsNum > view_size){
        pointsNum = view_size;
    }

    int size_all = 0;
    int indexes[CH_SIGNAL_DATA];

    int last_index = -1;
    for (size_t i = start, j = 0; i < stop; ++i, ++j)
    {
        int index = indexArray[i-start];

        if (index != last_index) {
            indexes[size_all++] = i;
            last_index = index;
        }
    }
    dest.Resize(size_all);
    for(size_t i = 0 ; i < size_all; i++){
        dest[i] = src[indexes[i]];
    }
}

void decimateDataAvg(CFloatBase64Signal &dest, float *src,int start,int stop,int view_size,int log_mode,int *indexArray){

//     auto timeNowP1 = std::chrono::system_clock::now();

    int sum_size = 0;
    float sum_arr[CH_SIGNAL_DATA];
    int sum_arr_count[CH_SIGNAL_DATA];
    int last_index = -1;
    for (size_t i = start, j = 0; i < stop; ++i, ++j)
    {
        int index = indexArray[i-start];

        if (index != last_index) {
            sum_arr[sum_size] = 0;
            sum_arr_count[sum_size] = 0;
            sum_size++;
            last_index = index;
        }

        sum_arr[sum_size-1] += src[i];
        sum_arr_count[sum_size-1]++;
    }
    dest.Resize(sum_size);
    for(size_t i = 0 ; i < sum_size; i++){
        if (sum_arr_count[i])
            dest[i] = sum_arr[i] / sum_arr_count[i];
        else
            dest[i] = 0;
    }
}

void decimateDataMax(CFloatBase64Signal &dest, float *src,int start,int stop,int view_size,int log_mode,int *indexArray){

    auto timeNowP1 = std::chrono::system_clock::now();

    float koef = 1;
    float koefMax = 1;
    int pointsNumOrig = stop - start;
    int pointsNum = pointsNumOrig;
    if (pointsNum > view_size)
    {
        koefMax = (float)pointsNum / view_size;
        koef = (float)pointsNum / view_size;
    }
    float v_max = 0;
    float max[CH_SIGNAL_DATA];
    int all_max = 0;
    int last_index = -1;
    for (size_t i = start, j = 0; i < stop; ++i, ++j)
    {
        int index = indexArray[i-start];

        if (index != last_index) {
            last_index = index;
            all_max++;
            max[all_max-1] = src[i];;
        }
        if (max[all_max-1] < src[i]) {
            max[all_max-1] = src[i];
        }

        // if (v_min > src[i]) {
        //     v_min = src[i];
        //     min[min.size()-1] = i;
        // }
    }
    dest.Resize(all_max);
    for(int i = 0 ; i < all_max; i++){
        dest[i] = max[i];
    }
}

void decimateData(CFloatBase64Signal &dest, float *src,int start,int stop,int view_size,int log_mode,int *indexArray){
//    decimateDataMinMax(dest,src,start,stop,view_size,log_mode,indexArray);
//    decimateDataFirstN(dest,src,start,stop,view_size,log_mode,indexArray);
//    decimateDataAvg(dest,src,start,stop,view_size,log_mode,indexArray);
    decimateDataMax(dest,src,start,stop,view_size,log_mode,indexArray);
}

// use in waterfall
void decimateDataByMax(CFloatBase64Signal &dest, float *src,int start,int stop,int view_size){
    float koef = 1;
    int pointsNumOrig = stop - start;
    int pointsNum = pointsNumOrig;
    if (pointsNum > view_size)
    {
        koef = (float)pointsNum / view_size;
        pointsNum = view_size;
    }
    dest.Resize(pointsNum);
    int last_index = -1;
    for (size_t i = start, j = 0; i < stop; ++i, ++j)
    {
        int index = (int)(j / koef);
        if (index != last_index) {
            last_index = index;
            dest[index] = std::numeric_limits<float>::lowest();
        }
        if (dest[index] < src[i]) dest[index] = src[i];
    }
}

void copyData(CFloatBase64Signal &dest, float *src,int size){
    if (dest.GetSize() != size) dest.Resize(size);
    memcpy_neon(&dest[0], src, size * sizeof(float));
}

void UpdateSignals(void)
{
	inRun.Update();
	if (inRun.Value() == false) {
    	return;
    }
    size_t signal_size = 0;
    rpApp_SpecGetViewSize(&signal_size);
    bool isShow = true;
    for(auto i = 0u; i < g_adc_count; i++){
        isShow |= inShow[i].Value();
    }

    int ret = rpApp_SpecGetViewData(data, signal_size);
    if (ret != 0) {
        return;
    }


    auto mode = rp_dsp_api::mode_t::DBM;
    rpApp_SpecGetMode(&mode);
    signal_mode[0] = mode;

    for(auto i = 0u; i < g_adc_count; i++){
        if (inFreeze[i].Value() && g_old_signalSize == signal_size) {
            memcpy_neon(data[i + 1],data_freeze[i],signal_size * sizeof(float));
        }else{
            memcpy_neon(data_freeze[i],data[i + 1],signal_size * sizeof(float));
        }
    }


    if (g_old_signalSize != signal_size || inReset.Value()) {
        resetAllMinMax();
    }

    std::lock_guard lock(g_data_mutex);


    int width = view_port_width.Value();
    if (width > signal_size) width = signal_size;

    // Resize X axis
    int i_start = 0;
    int i_stop  = signal_size;
    for(int i = 0;i < signal_size; ++i){
        if (view_port_start.Value() < data[0][i]){
            break;
        }
        i_start = i;
    }
    if (i_start > 0) i_start--;

    for(int i = signal_size-1;i >=0; --i){
        if (view_port_end.Value() > data[0][i]){
            break;
        }
        i_stop = i;
    }
    if (i_stop < signal_size-1) i_stop++;

    int i_start_w = 0;
    int i_stop_w  = signal_size;
    for(int i = 0;i < signal_size; ++i){
        if (xmin.Value() < data[0][i]){
            break;
        }
        i_start_w = i;
    }
        for(int i = signal_size-1;i >=0; --i){
        if (xmax.Value() > data[0][i]){
            break;
        }
        i_stop_w = i;
    }

    prepareIndexArray(&g_indexArray, i_start,i_stop,width,xAxisLogMode.Value());

    decimateData(s_xaxis,data[0],i_start,i_stop,width,xAxisLogMode.Value(),g_indexArray.data());
    // End resize

    if (requestFullData.Value()){
            requestFullData.Value() = false;
            if (isShow){
                copyData(s_xaxis_full,data[0],signal_size);
                for(auto ch = 0u; ch < g_adc_count; ch++){
                    if (inShow[ch].Value()){
                        if (inFreeze[ch].Value())
                            copyData(s_full[ch],data_freeze[ch],signal_size);
                        else
                            copyData(s_full[ch],data[ch+1],signal_size);

                        if (inShowMin[ch].Value()){
                            copyData(s_min_full[ch],data_min[ch],signal_size);
                        }else{
                            s_min_full[ch].Resize(0);
                        }
                        if (inShowMax[ch].Value()){
                            copyData(s_max_full[ch],data_max[ch],signal_size);
                        }else{
                            s_max_full[ch].Resize(0);
                        }
                    }else{
                        s_full[ch].Resize(0);
                        s_min_full[ch].Resize(0);
                        s_max_full[ch].Resize(0);
                    }
                }
            }else{
                s_xaxis_full.Resize(0);
                for(auto ch = 0u; ch < g_adc_count; ch++){
                    s_full[ch].Resize(0);
                    s_min_full[ch].Resize(0);
                    s_max_full[ch].Resize(0);
                }
            }
    }else{
        s_xaxis_full.Resize(0);
        for(auto ch = 0u; ch < g_adc_count; ch++){
            s_full[ch].Resize(0);
            s_min_full[ch].Resize(0);
            s_max_full[ch].Resize(0);
        }
    }

    for(auto ch = 0u; ch < g_adc_count; ch++){
        updateMin(ch,data[ch + 1],signal_size);
        updateMax(ch,data[ch + 1],signal_size);



        if (inShow[ch].Value()) {
            decimateData(s_view[ch],data[ch + 1],i_start,i_stop,width,xAxisLogMode.Value(),g_indexArray.data());
            if (inShowMin[ch].Value()) {
                decimateData(s_view_min[ch],data_min[ch],i_start,i_stop,width,xAxisLogMode.Value(),g_indexArray.data());
            }else{
                s_view_min[ch].Resize(0);
            }
            if (inShowMax[ch].Value()) {
                decimateData(s_view_max[ch],data_max[ch],i_start,i_stop,width,xAxisLogMode.Value(),g_indexArray.data());
            }else{
                s_view_max[ch].Resize(0);
            }
        }else{
            s_view[ch].Resize(0);
            s_view_min[ch].Resize(0);
            s_view_max[ch].Resize(0);
        }
    }

    inReset.Value() = false;
    g_old_signalSize = signal_size;
}

static void UpdateGeneratorParameters(bool force)
{
    for(auto ch = 0u; ch < g_dac_count ; ch++){
        // OUT1
        if (outState[ch].IsNewValue() || force) {
            if (outState[ch].NewValue()) {

                if (rp_HPGetFastDACIsTempProtectionOrDefault())
                    rp_SetLatchTempAlarm((rp_channel_t)ch,false);

                rp_GenOutEnable((rp_channel_t)ch);
                rp_GenResetTrigger((rp_channel_t)ch);
            } else {
                rp_GenOutDisable((rp_channel_t)ch);
            }
            outState[ch].Update();
            RESEND(outState[ch])
        }


        if (outAmplitude[ch].IsNewValue() || outOffset[ch].IsNewValue()  || force){
            if (rp_HPGetIsGainDACx5OrDefault()){
                float Coff = outImp[ch].Value() == 1 ?  2.0 : 1.0;
                if ((fabs(outAmplitude[ch].NewValue())  + fabs(outOffset[ch].NewValue() )) * Coff > 1.0){
                    if (rp_GenSetGainOut((rp_channel_t)ch, RP_GAIN_5X) == RP_OK) {
                        outGain[ch].Value() = 1;
                        outGain[ch].Update();
                    }
                    rp_GenAmp((rp_channel_t)ch, outAmplitude[ch].NewValue() / 5.0 * Coff);
                } else {
                    if (rp_GenSetGainOut((rp_channel_t)ch, RP_GAIN_1X) == RP_OK) {
                        outGain[ch].Value() = 0;
                        outGain[ch].Update();
                    }
                    rp_GenAmp((rp_channel_t)ch, outAmplitude[ch].NewValue() * Coff);
                }
                rp_GenOffset((rp_channel_t)ch, (outOffset[ch].NewValue() * Coff) / (outGain[ch].Value() == 1 ? 5.0 : 1.0));
            }
            else{
                rp_GenAmp((rp_channel_t)ch, outAmplitude[ch].NewValue());
                rp_GenOffset((rp_channel_t)ch, outOffset[ch].NewValue());
            }
            outOffset[ch].Update();
            outAmplitude[ch].Update();
            RESEND(outOffset[ch])
            RESEND(outAmplitude[ch])
        }

        if (outFrequancy[ch].IsNewValue() || force) {
            float period = 1000000.0 / outFrequancy[ch].NewValue();
            outRiseTime[ch].SetMin(period * RISE_FALL_MIN_RATIO);
            outRiseTime[ch].SetMax(period * RISE_FALL_MAX_RATIO);
            outRiseTime[ch].Update();
            outFallTime[ch].SetMin(period * RISE_FALL_MIN_RATIO);
            outFallTime[ch].SetMax(period * RISE_FALL_MAX_RATIO);
            outFallTime[ch].Update();
            rp_GenFreq((rp_channel_t)ch, outFrequancy[ch].NewValue());
            outFrequancy[ch].Update();
            rp_GenTriggerOnly((rp_channel_t)ch);
            RESEND(outFrequancy[ch])
            RESEND(outRiseTime[ch])
            RESEND(outFallTime[ch])
        }

        if (outPhase[ch].IsNewValue() || force) {
            rp_GenPhase((rp_channel_t)ch, outPhase[ch].NewValue());
            outPhase[ch].Update();
            RESEND(outPhase[ch])
        }

        if (outDCYC[ch].IsNewValue() || force) {
            rp_GenDutyCycle((rp_channel_t)ch, outDCYC[ch].NewValue() / 100);
            outDCYC[ch].Update();
            RESEND(outDCYC[ch])
        }

        if (outRiseTime[ch].IsNewValue() || force) {
            rp_GenRiseTime((rp_channel_t)ch, outRiseTime[ch].NewValue());
            outRiseTime[ch].Update();
            RESEND(outRiseTime[ch])
        }

        if (outFallTime[ch].IsNewValue() || force) {
            rp_GenFallTime((rp_channel_t)ch, outFallTime[ch].NewValue());
            outFallTime[ch].Update();
            RESEND(outFallTime[ch])
        }

        if (outWaveform[ch].IsNewValue() || force){
            auto wf = outWaveform[ch].NewValue();
            if (wf[0] == 'A'){
                auto signame = wf.erase(0, 1);
                float data[DAC_BUFFER_SIZE];
                uint32_t size;
                if (!rp_ARBGetSignalByName(signame,data,&size)){
                    rp_GenArbWaveform((rp_channel_t)ch,data,size);
                    rp_GenWaveform((rp_channel_t)ch,RP_WAVEFORM_ARBITRARY);
                    outWaveform[ch].Update();
                }else{
                    outWaveform[ch].Update();
                    rp_GenWaveform((rp_channel_t)ch, RP_WAVEFORM_SINE);
                    outWaveform[ch].Value() = "0";
                }

            }else{
                try{
                    rp_waveform_t w = (rp_waveform_t)stoi(wf);
                    rp_GenWaveform((rp_channel_t)ch, w);
                    outWaveform[ch].Update();
                }
                catch (const std::exception&) {
                    rp_GenWaveform((rp_channel_t)ch, RP_WAVEFORM_SINE);
                    outWaveform[ch].Update();
                    outWaveform[ch].Value() = "0";
                }
            }
        }
    }


    if (rp_HPGetFastDACIsTempProtectionOrDefault()){
        for(auto ch = 0u; ch < g_dac_count ; ch++){
            bool temperature_runtime = false;
            if (rp_GetRuntimeTempAlarm((rp_channel_t)ch, &temperature_runtime) == RP_OK){
                outTemperatureRuntime[ch].Value() = temperature_runtime;
                outTemperatureRuntime[ch].Update();
            }

            bool temperature_latched = false;
            if (rp_GetLatchTempAlarm((rp_channel_t)ch, &temperature_latched) == RP_OK){
                outTemperatureLatched[ch].Value() = temperature_latched;
                outTemperatureLatched[ch].Update();
            }
        }
    }

    if (rp_HPGetIsPLLControlEnableOrDefault()){
        bool pll_control_locked = false;
        if (rp_GetPllControlLocked(&pll_control_locked) == RP_OK){
            pllControlLocked.Value() = pll_control_locked;
            pllControlLocked.Update();
        }
    }


    bool needUpdate = force | outSweepReset.IsNewValue();
    for(auto ch = 0u; ch < g_dac_count ; ch++){
        needUpdate |= outSweepStartFrequancy[ch].IsNewValue() | outSweepEndFrequancy[ch].IsNewValue() | outSweepMode[ch].IsNewValue() | outSweepDir[ch].IsNewValue() | outSweepTime[ch].IsNewValue() | outSweepState[ch].IsNewValue();
    }

    if (needUpdate){
        for(auto ch = 0u; ch < g_dac_count ; ch++){
            outSweepStartFrequancy[ch].Update();
            outSweepEndFrequancy[ch].Update();
            outSweepMode[ch].Update();
            outSweepDir[ch].Update();
            outSweepTime[ch].Update();
            outSweepState[ch].Update();

            g_sweepController->setStartFreq((rp_channel_t)ch,outSweepStartFrequancy[ch].Value());
            g_sweepController->setStopFreq((rp_channel_t)ch,outSweepEndFrequancy[ch].Value());
            g_sweepController->setMode((rp_channel_t)ch,(rp_gen_sweep_mode_t)outSweepMode[ch].Value());
            g_sweepController->setDir((rp_channel_t)ch,(rp_gen_sweep_dir_t)outSweepDir[ch].Value());
            g_sweepController->setTime((rp_channel_t)ch,outSweepTime[ch].Value());
            g_sweepController->genSweep((rp_channel_t)ch,outSweepState[ch].Value());
        }

        if (outSweepReset.NewValue()){
                g_sweepController->resetAll();
                if (outState[0].Value() && outState[1].Value()){
                    rp_GenOutEnableSync(true);
                    rp_GenSynchronise();
                }
        }

        if (outSweepState[0].Value() || outSweepState[1].Value()){
            g_sweepController->run();
        }else{
            g_sweepController->stop();
        }
    }
}

void resetAllMinMax(){
    for(auto ch = 0u; ch < g_adc_count; ch++){
        if (inShowMin[ch].Value()) resetMinMax(ch,0);
        if (inShowMax[ch].Value()) resetMinMax(ch,1);
    }
}

void OnNewParams(void)
{

    if (controlSettings.IsNewValue()){
        if (controlSettings.NewValue() == controlSettings::REQUEST_RESET){
            deleteConfig();
            configSetWithList(g_savedParams);
            controlSettings.Update();
            controlSettings.SendValue(controlSettings::RESET_DONE);
            return;
        }

        if (controlSettings.NewValue() == controlSettings::SAVE){
            controlSettings.Update();
            fileSettings.Update();
            configSet();
            saveCurrentSettingToStore(fileSettings.Value());
            controlSettings.SendValue(controlSettings::NONE);
            listFileSettings.SendValue(getListOfSettingsInStore());
        }

        if (controlSettings.NewValue() == controlSettings::LOAD){
            controlSettings.Update();
            fileSettings.Update();
            loadSettingsFromStore(fileSettings.Value());
            configGet();
            controlSettings.SendValue(controlSettings::LOAD_DONE);
        }

        if (controlSettings.NewValue() == controlSettings::DELETE){
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

    for(auto ch = 0u; ch < g_adc_count; ch++){
        if (inShow[ch].IsNewValue()) {
            inShow[ch].Update();
            inShow[ch].SendValue(inShow[ch].Value());
            RESEND(inShow[ch])
        }
    }

    if (inReset.IsNewValue()){
        inReset.Update();
    }

	if (xmin.IsNewValue() || xmax.IsNewValue() || freq_unit.IsNewValue()){
		freq_unit.Update();
        xmin.Update();
		xmax.Update();

        if (xmax.Value()  <= xmin.Value()){
            xmin.Value() = xmax.Value() * 0.95;
        }
 		rpApp_SpecSetFreqRange(xmin.Value(), xmax.Value());
        resetAllMinMax();
	}

    for(auto ch = 0u; ch < g_adc_count; ch++){
        if (inShowMin[ch].IsNewValue()){
            inShowMin[ch].Update();
            if (inShowMin[ch].Value()){
                if (!inFreeze[ch].Value())
                    resetMinMax(ch,0);
            }
            RESEND(inShowMin[ch])
        }

        if (inShowMax[ch].IsNewValue()){
            inShowMax[ch].Update();
            if (inShowMax[ch].Value()){
                if (!inFreeze[ch].Value())
                    resetMinMax(ch,1);
            }
            RESEND(inShowMax[ch])
        }

        if (inFreeze[ch].IsNewValue()){
            inFreeze[ch].Update();
            RESEND(inFreeze[ch])
        }
    }

    for(auto i = 0u; i < CURSORS_COUNT; i++){
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


    if (requestFullData.IsNewValue()){
        requestFullData.Update();
    }

    if (xAxisLogMode.IsNewValue()){
        xAxisLogMode.Update();
        xAxisLogMode.SendValue(xAxisLogMode.Value());
    }

    if (view_port_width.IsNewValue()){
        view_port_width.Update();
    }

    if (view_port_start.IsNewValue()){
        view_port_start.Update();
    }

    if (view_port_end.IsNewValue()){
        view_port_end.Update();
    }

    if (windowMode.IsNewValue()) {
        if (rpApp_SpecSetWindow((rp_dsp_api::window_mode_t)windowMode.NewValue()) == RP_OK){
            windowMode.Update();
            resetAllMinMax();
        }
    }

    if (impedance.IsNewValue()) {
        if (rpApp_SpecSetImpedance(impedance.NewValue()) == RP_OK)
            impedance.Update();
    }


    if (y_axis_mode.IsNewValue()){
        if (rpApp_SpecSetMode((rp_dsp_api::mode_t)y_axis_mode.NewValue()) == RP_OK){
            y_axis_mode.Update();
            resetAllMinMax();
        }
    }

    if (bufferSize.IsNewValue()) {
        if (rpApp_SpecSetADCBufferSize(bufferSize.NewValue()) == RP_OK){
            bufferSize.Update();
            resetAllMinMax();
        }
    }

    if (cutDC.IsNewValue()) {
        if (rpApp_SpecSetRemoveDC(cutDC.NewValue()) == RP_OK){
            cutDC.Update();
            RESEND(cutDC)
        }
    }

    if (rp_HPGetFastADCIsAC_DCOrDefault()){
        for(auto ch = 0u; ch < g_adc_count; ch++){
            if (inAC_DC[ch].IsNewValue()) {
                if (rp_AcqSetAC_DC((rp_channel_t)ch,inAC_DC[ch].NewValue() == 0 ? RP_AC:RP_DC) == RP_OK){
                    inAC_DC[ch].Update();
                    RESEND(inAC_DC[ch])
                }
            }
        }
    }

    if (rp_HPGetIsPLLControlEnableOrDefault()){
        if(pllControlEnable.IsNewValue()) {
            if (rp_SetPllControlEnable(pllControlEnable.NewValue()) == RP_OK){
                pllControlEnable.Update();
                RESEND(pllControlEnable)
            }
        }
    }

    if (rp_HPGetFastADCIsLV_HVOrDefault()){
        for(auto ch = 0u; ch < g_adc_count; ch++){
            if (inGain[ch].IsNewValue()) {
                if (rp_AcqSetGain((rp_channel_t)ch, inGain[ch].NewValue() == 0 ? RP_LOW : RP_HIGH) == 0){
                    inGain[ch].Update();
                    rpApp_SpecReset();
                    RESEND(inGain[ch])
                }
            }
        }
    }

    for(auto ch = 0u; ch < g_adc_count; ch++){
        if (inProbe[ch].IsNewValue()) {
            if (rpApp_SpecSetProbe((rp_channel_t)ch, inProbe[ch].NewValue()) == RP_OK){
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

extern "C" int rp_app_init(void)
{
    fprintf(stderr, "Loading spectrum version %s-%s.\n", VERSION_STR, REVISION_STR);

    for(auto ch = 0u; ch < MAX_ADC_CHANNELS + 1; ch++)
        data[ch] = new float[CH_SIGNAL_DATA];

    g_indexArray.reserve(CH_SIGNAL_DATA);

    if (getModelName() == "Z20"){
        for(int i = 0; i < g_dac_count; i++){
            auto ch = (rp_channel_t)i;
            outFrequancy[ch].SetMin(300000);
        }
    }

    setHomeSettingsPath("/.config/redpitaya/apps/spectrumpro/");

	rp_WC_Init();


    rp_Init();
    rp_Reset();

    g_sweepController = new rp_sweep_api::CSweepController();
    if (rp_HPIsFastDAC_PresentOrDefault()){
        for(auto ch = 0u; ch < g_dac_count; ch++){
            rp_GenAmp((rp_channel_t)ch, outAmplitude[ch].NewValue());
        }
    }
    rpApp_SpecRun();

    for(auto ch = 0u; ch < g_adc_count; ch++){
        if (rp_HPGetFastADCIsLV_HVOrDefault()){
            rp_AcqSetGain((rp_channel_t)ch,RP_LOW);
        }
        if (rp_HPGetFastADCIsAC_DCOrDefault())
            rp_AcqSetAC_DC((rp_channel_t)ch, RP_AC);
        if (rp_HPGetFastDACIsTempProtectionOrDefault()){
            rp_SetEnableTempProtection((rp_channel_t)ch,true);
        }
        if (rp_HPGetIsGainDACx5OrDefault()){
            rp_GenSetGainOut((rp_channel_t)ch,RP_GAIN_1X);
        }
    }

    if (rp_HPGetIsPLLControlEnableOrDefault())
       rp_SetPllControlEnable(false);

    listFileSettings.Value() = getListOfSettingsInStore();
    updateParametersByConfig();

    return 0;
}

extern "C" int rp_app_exit(void)
{
    fprintf(stderr, "Unloading spectrum version %s-%s.\n", VERSION_STR, REVISION_STR);
    rpApp_SpecStop();
    delete g_sweepController;

    if (rp_HPGetFastDACIsTempProtectionOrDefault()){
        for(auto ch = 0u; ch < g_dac_count; ch++){
            rp_SetEnableTempProtection((rp_channel_t)ch,false);
        }
    }
    if (rp_HPGetIsPLLControlEnableOrDefault())
        rp_SetPllControlEnable(false);

    return 0;
}

extern "C" const char *rp_app_desc(void) {
    return (const char*)"Red Pitaya spectrometer application.\n";
}

extern "C" int rp_set_params(rp_app_params_t *p, int len) {
    return 0;
}

extern "C" int rp_get_params(rp_app_params_t **p) {
    return 0;
}

extern "C" int rp_get_signals(float ***s, int *sig_num, int *sig_len) {
    return 0;
}

void updateGen(void) {
    for(auto ch = 0u; ch < g_dac_count ; ch++){
        float freq = 0;
        rp_GenGetFreq((rp_channel_t)ch,&freq);
        if ((int)freq != outFrequancy[ch].Value()){
            outFrequancy[ch].SendValue((int)freq);
        }
    }
}

void PostUpdateSignals(void){}

void updateParametersByConfig(){
    configGet();

    if (rp_HPGetFastADCIsAC_DCOrDefault()){
        for(auto ch = 0u; ch < g_adc_count ; ch++){
            rp_AcqSetAC_DC((rp_channel_t)ch,inAC_DC[ch].Value() == 0 ? RP_AC:RP_DC);
        }
    }

    if (rp_HPGetIsPLLControlEnableOrDefault())
        rp_SetPllControlEnable(pllControlEnable.Value());

    if (rp_HPGetFastADCIsLV_HVOrDefault()){
        for(auto ch = 0u; ch < g_adc_count ; ch++){
            rp_AcqSetGain((rp_channel_t)ch, inGain[ch].Value() == 0 ? RP_LOW : RP_HIGH);
        }
    }

    for(auto ch = 0u; ch < g_adc_count ; ch++){
            rpApp_SpecSetProbe((rp_channel_t)ch, inProbe[ch].Value());
        }

    if (rp_HPIsFastDAC_PresentOrDefault()){
        UpdateGeneratorParameters(true);
    }

    rpApp_SpecSetImpedance(impedance.Value());
    rpApp_SpecSetWindow((rp_dsp_api::window_mode_t)windowMode.Value());
    rpApp_SpecSetRemoveDC(cutDC.Value());
    rpApp_SpecSetADCBufferSize(bufferSize.Value());

    CDataManager::GetInstance()->SendAllParams();
}
