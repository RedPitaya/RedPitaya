
#include "main.h"

#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <sys/syslog.h>
#include <complex.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include <vector>
#include <algorithm>
#include <thread>
#include <mutex>

#include "version.h"
#include "bodeApp.h"

#include "rp_hw-calib.h"
#include "rp_hw-profiles.h"

extern "C" {
    #include "rpApp.h"
}

/***************************************************************************************
*                                     BODE ANALYSER                                    *
***************************************************************************************/


//Parameters

CIntParameter parameterPeriod("DEBUG_PARAM_PERIOD", CBaseParameter::RW, 200, 0, 0, 100);
CIntParameter signalPeriod("DEBUG_SIGNAL_PERIOD", CBaseParameter::RW, 100, 0, 0, 10000);

CBooleanParameter 	ba_measure_start(	"BA_MEASURE_START", 	CBaseParameter::RW, false, 	0);
CBooleanParameter 	ba_calibrate_start(	"BA_CALIBRATE_START", 	CBaseParameter::RW, false, 	0);
CBooleanParameter 	ba_calibrate_reset(	"BA_CALIBRATE_RESET", 	CBaseParameter::RW, false, 	0);
CBooleanParameter 	ba_calibrate_enable("BA_CALIBRATE_ENABLE", 	CBaseParameter::RW, false, 	0);
CIntParameter		ba_start_freq(		"BA_START_FREQ", 		CBaseParameter::RW, 1000, 	0, 		1, 		62500000);
CIntParameter		ba_end_freq(		"BA_END_FREQ",			CBaseParameter::RW, 10000, 	0, 		2, 		62500000);
CIntParameter		ba_steps(			"BA_STEPS",				CBaseParameter::RW, 25, 	0, 		2, 		3000);
CIntParameter		ba_periods_number(	"BA_PERIODS_NUMBER",	CBaseParameter::RW, 8,   	0, 		1, 		8);
CIntParameter		ba_averaging(		"BA_AVERAGING", 		CBaseParameter::RW, 1, 		0, 		1, 		10);
CFloatParameter 	ba_amplitude(		"BA_AMPLITUDE", 		CBaseParameter::RW, 1, 		0, 		0, 		2000000);
CFloatParameter 	ba_dc_bias(			"BA_DC_BIAS", 			CBaseParameter::RW, 0, 		0, 		-1,		1);
CFloatParameter 	ba_gain_min(		"BA_GAIN_MIN", 			CBaseParameter::RW, -30, 	0, 		-100, 	100);
CFloatParameter 	ba_gain_max(		"BA_GAIN_MAX", 			CBaseParameter::RW, 10, 	0, 		-100, 	100);
CFloatParameter 	ba_phase_min(		"BA_PHASE_MIN", 		CBaseParameter::RW, -90, 	0, 		-90, 	90);
CFloatParameter 	ba_phase_max(		"BA_PHASE_MAX", 		CBaseParameter::RW, 90, 	0, 		-90, 	90);
CBooleanParameter 	ba_scale(			"BA_SCALE", 			CBaseParameter::RW, true, 	0);
CFloatParameter 	ba_current_freq(	"BA_CURRENT_FREQ", 		CBaseParameter::RW, 1, 		0, 		0, 		62500000);
CIntParameter		ba_current_step(	"BA_CURRENT_STEP", 		CBaseParameter::RW, 1, 		0, 		1, 		62500000);
CFloatParameter 	ba_input_threshold(	"BA_INPUT_THRESHOLD",	CBaseParameter::RW, 0.003,	0,	    0, 		1);
CStringParameter 	redpitaya_model(	"RP_MODEL_STR", 		CBaseParameter::RO, getModelS(), 0);


//Singals
CIntSignal   ba_bad_signal("BA_BAD_SIGNAL" , CH_SIGNAL_SIZE_DEFAULT, 0);
CFloatSignal ba_signal_1  ("BA_SIGNAL_1"   , CH_SIGNAL_SIZE_DEFAULT, 0.0f);
CFloatSignal ba_signal_2  ("BA_SIGNAL_2"   , CH_SIGNAL_SIZE_DEFAULT, 0.0f);

static std::vector<float> signal;
static std::vector<float> phase;
static std::vector<int>   bad_signal;

 auto getModelS() -> std::string{
    rp_HPeModels_t c = STEM_125_14_v1_0;
    if (rp_HPGetModel(&c) != RP_HP_OK){
        fprintf(stderr,"[Error] Can't get board model\n");
    }

    switch (c)
    {
        case STEM_125_10_v1_0:
        case STEM_125_14_v1_0:
        case STEM_125_14_v1_1:
        case STEM_125_14_LN_v1_1:
        case STEM_125_14_Z7020_v1_0:
        case STEM_125_14_Z7020_LN_v1_1:
            return "Z10";

        case STEM_122_16SDR_v1_0:
        case STEM_122_16SDR_v1_1:
            return "Z20";

        case STEM_125_14_Z7020_4IN_v1_0:
        case STEM_125_14_Z7020_4IN_v1_2:
        case STEM_125_14_Z7020_4IN_v1_3:
            return "Z10";

	    case STEM_250_12_v1_0:
        case STEM_250_12_v1_1:
        case STEM_250_12_v1_2:
		case STEM_250_12_v1_2a:
            return "Z20_250_12";
        case STEM_250_12_120:
            return "Z20_250_12_120";

        default:
            fprintf(stderr,"[Error] Can't get board model\n");
            exit(-1);
    }
    return "Z10";
}

//Application description
const char *rp_app_desc(void)
{
	return (const char *)"Red Pitaya Bode analyser application.\n";
}

//Application init
int rp_app_init(void)
{
	fprintf(stderr, "Loading bode analyser version %s-%s.\n", VERSION_STR, REVISION_STR);
	CDataManager::GetInstance()->SetParamInterval(50);
	CDataManager::GetInstance()->SetSignalInterval(50);

	rp_Init();
    rp_AcqSetAC_DC(RP_CH_1,RP_DC);
    rp_AcqSetAC_DC(RP_CH_2,RP_DC);

	return 0;
}

//Application exit
int rp_app_exit(void)
{
	rp_Release();
	fprintf(stderr, "Unloading bode analyser version %s-%s.\n", VERSION_STR, REVISION_STR);
	return 0;
}

//Set parameters
int rp_set_params(rp_app_params_t *p, int len)
{
    return 0;
}

//Get parameters
int rp_get_params(rp_app_params_t **p)
{
    return 0;
}

//Get signals
int rp_get_signals(float ***s, int *sig_num, int *sig_len)
{
    return 0;
}

//Update signals
void UpdateSignals(void)
{
    ba_signal_1.Set(signal);
    ba_signal_2.Set(phase);
	ba_bad_signal.Set(bad_signal);
}

//Update parameters
void UpdateParams(void)
{
	//Measure start update
	if (ba_measure_start.IsNewValue())
	{
		ba_measure_start.Update();
		fprintf(stderr, "ba_measure_start = %d\n", ba_measure_start.Value());
	}

	//Start frequency update
	if (ba_start_freq.IsNewValue())
	{
		ba_start_freq.Update();
		//fprintf(stderr, "ba_start_freq = %d\n", ba_start_freq.Value());
	}

	//End frequency update
	if (ba_end_freq.IsNewValue())
	{
		ba_end_freq.Update();
		fprintf(stderr, "ba_end_freq = %d\n", ba_end_freq.Value());
	}

	//Steps update
	if (ba_steps.IsNewValue())
	{
		ba_steps.Update();
		fprintf(stderr, "ba_steps = %d\n", ba_steps.Value());
	}

	//Periods number update
	if (ba_periods_number.IsNewValue())
	{
		ba_periods_number.Update();
		fprintf(stderr, "ba_periods_number = %d\n", ba_periods_number.Value());
	}

	//Averaging update
	if (ba_averaging.IsNewValue())
	{
		ba_averaging.Update();
		fprintf(stderr, "ba_averaging = %d\n", ba_averaging.Value());
	}

	//Amplitude update
	if (ba_amplitude.IsNewValue())
	{
		ba_amplitude.Update();
		fprintf(stderr, "ba_amplitude = %f\n", ba_amplitude.Value());
	}

	//DC bias update
	if (ba_dc_bias.IsNewValue())
	{
		ba_dc_bias.Update();
		fprintf(stderr, "ba_dc_bias = %f\n", ba_dc_bias.Value());
	}

	//Gain min update
	if (ba_gain_min.IsNewValue())
	{
		ba_gain_min.Update();
		fprintf(stderr, "ba_gain_min = %f\n", ba_gain_min.Value());
	}

	//Gain max update
	if (ba_gain_max.IsNewValue())
	{
		ba_gain_max.Update();
		fprintf(stderr, "ba_gain_max = %f\n", ba_gain_max.Value());
	}

	//Phase min update
	if (ba_phase_min.IsNewValue())
	{
		ba_phase_min.Update();
		fprintf(stderr, "ba_phase_min = %f\n", ba_phase_min.Value());
	}

	//Phase max update
	if (ba_phase_max.IsNewValue())
	{
		ba_phase_max.Update();
		fprintf(stderr, "ba_phase_max = %f\n", ba_phase_max.Value());
	}

	//Scale update
	if (ba_scale.IsNewValue())
	{
		ba_scale.Update();
		fprintf(stderr, "ba_scale = %d\n", static_cast<int>(ba_scale.Value()));
	}

	//Scale update
	if (IS_NEW(ba_input_threshold))
	{
		ba_input_threshold.Update();
		fprintf(stderr, "ba_input_threshold = %d\n", static_cast<float>(ba_input_threshold.Value()));
	}

}



void bode_ResetCalib()
{
	rpApp_BaResetCalibration();
	ba_calibrate_reset.SendValue(false);
	fprintf(stderr, "Calibration reseted\n");
}


void PostUpdateSignals(){}

void OnNewParams(void)
{
	parameterPeriod.Update();
	signalPeriod.Update();

	//Update parameters
	UpdateParams();

    static float freq_step = 0;
    static int steps = 0;
    static float old_freq = 0;
    static int old_steps = 0;

	static float a, b, c;
	static int cur_step = 0;
	static bool first_step = true;
	static rp_ba_buffer_t buffer(ADC_BUFFER_SIZE);

    ba_calibrate_start.Update();

    if (!ba_measure_start.NewValue() && !ba_calibrate_start.NewValue())
    {
    	first_step = true;
        steps = ba_steps.NewValue();
        cur_step = 0;

		if (ba_scale.NewValue())
		{
			b = log10f(ba_end_freq.NewValue());
			a = log10f(ba_start_freq.NewValue());
			c = (b - a)/(steps - 1);
		}
		else
		{
			freq_step = (static_cast<float>(ba_end_freq.NewValue()) - static_cast<float>(ba_start_freq.NewValue())) / (ba_steps.NewValue() - 1);
		}
		//fprintf(stderr, "a %f b %f c %f\n", a, b, c);

        old_freq = ba_start_freq.NewValue();
        old_steps = ba_steps.NewValue();

		signal.clear();
		phase.clear();
		bad_signal.clear();

		// If reset calibration
		if (ba_calibrate_reset.NewValue())
		{
			bode_ResetCalib();
		}

        rpApp_BaReadCalibration();
		ba_calibrate_enable.SendValue(rpApp_BaGetCalibStatus());
    }
    else if (!ba_measure_start.NewValue() && ba_calibrate_start.NewValue()) // user start calibration
    {
        rpApp_BaResetCalibration(); // clear old calibrations

        // default settings
        ba_start_freq.SendValue(100);
        ba_end_freq.SendValue(125e6/2);
        ba_steps.SendValue(500);
        ba_periods_number.SendValue(2);
        ba_averaging.SendValue(1);
        steps = 500;
		old_freq = 100;
        old_steps = 500;

        if (ba_scale.NewValue())
		{
			b = log10f(ba_end_freq.NewValue());
			a = log10f(ba_start_freq.NewValue());
			c = (b - a)/(steps - 1);
		}
		else
		{
			freq_step = (static_cast<float>(ba_end_freq.NewValue()) - static_cast<float>(ba_start_freq.NewValue())) / (ba_steps.NewValue() - 1);
		}
		//fprintf(stderr, "a %f b %f c %f\n", a, b, c);

        // start meas
        ba_measure_start.SendValue(true);
    }
	else if (ba_measure_start.NewValue()) // START
	{
		if (first_step == true)
		{
			ba_start_freq.Update();
	    	ba_end_freq.Update();
	    	ba_steps.Update();

	    	steps = ba_steps.NewValue();
	        cur_step = 0;

			if (ba_scale.NewValue())
			{
				b = log10f(ba_end_freq.NewValue());
				a = log10f(ba_start_freq.NewValue());
				c = (b - a)/(steps - 1);
			}
			else
			{
				freq_step = (static_cast<float>(ba_end_freq.NewValue()) - static_cast<float>(ba_start_freq.NewValue())) / (ba_steps.NewValue() - 1);
			}

			//fprintf(stderr, "a %f b %f c %f\n", a, b, c);

	        old_freq = ba_start_freq.NewValue();
	        old_steps = ba_steps.NewValue();

	    	first_step = false;
			rpApp_BaSafeThreadAcqPrepare();
	    }

        if (steps <= 0)
        {
            ba_calibrate_start.SendValue(false);
            ba_measure_start.SendValue(false);

            signal.clear();
            phase.clear();
			bad_signal.clear();

            ba_start_freq.SendValue(old_freq);
            ba_steps.SendValue(old_steps);

            return;
        }

        float amplitude = 0, phase_out = 0;
		float current_freq = 0.;
		float next_freq = 0.;
		int   ret = RP_OK;

		if (ba_scale.NewValue()) {
			// Log
			current_freq = pow(10.f, c * cur_step + a);
			next_freq = pow(10.f, c * (cur_step + 1) + a);
		} else {
			// Linear
			current_freq = static_cast<float>(old_freq) + freq_step * cur_step;
			next_freq = static_cast<float>(old_freq) + freq_step * (cur_step + 1);
		}

		for (int i = 0; i < ba_averaging.NewValue(); ++i)
        {
            float ampl_out = 0;
			float threshold = ba_calibrate_start.NewValue() ? 0 : ba_input_threshold.NewValue();
			ret = rpApp_BaGetAmplPhase(ba_amplitude.NewValue(), ba_dc_bias.NewValue(),ba_periods_number.NewValue(), buffer, &ampl_out, &phase_out, current_freq,threshold);
            if (ret ==  RP_EOOR) // isnan && isinf
            {
                // if error - next step
				ba_start_freq.SendValue(next_freq);
                ba_steps.SendValue(--steps);
                return;
            }
			ba_current_freq.Set(current_freq);
            amplitude += ampl_out;
        }
        amplitude /= ba_averaging.NewValue();

        ba_steps.SendValue(--steps);
        cur_step++;
        ba_current_step.Set(cur_step);
		ba_start_freq.SendValue(next_freq);
        if (ba_calibrate_start.NewValue()){ // save data in calibration mode
			rpApp_BaWriteCalib(current_freq,amplitude,phase_out);
			// signal.push_back(amplitude);
			// phase.push_back(phase_out);
			signal.push_back(rpApp_BaCalibGain(next_freq, amplitude));
			phase.push_back(rpApp_BaCalibPhase(next_freq, phase_out));
		}
		else{
			signal.push_back(rpApp_BaCalibGain(next_freq, amplitude));
			phase.push_back(rpApp_BaCalibPhase(next_freq, phase_out));
		}
		if (ret == RP_OK){
	        bad_signal.push_back(0);
		}else{
			bad_signal.push_back(1);
		}

	}
}

void OnNewSignals(void)
{
	UpdateSignals();
}
