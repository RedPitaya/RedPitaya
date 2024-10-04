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
#include "common/version.h"
#include "acq.h"
#include "calib.h"
#include "calib_man.h"

#include "rp-spi.h"
#include "rp-i2c-max7311.h"

#include "rp_hw-calib.h"
#include "rp_hw-profiles.h"


#include "filter_logic.h"
#include "filter_logicNch.h"

#include "web/rp_client.h"

CFilter_logic::Ptr g_filter_logic = nullptr;
CFilter_logicNch::Ptr g_filter_logicNch  = nullptr;
int g_sub_progress = 0;

COscilloscope::Ptr g_acq  = nullptr;
CCalib::Ptr        g_calib  = nullptr;
CCalibMan::Ptr     g_calib_man = nullptr;
std::mutex	g_mtx;


//#define DEBUG_MODE

#define DAC_DEVIDER getDACDevider()

#define CURSORS_COUNT 2

#define XSTR(s) STR(s)
#define STR(s) #s

#define INIT2(PREF,SUFF,args...) {{PREF "1" SUFF,args},{PREF "2" SUFF,args}}

#define INIT(PREF,SUFF,args...) {{PREF "1" SUFF,args},{PREF "2" SUFF,args},{PREF "3" SUFF,args},{PREF "4" SUFF,args}}

#define DEFAULT_CURSOR_1 0.2
#define DEFAULT_CURSOR_2 0.4
//Parameters

CStringParameter 	redpitaya_model("RP_MODEL_STR", 	CBaseParameter::RO, getModelName(), 10);

// Parameters for AUTO mode
CFloatParameter 	ch_min[MAX_ADC_CHANNELS] 		= INIT("ch","_min",	CBaseParameter::ROSA, 0, 0, -1e6f, +1e6f);
CFloatParameter 	ch_max[MAX_ADC_CHANNELS] 		= INIT("ch","_max",	CBaseParameter::ROSA, 0, 0, -1e6f, +1e6f);
CFloatParameter 	ch_avg[MAX_ADC_CHANNELS] 		= INIT("ch","_avg",	CBaseParameter::ROSA, 0, 0, -1e6f, +1e6f);
CFloatParameter 	ch_p_p[MAX_ADC_CHANNELS] 		= INIT("ch","_p_p",	CBaseParameter::ROSA, 0, 0, -1e6f, +1e6f);
CFloatParameter 	ch_per_buff[MAX_ADC_CHANNELS] 	= INIT("ch","_perBuff",	CBaseParameter::ROSA, 0, 0, -1e6f, +1e6f);
CBooleanParameter 	ch_is_sine[MAX_ADC_CHANNELS] 	= INIT("ch","_issine",	CBaseParameter::ROSA, false, 0);

CIntParameter		ch_calib_pass[MAX_ADC_CHANNELS] = INIT("ch","_calib_pass", CBaseParameter::RW,   0 ,0,	-2147483647, 2147483647);

CFloatParameter 	ref_volt("ref_volt",				CBaseParameter::RW,   1, 0, 0.001, 20);
CIntParameter       ss_state("SS_STATE", 			CBaseParameter::RW,  -1, 0, -1,100); // Current completed step
CIntParameter       ss_next_step("SS_NEXTSTEP",		CBaseParameter::RW,  -1, 0, -2,100);

// Parameters for MANUAL mode
CFloatParameter		ch_gain_dac[MAX_DAC_CHANNELS]	= INIT2("ch","_gain_dac", 	CBaseParameter::RW,   1.0 ,0,	0.5, 1.5);
CIntParameter		ch_off_dac[MAX_DAC_CHANNELS]	= INIT2("ch","_off_dac", 	CBaseParameter::RW,   0 ,0,	-16382, 16382);

CFloatParameter		ch_gain_adc[MAX_ADC_CHANNELS]	= INIT("ch","_gain_adc", 	CBaseParameter::RW,   1.0 ,0,	0.5, 1.5);
CIntParameter		ch_off_adc[MAX_ADC_CHANNELS] 	= INIT("ch","_off_adc", 	CBaseParameter::RW,   0 ,0,	-16382, 16382);

CFloatParameter		ch_gain_dac_new[MAX_DAC_CHANNELS] = INIT2("ch","_gain_dac_new", 	CBaseParameter::RW,   1.0 ,0,	0.5, 1.5);
CIntParameter		ch_off_dac_new[MAX_DAC_CHANNELS]  = INIT2("ch","_off_dac_new", 	CBaseParameter::RW,   0 ,0,	-16382, 16382);

CFloatParameter		ch_gain_adc_new[MAX_ADC_CHANNELS]	= INIT("ch","_gain_adc_new", 	CBaseParameter::RW,   1.0 ,0,	0.5, 1.5);
CIntParameter		ch_off_adc_new[MAX_ADC_CHANNELS] 	= INIT("ch","_off_adc_new", 	CBaseParameter::RW,   0 ,0,	-16382, 16382);

CIntParameter		calib_sig(    "calib_sig", 	CBaseParameter::RW,   0 ,0,	-2147483647, 2147483647);
CBooleanParameter 	hv_lv_mode(	  "hv_lv_mode", 	        CBaseParameter::RW, false,0);


CBooleanParameter 	gen_enable[MAX_DAC_CHANNELS]	= INIT2( "gen","_enable", 	        CBaseParameter::RW,   false,0);
CIntParameter		gen_type[MAX_DAC_CHANNELS]		= INIT2( "gen","_type", 				CBaseParameter::RW,   0 ,0,	0, 10);
CFloatParameter		gen_offset[MAX_DAC_CHANNELS]	= INIT2( "gen","_offset",			CBaseParameter::RW,   0 ,0,	-1, 1);
CFloatParameter		gen_amp[MAX_DAC_CHANNELS]		= INIT2( "gen","_amp",				CBaseParameter::RW,   0.9 ,0,	0.001, 1);
CIntParameter		gen_freq[MAX_DAC_CHANNELS]		= INIT2( "gen","_freq",				CBaseParameter::RW,   1000 ,0,	1, (int)(getDACRate() / DAC_DEVIDER));


CBooleanParameter 	gen_gain(	  "gen_gain", 		        CBaseParameter::RW, false,0);
CBooleanParameter 	ac_dc_mode(	  "ac_dc_mode", 	        CBaseParameter::RW, false,0);
CBooleanParameter 	avg_last_mode(	  "avg_last_mode", 	        CBaseParameter::RW, true,0);
CBooleanParameter 	request_reset(	  "request_reset_avg_filter", 	        CBaseParameter::RW, false,0);

CIntParameter		man_decimation( "manual_decimation", 				CBaseParameter::RW,   64 ,0,	1, 4096);


CIntParameter		adc_mode(     		 "adc_acquire_mode", 		CBaseParameter::RW,   0 ,0,	0, 10);
CFloatSignal 		waveSignal(   		 "wave", 					SCREEN_BUFF_SIZE,     0.0f);
CFloatParameter		cursor_x1(    		 "cursor_x1",				CBaseParameter::RW,   DEFAULT_CURSOR_1 ,0,	0, 1);
CFloatParameter		cursor_x2(    		 "cursor_x2",				CBaseParameter::RW,   DEFAULT_CURSOR_2 ,0,	0, 1);
CBooleanParameter 	zoom_mode(    		 "zoom_mode", 	        	CBaseParameter::RW,   false, 0);
CIntParameter		adc_decimation(      "adc_decimation", 			CBaseParameter::RW,   8 ,0,	1, 65535);
CIntParameter		adc_channel(     	 "adc_channel", 			CBaseParameter::RW,   0 ,0,	0, MAX_ADC_CHANNELS-1);
CBooleanParameter 	filter_hv_lv_mode(	 "filter_hv_lv_mode", 	    CBaseParameter::RW,   false,0);
CFloatParameter		adc_hyst(    		 "adc_hyst",				CBaseParameter::RW,   0.05 ,0,	0, 1);
CBooleanParameter 	filt_gen1_enable(	 "filt_gen1_enable", 		CBaseParameter::RW,   false,0);
CBooleanParameter 	filt_gen2_enable(	 "filt_gen2_enable", 		CBaseParameter::RW,   false,0);
CFloatParameter		filt_gen_offset(  	 "filt_gen_offset",			CBaseParameter::RW,   0 	,0,	-1, 1);
CFloatParameter		filt_gen_amp(  		 "filt_gen_amp",			CBaseParameter::RW,   0.9   ,0,	0.001, 1);
CFloatParameter		filt_gen_freq(  	 "filt_gen_freq",			CBaseParameter::RW,   1000  ,0,	1, getDACRate()  / DAC_DEVIDER);
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

CIntParameter		fauto_aa_Ch[MAX_ADC_CHANNELS] = INIT(	     	 "fauto_aa_Ch","", 				CBaseParameter::RW,   0 ,0,	0, 0x1FFFFFF);
CIntParameter		fauto_bb_Ch[MAX_ADC_CHANNELS] = INIT(	     	 "fauto_bb_Ch","", 				CBaseParameter::RW,   0 ,0,	0, 0x1FFFFFF);
CIntParameter		fauto_pp_Ch[MAX_ADC_CHANNELS] = INIT(	     	 "fauto_pp_Ch","", 				CBaseParameter::RW,   0 ,0,	0, 0x1FFFFFF);
CIntParameter		fauto_kk_Ch[MAX_ADC_CHANNELS] = INIT(	     	 "fauto_kk_Ch","", 				CBaseParameter::RW,   0 ,0,	0, 0x1FFFFFF);

CFloatParameter 	fauto_value_ch_before[MAX_ADC_CHANNELS] = INIT( "fauto_value_ch","_before",	CBaseParameter::RW, 0, 0, -1e6f, +1e6f);
CFloatParameter 	fauto_value_ch_after[MAX_ADC_CHANNELS]  = INIT( "fauto_value_ch","_after", CBaseParameter::RW, 0, 0, -1e6f, +1e6f);
CIntParameter		fauto_calib_progress( 	 "fauto_calib_progress", CBaseParameter::RW,   0 ,0,	0, 140);

void sendFilterCalibValues(rp_channel_t _ch);

//Application description
const char *rp_app_desc(void)
{
	return (const char *)"Red Pitaya calibration application.\n";
}



//Application init
int rp_app_init(void)
{
	std::lock_guard<std::mutex> lock(g_mtx);
	fprintf(stderr, "Loading calibraton app version %s-%s.\n", VERSION_STR, REVISION_STR);
	rp_WC_Init();
	CDataManager::GetInstance()->SetParamInterval(100);
	rp_Init();
	auto dec = 64;
	g_acq = COscilloscope::Create(dec);
	g_acq->setAvgFilter(avg_last_mode.Value());
	g_calib = CCalib::Create(g_acq);
	g_calib_man = CCalibMan::Create(g_acq);

	if (rp_HPGetFastADCIsFilterPresentOrDefault()){
		g_filter_logic = CFilter_logic::Create(g_calib_man);
		g_filter_logicNch = CFilter_logicNch::Create(g_calib_man);
		g_acq->setCursor1(cursor_x1.Value());
		g_acq->setCursor2(cursor_x2.Value());
	}

	g_acq->start();
	return 0;
}

//Application exit
int rp_app_exit(void)
{
	std::lock_guard<std::mutex> lock(g_mtx);
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
		if (rp_HPGetFastADCIsFilterPresentOrDefault()){
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
		}
	}catch (std::exception& e)
	{
		ERROR("UpdateSignals() %s",e.what());
	}
}

void sendCalibInManualMode(bool _force){
	if (!g_calib_man) return;

	for(auto i = 0u; i < getADCChannels(); i++){
		if (ch_gain_adc[i].Value() != g_calib_man->getCalibValue((rp_channel_t)i, ADC_CH_GAIN) || _force){
			ch_gain_adc[i].SendValue(g_calib_man->getCalibValue((rp_channel_t)i, ADC_CH_GAIN));
		}

		if (ch_off_adc[i].Value() != g_calib_man->getCalibValue((rp_channel_t)i, ADC_CH_OFF) || _force){
			ch_off_adc[i].SendValue(g_calib_man->getCalibValue((rp_channel_t)i, ADC_CH_OFF));
		}
	}

	for(auto i = 0u; i < getDACChannels(); i++){
		if (ch_gain_dac[i].Value() != g_calib_man->getCalibValue((rp_channel_t)i, DAC_CH_GAIN) || _force){
			ch_gain_dac[i].SendValue(g_calib_man->getCalibValue((rp_channel_t)i, DAC_CH_GAIN));
		}

		if (ch_off_dac[i].Value() != g_calib_man->getCalibValue((rp_channel_t)i, DAC_CH_OFF) || _force){
			ch_off_dac[i].SendValue(g_calib_man->getCalibValue((rp_channel_t)i, DAC_CH_OFF));
		}
	}
}

void getNewCalib(){
	if (!g_calib_man) return;

	bool update = false;
	for(auto i = 0u; i < getADCChannels(); i++){
		if (ch_gain_adc_new[i].IsNewValue()){
			ch_gain_adc_new[i].Update();
			g_calib_man->setCalibValue((rp_channel_t)i, ADC_CH_GAIN,ch_gain_adc_new[i].Value());
			update = true;
		}

		if (ch_off_adc_new[i].IsNewValue()){
			ch_off_adc_new[i].Update();
			g_calib_man->setCalibValue((rp_channel_t)i, ADC_CH_OFF,ch_off_adc_new[i].Value());
			update = true;
		}
	}

	for(auto i = 0u; i < getDACChannels(); i++){
		if (ch_gain_dac_new[i].IsNewValue()){
			ch_gain_dac_new[i].Update();
			g_calib_man->setCalibValue((rp_channel_t)i,DAC_CH_GAIN,ch_gain_dac_new[i].Value());
			update = true;
		}
		if (ch_off_dac_new[i].IsNewValue()){
			ch_off_dac_new[i].Update();
			g_calib_man->setCalibValue((rp_channel_t)i,DAC_CH_OFF,ch_off_dac_new[i].Value());
			update = true;
		}
	}

	if (update){
		g_calib_man->updateCalib();
		if (getDACChannels() > 0)
			g_calib_man->updateGen();

		sendCalibInManualMode(true);
	}
}


void setupGen(){
	if (!g_calib_man) return;
	for(auto i = 0u; i < getDACChannels(); i++){
		if (gen_enable[i].IsNewValue()){
			gen_enable[i].Update();
			g_calib_man->enableGen((rp_channel_t)i,gen_enable[i].Value());
		}
		if (gen_type[i].IsNewValue()){
			gen_type[i].Update();
			g_calib_man->setGenType((rp_channel_t)i,gen_type[i].Value());
		}
		if (gen_offset[i].IsNewValue()){
			gen_offset[i].Update();
			g_calib_man->setOffset((rp_channel_t)i,gen_offset[i].Value());
		}
		if (gen_freq[i].IsNewValue()){
			gen_freq[i].Update();
			g_calib_man->setFreq((rp_channel_t)i,gen_freq[i].Value());
		}
		if (gen_amp[i].IsNewValue()){
			gen_amp[i].Update();
			g_calib_man->setAmp((rp_channel_t)i,gen_amp[i].Value());
		}
	}
}




void getNewFilterCalib(){
	if(!rp_HPGetFastADCIsFilterPresentOrDefault()) return;

	if (!g_calib_man) return;
        bool update = false;
        if (filt_aa.IsNewValue()){
                filt_aa.Update();
                g_calib_man->setCalibValue((rp_channel_t)adc_channel.Value(), F_AA_CH, filt_aa.Value());
                update = true;
        }

        if (filt_bb.IsNewValue()){
                filt_bb.Update();
                g_calib_man->setCalibValue((rp_channel_t)adc_channel.Value(), F_BB_CH, filt_bb.Value());
                update = true;
        }

        if (filt_pp.IsNewValue()){
                filt_pp.Update();
                g_calib_man->setCalibValue((rp_channel_t)adc_channel.Value(), F_PP_CH, filt_pp.Value());
                update = true;
        }

        if (filt_kk.IsNewValue()){
                filt_kk.Update();
                g_calib_man->setCalibValue((rp_channel_t)adc_channel.Value(), F_KK_CH, filt_kk.Value());
                update = true;
        }

        if (update){
                g_calib_man->updateCalib();
                g_calib_man->updateAcqFilter((rp_channel_t)adc_channel.Value());
                sendFilterCalibValues((rp_channel_t)adc_channel.Value());
        }
}

void setupGenFilter(){
	if (getDACChannels() == 0) return;
	if (!g_calib_man) return;
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
	if (!rp_HPGetFastADCIsFilterPresentOrDefault()) return;
	if (!g_acq || !g_calib_man) return;
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
		g_calib_man->changeChannel((rp_channel_t)adc_channel.Value());
		sendFilterCalibValues((rp_channel_t)adc_channel.Value());
	}

	if (filter_hv_lv_mode.IsNewValue()){
		filter_hv_lv_mode.Update();
		g_calib_man->setModeLV_HV(filter_hv_lv_mode.Value() ? RP_HIGH :RP_LOW);
		sendFilterCalibValues((rp_channel_t)adc_channel.Value());
	}

	if (adc_hyst.IsNewValue()){
		adc_hyst.Update();
		g_acq->setHyst(adc_hyst.Value());
	}

	if(filt_calib_auto_mode.IsNewValue() && g_filter_logic){
		filt_calib_auto_mode.Update();
		g_filter_logic->setCalibMode(filt_calib_auto_mode.Value());
	}

	if(filt_calib_ref_amp.IsNewValue() && g_filter_logic){
		filt_calib_ref_amp.Update();
		g_filter_logic->setCalibRef(filt_calib_ref_amp.Value());
	}
}

void calibFilter(){
	if (!g_filter_logic || !g_calib_man) return;

	if (filt_calib_step.Value() == 1){
		g_filter_logic->init((rp_channel_t)adc_channel.Value());

		if (getDACChannels() >= 2){
			g_calib_man->setOffset(RP_CH_1,0);
			g_calib_man->setFreq(RP_CH_1,1000);
			g_calib_man->setAmp(RP_CH_1,0.9);
			g_calib_man->setOffset(RP_CH_2,0);
			g_calib_man->setFreq(RP_CH_2,1000);
			g_calib_man->setAmp(RP_CH_2,0.9);
			g_calib_man->enableGen(RP_CH_1,true);
			g_calib_man->enableGen(RP_CH_2,true);
		}

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
		sendFilterCalibValues((rp_channel_t)adc_channel.Value());
		g_acq->setHyst(adc_hyst.Value());
		g_calib_man->changeDecimation(adc_decimation.Value());
		if (getDACChannels() >= 2){
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

}

void calibAutoFilter(){
	if (!g_filter_logicNch || !g_calib_man || !g_acq) return;

	if (f_ref_volt.IsNewValue()){
		f_ref_volt.Update();
	}

	if (f_external_gen.IsNewValue()){
		f_external_gen.Update();
	}

	if (f_ss_next_step.Value() == 0){
		for(int i = 0; i < getADCChannels(); i++){
			fauto_value_ch_before[i].Value() = 0;
			fauto_value_ch_after[i].Value() = 0;
		}
		g_sub_progress = 0;
		g_calib_man->setModeLV_HV( RP_LOW );
		g_filter_logicNch->init();
		if (getDACChannels() >= 2){
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
		}
		g_acq->startAutoFilterNCh(8);
		for(int i = 0; i < getADCChannels(); i++){
			g_acq->updateAcqFilter((rp_channel_t)i);
		}
		f_ss_state.SendValue(f_ss_next_step.Value());
		f_ss_next_step.SendValue(-1);
		return;
	}

	if (f_ss_next_step.Value() == 1) {
		if (g_sub_progress == 0){
			auto d = g_acq->getDataAutoFilterSync();
			for(int i = 0; i < getADCChannels(); i++){
				auto v = (d.valueCH[i].calib_value_raw   + d.valueCH[i].deviation );

				fauto_value_ch_before[i].SendValue(v);
				fauto_aa_Ch[i].SendValue(0);
				fauto_bb_Ch[i].SendValue(0);
				fauto_pp_Ch[i].SendValue(0);
				fauto_kk_Ch[i].SendValue(0);
			}
			g_sub_progress = 1;
			return;
		}

		if (g_sub_progress == 1){
			while (g_filter_logicNch->setCalibParameters() != -1){
				auto dp = g_acq->getDataAutoFilterSync();
				g_filter_logicNch->setCalculatedValue(dp);
			}
			g_filter_logicNch->removeHalfCalib();
			if (g_filter_logicNch->nextSetupCalibParameters() == -1){
				g_sub_progress = 2;
				return;
			}
			fauto_calib_progress.SendValue(g_filter_logicNch->calcProgress());
		}

		if (g_sub_progress == 2){
			g_filter_logicNch->setGoodCalibParameterCh(RP_CH_1);
			std::this_thread::sleep_for(std::chrono::microseconds(1000000));
			auto ext = f_external_gen.Value();
			auto volt_ref = f_ref_volt.Value();
			if (!ext) volt_ref = 0.9;
			while(1){
        		auto d = g_acq->getDataAutoFilterSync();
        		if (g_filter_logicNch->calibPPCh(RP_CH_1,d,volt_ref) != 0) break;
    		}
			fauto_calib_progress.SendValue(110);
			g_sub_progress = 3;
			return;
		}

		if (g_sub_progress == 3){
			g_filter_logicNch->setGoodCalibParameterCh(RP_CH_2);
			std::this_thread::sleep_for(std::chrono::microseconds(1000000));
			auto ext = f_external_gen.Value();
			auto volt_ref = f_ref_volt.Value();
			if (!ext) volt_ref = 0.9;
			while(1){
        		auto d = g_acq->getDataAutoFilterSync();
        		if (g_filter_logicNch->calibPPCh(RP_CH_2,d,volt_ref) != 0) break;
    		}
			fauto_calib_progress.SendValue(120);

			g_sub_progress = 4;
			if (getADCChannels() == 2){
				g_sub_progress = 6;
			}

			return;
		}

		if (g_sub_progress == 4){
			g_filter_logicNch->setGoodCalibParameterCh(RP_CH_3);
			std::this_thread::sleep_for(std::chrono::microseconds(1000000));
			auto ext = f_external_gen.Value();
			auto volt_ref = f_ref_volt.Value();
			if (!ext) volt_ref = 0.9;
			while(1){
        		auto d = g_acq->getDataAutoFilterSync();
        		if (g_filter_logicNch->calibPPCh(RP_CH_3,d,volt_ref) != 0) break;
    		}
			fauto_calib_progress.SendValue(130);
			g_sub_progress = 5;
			return;
		}

		if (g_sub_progress == 5){
			g_filter_logicNch->setGoodCalibParameterCh(RP_CH_4);
			std::this_thread::sleep_for(std::chrono::microseconds(1000000));
			auto ext = f_external_gen.Value();
			auto volt_ref = f_ref_volt.Value();
			if (!ext) volt_ref = 0.9;
			while(1){
        		auto d = g_acq->getDataAutoFilterSync();
        		if (g_filter_logicNch->calibPPCh(RP_CH_4,d,volt_ref) != 0) break;
    		}
			fauto_calib_progress.SendValue(140);
			g_sub_progress = 6;
			return;
		}

		if (g_sub_progress == 6){
			std::this_thread::sleep_for(std::chrono::microseconds(1000000));
			auto d = g_acq->getDataAutoFilterSync();
			for(int i = 0; i < getADCChannels(); i++){
				auto v1 = (d.valueCH[i].calib_value_raw   + d.valueCH[i].deviation );
				fauto_value_ch_after[i].SendValue(v1);
				fauto_aa_Ch[i].SendValue(d.valueCH[i].f_aa);
				fauto_bb_Ch[i].SendValue(d.valueCH[i].f_bb);
				fauto_pp_Ch[i].SendValue(d.valueCH[i].f_pp);
				fauto_kk_Ch[i].SendValue(d.valueCH[i].f_kk);
			}
			fauto_calib_progress.SendValue(0);
			g_sub_progress = 0;
			f_ss_state.SendValue(f_ss_next_step.Value());
			f_ss_next_step.SendValue(-1);
			return;
		}

	}

	if (f_ss_next_step.Value() == 2){
		for(int i = 0; i < getADCChannels(); i++){
			fauto_value_ch_before[i].Value() = 0;
			fauto_value_ch_after[i].Value() = 0;
		}
		g_sub_progress = 0;
		g_calib_man->setModeLV_HV( RP_HIGH );
		g_filter_logicNch->init();

		if (getDACChannels() > 0){
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
		}

		g_acq->startAutoFilterNCh(8);
		for(int i = 0; i < getADCChannels(); i++){
			g_acq->updateAcqFilter((rp_channel_t)i);
		}
		f_ss_state.SendValue(f_ss_next_step.Value());
		f_ss_next_step.SendValue(-1);
		return;
	}

	if (f_ss_next_step.Value() == 3) {
			if (g_sub_progress == 0){
				auto d = g_acq->getDataAutoFilterSync();
				for(int i = 0; i < getADCChannels(); i++){
					auto v = (d.valueCH[i].calib_value_raw   + d.valueCH[i].deviation );

					fauto_value_ch_before[i].SendValue(v);
					fauto_aa_Ch[i].SendValue(0);
					fauto_bb_Ch[i].SendValue(0);
					fauto_pp_Ch[i].SendValue(0);
					fauto_kk_Ch[i].SendValue(0);
				}
				g_sub_progress = 1;
				return;
			}

			if (g_sub_progress == 1){
				while (g_filter_logicNch->setCalibParameters() != -1){
					auto dp = g_acq->getDataAutoFilterSync();
					g_filter_logicNch->setCalculatedValue(dp);
				}
				g_filter_logicNch->removeHalfCalib();
				if (g_filter_logicNch->nextSetupCalibParameters() == -1){
					g_sub_progress = 2;
					return;
				}
				fauto_calib_progress.SendValue(g_filter_logicNch->calcProgress());
			}

			if (g_sub_progress == 2){
				g_filter_logicNch->setGoodCalibParameterCh(RP_CH_1);
				std::this_thread::sleep_for(std::chrono::microseconds(1000000));
				auto ext = f_external_gen.Value();
				auto volt_ref = f_ref_volt.Value();
				if (!ext) volt_ref = 0.9;
				while(1){
					auto d = g_acq->getDataAutoFilterSync();
					if (g_filter_logicNch->calibPPCh(RP_CH_1,d,volt_ref) != 0) break;
				}
				fauto_calib_progress.SendValue(110);
				g_sub_progress = 3;
				return;
			}

			if (g_sub_progress == 3){
				g_filter_logicNch->setGoodCalibParameterCh(RP_CH_2);
				std::this_thread::sleep_for(std::chrono::microseconds(1000000));
				auto ext = f_external_gen.Value();
				auto volt_ref = f_ref_volt.Value();
				if (!ext) volt_ref = 0.9;
				while(1){
					auto d = g_acq->getDataAutoFilterSync();
					if (g_filter_logicNch->calibPPCh(RP_CH_2,d,volt_ref) != 0) break;
				}
				fauto_calib_progress.SendValue(120);

				g_sub_progress = 4;
				if (getADCChannels() == 2){
					g_sub_progress = 6;
				}
				return;
			}

			if (g_sub_progress == 4){
				g_filter_logicNch->setGoodCalibParameterCh(RP_CH_3);
				std::this_thread::sleep_for(std::chrono::microseconds(1000000));
				auto ext = f_external_gen.Value();
				auto volt_ref = f_ref_volt.Value();
				if (!ext) volt_ref = 0.9;
				while(1){
					auto d = g_acq->getDataAutoFilterSync();
					if (g_filter_logicNch->calibPPCh(RP_CH_3,d,volt_ref) != 0) break;
				}
				fauto_calib_progress.SendValue(120);
				g_sub_progress = 5;
				return;
			}

			if (g_sub_progress == 5){
				g_filter_logicNch->setGoodCalibParameterCh(RP_CH_4);
				std::this_thread::sleep_for(std::chrono::microseconds(1000000));
				auto ext = f_external_gen.Value();
				auto volt_ref = f_ref_volt.Value();
				if (!ext) volt_ref = 0.9;
				while(1){
					auto d = g_acq->getDataAutoFilterSync();
					if (g_filter_logicNch->calibPPCh(RP_CH_4,d,volt_ref) != 0) break;
				}
				fauto_calib_progress.SendValue(120);
				g_sub_progress = 6;
				return;
			}

			if (g_sub_progress == 6){
				std::this_thread::sleep_for(std::chrono::microseconds(1000000));
				auto d = g_acq->getDataAutoFilterSync();
				for(int i = 0; i < getADCChannels(); i++){
					auto v1 = (d.valueCH[i].calib_value_raw   + d.valueCH[i].deviation );
					fauto_value_ch_after[i].SendValue(v1);
					fauto_aa_Ch[i].SendValue(d.valueCH[i].f_aa);
					fauto_bb_Ch[i].SendValue(d.valueCH[i].f_bb);
					fauto_pp_Ch[i].SendValue(d.valueCH[i].f_pp);
					fauto_kk_Ch[i].SendValue(d.valueCH[i].f_kk);
				}
				fauto_calib_progress.SendValue(0);

				g_sub_progress = 0;
				f_ss_state.SendValue(f_ss_next_step.Value());
				f_ss_next_step.SendValue(-1);
				return;
			}

		}

	if (f_ss_next_step.Value() == 4){
			for(int i = 0; i < getADCChannels(); i++){
				fauto_value_ch_before[i].Value() = 0;
				fauto_value_ch_after[i].Value() = 0;
			}
			g_calib_man->writeCalib();
			if (getDACChannels() >= 2){
				g_calib_man->enableGen(RP_CH_1,false);
				g_calib_man->enableGen(RP_CH_2,false);
			}

			f_ss_state.SendValue(f_ss_next_step.Value());
			f_ss_next_step.SendValue(-1);
			return;
		}

}

void sendFilterCalibValues(rp_channel_t _ch){
	if (rp_HPGetFastADCIsFilterPresentOrDefault()){
		filt_aa.SendValue(g_calib_man->getCalibValue(_ch, F_AA_CH));
		filt_bb.SendValue(g_calib_man->getCalibValue(_ch, F_BB_CH));
		filt_pp.SendValue(g_calib_man->getCalibValue(_ch, F_PP_CH));
		filt_kk.SendValue(g_calib_man->getCalibValue(_ch, F_KK_CH));
	}
}

void updateAcqData(){
    auto x = g_acq->getData();
    for(int i = 0; i < getADCChannels(); i++){
        ch_max[i].Value() = x.ch_max[i];
        ch_min[i].Value() = x.ch_min[i];
        ch_avg[i].Value() = x.ch_avg[i];
        ch_p_p[i].Value() = x.ch_p_p[i];
        ch_per_buff[i].Value() = x.periodsByBuffer[i];
        ch_is_sine[i].Value() = x.isSineSignal[i];
    }
}

//Update parameters
void UpdateParams(void)
{	if (!g_calib || !g_acq || !g_calib_man) return;
	try{
        updateAcqData();

// AUTO MODE
		if (ss_next_step.IsNewValue() && ref_volt.IsNewValue())
		{
			ss_next_step.Update();
			ref_volt.Update();
			if (ss_next_step.Value() != -2){
				if (g_calib->calib(ss_next_step.Value(),ref_volt.Value()) == RP_OK){
					auto x = g_calib->getCalibData();
					for(int i = 0; i < getADCChannels(); i++){
						ch_calib_pass[i].SendValue(x.ch[i]);
					}
					ss_state.SendValue(ss_next_step.Value());
				}
			}else{
				g_calib->restoreCalib();
			}
		}


		if (rp_HPGetFastADCIsFilterPresentOrDefault()){
			// FILTER AUTO MODE
			if (f_ss_next_step.IsNewValue())
			{
				f_ss_next_step.Update();
			}

			calibAutoFilter();
		}

// MANUAL MODE
		if (calib_sig.IsNewValue()){
			calib_sig.Update();
			int sig = calib_sig.Value();
			calib_sig.Value() = 0; // reset signal
			g_acq->resetAvgFilter();
			if (sig == 1){
				g_calib_man->init();
				if (getDACChannels() >= 2){
					if (getModelName() == "Z20"){
						gen_freq[0].SendValue(1000000);
						gen_freq[1].SendValue(1000000);
						gen_offset[0].SendValue(0);
						gen_offset[1].SendValue(0);
						gen_type[0].SendValue(0);
						gen_type[1].SendValue(0);
						gen_amp[0].SendValue(0.4);
						gen_amp[1].SendValue(0.4);
						g_calib_man->setFreq(RP_CH_1,1000000);
						g_calib_man->setFreq(RP_CH_2,1000000);
						g_calib_man->setAmp(RP_CH_1,0.4);
						g_calib_man->setAmp(RP_CH_2,0.4);
					}else{
						gen_freq[0].SendValue(1000);
						gen_freq[1].SendValue(1000);
						gen_offset[0].SendValue(0);
						gen_offset[1].SendValue(0);
						gen_type[0].SendValue(0);
						gen_type[1].SendValue(0);
						gen_amp[0].SendValue(0.9);
						gen_amp[1].SendValue(0.9);
					}
				}

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
			if (rp_HPGetFastADCIsFilterPresentOrDefault()){

				if (sig == 6){
					g_calib_man->setDefualtFilter((rp_channel_t)adc_channel.Value());
					g_calib_man->updateCalib();
					g_calib_man->updateAcqFilter((rp_channel_t)adc_channel.Value());
					sendFilterCalibValues((rp_channel_t)adc_channel.Value());
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
					if (getDACChannels() >= 2){
						g_calib_man->setOffset(RP_CH_1,filt_gen_offset.Value());
						g_calib_man->setFreq(RP_CH_1,filt_gen_freq.Value());
						g_calib_man->setAmp(RP_CH_1,filt_gen_amp.Value());
						g_calib_man->setOffset(RP_CH_2,filt_gen_offset.Value());
						g_calib_man->setFreq(RP_CH_2,filt_gen_freq.Value());
						g_calib_man->setAmp(RP_CH_2,filt_gen_amp.Value());
						filt_gen_freq.SendValue(filt_gen_freq.Value());
						filt_gen_amp.SendValue(filt_gen_amp.Value());
						filt_gen_offset.SendValue(filt_gen_offset.Value());
					}
					sendFilterCalibValues((rp_channel_t)adc_channel.Value());
				}
			}
		}

		if (hv_lv_mode.IsNewValue()){
			hv_lv_mode.Update();
			g_calib_man->setModeLV_HV(hv_lv_mode.Value() ? RP_HIGH :RP_LOW);
			if (rp_HPGetFastADCIsFilterPresentOrDefault()){
				g_calib_man->updateAcqFilter((rp_channel_t)adc_channel.Value());
			}
			sendCalibInManualMode(true);
		}

		if (man_decimation.IsNewValue()){
			man_decimation.Update();
			g_acq->setDeciamtion(man_decimation.Value());
		}

		if (avg_last_mode.IsNewValue()){
			avg_last_mode.Update();
			g_acq->setAvgFilter(avg_last_mode.Value());
			g_acq->resetAvgFilter();
			avg_last_mode.SendValue(g_acq->getAvgFilter());
		}

		if (request_reset.IsNewValue()){
			request_reset.Update();
			request_reset.Value() = false;
			g_acq->resetAvgFilter();
		}

		if (rp_HPGetFastADCIsAC_DCOrDefault()){
			if (ac_dc_mode.IsNewValue()){
				ac_dc_mode.Update();
				g_calib_man->setModeAC_DC(ac_dc_mode.Value() ? RP_AC : RP_DC);
				sendCalibInManualMode(true);
			}
		}

		if (rp_HPGetIsGainDACx5OrDefault()){
			if (gen_gain.IsNewValue()){
				gen_gain.Update();
				g_calib_man->setGenGain(gen_gain.Value() ? RP_GAIN_5X : RP_GAIN_1X);
				sendCalibInManualMode(true);
			}
		}

		getNewCalib();
		setupGen();

		if (rp_HPGetFastADCIsFilterPresentOrDefault()){
			if (filt_calib_step.IsNewValue() || (filt_calib_step.Value() >= 1 && filt_calib_step.Value() <= 100)){
				filt_calib_step.Update();
				calibFilter();
			}
			setupGenFilter();
			updateFilterModeParameter();
			getNewFilterCalib();
		}
	}catch (std::exception& e)
	{
		ERROR("UpdateParams() %s",e.what());
	}

}

void PostUpdateSignals(){}

void OnNewParams(void)
{
	std::lock_guard<std::mutex> lock(g_mtx);
	//Update parameters
	UpdateParams();
}

void OnNewSignals(void)
{
	std::lock_guard<std::mutex> lock(g_mtx);
	UpdateSignals();
}
