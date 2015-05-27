#include <DataManager.h>
#include <CustomParameters.h>
#include "rpApp.h"

enum { CH_SIGNAL_SIZE = 1024, INTERVAL = 500 };

CFloatSignal ch1("ch1", CH_SIGNAL_SIZE, 0.0f);
CFloatSignal ch2("ch2", CH_SIGNAL_SIZE, 0.0f);

CIntParameter freq_range("freq_range", CBaseParameter::RW, 5, 1, 0, 5); // IN
CIntParameter freq_unit("freq_unit", CBaseParameter::RO, 0, 0, 0, 2);

CFloatParameter peak1_freq("peak1_freq", CBaseParameter::RO, 0, 0, 0, +1e6);
CFloatParameter peak1_power("peak1_power", CBaseParameter::RO, 0, 0, -10000000, +10000000);
CFloatParameter peak1_unit("peak1_unit", CBaseParameter::RO, 0, 1, 0, 2);

CFloatParameter peak2_freq("peak2_freq", CBaseParameter::RO, 0, 0, 0, 1e6);
CFloatParameter peak2_power("peak2_power", CBaseParameter::RO, 0, 0, -1e7, 1e7);
CFloatParameter peak2_unit("peak2_unit", CBaseParameter::RO, 0, 0, 0, 2);

CFloatParameter w_idx("w_idx", CBaseParameter::RO, 0, 0, 0, 1000);
CBooleanParameter en_avg_at_dec("en_avg_at_dec", CBaseParameter::RO, true, 0);

/* WEB GUI Buttons */
CBooleanParameter inReset("SPEC_RST", CBaseParameter::RW, false, 0);
CBooleanParameter inRun("SPEC_RUN", CBaseParameter::RW, false, 0);
CBooleanParameter inAutoscale("SPEC_AUTOSCALE", CBaseParameter::RW, false, 0);
CBooleanParameter inSingle("SPEC_SINGLE", CBaseParameter::RW, false, 0);
CBooleanParameter in1Show("CH1_SHOW", CBaseParameter::RW, true, 0);
CBooleanParameter in2Show("CH2_SHOW", CBaseParameter::RW, false, 0);

void UpdateParams(void)
{
	//rp_EnableDigitalLoop(false);
	CDataManager::GetInstance()->SetParamInterval(INTERVAL);
	rpApp_SpecGetJpgIdx(&w_idx.Value());

	rpApp_SpecGetPeakPower(RP_CH_1, &peak1_power.Value());
	rpApp_SpecGetPeakPower(RP_CH_2, &peak2_power.Value());

	rpApp_SpecGetPeakFreq(RP_CH_1, &peak1_freq.Value());
	rpApp_SpecGetPeakFreq(RP_CH_2, &peak2_freq.Value());
}

void UpdateSignals(void)
{
	CDataManager::GetInstance()->SetSignalInterval(INTERVAL);
	static float data[CH_SIGNAL_SIZE];

	//if (in1Show.Value()) {
		rpApp_SpecGetViewData(RP_CH_1, data, CH_SIGNAL_SIZE);
		if (ch1.GetSize() != CH_SIGNAL_SIZE)
			ch1.Resize(CH_SIGNAL_SIZE);
		for (size_t i = 0; i < CH_SIGNAL_SIZE; i++)
			ch1[i] = data[i];
	/*} else {
		ch1.Resize(0);
	}*/

	//if (in2Show.Value()) {

		rpApp_SpecGetViewData(RP_CH_2, data, CH_SIGNAL_SIZE);
		if (ch2.GetSize() != CH_SIGNAL_SIZE)
			ch2.Resize(CH_SIGNAL_SIZE);
		for (size_t i = 0; i < CH_SIGNAL_SIZE; i++)
			ch2[i] = data[i];

	/*} else {
		ch2.Resize(0);
	}*/
}

void OnNewParams(void)
{
/* ------------------------------ IN PARAMETERS ---------------------------------------- */
	in1Show.Update();
	in2Show.Update();

	//if (inRun.NewValue())
		rpApp_SpecRun();
	//else
	//	rpApp_SpecStop();

	if (freq_range.NewValue() != freq_range.Value()) // user change freq range?
	{
		freq_range.Update();
		rpApp_SpecSetUnit(freq_range.Value()); // set unit by range
	}

	inRun.Update();
}
