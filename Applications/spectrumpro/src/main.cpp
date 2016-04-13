#include <math.h>
#include <DataManager.h>
#include <CustomParameters.h>

extern "C" {
    #include "rpApp.h"
    #include "waterfall.h"
    #include "version.h"
}


/* Parameters description structure - must be the same for all RP controllers */
typedef struct rp_app_params_s {
    char  *name;
    float  value;
    int    fpga_update;
    int    read_only;
    float  min_val;
    float  max_val;
} rp_app_params_t;


enum { CH_SIGNAL_SIZE = 2048, INTERVAL = 300 };
enum { FREQ_CHANNEL = -1 };

enum { GEN_BUFFER_LENGTH =  (16 * 1024)};

static const float OUT1_FREQ_MIN = 400000.f;
static const float OUT1_FREQ_MAX = 700000.f;
static const float OUT1_AMP_MIN = 0.5f;
static const float OUT1_AMP_MAX = 1.f;

static const float OUT2_FREQ_MIN = 500000.f;
static const float OUT2_FREQ_MAX = 800000.f;
static const float OUT2_AMP_MIN = 0.3f;
static const float OUT2_AMP_MAX = 1.f;

static const float OUT_FREQ_CHANGE = 1000.f;
static const float OUT_AMP_CHANGE = 0.01f;

float genCh1Data[GEN_BUFFER_LENGTH];
float genCh2Data[GEN_BUFFER_LENGTH];

CFloatSignal ch1("ch1", CH_SIGNAL_SIZE, 0.0f);
CFloatSignal ch2("ch2", CH_SIGNAL_SIZE, 0.0f);
//CFloatSignal freq_ch("freq_ch", CH_SIGNAL_SIZE, 0.0f);

CIntParameter freq_range("freq_range", CBaseParameter::RW, 0, 1, 0, 5);
CIntParameter freq_unit("freq_unit", CBaseParameter::RWSA, 2, 0, 0, 2);

//power
CFloatParameter in1Scale("SPEC_CH1_SCALE", CBaseParameter::RW, 10, 0, 0, 1000);
CFloatParameter in2Scale("SPEC_CH2_SCALE", CBaseParameter::RW, 10, 0, 0, 1000);

CFloatParameter inFreqScale("SPEC_TIME_SCALE", CBaseParameter::RW, 1, 0, 0, 50000);
CFloatParameter xmin("xmin", CBaseParameter::RW, 0, 0, -1000, 1000);
CFloatParameter xmax("xmax", CBaseParameter::RW, 63, 0, -1000, 1000);


CFloatParameter peak1_freq("peak1_freq", CBaseParameter::ROSA, 0, 0, 0, +1e6);
CFloatParameter peak1_power("peak1_power", CBaseParameter::ROSA, 0, 0, -10000000, +10000000);
CFloatParameter peak1_unit("peak1_unit", CBaseParameter::ROSA, 0, 1, 0, 2);

CFloatParameter peak2_freq("peak2_freq", CBaseParameter::ROSA, 0, 0, 0, 1e6);
CFloatParameter peak2_power("peak2_power", CBaseParameter::ROSA, 0, 0, -10000000, +10000000);
CFloatParameter peak2_unit("peak2_unit", CBaseParameter::ROSA, 0, 0, 0, 2);

CIntParameter w_idx("w_idx", CBaseParameter::RO, 0, 0, 0, 1000);
CBooleanParameter en_avg_at_dec("en_avg_at_dec", CBaseParameter::RO, true, 0);

/* WEB GUI Buttons */
CBooleanParameter inReset("SPEC_RST", CBaseParameter::RW, false, 0);
CBooleanParameter inRun("SPEC_RUN", CBaseParameter::RW, true, 0);
CBooleanParameter inAutoscale("SPEC_AUTOSCALE", CBaseParameter::RW, false, 0);
CBooleanParameter inSingle("SPEC_SINGLE", CBaseParameter::RW, false, 0);
CBooleanParameter in1Show("CH1_SHOW", CBaseParameter::RW, true, 0);
CBooleanParameter in2Show("CH2_SHOW", CBaseParameter::RW, false, 0);
CBooleanParameter genEnable("SPEC_GEN_ENABLE", CBaseParameter::RW, true, 0);

/* --------------------------------  CURSORS  ------------------------------ */
CBooleanParameter cursorx1("SPEC_CURSOR_X1", CBaseParameter::RW, false, 0);
CBooleanParameter cursorx2("SPEC_CURSOR_X2", CBaseParameter::RW, false, 0);
CBooleanParameter cursory1("SPEC_CURSOR_Y1", CBaseParameter::RW, false, 0);
CBooleanParameter cursory2("SPEC_CURSOR_Y2", CBaseParameter::RW, false, 0);
CIntParameter cursorSrc("SPEC_CURSOR_SRC", CBaseParameter::RW, RPAPP_OSC_SOUR_CH1, 0, RPAPP_OSC_SOUR_CH1, RPAPP_OSC_SOUR_MATH);

CFloatParameter cursor1V("SPEC_CUR1_V", CBaseParameter::RW, -1, 0, -1000, 1000);
CFloatParameter cursor2V("SPEC_CUR2_V", CBaseParameter::RW, -1, 0, -1000, 1000);
CFloatParameter cursor1T("SPEC_CUR1_T", CBaseParameter::RW, -1, 0, -1000, 1000);
CFloatParameter cursor2T("SPEC_CUR2_T", CBaseParameter::RW, -1, 0, -1000, 1000);

static float g_max_freq = 63000000;

static float out1_freq = OUT1_FREQ_MIN;
static float out1_amp = OUT1_AMP_MAX;
static float out1_freq_change = OUT_FREQ_CHANGE;
static float out1_amp_change = OUT_AMP_CHANGE;

static float out2_freq = OUT2_FREQ_MAX;
static float out2_amp = OUT2_AMP_MIN;
static float out2_freq_change = OUT_FREQ_CHANGE;
static float out2_amp_change = OUT_AMP_CHANGE;

void UpdateGen(void);
void InitGen(void);

void UpdateParams(void)
{
	inRun.Update();
	if (inRun.Value() == false)
		return;

	if (in1Show.Value() || in2Show.Value())
		rpApp_SpecGetJpgIdx(&w_idx.Value());

	if (in1Show.Value()) {
		//rpApp_SpecGetPeakPower(RP_CH_1, &peak1_power.Value());
		rpApp_SpecGetPeakFreq(RP_CH_1, &peak1_freq.Value());
	}

	if (in2Show.Value()) {
		//rpApp_SpecGetPeakPower(RP_CH_2, &peak2_power.Value());
		rpApp_SpecGetPeakFreq(RP_CH_2, &peak2_freq.Value());
	}

	static bool inited_loop = false;
	if (!inited_loop) {
		rp_EnableDigitalLoop(IsDemoParam.Value());
		inited_loop = true;
	}

	UpdateGen();
}

void UpdateSignals(void)
{
	static float* data[3] = {0};
	if (data[0] == 0) // TODO
	{
		for (size_t i = 0; i < 3; ++i)
			data[i] = new float[2048];
	}

	inRun.Update();
	if (inRun.Value() == false)
		return;

	if (in1Show.Value() || in2Show.Value())
	{
		int ret = rpApp_SpecGetViewData(data, 2048);
		if (ret != 0)
			return;
	}

	float fpga_freq, k1 = 1, k2 = 1;
	rpApp_SpecGetFpgaFreq(&fpga_freq);

	if (g_max_freq < fpga_freq)
		k2 = fpga_freq/g_max_freq; // send xmax limit koeff

    double max_pw_cha = -1e5;
    double max_pw_chb = -1e5;

	if (in1Show.Value()) {
		ch1.Resize(CH_SIGNAL_SIZE/k2);
		for (size_t i = 0; i < ch1.GetSize(); ++i) {
			ch1[i] = data[1][i];
		    /* Find peaks */
		    if(data[1][i] > max_pw_cha) {
		        max_pw_cha     = data[1][i];
		    }
		}
	}
	else if (ch1.GetSize() == CH_SIGNAL_SIZE)
		ch1.Resize(0);

	if (in2Show.Value()) {
			ch2.Resize(CH_SIGNAL_SIZE/k2);
		for (size_t i = 0; i < ch2.GetSize(); ++i) {
			ch2[i] = data[2][i];
		    /* Find peaks */
		    if(data[1][i] > max_pw_chb) {
		        max_pw_chb     = data[1][i];
		    }
		}
	}
	else if (ch2.GetSize() == CH_SIGNAL_SIZE)
		ch2.Resize(0);

	peak1_power.Value() = max_pw_cha;
	peak2_power.Value() = max_pw_chb;
}

extern "C" int rp_app_exit(void);
void OnNewParams(void)
{
	static bool run = false;
	if (!run)
	{
		static wf_func_table_t wf_f = {
			rp_spectr_wf_init,
    		rp_spectr_wf_clean,
    		rp_spectr_wf_clean_map,
    		rp_spectr_wf_calc,
	    	rp_spectr_wf_save_jpeg
		};
		rpApp_SpecRun(&wf_f);
		run = true;
		CDataManager::GetInstance()->SetParamInterval(INTERVAL);
		CDataManager::GetInstance()->SetSignalInterval(INTERVAL);
	}

	in1Show.Update();
	in2Show.Update();

    cursorx1.Update();
    cursorx2.Update();
    cursory1.Update();
    cursory2.Update();
    cursorSrc.Update();
    cursor1V.Update();
    cursor2V.Update();
    cursor1T.Update();
    cursor2T.Update();

	xmin.Update();
	if (xmin.Value() < 0)
		xmin.Value() = 0;

	if (xmax.IsNewValue() || freq_unit.IsNewValue())
	{
		freq_unit.Update();
		xmax.Update();

		if (freq_unit.Value() == 2 && xmax.Value() > 63)
			xmax.Value() = 63;

		float min = xmin.Value();
		g_max_freq = xmax.Value();
		for (int i = 0; i < freq_unit.Value(); ++i)
		{
			g_max_freq *= 1000; // set x max freq (Hz, kHz or MHz) by unit
			min *= 1000;
		}

		rpApp_SpecSetUnit(freq_unit.Value());
		rpApp_SpecSetFreqRange(min, g_max_freq);
		peak1_unit.Value() = peak2_unit.Value() = rpApp_SpecGetUnit();
	}

    if(IsDemoParam.Value()) {
        static bool once = true;
        if(genEnable.IsNewValue() || once) {
            once = false;
            genEnable.Update();

            if(genEnable.Value()) {
                InitGen();
            } else {
                rp_GenReset();
            }
        }
    }
}

extern "C" void SpecIntervalInit()
{
	CDataManager::GetInstance()->SetParamInterval(INTERVAL);
	CDataManager::GetInstance()->SetSignalInterval(INTERVAL);
}

extern "C" int rp_app_init(void)
{
    fprintf(stderr, "Loading spectrum version %s-%s.\n", VERSION_STR, REVISION_STR);
    //rpApp_Init();
    rp_Init();
    rp_Reset();

   if(IsDemoParam.Value())
       rp_GenReset();

    return 0;
}

extern "C" int rp_app_exit(void)
{
    fprintf(stderr, "Unloading spectrum version %s-%s.\n", VERSION_STR, REVISION_STR);
	rpApp_SpecStop();

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

static inline float my_sin(int idx) {
    return (float) (sin(2 * M_PI * (float) idx / (float) GEN_BUFFER_LENGTH));
}

static inline void synthesis_ch1() {
/*
    Signal for IN1 is given by: x = 0.9*sin(w0t)+1/4*sin(3*w0t)+1/6*sin(6*w0t);
    where w0 = 2*pi*f1, f1= 400 000 Hz
*/

    for(int unsigned i = 0; i < GEN_BUFFER_LENGTH; i++) {
        genCh1Data[i] = (0.9f * my_sin(i)) + ((1.f/4.f) * my_sin(3 * i)) + ((1.f/6.f) * my_sin(6 * i));
    }
}

static inline void synthesis_ch2() {
 /*
    Signal for IN2 is given by: y=0.8*sin(w0*t)+1/3*sin(5*w0*t)+1/6*sin(7*w0*t);
    where w0 = 2*pi*f2, f2= 600 000 Hz
*/

    for(int unsigned i = 0; i < GEN_BUFFER_LENGTH; i++) {
        genCh2Data[i] = (0.8f * my_sin(i)) + ((1.f/3.f) * my_sin(5 * i)) + ((1.f/6.f) * my_sin(7 * i));
    }
}

void InitGen(void) {
    out1_freq = OUT1_FREQ_MIN;
    out1_amp = OUT1_AMP_MAX;
    out1_freq_change = OUT_FREQ_CHANGE;
    out1_amp_change = OUT_AMP_CHANGE;

    out2_freq = OUT2_FREQ_MAX;
    out2_amp = OUT2_AMP_MIN;
    out2_freq_change = OUT_FREQ_CHANGE;
    out2_amp_change = OUT_AMP_CHANGE;

    synthesis_ch1();
    rp_GenOffset(RP_CH_1, 0.f);
    rp_GenAmp(RP_CH_1, out1_amp);
    rp_GenFreq(RP_CH_1, out1_freq);
    rp_GenWaveform(RP_CH_1, RP_WAVEFORM_ARBITRARY);
    rp_GenArbWaveform(RP_CH_1, genCh1Data, GEN_BUFFER_LENGTH);

    synthesis_ch2();
    rp_GenOffset(RP_CH_2, 0.f);
    rp_GenAmp(RP_CH_2, out2_amp);
    rp_GenFreq(RP_CH_2, out2_freq);
    rp_GenWaveform(RP_CH_2, RP_WAVEFORM_ARBITRARY);
    rp_GenArbWaveform(RP_CH_2, genCh2Data, GEN_BUFFER_LENGTH);

    rp_GenOutEnable(RP_CH_1);
    rp_GenOutEnable(RP_CH_2);
}

static inline float changeVal(float cur_value, float* change, float min, float max) {
    float value = cur_value + *change;

    if(value <= min) {
        value = min;
        *change = -*change;
    }

    if(value >= max) {
        value = max;
        *change = -*change;
    }
    return value;
}

static inline double _clock() {
    struct timespec tp;
    clock_gettime(CLOCK_REALTIME, &tp);
    return ((double)tp.tv_sec * 1000.f) + ((double)tp.tv_nsec / 1000000.f);
}

void UpdateGen(void) {
    static double timer = _clock() + 100.f;

    if(genEnable.Value() && timer <= _clock()) {
        timer = _clock() + 100.f;

        out1_freq = changeVal(out1_freq, &out1_freq_change, OUT1_FREQ_MIN, OUT1_FREQ_MAX);
        out2_freq = changeVal(out2_freq, &out2_freq_change, OUT2_FREQ_MIN, OUT2_FREQ_MAX);
        rp_GenFreq(RP_CH_1, out1_freq);
        rp_GenFreq(RP_CH_2, out2_freq);

//        out1_amp = changeVal(out1_amp, &out1_amp_change, OUT1_AMP_MIN, OUT1_AMP_MAX);
//        out2_amp = changeVal(out2_amp, &out2_amp_change, OUT2_AMP_MIN, OUT2_AMP_MAX);
//        rp_GenAmp(RP_CH_1, out1_amp);
//        rp_GenAmp(RP_CH_2, out2_amp);
    }
}
