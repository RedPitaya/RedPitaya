#include "main.h"

#include <DataManager.h>
#include <CustomParameters.h>
#include <fstream>  
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
#ifdef Z20_250_12
#include "rp-spi.h"
#include "rp-i2c-max7311.h"
#endif


#if defined Z10 || defined Z20_125
#include "filter_logic.h"
#include "filter_logic2ch.h"
CFilter_logic::Ptr g_filter_logic;
CFilter_logic2ch::Ptr g_filter_logic2ch;
int g_sub_progress = 0;
#endif

COscilloscope::Ptr g_acq;
CCalib::Ptr        g_calib;
CCalibMan::Ptr     g_calib_man;


#ifdef Z20_250_12
#define DAC_DEVIDER 4.0
#else
#define DAC_DEVIDER 2.0
#endif
//#define DEBUG_MODE

#define DEFAULT_CURSOR_1 0.2
#define DEFAULT_CURSOR_2 0.4
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
CBooleanParameter 	gen1_enable(  "gen1_enable", 	        CBaseParameter::RW,   false,0);
CBooleanParameter 	gen2_enable(  "gen2_enable", 	        CBaseParameter::RW,   false,0);
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

#if defined Z10 || defined Z20_125
CIntParameter		adc_mode(     		 "adc_acquire_mode", 		CBaseParameter::RW,   0 ,0,	0, 10);
CFloatSignal 		waveSignal(   		 "wave", 					SCREEN_BUFF_SIZE,     0.0f);
CFloatParameter		cursor_x1(    		 "cursor_x1",				CBaseParameter::RW,   DEFAULT_CURSOR_1 ,0,	0, 1);
CFloatParameter		cursor_x2(    		 "cursor_x2",				CBaseParameter::RW,   DEFAULT_CURSOR_2 ,0,	0, 1);
CBooleanParameter 	zoom_mode(    		 "zoom_mode", 	        	CBaseParameter::RW,   false, 0);
CIntParameter		adc_decimation(      "adc_decimation", 			CBaseParameter::RW,   8 ,0,	1, 65535);
CIntParameter		adc_channel(     	 "adc_channel", 			CBaseParameter::RW,   0 ,0,	0, 1);
CBooleanParameter 	filter_hv_lv_mode(	 "filter_hv_lv_mode", 	    CBaseParameter::RW,   false,0);
CFloatParameter		adc_hyst(    		 "adc_hyst",				CBaseParameter::RW,   0.05 ,0,	0, 1);
CBooleanParameter 	filt_gen1_enable(	 "filt_gen1_enable", 		CBaseParameter::RW,   false,0);
CBooleanParameter 	filt_gen2_enable(	 "filt_gen2_enable", 		CBaseParameter::RW,   false,0);
CFloatParameter		filt_gen_offset(  	 "filt_gen_offset",			CBaseParameter::RW,   0 	,0,	-1, 1);
CFloatParameter		filt_gen_amp(  		 "filt_gen_amp",			CBaseParameter::RW,   0.9   ,0,	0.001, 1);
CFloatParameter		filt_gen_freq(  	 "filt_gen_freq",			CBaseParameter::RW,   1000  ,0,	1, DAC_FREQUENCY / DAC_DEVIDER);
CIntParameter		filt_aa(	     	 "filt_aa", 				CBaseParameter::RW,   0 ,0,	0, 0x3FFFF);
CIntParameter		filt_bb(	     	 "filt_bb", 				CBaseParameter::RW,   0 ,0,	0, 0x1FFFFFF);
CIntParameter		filt_pp(	     	 "filt_pp", 				CBaseParameter::RW,   0 ,0,	0, 0x1FFFFFF);
CIntParameter		filt_kk(	     	 "filt_kk", 				CBaseParameter::RW,   0 ,0,	0, 0x1FFFFFF);
CIntParameter		filt_calib_step(	 "filt_calib_step",			CBaseParameter::RW,   0 ,0,	0, 100000);
CIntParameter		filt_calib_progress( "filt_calib_progress",		CBaseParameter::RW,   0 ,0,	0, 100);
CIntParameter		filt_calib_auto_mode("filt_calib_auto_mode",	CBaseParameter::RW,   0 ,0,	0, 1);
CFloatParameter		filt_calib_ref_amp(	 "filt_calib_ref_amp",		CBaseParameter::RW,   0.9   ,0,	0.001, 20);

// AUTO MODE FOR FILTER CALIB
CBooleanParameter 	f_external_gen(	"f_external_gen", 		CBaseParameter::RW,   false,0);
CFloatParameter 	f_ref_volt(		"f_ref_volt",			CBaseParameter::RW,   1, 0, 0.001, 20);
CIntParameter       f_ss_state(		"F_SS_STATE", 			CBaseParameter::RW,  -1, 0, -1,100); // Current completed step
CIntParameter       f_ss_next_step(	"F_SS_NEXTSTEP",		CBaseParameter::RW,  -1, 0, -2,100);

CIntParameter		fauto_aa_Ch1(	     	 "fauto_aa_Ch1", 				CBaseParameter::RW,   0 ,0,	0, 0x1FFFFFF);
CIntParameter		fauto_bb_Ch1(	     	 "fauto_bb_Ch1", 				CBaseParameter::RW,   0 ,0,	0, 0x1FFFFFF);
CIntParameter		fauto_pp_Ch1(	     	 "fauto_pp_Ch1", 				CBaseParameter::RW,   0 ,0,	0, 0x1FFFFFF);
CIntParameter		fauto_kk_Ch1(	     	 "fauto_kk_Ch1", 				CBaseParameter::RW,   0 ,0,	0, 0x1FFFFFF);

CIntParameter		fauto_aa_Ch2(	     	 "fauto_aa_Ch2", 				CBaseParameter::RW,   0 ,0,	0, 0x1FFFFFF);
CIntParameter		fauto_bb_Ch2(	     	 "fauto_bb_Ch2", 				CBaseParameter::RW,   0 ,0,	0, 0x1FFFFFF);
CIntParameter		fauto_pp_Ch2(	     	 "fauto_pp_Ch2", 				CBaseParameter::RW,   0 ,0,	0, 0x1FFFFFF);
CIntParameter		fauto_kk_Ch2(	     	 "fauto_kk_Ch2", 				CBaseParameter::RW,   0 ,0,	0, 0x1FFFFFF);

CFloatParameter 	fauto_value_ch1_before(	 "fauto_value_ch1_before", 			CBaseParameter::RW, 0, 0, -1e6f, +1e6f);
CFloatParameter 	fauto_value_ch2_before(	 "fauto_value_ch2_before", 			CBaseParameter::RW, 0, 0, -1e6f, +1e6f);
CFloatParameter 	fauto_value_ch1_after(	 "fauto_value_ch1_after", 			CBaseParameter::RW, 0, 0, -1e6f, +1e6f);
CFloatParameter 	fauto_value_ch2_after(	 "fauto_value_ch2_after", 			CBaseParameter::RW, 0, 0, -1e6f, +1e6f);
CIntParameter		fauto_calib_progress( 	 "fauto_calib_progress",		CBaseParameter::RW,   0 ,0,	0, 120);
#endif

void PrintLogInFile(const char *message){
#ifdef DEBUG_MODE
	std::time_t result = std::time(nullptr);
	std::fstream fs;
  	fs.open ("/tmp/debug2.log", std::fstream::in | std::fstream::out | std::fstream::app);
	fs << std::asctime(std::localtime(&result)) << " : " << message << "\n";
	fs.close();
#endif
}

void sendFilterCalibValues(rp_channel_t _ch);

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
	rp_Init();
	g_acq = COscilloscope::Create(64);
	g_calib = CCalib::Create(g_acq);
	g_calib_man = CCalibMan::Create(g_acq);
#if defined Z10 || defined Z20_125
	g_filter_logic = CFilter_logic::Create(g_calib_man);
	g_filter_logic2ch = CFilter_logic2ch::Create(g_calib_man);
	g_acq->setCursor1(cursor_x1.Value());
	g_acq->setCursor2(cursor_x2.Value());
#endif
#ifdef Z20_250_12
    rp_spi_fpga::rp_spi_load_via_fpga("/opt/redpitaya/lib/configs/AD9613BCPZ-250.xml");
    rp_spi_fpga::rp_spi_load_via_fpga("/opt/redpitaya/lib/configs/AD9746BCPZ-250.xml");
#endif
	g_acq->start();
	return 0;
}

//Application exit
int rp_app_exit(void)
{
	g_acq->stop();
	rp_Release();
	fprintf(stderr, "Unloading stream server version %s-%s.\n", VERSION_STR, REVISION_STR);
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
#if defined Z10 || defined Z20_125
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

#if defined Z10 || defined Z20_125

void getNewFilterCalib(){
        bool update = false;
        if (filt_aa.IsNewValue()){
                filt_aa.Update();
                g_calib_man->setCalibValue(adc_channel.Value() == 0 ? F_AA_CH1 : F_AA_CH2  ,filt_aa.Value());
                update = true;
        }

        if (filt_bb.IsNewValue()){
                filt_bb.Update();
                g_calib_man->setCalibValue(adc_channel.Value() == 0 ? F_BB_CH1 : F_BB_CH2  ,filt_bb.Value());
                update = true;
        }

        if (filt_pp.IsNewValue()){
                filt_pp.Update();
                g_calib_man->setCalibValue(adc_channel.Value() == 0 ? F_PP_CH1 : F_PP_CH2  ,filt_pp.Value());
                update = true;
        }

        if (filt_kk.IsNewValue()){
                filt_kk.Update();
                g_calib_man->setCalibValue(adc_channel.Value() == 0 ? F_KK_CH1 : F_KK_CH2  ,filt_kk.Value());
                update = true;
        }

        if (update){
                g_calib_man->updateCalib();
                g_calib_man->updateAcqFilter(adc_channel.Value() == 0 ? RP_CH_1 : RP_CH_2);
                sendFilterCalibValues(adc_channel.Value() == 0 ? RP_CH_1 : RP_CH_2);
        }
}

void setupGenFilter(){
	if (filt_gen1_enable.IsNewValue()){
		filt_gen1_enable.Update();
		g_calib_man->enableGen(RP_CH_1,filt_gen1_enable.Value());
	}

	if (filt_gen2_enable.IsNewValue()){
		filt_gen2_enable.Update();
		g_calib_man->enableGen(RP_CH_2,filt_gen2_enable.Value());
	}

	if (filt_gen_freq.IsNewValue()){
		filt_gen_freq.Update();
		g_calib_man->setFreq(RP_CH_1,filt_gen_freq.Value());
		g_calib_man->setFreq(RP_CH_2,filt_gen_freq.Value());
	}

	if (filt_gen_amp.IsNewValue()){
		filt_gen_amp.Update();
		g_calib_man->setAmp(RP_CH_1,filt_gen_amp.Value());
		g_calib_man->setAmp(RP_CH_2,filt_gen_amp.Value());
	}

	if (filt_gen_offset.IsNewValue()){
		filt_gen_offset.Update();
		g_calib_man->setOffset(RP_CH_1,filt_gen_offset.Value());
		g_calib_man->setOffset(RP_CH_2,filt_gen_offset.Value());
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

	if (adc_decimation.IsNewValue()){
		adc_decimation.Update();
		g_calib_man->changeDecimation(adc_decimation.Value());
	}

	if (adc_channel.IsNewValue()){
		adc_channel.Update();
		g_calib_man->changeChannel(adc_channel.Value() == 0 ? RP_CH_1 : RP_CH_2 );
		sendFilterCalibValues(adc_channel.Value() == 0 ? RP_CH_1 : RP_CH_2);
	}

	if (filter_hv_lv_mode.IsNewValue()){
		filter_hv_lv_mode.Update();
		g_calib_man->setModeLV_HV(filter_hv_lv_mode.Value() ? RP_HIGH :RP_LOW);
		sendFilterCalibValues(adc_channel.Value() == 0 ? RP_CH_1 : RP_CH_2);
	}

	if (adc_hyst.IsNewValue()){
		adc_hyst.Update();
		g_acq->setHyst(adc_hyst.Value());
	}

	if(filt_calib_auto_mode.IsNewValue()){
		filt_calib_auto_mode.Update();
		g_filter_logic->setCalibMode(filt_calib_auto_mode.Value());
	}

	if(filt_calib_ref_amp.IsNewValue()){
		filt_calib_ref_amp.Update();
		g_filter_logic->setCalibRef(filt_calib_ref_amp.Value());
	}
}

void calibFilter(){
	if (filt_calib_step.Value() == 1){
		g_filter_logic->init(adc_channel.Value() == 0 ? RP_CH_1 : RP_CH_2);
		g_calib_man->setOffset(RP_CH_1,0);
		g_calib_man->setFreq(RP_CH_1,1000);
		g_calib_man->setAmp(RP_CH_1,0.9);	
		g_calib_man->setOffset(RP_CH_2,0);
		g_calib_man->setFreq(RP_CH_2,1000);
		g_calib_man->setAmp(RP_CH_2,0.9);	
		g_calib_man->enableGen(RP_CH_1,true);
		g_calib_man->enableGen(RP_CH_2,true);
		g_acq->startAutoFilter(8);
		filt_calib_step.SendValue(2);
		fauto_calib_progress.SendValue(0);
		return;
	}

	if (filt_calib_step.Value() == 2) {
		while (g_filter_logic->setCalibParameters() != -1){
            auto dp = g_acq->getDataAutoFilter();
            if (dp.is_valid == true) {
                g_filter_logic->setCalculatedValue(dp);                
            }
        }
        g_filter_logic->removeHalfCalib();
        if (g_filter_logic->nextSetupCalibParameters() == -1) {
			filt_calib_step.SendValue(3);
			return;
		}
		filt_calib_progress.SendValue(g_filter_logic->calcProgress());
	}

	if (filt_calib_step.Value() == 3){
		g_filter_logic->setGoodCalibParameter();
		std::this_thread::sleep_for(std::chrono::microseconds(1000000));
		while(1){
			auto d = g_acq->getDataAutoFilter();
			if (d.ampl > 0){
				float nominal = 0.9;
				if (filt_calib_auto_mode.Value() == 0) {
					nominal = filt_calib_ref_amp.Value();
				}

				if (g_filter_logic->calibPP(d, nominal) != 0) break;
			}
		}
		filt_calib_step.SendValue(4);
	}

	if (filt_calib_step.Value() == 4){
		filt_calib_step.SendValue(100);
		sendFilterCalibValues(adc_channel.Value() == 0 ? RP_CH_1 : RP_CH_2);
		g_acq->setHyst(adc_hyst.Value());
		g_calib_man->changeDecimation(adc_decimation.Value());		
		g_calib_man->setGenType(RP_CH_1,(int)RP_WAVEFORM_SQUARE);
    	g_calib_man->setGenType(RP_CH_2,(int)RP_WAVEFORM_SQUARE);    
		g_calib_man->enableGen(RP_CH_1,filt_gen1_enable.Value());
		g_calib_man->enableGen(RP_CH_2,filt_gen2_enable.Value());
		g_calib_man->setOffset(RP_CH_1,filt_gen_offset.Value());
		g_calib_man->setFreq(RP_CH_1,filt_gen_freq.Value());
		g_calib_man->setAmp(RP_CH_1,filt_gen_amp.Value());	
		g_calib_man->setOffset(RP_CH_2,filt_gen_offset.Value());
		g_calib_man->setFreq(RP_CH_2,filt_gen_freq.Value());
		g_calib_man->setAmp(RP_CH_2,filt_gen_amp.Value());
	}

}

void calibAutoFilter(){

	if (f_ref_volt.IsNewValue()){
		f_ref_volt.Update();
	}

	if (f_external_gen.IsNewValue()){
		f_external_gen.Update();
	}

	if (f_ss_next_step.Value() == 0){
		fauto_value_ch1_before.Value() = 0;
		fauto_value_ch1_before.Value() = 0;
		fauto_value_ch1_after.Value() = 0;
		fauto_value_ch2_after.Value() = 0;
		g_sub_progress = 0;
		g_calib_man->setModeLV_HV( RP_LOW );
		g_filter_logic2ch->init();
		g_calib_man->setOffset(RP_CH_1,0);
		g_calib_man->setFreq(RP_CH_1,1000);
		g_calib_man->setAmp(RP_CH_1,0.9);	
		g_calib_man->setOffset(RP_CH_2,0);
		g_calib_man->setFreq(RP_CH_2,1000);
		g_calib_man->setAmp(RP_CH_2,0.9);	
		g_calib_man->setGenType(RP_CH_1,(int)RP_WAVEFORM_SQUARE);
    	g_calib_man->setGenType(RP_CH_2,(int)RP_WAVEFORM_SQUARE);  
		g_calib_man->enableGen(RP_CH_1,true);
		g_calib_man->enableGen(RP_CH_2,true);
		g_acq->startAutoFilter2Ch(8);
		g_acq->updateAcqFilter(RP_CH_1);
    	g_acq->updateAcqFilter(RP_CH_2);
		f_ss_state.SendValue(f_ss_next_step.Value());
		f_ss_next_step.SendValue(-1);
		return;
	}

	if (f_ss_next_step.Value() == 1) {
		if (g_sub_progress == 0){
			auto d = g_acq->getDataAutoFilter2Ch();
			auto v1 = (d.valueCH1.calib_value_raw   + d.valueCH1.deviation );
			auto v2 = (d.valueCH2.calib_value_raw   + d.valueCH2.deviation );
			fauto_value_ch1_before.SendValue(v1);
			fauto_value_ch2_before.SendValue(v2);
			fauto_aa_Ch1.SendValue(0);
			fauto_bb_Ch1.SendValue(0);
			fauto_pp_Ch1.SendValue(0);
			fauto_kk_Ch1.SendValue(0);

			fauto_aa_Ch2.SendValue(0);
			fauto_bb_Ch2.SendValue(0);
			fauto_pp_Ch2.SendValue(0);
			fauto_kk_Ch2.SendValue(0);
			g_sub_progress = 1;
			return;
		}

		if (g_sub_progress == 1){
			while (g_filter_logic2ch->setCalibParameters() != -1){
				auto dp = g_acq->getDataAutoFilter2Ch();
				g_filter_logic2ch->setCalculatedValue(dp);                
			}
			g_filter_logic2ch->removeHalfCalib();
			if (g_filter_logic2ch->nextSetupCalibParameters() == -1){		
				g_sub_progress = 2;
				return;
			}
			fauto_calib_progress.SendValue(g_filter_logic2ch->calcProgress());
		}

		if (g_sub_progress == 2){
			g_filter_logic2ch->setGoodCalibParameterCh1();
			std::this_thread::sleep_for(std::chrono::microseconds(1000000));
			auto ext = f_external_gen.Value();
			auto volt_ref = f_ref_volt.Value();
			if (!ext) volt_ref = 0.9;
			while(1){
        		auto d = g_acq->getDataAutoFilter2Ch();
        		if (g_filter_logic2ch->calibPPCh1(d,volt_ref) != 0) break;
    		}
			fauto_calib_progress.SendValue(110);
			g_sub_progress = 3;
			return;
		}

		if (g_sub_progress == 3){
			g_filter_logic2ch->setGoodCalibParameterCh2();
			std::this_thread::sleep_for(std::chrono::microseconds(1000000));
			auto ext = f_external_gen.Value();
			auto volt_ref = f_ref_volt.Value();
			if (!ext) volt_ref = 0.9;
			while(1){
        		auto d = g_acq->getDataAutoFilter2Ch();
        		if (g_filter_logic2ch->calibPPCh2(d,volt_ref) != 0) break;
    		}
			fauto_calib_progress.SendValue(120);
			g_sub_progress = 4;
			return;
		}

		if (g_sub_progress == 4){
			std::this_thread::sleep_for(std::chrono::microseconds(1000000));
			auto d = g_acq->getDataAutoFilter2Ch();
			auto v1 = (d.valueCH1.calib_value_raw   + d.valueCH1.deviation );
			auto v2 = (d.valueCH2.calib_value_raw   + d.valueCH2.deviation );
			fauto_value_ch1_after.SendValue(v1);
			fauto_value_ch2_after.SendValue(v2);
			fauto_calib_progress.SendValue(0);
			fauto_aa_Ch1.SendValue(d.valueCH1.f_aa);
			fauto_bb_Ch1.SendValue(d.valueCH1.f_bb);
			fauto_pp_Ch1.SendValue(d.valueCH1.f_pp);
			fauto_kk_Ch1.SendValue(d.valueCH1.f_kk);

			fauto_aa_Ch2.SendValue(d.valueCH2.f_aa);
			fauto_bb_Ch2.SendValue(d.valueCH2.f_bb);
			fauto_pp_Ch2.SendValue(d.valueCH2.f_pp);
			fauto_kk_Ch2.SendValue(d.valueCH2.f_kk);
			
			g_sub_progress = 0;
			f_ss_state.SendValue(f_ss_next_step.Value());
			f_ss_next_step.SendValue(-1);
			return;
		}
		
	}

	if (f_ss_next_step.Value() == 2){
			fauto_value_ch1_before.Value() = 0;
			fauto_value_ch1_before.Value() = 0;
			fauto_value_ch1_after.Value() = 0;
			fauto_value_ch2_after.Value() = 0;
			g_sub_progress = 0;
			g_calib_man->setModeLV_HV( RP_HIGH );
			g_filter_logic2ch->init();
			g_calib_man->setOffset(RP_CH_1,0);
			g_calib_man->setFreq(RP_CH_1,1000);
			g_calib_man->setAmp(RP_CH_1,0.9);	
			g_calib_man->setOffset(RP_CH_2,0);
			g_calib_man->setFreq(RP_CH_2,1000);
			g_calib_man->setAmp(RP_CH_2,0.9);	
			g_calib_man->setGenType(RP_CH_1,(int)RP_WAVEFORM_SQUARE);
	    	g_calib_man->setGenType(RP_CH_2,(int)RP_WAVEFORM_SQUARE);    
			g_calib_man->enableGen(RP_CH_1,true);
			g_calib_man->enableGen(RP_CH_2,true);
			g_acq->startAutoFilter2Ch(8);
			g_acq->updateAcqFilter(RP_CH_1);
			g_acq->updateAcqFilter(RP_CH_2);
			f_ss_state.SendValue(f_ss_next_step.Value());
			f_ss_next_step.SendValue(-1);
			return;
		}

	if (f_ss_next_step.Value() == 3) {
			if (g_sub_progress == 0){
				auto d = g_acq->getDataAutoFilter2Ch();
				auto v1 = (d.valueCH1.calib_value_raw   + d.valueCH1.deviation );
				auto v2 = (d.valueCH2.calib_value_raw   + d.valueCH2.deviation );
				fauto_value_ch1_before.SendValue(v1);
				fauto_value_ch2_before.SendValue(v2);
				fauto_aa_Ch1.SendValue(0);
				fauto_bb_Ch1.SendValue(0);
				fauto_pp_Ch1.SendValue(0);
				fauto_kk_Ch1.SendValue(0);

				fauto_aa_Ch2.SendValue(0);
				fauto_bb_Ch2.SendValue(0);
				fauto_pp_Ch2.SendValue(0);
				fauto_kk_Ch2.SendValue(0);
				g_sub_progress = 1;
				return;
			}

			if (g_sub_progress == 1){
				while (g_filter_logic2ch->setCalibParameters() != -1){
					auto dp = g_acq->getDataAutoFilter2Ch();
					g_filter_logic2ch->setCalculatedValue(dp);                
				}
				g_filter_logic2ch->removeHalfCalib();
				if (g_filter_logic2ch->nextSetupCalibParameters() == -1){		
					g_sub_progress = 2;
					return;
				}
				fauto_calib_progress.SendValue(g_filter_logic2ch->calcProgress());
			}

			if (g_sub_progress == 2){
				g_filter_logic2ch->setGoodCalibParameterCh1();
				std::this_thread::sleep_for(std::chrono::microseconds(1000000));
				auto ext = f_external_gen.Value();
				auto volt_ref = f_ref_volt.Value();
				if (!ext) volt_ref = 0.9;
				while(1){
					auto d = g_acq->getDataAutoFilter2Ch();
					if (g_filter_logic2ch->calibPPCh1(d,volt_ref) != 0) break;
				}
				fauto_calib_progress.SendValue(110);
				g_sub_progress = 3;
				return;
			}

			if (g_sub_progress == 3){
				g_filter_logic2ch->setGoodCalibParameterCh2();
				std::this_thread::sleep_for(std::chrono::microseconds(1000000));
				auto ext = f_external_gen.Value();
				auto volt_ref = f_ref_volt.Value();
				if (!ext) volt_ref = 0.9;
				while(1){
					auto d = g_acq->getDataAutoFilter2Ch();
					if (g_filter_logic2ch->calibPPCh2(d,volt_ref) != 0) break;
				}
				fauto_calib_progress.SendValue(120);
				g_sub_progress = 4;
				return;
			}

			if (g_sub_progress == 4){
				std::this_thread::sleep_for(std::chrono::microseconds(1000000));
				auto d = g_acq->getDataAutoFilter2Ch();
				auto v1 = (d.valueCH1.calib_value_raw   + d.valueCH1.deviation );
				auto v2 = (d.valueCH2.calib_value_raw   + d.valueCH2.deviation );
				fauto_value_ch1_after.SendValue(v1);
				fauto_value_ch2_after.SendValue(v2);
				fauto_calib_progress.SendValue(0);
				fauto_aa_Ch1.SendValue(d.valueCH1.f_aa);
				fauto_bb_Ch1.SendValue(d.valueCH1.f_bb);
				fauto_pp_Ch1.SendValue(d.valueCH1.f_pp);
				fauto_kk_Ch1.SendValue(d.valueCH1.f_kk);

				fauto_aa_Ch2.SendValue(d.valueCH2.f_aa);
				fauto_bb_Ch2.SendValue(d.valueCH2.f_bb);
				fauto_pp_Ch2.SendValue(d.valueCH2.f_pp);
				fauto_kk_Ch2.SendValue(d.valueCH2.f_kk);
				
				g_sub_progress = 0;
				f_ss_state.SendValue(f_ss_next_step.Value());
				f_ss_next_step.SendValue(-1);
				return;
			}

		}
	
	if (f_ss_next_step.Value() == 4){
			fauto_value_ch1_before.Value() = 0;
			fauto_value_ch1_before.Value() = 0;
			fauto_value_ch1_after.Value() = 0;
			fauto_value_ch2_after.Value() = 0;
			g_calib_man->writeCalib();
			g_calib_man->enableGen(RP_CH_1,false);
			g_calib_man->enableGen(RP_CH_2,false);
			f_ss_state.SendValue(f_ss_next_step.Value());
			f_ss_next_step.SendValue(-1);
			return;
		}

}

void sendFilterCalibValues(rp_channel_t _ch){
	filt_aa.SendValue(g_calib_man->getCalibValue(_ch == RP_CH_1 ? F_AA_CH1 : F_AA_CH2));
	filt_bb.SendValue(g_calib_man->getCalibValue(_ch == RP_CH_1 ? F_BB_CH1 : F_BB_CH2));
	filt_pp.SendValue(g_calib_man->getCalibValue(_ch == RP_CH_1 ? F_PP_CH1 : F_PP_CH2));
	filt_kk.SendValue(g_calib_man->getCalibValue(_ch == RP_CH_1 ? F_KK_CH1 : F_KK_CH2));
}
#endif

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

#if defined Z10 || defined Z20_125
// FILTER AUTO MODE 
		if (f_ss_next_step.IsNewValue())
		{
			f_ss_next_step.Update();
		}

		calibAutoFilter();
#endif

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
				g_calib_man->readCalibEpprom();
				sendCalibInManualMode(true);
			}

			if (sig == 4){
				g_calib->resetCalibToFactory();
				g_calib_man->readCalibEpprom();
				sendCalibInManualMode(true);
			}

			if (sig == 5){
				g_calib_man->writeCalib();
			}



// FREQ CALIB
#if defined Z10 || defined Z20_125

			if (sig == 6){
				g_calib_man->setDefualtFilter(adc_channel.Value() == 0 ? RP_CH_1 : RP_CH_2);
  			 	g_calib_man->updateCalib();
				g_calib_man->updateAcqFilter(adc_channel.Value() == 0 ? RP_CH_1 : RP_CH_2);
				sendFilterCalibValues(adc_channel.Value() == 0 ? RP_CH_1 : RP_CH_2);
			}

			if (sig == 100){
				g_calib_man->initSq(adc_decimation.Value());
				adc_decimation.SendValue(adc_decimation.Value());
				g_calib_man->readCalibEpprom();
				cursor_x1.SendValue(DEFAULT_CURSOR_1);
				cursor_x2.SendValue(DEFAULT_CURSOR_2);
				zoom_mode.SendValue(false);
				filter_hv_lv_mode.SendValue(false);
				adc_channel.SendValue(RP_CH_1);
				adc_hyst.SendValue(0.05);
				g_calib_man->setOffset(RP_CH_1,filt_gen_offset.Value());
				g_calib_man->setFreq(RP_CH_1,filt_gen_freq.Value());
				g_calib_man->setAmp(RP_CH_1,filt_gen_amp.Value());	
				g_calib_man->setOffset(RP_CH_2,filt_gen_offset.Value());
				g_calib_man->setFreq(RP_CH_2,filt_gen_freq.Value());
				g_calib_man->setAmp(RP_CH_2,filt_gen_amp.Value());	
				filt_gen_freq.SendValue(filt_gen_freq.Value());
				filt_gen_amp.SendValue(filt_gen_amp.Value());
				filt_gen_offset.SendValue(filt_gen_offset.Value());
				sendFilterCalibValues(adc_channel.Value() == 0 ? RP_CH_1 : RP_CH_2);
			}	
#endif
		}

		if (hv_lv_mode.IsNewValue()){
			hv_lv_mode.Update();
			g_calib_man->setModeLV_HV(hv_lv_mode.Value() ? RP_HIGH :RP_LOW);		
#if defined Z10 || defined Z20_125
		
			g_calib_man->updateAcqFilter(adc_channel.Value() == 0 ? RP_CH_1 : RP_CH_2);	
#endif			
			sendCalibInManualMode(true);
		}
#ifdef Z20_250_12
		if (ac_dc_mode.IsNewValue()){
			ac_dc_mode.Update();
			g_calib_man->setModeAC_DC(ac_dc_mode.Value() ? RP_AC : RP_DC);		
			sendCalibInManualMode(true);
		}

		if (gen_gain.IsNewValue()){
			gen_gain.Update();
			g_calib_man->setGenGain(gen_gain.Value() ? RP_GAIN_5X : RP_GAIN_1X);
			sendCalibInManualMode(true);
		}
#endif
		getNewCalib();
		setupGen();
#if defined Z10 || defined Z20_125
		if (filt_calib_step.IsNewValue() || (filt_calib_step.Value() >= 1 && filt_calib_step.Value() <= 100)){
			filt_calib_step.Update();
			calibFilter();
		}
		setupGenFilter();
		updateFilterModeParameter();
		getNewFilterCalib();
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
