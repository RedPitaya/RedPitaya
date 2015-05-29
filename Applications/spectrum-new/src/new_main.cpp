#include <DataManager.h>
#include <CustomParameters.h>
#include "rpApp.h"

enum { CH_SIGNAL_SIZE = 1024, INTERVAL = 200 };

CFloatSignal ch1("ch1", CH_SIGNAL_SIZE, 0.0f);
CFloatSignal ch2("ch2", CH_SIGNAL_SIZE, 0.0f);

CIntParameter freq_range("freq_range", CBaseParameter::RW, 0, 1, 0, 5); // IN
CIntParameter freq_unit("freq_unit", CBaseParameter::RWSA, 0, 0, 0, 2);

//power 
CFloatParameter in1Scale("SPEC_CH1_SCALE", CBaseParameter::RW, 10, 0, 0, 1000);
CFloatParameter in2Scale("SPEC_CH2_SCALE", CBaseParameter::RW, 10, 0, 0, 1000);

CFloatParameter inFreqScale("SPEC_TIME_SCALE", CBaseParameter::RW, 1, 0, 0, 50000);
CFloatParameter xmin("xmin", CBaseParameter::RW, 0, 0, -1e6, +1e6);
CFloatParameter xmax("xmax", CBaseParameter::RW, 1000, 0, -1e6, +1e6);


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

void UpdateParams(void)
{
/*
	static bool set_period = false;
	if (!set_period)
	{
		CDataManager::GetInstance()->SetParamInterval(INTERVAL);
		CDataManager::GetInstance()->SetSignalInterval(INTERVAL);
		set_period = true;
	}
*/

	int ret = rpApp_SpecGetJpgIdx(&w_idx.Value());
	ret = rpApp_SpecGetPeakPower(RP_CH_1, &peak1_power.Value());
	ret = rpApp_SpecGetPeakPower(RP_CH_2, &peak2_power.Value());

	ret = rpApp_SpecGetPeakFreq(RP_CH_1, &peak1_freq.Value());
	ret = rpApp_SpecGetPeakFreq(RP_CH_2, &peak2_freq.Value());
}

void UpdateSignals(void)
{
	static float data[CH_SIGNAL_SIZE];


	int ret = -1;
	if (in1Show.Value())
	{
		ret = rpApp_SpecGetViewData(RP_CH_1, data, CH_SIGNAL_SIZE);
		if (!ret)
		{
			for (size_t i = 0; i < CH_SIGNAL_SIZE; i++)
				ch1[i] = data[i];
		}
	}

	if (in2Show.Value())
	{
		ret = rpApp_SpecGetViewData(RP_CH_2, data, CH_SIGNAL_SIZE);
		if (!ret)
		{
			for (size_t i = 0; i < CH_SIGNAL_SIZE; i++)
				ch2[i] = data[i];
		}
	}
}

void OnNewParams(void)
{
	static bool run = false;
	if (!run)
	{
		rpApp_SpecRun();
		run = true;
	}

	if (xmax.IsNewValue())
	{
		xmax.Update();
		//fprintf(stderr, "xmax = %f\n", xmax.Value());
	}
}

extern "C" void SpecIntervalInit()
{
	CDataManager::GetInstance()->SetParamInterval(INTERVAL);
	CDataManager::GetInstance()->SetSignalInterval(INTERVAL);

	fprintf(stderr, "Interval Init\n");
}
