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


enum { CH_SIGNAL_SIZE = 1024, INTERVAL = 100 };
enum { FREQ_CHANNEL = -1 };

CFloatSignal ch1("ch1", CH_SIGNAL_SIZE, 0.0f);
CFloatSignal ch2("ch2", CH_SIGNAL_SIZE, 0.0f);
//CFloatSignal freq_ch("freq_ch", CH_SIGNAL_SIZE, 0.0f);

CIntParameter freq_range("freq_range", CBaseParameter::RW, 0, 1, 0, 5);
CIntParameter freq_unit("freq_unit", CBaseParameter::RWSA, 2, 0, 0, 2);

//power 
CFloatParameter in1Scale("SPEC_CH1_SCALE", CBaseParameter::RW, 10, 0, 0, 1000);
CFloatParameter in2Scale("SPEC_CH2_SCALE", CBaseParameter::RW, 10, 0, 0, 1000);

CFloatParameter inFreqScale("SPEC_TIME_SCALE", CBaseParameter::RW, 1, 0, 0, 50000);
CFloatParameter xmin("xmin", CBaseParameter::RW, 0, 0, -1e6, +1e6);
CFloatParameter xmax("xmax", CBaseParameter::RW, 63, 0, -1e6, +1e6);


CFloatParameter peak1_freq("peak1_freq", CBaseParameter::ROSA, 0, 0, 0, +1e6);
CFloatParameter peak1_power("peak1_power", CBaseParameter::ROSA, 0, 0, -10000000, +10000000);
CFloatParameter peak1_unit("peak1_unit", CBaseParameter::ROSA, 0, 1, 0, 2);

CFloatParameter peak2_freq("peak2_freq", CBaseParameter::ROSA, 0, 0, 0, 1e6);
CFloatParameter peak2_power("peak2_power", CBaseParameter::ROSA, 0, 0, -1e7, 1e7);
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

void UpdateParams(void)
{
	inRun.Update();
	if (inRun.Value() == false)
		return;

	int ret = rpApp_SpecGetJpgIdx(&w_idx.Value());
	ret = rpApp_SpecGetPeakPower(RP_CH_1, &peak1_power.Value());
	ret = rpApp_SpecGetPeakPower(RP_CH_2, &peak2_power.Value());

	ret = rpApp_SpecGetPeakFreq(RP_CH_1, &peak1_freq.Value());
	ret = rpApp_SpecGetPeakFreq(RP_CH_2, &peak2_freq.Value());
	rp_EnableDigitalLoop(false); // IsDemoParam.Value()); // FIXME
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

	if (in1Show.Value()) {
		if (ch1.GetSize() != CH_SIGNAL_SIZE)
			ch1.Resize(CH_SIGNAL_SIZE);
		for (size_t i = 0; i < CH_SIGNAL_SIZE; i++)
			ch1[i] = data[1][i*2];
	}
	else if (ch1.GetSize() == CH_SIGNAL_SIZE)
		ch1.Resize(0);

	if (in2Show.Value()) {
		if (ch2.GetSize() != CH_SIGNAL_SIZE)
			ch2.Resize(CH_SIGNAL_SIZE);
		for (size_t i = 0; i < CH_SIGNAL_SIZE; i++)
			ch2[i] = data[2][i*2];
	}
	else if (ch2.GetSize() == CH_SIGNAL_SIZE)
		ch2.Resize(0);
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
		fprintf(stderr, "Interval Init\n");
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

	if (xmax.IsNewValue() || freq_unit.IsNewValue())
	{
		freq_unit.Update();
		xmax.Update();

		float max_freq = xmax.Value();
		for (int i = 0; i < freq_unit.Value(); ++i)
			max_freq *= 1000; // set x max freq (Hz, kHz or MHz) by unit

		rpApp_SpecSetUnit(freq_unit.Value());
		rpApp_SpecSetFreqRange(max_freq);
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
    rpApp_Init();
/*
    if(rp_spectr_worker_init() < 0) {
        return -1;
    }

    rp_set_params(&rp_main_params[0], PARAMS_NUM);

    rp_spectr_worker_change_state(rp_spectr_auto_state);
*/
    return 0;
}

extern "C" int rp_app_exit(void)
{
    fprintf(stderr, "Unloading spectrum version %s-%s.\n", VERSION_STR, REVISION_STR);
	rpApp_SpecStop();

    return 0;
}

extern "C" const char *rp_app_desc(void) {
    fprintf(stderr, "spec desc\n");
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

