#include "main.h"

#include <DataManager.h>
#include <CustomParameters.h>
#include <fstream>  
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <sys/syslog.h>
#include <complex.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <ctime>
#include <stdlib.h>

#include <vector>
#include <algorithm>
#include <thread>
#include <mutex>

#include "rp.h"
#include "version.h"
#include "acq.h"
#include "calib.h"
#include "calib_man.h"

COscilloscope::Ptr g_acq;
CCalib::Ptr        g_calib;
CCalibMan::Ptr     g_calib_man;

#ifdef Z20_250_12
#define DAC_DEVIDER 4.0
#else
#define DAC_DEVIDER 2.0
#endif
//#define DEBUG_MODE


//Parameters

CStringParameter 	redpitaya_model("RP_MODEL_STR", 	CBaseParameter::RO, RP_MODEL, 10);

// Parameters for AUTO mode
CFloatParameter 	ch1_min(		"ch1_min", 			CBaseParameter::ROSA, 0, 0, -1e6f, +1e6f);
CFloatParameter 	ch1_max(		"ch1_max", 			CBaseParameter::ROSA, 0, 0, -1e6f, +1e6f);
CFloatParameter 	ch1_avg(		"ch1_avg", 			CBaseParameter::ROSA, 0, 0, -1e6f, +1e6f);
CFloatParameter 	ch2_min(		"ch2_min", 			CBaseParameter::ROSA, 0, 0, -1e6f, +1e6f);
CFloatParameter 	ch2_max(		"ch2_max", 			CBaseParameter::ROSA, 0, 0, -1e6f, +1e6f);
CFloatParameter 	ch2_avg(		"ch2_avg", 			CBaseParameter::ROSA, 0, 0, -1e6f, +1e6f);
CIntParameter		ch1_calib_pass( "ch1_calib_pass", 	CBaseParameter::RW,   0 ,0,	-2147483647, 2147483647);
CIntParameter		ch2_calib_pass( "ch2_calib_pass", 	CBaseParameter::RW,   0 ,0,	-2147483647, 2147483647);
CFloatParameter 	ref_volt(		"ref_volt",			CBaseParameter::RW,   1, 0, 0.001, 20);
CIntParameter       ss_state(		"SS_STATE", 		CBaseParameter::RW,  -1, 0, -1,100); // Current completed step
CIntParameter       ss_next_step(	"SS_NEXTSTEP",		CBaseParameter::RW,  -1, 0, -2,100); 

// Parameters for MANUAL mode
CIntParameter		ch1_gain_dac( "ch1_gain_dac", 	CBaseParameter::RW,   0 ,0,	0, 2147483647);
CIntParameter		ch2_gain_dac( "ch2_gain_dac", 	CBaseParameter::RW,   0 ,0,	0, 2147483647);
CIntParameter		ch1_off_dac(  "ch1_off_dac", 	CBaseParameter::RW,   0 ,0,	-2147483647, 2147483647);
CIntParameter		ch2_off_dac(  "ch2_off_dac", 	CBaseParameter::RW,   0 ,0,	-2147483647, 2147483647);

CIntParameter		ch1_gain_adc( "ch1_gain_adc", 	CBaseParameter::RW,   0 ,0,	0, 2147483647);
CIntParameter		ch2_gain_adc( "ch2_gain_adc", 	CBaseParameter::RW,   0 ,0,	0, 2147483647);
CIntParameter		ch1_off_adc(  "ch1_off_adc", 	CBaseParameter::RW,   0 ,0,	-2147483647, 2147483647);
CIntParameter		ch2_off_adc(  "ch2_off_adc", 	CBaseParameter::RW,   0 ,0,	-2147483647, 2147483647);

CIntParameter		ch1_gain_dac_new( "ch1_gain_dac_new", 	CBaseParameter::RW,   0 ,0,	0, 2147483647);
CIntParameter		ch2_gain_dac_new( "ch2_gain_dac_new", 	CBaseParameter::RW,   0 ,0,	0, 2147483647);
CIntParameter		ch1_off_dac_new(  "ch1_off_dac_new", 	CBaseParameter::RW,   0 ,0,	-2147483647, 2147483647);
CIntParameter		ch2_off_dac_new(  "ch2_off_dac_new", 	CBaseParameter::RW,   0 ,0,	-2147483647, 2147483647);

CIntParameter		ch1_gain_adc_new( "ch1_gain_adc_new", 	CBaseParameter::RW,   0 ,0,	0, 2147483647);
CIntParameter		ch2_gain_adc_new( "ch2_gain_adc_new", 	CBaseParameter::RW,   0 ,0,	0, 2147483647);
CIntParameter		ch1_off_adc_new(  "ch1_off_adc_new", 	CBaseParameter::RW,   0 ,0,	-2147483647, 2147483647);
CIntParameter		ch2_off_adc_new(  "ch2_off_adc_new", 	CBaseParameter::RW,   0 ,0,	-2147483647, 2147483647);

CIntParameter		calib_sig(    "calib_sig", 	CBaseParameter::RW,   0 ,0,	-2147483647, 2147483647);
CBooleanParameter 	hv_lv_mode(	  "hv_lv_mode", 	        CBaseParameter::RW, false,0);

// GENERATOR SETUP
CBooleanParameter 	gen1_enable(  "gen1_enable", 	        CBaseParameter::RW, false,0);
CBooleanParameter 	gen2_enable(  "gen2_enable", 	        CBaseParameter::RW, false,0);
CIntParameter		gen1_type(    "gen1_type", 				CBaseParameter::RW,   0 ,0,	0, 10);
CIntParameter		gen2_type(    "gen2_type", 				CBaseParameter::RW,   0 ,0,	0, 10);	
CFloatParameter		gen1_offset(  "gen1_offset",			CBaseParameter::RW,   0 ,0,	-1, 1);
CFloatParameter		gen2_offset(  "gen2_offset", 			CBaseParameter::RW,   0 ,0,	-1, 1);	
CFloatParameter		gen1_amp(  	  "gen1_amp",				CBaseParameter::RW,   0.9 ,0,	0.001, 1);
CFloatParameter		gen2_amp(  	  "gen2_amp",	 			CBaseParameter::RW,   0.9 ,0,	0.001, 1);	
CIntParameter		gen1_freq( 	  "gen1_freq",				CBaseParameter::RW,   1000 ,0,	1, DAC_FREQUENCY / DAC_DEVIDER);
CIntParameter		gen2_freq( 	  "gen2_freq",	 			CBaseParameter::RW,   1000 ,0,	1, DAC_FREQUENCY / DAC_DEVIDER);	

#ifdef Z20_250_12
CBooleanParameter 	gen_gain(	  "gen_gain", 		        CBaseParameter::RW, false,0);
CBooleanParameter 	ac_dc_mode(	  "ac_dc_mode", 	        CBaseParameter::RW, false,0);
#endif

#ifdef Z10
CIntParameter		adc_mode(     "adc_acquire_mode", 		CBaseParameter::RW,   0 ,0,	0, 10);
CFloatSignal 		waveSignal(   "wave", 					SCREEN_BUFF_SIZE,     0.0f);
CFloatParameter		cursor_x1(    "cursor_x1",				CBaseParameter::RW,   0.2 ,0,	0, 1);
CFloatParameter		cursor_x2(    "cursor_x2",				CBaseParameter::RW,   0.4 ,0,	0, 1);
CBooleanParameter 	zoom_mode(    "zoom_mode", 	        	CBaseParameter::RW,   false, 0);
#endif

void PrintLogInFile(const char *message){
#ifdef DEBUG_MODE
	std::time_t result = std::time(nullptr);
	std::fstream fs;
  	fs.open ("/tmp/debug.log", std::fstream::in | std::fstream::out | std::fstream::app);
	fs << std::asctime(std::localtime(&result)) << " : " << message << "\n";
	fs.close();
#endif
}


//Application description
const char *rp_app_desc(void)
{
	return (const char *)"Red Pitaya calibration application.\n";
}



//Application init
int rp_app_init(void)
{
	fprintf(stderr, "Loading calibraton app version %s-%s.\n", VERSION_STR, REVISION_STR);
	CDataManager::GetInstance()->SetParamInterval(100);
	PrintLogInFile("rp_app_init");
	rp_Init();
	g_acq = COscilloscope::Create(64);
	g_calib = CCalib::Create(g_acq);
	g_calib_man = CCalibMan::Create(g_acq);
	g_acq->start();
	return 0;
}

//Application exit
int rp_app_exit(void)
{
	g_acq->stop();
	rp_Release();
	fprintf(stderr, "Unloading stream server version %s-%s.\n", VERSION_STR, REVISION_STR);
	PrintLogInFile("Unloading stream server version");
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
	static uint64_t m_old_index = 0; 
	try{
#ifdef Z10
		auto y = g_acq->getDataSq();
		if (y.index != m_old_index){
			if (waveSignal.GetSize() != y.wave_size)
				waveSignal.Resize(y.wave_size);
			for (int i = 0; i < y.wave_size; i++)
    	        waveSignal[i] = y.wave[i];
			m_old_index = y.index;
		}else{
			waveSignal.Resize(0);
		}

#endif
	}catch (std::exception& e)
	{
		fprintf(stderr, "Error: UpdateSignals() %s\n",e.what());
	}
}

void sendCalibInManualMode(bool _force){
	if (ch1_gain_adc.Value() != g_calib_man->getCalibValue(ADC_CH1_GAIN) || _force){
		ch1_gain_adc.SendValue(g_calib_man->getCalibValue(ADC_CH1_GAIN));
	}
	if (ch2_gain_adc.Value() != g_calib_man->getCalibValue(ADC_CH2_GAIN) || _force){
		ch2_gain_adc.SendValue(g_calib_man->getCalibValue(ADC_CH2_GAIN));
	}
	if (ch1_off_adc.Value() != g_calib_man->getCalibValue(ADC_CH1_OFF) || _force){
		ch1_off_adc.SendValue(g_calib_man->getCalibValue(ADC_CH1_OFF));
	}
	if (ch2_off_adc.Value() != g_calib_man->getCalibValue(ADC_CH2_OFF) || _force){
		ch2_off_adc.SendValue(g_calib_man->getCalibValue(ADC_CH2_OFF));
	}

	if (ch1_gain_dac.Value() != g_calib_man->getCalibValue(DAC_CH1_GAIN) || _force){
		ch1_gain_dac.SendValue(g_calib_man->getCalibValue(DAC_CH1_GAIN));
	}
	if (ch2_gain_dac.Value() != g_calib_man->getCalibValue(DAC_CH2_GAIN) || _force){
		ch2_gain_dac.SendValue(g_calib_man->getCalibValue(DAC_CH2_GAIN));
	}
	if (ch1_off_dac.Value() != g_calib_man->getCalibValue(DAC_CH1_OFF) || _force){
		ch1_off_dac.SendValue(g_calib_man->getCalibValue(DAC_CH1_OFF));
	}
	if (ch2_off_dac.Value() != g_calib_man->getCalibValue(DAC_CH2_OFF) || _force){
		ch2_off_dac.SendValue(g_calib_man->getCalibValue(DAC_CH2_OFF));
	}
}

void getNewCalib(){
	bool update = false;
	if (ch1_gain_adc_new.IsNewValue()){
		ch1_gain_adc_new.Update();
		g_calib_man->setCalibValue(ADC_CH1_GAIN,ch1_gain_adc_new.Value());
		update = true;
	}
	if (ch2_gain_adc_new.IsNewValue()){
		ch2_gain_adc_new.Update();
		g_calib_man->setCalibValue(ADC_CH2_GAIN,ch2_gain_adc_new.Value());
		update = true;
	}
	if (ch1_off_adc_new.IsNewValue()){
		ch1_off_adc_new.Update();
		g_calib_man->setCalibValue(ADC_CH1_OFF,ch1_off_adc_new.Value());
		update = true;
	}
	if (ch2_off_adc_new.IsNewValue()){
		ch2_off_adc_new.Update();
		g_calib_man->setCalibValue(ADC_CH2_OFF,ch2_off_adc_new.Value());
		update = true;
	}

	if (ch1_gain_dac_new.IsNewValue()){
		ch1_gain_dac_new.Update();
		g_calib_man->setCalibValue(DAC_CH1_GAIN,ch1_gain_dac_new.Value());
		update = true;
	}
	if (ch2_gain_dac_new.IsNewValue()){
		ch2_gain_dac_new.Update();
		g_calib_man->setCalibValue(DAC_CH2_GAIN,ch2_gain_dac_new.Value());
		update = true;
	}
	if (ch1_off_dac_new.IsNewValue()){
		ch1_off_dac_new.Update();
		g_calib_man->setCalibValue(DAC_CH1_OFF,ch1_off_dac_new.Value());
		update = true;
	}
	if (ch2_off_dac_new.IsNewValue()){
		ch2_off_dac_new.Update();
		g_calib_man->setCalibValue(DAC_CH2_OFF,ch2_off_dac_new.Value());
		update = true;
	}

	if (update){
		g_calib_man->updateCalib();
		g_calib_man->updateGen();
		sendCalibInManualMode(true);
	}
}

void setupGen(){
	if (gen1_enable.IsNewValue()){
		gen1_enable.Update();
		g_calib_man->enableGen(RP_CH_1,gen1_enable.Value());
	}

	if (gen2_enable.IsNewValue()){
		gen2_enable.Update();
		g_calib_man->enableGen(RP_CH_2,gen2_enable.Value());
	}

	if (gen1_type.IsNewValue()){
		gen1_type.Update();
		g_calib_man->setGenType(RP_CH_1,gen1_type.Value());
	}

	if (gen1_offset.IsNewValue()){
		gen1_offset.Update();
		g_calib_man->setOffset(RP_CH_1,gen1_offset.Value());
	}

	if (gen1_freq.IsNewValue()){
		gen1_freq.Update();
		g_calib_man->setFreq(RP_CH_1,gen1_freq.Value());
	}

	if (gen1_amp.IsNewValue()){
		gen1_amp.Update();
		g_calib_man->setAmp(RP_CH_1,gen1_amp.Value());
	}

	if (gen2_type.IsNewValue()){
		gen2_type.Update();
		g_calib_man->setGenType(RP_CH_2,gen2_type.Value());
	}

	if (gen2_offset.IsNewValue()){
		gen2_offset.Update();
		g_calib_man->setOffset(RP_CH_2,gen2_offset.Value());
	}

	if (gen2_freq.IsNewValue()){
		gen2_freq.Update();
		g_calib_man->setFreq(RP_CH_2,gen2_freq.Value());
	}

	if (gen2_amp.IsNewValue()){
		gen2_amp.Update();
		g_calib_man->setAmp(RP_CH_2,gen2_amp.Value());
	}
}

void updateFilterModeParameter(){
	if (cursor_x1.IsNewValue()){
		cursor_x1.Update();
		g_acq->setCursor1(cursor_x1.Value());
	}

	if (cursor_x2.IsNewValue()){
		cursor_x2.Update();
		g_acq->setCursor2(cursor_x2.Value());
	}

	if (zoom_mode.IsNewValue()) {
		zoom_mode.Update();
		g_acq->setZoomMode(zoom_mode.Value());
	}
}

//Update parameters
void UpdateParams(void)
{

	try{
		auto x = g_acq->getData();
		ch1_max.Value() = x.ch1_max;
		ch1_min.Value() = x.ch1_min;
		ch1_avg.Value() = x.ch1_avg;
		ch2_max.Value() = x.ch2_max;
		ch2_min.Value() = x.ch2_min;
		ch2_avg.Value() = x.ch2_avg;

// AUTO MODE 
		if (ss_next_step.IsNewValue() && ref_volt.IsNewValue())
		{
			ss_next_step.Update();
			ref_volt.Update();
			if (ss_next_step.Value() != -2){
				if (g_calib->calib(ss_next_step.Value(),ref_volt.Value()) == RP_OK){
					auto x = g_calib->getCalibData();
					ch1_calib_pass.SendValue(x.ch1);
					ch2_calib_pass.SendValue(x.ch2);
					ss_state.SendValue(ss_next_step.Value()); 
				}
			}else{
				g_calib->restoreCalib();
			}
		}
// MANUAL MODE
		if (calib_sig.IsNewValue()){
			calib_sig.Update();
			int sig = calib_sig.Value();
			calib_sig.Value() = 0; // reset signal
			if (sig == 1){
				g_calib_man->init();
				gen1_freq.SendValue(1000);
				gen2_freq.SendValue(1000);
				gen1_offset.SendValue(0);
				gen2_offset.SendValue(0);
				gen1_type.SendValue(0);
				gen2_type.SendValue(0);
				gen1_amp.SendValue(0.9);
				gen2_amp.SendValue(0.9);
				g_calib_man->readCalibEpprom();
				sendCalibInManualMode(true);
			}

			if (sig == 2){
				sendCalibInManualMode(true);
			}

			if (sig == 3){
				g_calib->resetCalibToZero();
			}

			if (sig == 4){
				g_calib->resetCalibToFactory();
			}

			if (sig == 5){
				g_calib_man->writeCalib();
			}

// FREQ CALIB
#ifdef Z10
			if (sig == 100){
				g_calib_man->initSq();
				g_calib_man->readCalibEpprom();
				cursor_x1.SendValue(0.2);
				cursor_x2.SendValue(0.4);
				zoom_mode.SendValue(false);
			}
#endif		
		}

		if (hv_lv_mode.IsNewValue()){
			hv_lv_mode.Update();
			if (hv_lv_mode.Value()){
				g_calib_man->setModeLV_HV(RP_HIGH);
			}else{
				g_calib_man->setModeLV_HV(RP_LOW);
			}
			sendCalibInManualMode(true);
		}
#ifdef Z20_250_12
		if (ac_dc_mode.IsNewValue()){
			ac_dc_mode.Update();
			if (ac_dc_mode.Value()){
				g_calib_man->setModeAC_DC(RP_AC);
			}else{
				g_calib_man->setModeAC_DC(RP_DC);
			}
			sendCalibInManualMode(true);
		}

		if (gen_gain.IsNewValue()){
			gen_gain.Update();
			if (gen_gain.Value()){
				g_calib_man->setGenGain(RP_GAIN_5X);
			}else{
				g_calib_man->setGenGain(RP_GAIN_1X);
			}
			sendCalibInManualMode(true);
		}
#endif
		getNewCalib();
		setupGen();
#ifdef Z10
		updateFilterModeParameter();
#endif

	}catch (std::exception& e)
	{
		fprintf(stderr, "Error: UpdateParams() %s\n",e.what());
		PrintLogInFile(e.what());
	}

}

void PostUpdateSignals(){}

void OnNewParams(void)
{
	//Update parameters
	UpdateParams();
}

void OnNewSignals(void)
{
	UpdateSignals();
}