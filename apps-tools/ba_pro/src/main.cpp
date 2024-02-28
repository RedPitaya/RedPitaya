
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

#include "common/version.h"
#include "bodeApp.h"

#include "rp_hw-calib.h"
#include "rp_hw-profiles.h"
#include "settings.h"
#include "main.h"

#include "rpApp.h"

/***************************************************************************************
*                                     BODE ANALYSER                                    *
***************************************************************************************/

enum{
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

// Control parameters
CIntParameter       ba_status(          "BA_STATUS",            CBaseParameter::RW, 0, 		0, 		0, 		100);

//Parameters
CIntParameter		ba_start_freq(		"BA_START_FREQ", 		CBaseParameter::RW, 1000, 	0, 		1, 		getMaxADC() , CONFIG_VAR);
CIntParameter		ba_end_freq(		"BA_END_FREQ",			CBaseParameter::RW, 10000, 	0, 		2, 		getMaxADC() , CONFIG_VAR);
CIntParameter		ba_steps(			"BA_STEPS",				CBaseParameter::RW, 25, 	0, 		2, 		3000 , CONFIG_VAR);
CIntParameter		ba_periods_number(	"BA_PERIODS_NUMBER",	CBaseParameter::RW, 8,   	0, 		1, 		8 , CONFIG_VAR);
CIntParameter		ba_averaging(		"BA_AVERAGING", 		CBaseParameter::RW, 1, 		0, 		1, 		10 , CONFIG_VAR);
CFloatParameter 	ba_amplitude(		"BA_AMPLITUDE", 		CBaseParameter::RW, 1, 		0, 		0, 		2000000 , CONFIG_VAR);
CFloatParameter 	ba_dc_bias(			"BA_DC_BIAS", 			CBaseParameter::RW, 0, 		0, 		-1,		1, CONFIG_VAR);
CFloatParameter 	ba_gain_min(		"BA_GAIN_MIN", 			CBaseParameter::RW, -30, 	0, 		-100, 	100, CONFIG_VAR);
CFloatParameter 	ba_gain_max(		"BA_GAIN_MAX", 			CBaseParameter::RW, 10, 	0, 		-100, 	100, CONFIG_VAR);
CFloatParameter 	ba_phase_min(		"BA_PHASE_MIN", 		CBaseParameter::RW, -90, 	0, 		-90, 	90, CONFIG_VAR);
CFloatParameter 	ba_phase_max(		"BA_PHASE_MAX", 		CBaseParameter::RW, 90, 	0, 		-90, 	90, CONFIG_VAR);
CBooleanParameter 	ba_scale(			"BA_SCALE", 			CBaseParameter::RW, true, 	0, CONFIG_VAR);
CBooleanParameter 	ba_auto_scale(		"BA_AUTO_SCALE",    	CBaseParameter::RW, true, 	0, CONFIG_VAR);
CFloatParameter 	ba_input_threshold(	"BA_INPUT_THRESHOLD",	CBaseParameter::RW, 0.001,	0,	    0, 		1 , CONFIG_VAR);
CBooleanParameter 	ba_show_all(		"BA_SHOW_ALL",      	CBaseParameter::RW, true, 	0, CONFIG_VAR);
CIntParameter		ba_logic_mode(		"BA_LOGIC_MODE", 		CBaseParameter::RW, 0, 		0, 		0, 		10 , CONFIG_VAR);

// Status parameters
CStringParameter 	redpitaya_model(	"RP_MODEL_STR", 		CBaseParameter::RO, getModelS(), 0);
CFloatParameter 	ba_current_freq(	"BA_CURRENT_FREQ", 		CBaseParameter::RW, 1, 		0, 		0, 		getMaxADC());
CIntParameter		ba_current_step(	"BA_CURRENT_STEP", 		CBaseParameter::RW, 1, 		0, 		1, 		getMaxADC());
CBooleanParameter 	ba_calibrate_enable("BA_CALIBRATE_ENABLE", 	CBaseParameter::RW, false, 	0);

//Singals
CIntSignal   ba_bad_signal ("BA_BAD_SIGNAL" , CH_SIGNAL_SIZE_DEFAULT, 0);
CFloatSignal ba_signal_1  ("BA_SIGNAL_1"   , CH_SIGNAL_SIZE_DEFAULT, 0.0f);
CFloatSignal ba_signal_2  ("BA_SIGNAL_2"   , CH_SIGNAL_SIZE_DEFAULT, 0.0f);
CIntSignal   ba_signal_parameters ("BA_SIGNAL_PARAMETERS" , 4, 0);

static std::vector<float> signal;
static std::vector<float> phase;
static std::vector<int>   bad_signal;
static std::vector<int>   signal_parameters;

std::thread *g_thread = NULL;
std::mutex   g_signalMutex;
bool         g_exit_flag;

void threadLoop();

auto getModelS() -> std::string{
    rp_HPeModels_t c = STEM_125_14_v1_0;
    if (rp_HPGetModel(&c) != RP_HP_OK){
        ERROR("Can't get board model");
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
        case STEM_250_12_v1_2b:
            return "Z20_250_12";
        case STEM_250_12_120:
            return "Z20_250_12_120";

        default:
            ERROR("Can't get board model");
            exit(-1);
    }
    return "Z10";
}

auto getMaxADC() -> uint32_t{
    rp_HPeModels_t c = STEM_125_14_v1_0;
    int dev = 0;
    if (rp_HPGetModel(&c) != RP_HP_OK){
        ERROR("Can't get board model");
    }

    switch (c)
    {
        case STEM_125_10_v1_0:
        case STEM_125_14_v1_0:
        case STEM_125_14_v1_1:
        case STEM_125_14_LN_v1_1:
        case STEM_125_14_Z7020_v1_0:
        case STEM_125_14_Z7020_LN_v1_1:
            dev = 2;
            break;

        case STEM_122_16SDR_v1_0:
        case STEM_122_16SDR_v1_1:
            dev = 2;
            break;

        case STEM_125_14_Z7020_4IN_v1_0:
        case STEM_125_14_Z7020_4IN_v1_2:
        case STEM_125_14_Z7020_4IN_v1_3:
            dev = 2;
            break;

	    case STEM_250_12_v1_0:
        case STEM_250_12_v1_1:
        case STEM_250_12_v1_2:
        case STEM_250_12_v1_2a:
        case STEM_250_12_v1_2b:
            dev = 4;
            break;
        case STEM_250_12_120:
            dev = 4;
            break;

        default:
            ERROR("Can't get board model");
            exit(-1);
    }
    uint32_t adc = rpApp_BaGetADCSpeed();
    return adc / dev;
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
    rpApp_BaInit();
    rpApp_BaReadCalibration();
    updateParametersByConfig();

    g_thread = new std::thread(threadLoop);
	return 0;
}

//Application exit
int rp_app_exit(void)
{
    g_exit_flag = true;
    if (g_thread){
        g_thread->join();
    }
	rp_Release();
    rpApp_BaRelease();
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
    std::lock_guard<std::mutex> lock(g_signalMutex);
    ba_signal_1.Set(signal);
    ba_signal_2.Set(phase);
	ba_bad_signal.Set(bad_signal);
    ba_signal_parameters.Set(signal_parameters);
}

//Update parameters
void UpdateParams(void)
{
	//Measure start update
	if (ba_status.IsNewValue()) {
		ba_status.Update();
	}

	//Start frequency update
	if (ba_start_freq.IsNewValue())
	{
		ba_start_freq.Update();
	}

	//End frequency update
	if (ba_end_freq.IsNewValue()) {
		ba_end_freq.Update();
	}

	//Steps update
	if (ba_steps.IsNewValue()) {
		ba_steps.Update();
	}

	//Periods number update
	if (ba_periods_number.IsNewValue()) {
		ba_periods_number.Update();
	}

	//Averaging update
	if (ba_averaging.IsNewValue()) {
		ba_averaging.Update();
	}

	//Amplitude update
	if (ba_amplitude.IsNewValue()) {
		ba_amplitude.Update();
	}

	//DC bias update
	if (ba_dc_bias.IsNewValue()) {
		ba_dc_bias.Update();
	}

	//Gain min update
	if (ba_gain_min.IsNewValue()) {
		ba_gain_min.Update();
	}

	//Gain max update
	if (ba_gain_max.IsNewValue()) {
		ba_gain_max.Update();
	}

	//Phase min update
	if (ba_phase_min.IsNewValue()) {
		ba_phase_min.Update();
	}

	//Phase max update
	if (ba_phase_max.IsNewValue()) {
		ba_phase_max.Update();
	}

	//Scale update
	if (ba_scale.IsNewValue()) {
		ba_scale.Update();
	}

    if (ba_logic_mode.IsNewValue()) {
        ba_logic_mode.Update();
    }

	//Scale update
	if (IS_NEW(ba_input_threshold)) {
		ba_input_threshold.Update();
	}

    if (IS_NEW(ba_auto_scale)){
        ba_auto_scale.Update();
    }

    if (IS_NEW(ba_show_all)){
        ba_show_all.Update();
    }

    auto is_calib = rpApp_BaGetCalibStatus();
    if (ba_calibrate_enable.Value() != is_calib){
        ba_calibrate_enable.SendValue(is_calib);
    }

}



void bode_ResetCalib() {
    std::lock_guard<std::mutex> lock(g_signalMutex);
	rpApp_BaResetCalibration();
    rpApp_BaReadCalibration();
	TRACE_SHORT("Calibration reseted");
}


void PostUpdateSignals(){}

void OnNewParams(void) {

    if (ba_status.IsNewValue() ){
        if (ba_status.NewValue() == BA_RESET_CONFIG_SETTINGS){
            TRACE_SHORT("Delete config");
            deleteConfig(getHomeDirectory() + "/.config/redpitaya/apps/ba_pro/config.json");
            ba_status.Update();
            ba_status.SendValue(BA_RESET_CONFIG_SETTINGS_DONE);
            return;
        }
    }

    bool config_changed = isChanged();

	//Update parameters
	UpdateParams();

    if (ba_status.Value() == BA_RESET_CALIB){
        bode_ResetCalib();
        ba_status.SendValue(0);
    }

    if (config_changed) {
        configSet(getHomeDirectory() + "/.config/redpitaya/apps/ba_pro", "config.json");
    }
}

void OnNewSignals(void)
{
	UpdateSignals();
}

void updateParametersByConfig(){
    configGet(getHomeDirectory() + "/.config/redpitaya/apps/ba_pro/config.json");
    CDataManager::GetInstance()->SendAllParams();
}

void threadLoop(){
    g_exit_flag = false;
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
    rp_ba_logic_t logic_mode = RP_BA_LOGIC_TRAP;

    while (!g_exit_flag)
    {
        usleep(1000); // 1 ms


        int status = ba_status.Value();

        // user start calibration
        if (status == BA_START_CALIB || status == BA_START_CALIB_PROCESS){
            if (status == BA_START_CALIB){
                bode_ResetCalib();
                std::lock_guard<std::mutex> lock(g_signalMutex);
                signal.clear();
                phase.clear();
                bad_signal.clear();
                signal_parameters.clear();
                cur_step = 0;

                avaraging = 1;
                start_freq = 100;
                end_freq = getMaxADC();
                steps = 500;
                threshold = ba_input_threshold.Value();
                logic_mode = (rp_ba_logic_t)ba_logic_mode.Value();
                per_number = ba_periods_number.Value();
                gen_ampl = ba_amplitude.Value();
                dc_bias = ba_dc_bias.Value();
                signal_parameters.push_back(start_freq);
                signal_parameters.push_back(end_freq);
                signal_parameters.push_back(steps);
                ba_status.SendValue(BA_START_CALIB_PROCESS);
            }

            if (cur_step < steps){

                float amplitude = 0, phase_out = 0;
                float current_freq = 0.;
                float freq_step = 0;
                float next_freq = 0.;
                bool  low_signal = false;

                if (ba_scale.NewValue()) {
                    // Log
                    auto a = log10f(start_freq);
                    auto b = log10f(end_freq);
                    auto c = (b - a)/(steps - 1);

                    current_freq = pow(10.f, c * cur_step + a);
                    next_freq = pow(10.f, c * (cur_step + 1) + a);
                } else {
                    // Linear
                    freq_step = (end_freq - start_freq) / (steps - 1);
                    current_freq = start_freq + freq_step * cur_step;
                    next_freq = start_freq + freq_step * (cur_step - 1);
                }



                for (int i = 0; i < avaraging; ++i)
                {
                    float ampl_step = 0;
                    float phase_step = 0;
                    rpApp_BaSafeThreadAcqPrepare();
                    auto ret = rpApp_BaGetAmplPhase(logic_mode,gen_ampl, dc_bias, per_number , buffer, &ampl_step, &phase_step, current_freq,threshold);
                    if (ret ==  RP_EOOR) { // isnan && isinf
                        cur_step++;
                        return;
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
                ba_current_step.SendValue(cur_step);
                ba_current_freq.SendValue(current_freq);

                std::lock_guard<std::mutex> lock(g_signalMutex);
                rpApp_BaWriteCalib(current_freq,amplitude,phase_out);
                signal.push_back(rpApp_BaCalibGain(next_freq, amplitude));
                phase.push_back(rpApp_BaCalibPhase(next_freq, phase_out));

                if (low_signal){
                    bad_signal.push_back(1);
                }else{
                    bad_signal.push_back(0);
                }
            }else{
                rpApp_BaReadCalibration();
                ba_calibrate_enable.SendValue(rpApp_BaGetCalibStatus());
                ba_status.SendValue(BA_START_CALIB_DONE);
            }

        }

        if (status == BA_START || status == BA_START_PROCESS){
            if (status == BA_START){
                std::lock_guard<std::mutex> lock(g_signalMutex);
                signal.clear();
                phase.clear();
                bad_signal.clear();
                signal_parameters.clear();
                cur_step = 0;

                avaraging = ba_averaging.Value();
                start_freq = ba_start_freq.Value();
                end_freq = ba_end_freq.Value();
                steps = ba_steps.Value();
                threshold = ba_input_threshold.Value();
                logic_mode = (rp_ba_logic_t)ba_logic_mode.Value();
                per_number = ba_periods_number.Value();
                gen_ampl = ba_amplitude.Value();
                dc_bias = ba_dc_bias.Value();

                signal_parameters.push_back(start_freq);
                signal_parameters.push_back(end_freq);
                signal_parameters.push_back(steps);

                ba_status.SendValue(BA_START_PROCESS);
            }

            if (cur_step < steps){

                float amplitude = 0, phase_out = 0;
                float current_freq = 0.;
                float freq_step = 0;
                float next_freq = 0.;
                bool  low_signal = false;

                if (ba_scale.NewValue()) {
                    // Log
                    auto a = log10f(start_freq);
                    auto b = log10f(end_freq);
                    auto c = (b - a)/(steps - 1);

                    current_freq = pow(10.f, c * cur_step + a);
                    next_freq = pow(10.f, c * (cur_step + 1) + a);
                } else {
                    // Linear
                    freq_step = (end_freq - start_freq) / (steps - 1);
                    current_freq = start_freq + freq_step * cur_step;
                    next_freq = start_freq + freq_step * (cur_step - 1);
                }



                for (int i = 0; i < avaraging; ++i)
                {
                    float ampl_step = 0;
                    float phase_step = 0;
                    rpApp_BaSafeThreadAcqPrepare();
                    auto ret = rpApp_BaGetAmplPhase(logic_mode,gen_ampl, dc_bias, per_number , buffer, &ampl_step, &phase_step, current_freq,threshold);
                    if (ret ==  RP_EOOR) { // isnan && isinf
                        cur_step++;
                        return;
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
                ba_current_step.SendValue(cur_step);
                ba_current_freq.SendValue(current_freq);

                std::lock_guard<std::mutex> lock(g_signalMutex);

                signal.push_back(rpApp_BaCalibGain(next_freq, amplitude));
                phase.push_back(rpApp_BaCalibPhase(next_freq, phase_out));

                if (low_signal){
                    bad_signal.push_back(1);
                }else{
                    bad_signal.push_back(0);
                }
            }else{
                ba_status.SendValue(BA_START_DONE);
            }

        }

    }

}
