#include <stdio.h>
#include <unistd.h>

#include <string>
#include <map>
#include <list>
#include <memory>
#include <thread>
#include <atomic>

#include "rp.h"
#include "rp_log.h"
#include "decoder.h"
#include "web/rp_client.h"
#include "web/rp_system.h"

#include "main.h"
#include "common.h"
#include "settings.h"

#define FILE_NAME "/tmp/logic/rle_logic_data.bin"
#define FILE_NAME_TRIGGER "/tmp/logic/trig.bin"

CIntParameter 		inRun			("LA_RUN", CBaseParameter::RW, 0, 0, 0, 3);
CIntParameter 		measureState	("LA_MEASURE_STATE", CBaseParameter::RW, 1, 0, 1, 4);
CIntParameter 		measureSelect	("LA_MEASURE_MODE", CBaseParameter::RW, 1, 0, 1, 2, CONFIG_VAR);
CIntParameter 		controlSettings ("CONTROL_CONFIG_SETTINGS", CBaseParameter::RW, 0, 0, 0, 10);
CStringParameter 	fileSettings	("FILE_SATTINGS", CBaseParameter::RW, "", 0);
CStringParameter 	listFileSettings("LIST_FILE_SATTINGS", CBaseParameter::RW, "", 0);

CIntParameter 		max_freq		("LA_MAX_FREQ", CBaseParameter::RO, getMAXFreq(), 0, 1, 2e9);
CIntParameter 		cur_freq		("LA_CUR_FREQ", CBaseParameter::RW, getMAXFreq(), 0, 1, 2e9, CONFIG_VAR);

CByteBase64Signal	data_rle		("data_rle", CBaseParameter::RO, 0, 0);

CBooleanParameter 	dins_enable[8] 	= INIT8("LA_DIN_","", CBaseParameter::RW, true, 0, CONFIG_VAR);
CStringParameter 	dins_name[8] 	= INIT8("LA_DIN_NAME_","", CBaseParameter::RW, "", 0, CONFIG_VAR);
CIntParameter	 	dins_trigger[8] = INIT8("LA_DIN_","_TRIGGER", CBaseParameter::RW, 0, 0, 0 , 5 , CONFIG_VAR);
CFloatParameter	 	dins_position[8] = INIT8("LA_DIN_","_POS", CBaseParameter::RW, -1, 0, 0 , 9 , CONFIG_VAR);

CIntParameter 		decimate		("LA_DECIMATE", CBaseParameter::RW, 1, 0, 0, 1024, CONFIG_VAR);
CDoubleParameter    timeScale		("LA_SCALE", CBaseParameter::RW, 1, 0,0.005,1000,CONFIG_VAR);
CIntParameter 		preSampleBufMs	("LA_PRE_TRIGGER_BUFFER_MS", CBaseParameter::RW, 1, 0, 1, 360000, CONFIG_VAR);
CIntParameter 		postSampleBufMs	("LA_POST_TRIGGER_BUFFER_MS", CBaseParameter::RW, 1, 0, 1, 360000, CONFIG_VAR);

CIntParameter 		preSampleCount	("LA_PRE_TRIGGER_SAMPLES", CBaseParameter::RO, 0, 0, 0, 2000000000);
CIntParameter 		postSampleCount	("LA_POST_TRIGGER_SAMPLES", CBaseParameter::RO, 0, 0, 0, 2000000000);
CIntParameter 		sampleCount		("LA_TOTAL_SAMPLES", CBaseParameter::RO, 0, 0, 0, 2000000000);
CStringParameter 	view_port_pos	("LA_VIEW_PORT_POS", CBaseParameter::RW, "0.5",0, CONFIG_VAR); // For greater accuracy, double the value is required.


CStringParameter    decoders[4] =  INIT4("DECODER_", "", CBaseParameter::RW, "", 0, CONFIG_VAR);
CBooleanParameter   decoders_enabled[4] =  INIT4("DECODER_ENABLED_", "", CBaseParameter::RW, false, 0, CONFIG_VAR);

CStringParameter 	decoder_def_settings_uart 	("DECODER_DEF_UART", CBaseParameter::RO,"", 0);
CStringParameter 	decoder_def_settings_can 	("DECODER_DEF_CAN", CBaseParameter::RO,"", 0);
CStringParameter 	decoder_def_settings_spi 	("DECODER_DEF_SPI", CBaseParameter::RO,"", 0);
CStringParameter 	decoder_def_settings_i2c 	("DECODER_DEF_I2C", CBaseParameter::RO,"", 0);

CStringParameter 	decoder_anno_uart 	("DECODER_ANNOTATION_UART", CBaseParameter::RO,"", 0);
CStringParameter 	decoder_anno_can 	("DECODER_ANNOTATION_CAN", CBaseParameter::RO,"", 0);
CStringParameter 	decoder_anno_spi 	("DECODER_ANNOTATION_SPI", CBaseParameter::RO,"", 0);
CStringParameter 	decoder_anno_i2c 	("DECODER_ANNOTATION_I2C", CBaseParameter::RO,"", 0);


CBooleanParameter   cursorx[2]      = INIT2("LA_CURSOR_X","", CBaseParameter::RW, false, 0 ,CONFIG_VAR);
CFloatParameter     cursorx_pos[2]  = INIT2("LA_CURSOR_X","_POS", CBaseParameter::RW, 0.25, 0, 0, 1, CONFIG_VAR);


CIntParameter 		display_radix	("LA_DISPLAY_RADIX", CBaseParameter::RW, 16, 0, 1, 20, CONFIG_VAR);

CDecodedSignal		decoder_signal[4] = INIT4("DECODER_SIGNAL_","", CBaseParameter::RO, 0, rp_la::OutputPacket());

CBooleanParameter 	decoded_window_show	("LA_WIN_SHOW", CBaseParameter::RW, false, 0, CONFIG_VAR);
CIntParameter 		decoded_window_x	("LA_WIN_X", CBaseParameter::RW, 300, 0, 0, 65536, CONFIG_VAR);
CIntParameter 		decoded_window_y	("LA_WIN_Y", CBaseParameter::RW, 300, 0, 0, 65536, CONFIG_VAR);
CIntParameter 		decoded_window_w	("LA_WIN_W", CBaseParameter::RW, 400, 0, 0, 65536, CONFIG_VAR);
CIntParameter 		decoded_window_h	("LA_WIN_H", CBaseParameter::RW, 400, 0, 0, 65536, CONFIG_VAR);


/// LOGGER settings
CBooleanParameter   log_bus_enabled[4] =  INIT4("LA_LOGGER_BUS_", "", CBaseParameter::RW, false, 0, CONFIG_VAR);
CIntParameter 		log_radix				("LA_LOGGER_RADIX", CBaseParameter::RW, 16, 0, 1, 20, CONFIG_VAR);


std::atomic_bool g_needUpdateSignals = false;
std::atomic_bool g_needUpdateDecoders = false;
std::atomic_bool g_config_changed = false;
std::atomic_int32_t g_save_counter = 0;

std::shared_ptr<rp_la::CLAController> g_la_controller = nullptr;
CLACallbackHandler g_la_callback;
const std::vector<std::string> g_savedParams;

// Set measure mode
void set_measure_mode(MEASURE_MODE mode){
	if (mode == LA_APP_BASIC){
		WARNING("Set measure mode: Basic")
		g_la_controller->setMode(rp_la::LA_BASIC);
	}
	else if (mode == LA_APP_PRO){
		WARNING("Set measure mode: Pro")
		g_la_controller->setMode(rp_la::LA_PRO);
	}
}

int rp_set_params(rp_app_params_t *, int) {
	return 0;
}

int rp_get_params(rp_app_params_t **) {
	return 0;
}

int rp_get_signals(float ***, int *, int *) {
	return 0;
}

const char *rp_app_desc(void) {
	return (const char *)"Red Pitaya Logic analizer application.\n";
}

void CLACallbackHandler::captureStatus(rp_la::CLAController* controller,
										bool isTimeout,
										uint32_t numBytes,
										uint64_t numSamples,
										uint64_t preSamples,
										uint64_t postSamples){
	if (isTimeout == false){
		if (numSamples){
			controller->saveCaptureDataToFile(FILE_NAME);
			FILE* f = fopen(FILE_NAME_TRIGGER, "wb");
			if (!f){
				ERROR_LOG("File opening failed %s",FILE_NAME_TRIGGER);
			}else{
				fwrite(&preSamples, sizeof(uint64_t), 1, f);
				fclose(f);
			}
		}
		g_needUpdateSignals = true;
		preSampleCount.SendValue(preSamples);
		postSampleCount.SendValue(postSamples);
		sampleCount.SendValue(numSamples);
		measureState.SendValue(LA_APP_DONE);
		view_port_pos.SendValue(std::to_string((double)preSamples/(double)numSamples));
		cur_freq.SendValue(max_freq.Value() / g_la_controller->getDecimation());
		controller->printRLE(false);
		TRACE("Done")
	}else{
		measureState.SendValue(LA_APP_TIMEOUT);
		TRACE("Timeout")
	}
	inRun.SendValue(0);
}

void CLACallbackHandler::decodeStatus(rp_la::CLAController* controller,
										uint32_t numBytes,
										uint64_t numSamples,
										uint64_t preSamples,
										uint64_t postSamples){
	if (numSamples){
		controller->saveCaptureDataToFile(FILE_NAME);
		FILE* f = fopen(FILE_NAME_TRIGGER, "wb");
		if (!f){
			ERROR_LOG("File opening failed %s",FILE_NAME_TRIGGER);
		}else{
			fwrite(&preSamples, sizeof(uint64_t), 1, f);
			fclose(f);
		}
	}
	g_needUpdateSignals = true;
	preSampleCount.SendValue(preSamples);
	postSampleCount.SendValue(postSamples);
	sampleCount.SendValue(numSamples);
	measureState.SendValue(LA_APP_DONE);
	view_port_pos.SendValue(std::to_string((double)preSamples/(double)numSamples));
	controller->printRLE(false);
	TRACE("Done")
}

void CLACallbackHandler::decodeDone(rp_la::CLAController* controller, std::string name){
	g_needUpdateDecoders = true;
	inRun.SendValue(0);
	TRACE("Done")
}

void updateParametersByConfig(){
    listFileSettings.Value() = getListOfSettingsInStore();
    configGet();
	for(int i = 0 ; i < 8; i++){
		if (dins_name[i].Value() == ""){
			dins_name[i].Set("DIN"+std::to_string(i));
		}
		if (dins_position[i].Value() == -1){
			dins_position[i].Set(i + 1);
		}
	}
    updateFromFront(true);
}

int rp_app_init(void) {
    setHomeSettingsPath("/.config/redpitaya/apps/la_pro/");
	data_rle.Reserve(BUFFER_MAX_SIZE);
	g_la_controller = std::make_shared<rp_la::CLAController>();
	g_la_controller->initFpga();
	g_la_controller->setDelegate(&g_la_callback);
	g_la_controller->setEnableRLE(true);

	decoder_def_settings_uart.SendValue(g_la_controller->getDefaultSettings(LA_DECODER_UART));
	decoder_def_settings_spi.SendValue(g_la_controller->getDefaultSettings(LA_DECODER_SPI));
	decoder_def_settings_i2c.SendValue(g_la_controller->getDefaultSettings(LA_DECODER_I2C));
	decoder_def_settings_can.SendValue(g_la_controller->getDefaultSettings(LA_DECODER_CAN));

	decoder_anno_uart.SendValue(annoToJSON(g_la_controller->getAnnotationList(LA_DECODER_UART)));
	decoder_anno_can.SendValue(annoToJSON(g_la_controller->getAnnotationList(LA_DECODER_CAN)));
	decoder_anno_spi.SendValue(annoToJSON(g_la_controller->getAnnotationList(LA_DECODER_SPI)));
	decoder_anno_i2c.SendValue(annoToJSON(g_la_controller->getAnnotationList(LA_DECODER_I2C)));

    rp_WC_Init();
	rp_WS_Init();
	rp_WS_SetInterval(RP_WS_CPU,1000);
    rp_WS_SetInterval(RP_WS_RAM,1000);
	rp_WS_SetMode((rp_system_mode_t)(RP_WS_CPU | RP_WS_RAM | RP_WS_TEMPERATURE));
    rp_WS_UpdateParameters(true);
	CDataManager::GetInstance()->SetParamInterval(DEBUG_PARAM_PERIOD);
	CDataManager::GetInstance()->SetSignalInterval(DEBUG_SIGNAL_PERIOD);
	updateParametersByConfig();
	uint64_t preSamples = 0;
	FILE* f = fopen(FILE_NAME_TRIGGER, "rb");
	if (!f){
		ERROR_LOG("File opening failed %s",FILE_NAME_TRIGGER);
	}else{
		if (fread(&preSamples, sizeof(uint64_t), 1, f) != 1){
			preSamples = 0;
		}
		fclose(f);
	}
	g_la_controller->loadFromFileAndDecode(FILE_NAME,true, preSamples);
	fprintf(stderr, "Loading logic analyzer version.\n");
	return 0;
}

int rp_app_exit(void) {
	fprintf(stderr, "Unloading logic analyzer version.\n");
	g_la_controller = nullptr;
	return 0;
}

void UpdateParams(void) {
	rp_WS_UpdateParameters(false);

	if (g_config_changed && (g_save_counter++ % 40 == 0)){
        g_config_changed = false;
        // Save the configuration file
        configSet();
    }
}


void UpdateSignals(void) {
	if (g_needUpdateSignals){
		g_needUpdateSignals = false;
		auto size = g_la_controller->getCapturedDataSize();
		data_rle.Resize(size);
		g_la_controller->getDataNP(data_rle.GetDataPtr()->data(),size);
		data_rle.ForceSend();
	}

	if (g_needUpdateDecoders){
		for(int i = 0 ;i < 4; i++){
			auto vec = g_la_controller->getDecodedData(std::to_string(i));
			decoder_signal[i].Value() = vec;
			decoder_signal[i].ForceSend();
		}
		g_needUpdateDecoders = false;
	}
}

void PostUpdateSignals(void){}


void updateFromFront(bool force){

	bool needRedecode = false;

	auto getName = [](rp_la::la_Decoder_t tp){
		switch (tp)
		{
		case LA_DECODER_CAN: return "CAN";
		case LA_DECODER_I2C: return "I2C";
		case LA_DECODER_SPI: return "SPI";
		case LA_DECODER_UART: return "UART";
		default:
			break;
		}
		return "";
	};

	auto getType= [](std::string name){
		if (name == "CAN") return LA_DECODER_CAN;
		if (name == "I2C") return LA_DECODER_I2C;
		if (name == "SPI") return LA_DECODER_SPI;
		if (name == "UART") return LA_DECODER_UART;
		return LA_DECODER_NONE;
	};

	for(size_t i = 0; i < 4; i++){
		if (IS_NEW(log_bus_enabled[i]) || force){
			log_bus_enabled[i].Update();
		}
	}

	if (IS_NEW(log_radix) || force){
		log_radix.Update();
	}

	if (IS_NEW(decoded_window_show) || force){
		decoded_window_show.Update();
	}

	if (IS_NEW(decoded_window_x) || force){
		decoded_window_x.Update();
	}

	if (IS_NEW(decoded_window_y) || force){
		decoded_window_y.Update();
	}

	if (IS_NEW(decoded_window_h) || force){
		decoded_window_h.Update();
	}

	if (IS_NEW(decoded_window_w) || force){
		decoded_window_w.Update();
	}

	// Update measure select
	if (IS_NEW(measureSelect) || force){
		measureSelect.Update();
		if (measureSelect.Value() == 1)
			set_measure_mode(LA_APP_BASIC);
		if (measureSelect.Value() == 2)
			set_measure_mode(LA_APP_PRO);
	}

    for(int i = 0; i < 2 ; i++){
        if (IS_NEW(cursorx[i]) || force)
            cursorx[i].Update();
        if (IS_NEW(cursorx_pos[i]) || force)
            cursorx_pos[i].Update();
    }

	if (IS_NEW(view_port_pos) || force){
		view_port_pos.Update();
	}

	if (IS_NEW(display_radix) || force){
		display_radix.Update();
	}

	if (IS_NEW(cur_freq) || force){
		cur_freq.Update();
		needRedecode = true;
	}

	bool reqRecalcPresample = false;

	if (IS_NEW(decimate) || force){
		decimate.Update();
		g_la_controller->setDecimation(decimate.Value());
		reqRecalcPresample = true;
	}

	if (IS_NEW(timeScale) || force){
		timeScale.Update();
	}

	if (IS_NEW(preSampleBufMs) || IS_NEW(postSampleBufMs) || force || reqRecalcPresample){
		auto freq = max_freq.Value() / decimate.Value();
		preSampleBufMs.Update();
		postSampleBufMs.Update();
        double val = preSampleBufMs.Value() / 1000.0; // to sec convert
		double maxValue = (MAX_SAMPLES * 1000.0) / (double)freq;
		preSampleBufMs.SetMax(maxValue);
		postSampleBufMs.SetMax(maxValue);
        val *= freq;
        if (val > MAX_SAMPLES) {
			val = MAX_SAMPLES;
            uint32_t newVal = (MAX_SAMPLES * 1000.0) / (double)freq;
			preSampleBufMs.SendValue(newVal);
		}

        double valPost = postSampleBufMs.Value() / 1000.0; // to sec convert
        valPost *= freq;
        if (valPost > MAX_SAMPLES) {
			valPost = MAX_SAMPLES;
            uint32_t newVal = (MAX_SAMPLES * 1000.0) / (double)freq;
			postSampleBufMs.SendValue(newVal);
		}

		g_la_controller->setPreTriggerSamples(val);
		g_la_controller->setPostTriggerSamples(valPost);
	}

	for(size_t i = 0 ; i < 8; i++){
		if (IS_NEW(dins_enable[i]) || IS_NEW(dins_trigger[i]) || force){
			dins_enable[i].Update();
			dins_trigger[i].Update();

			auto enabled = dins_enable[i].Value();
			auto dins_t = dins_trigger[i].Value();

			if (enabled){
				g_la_controller->setTrigger(i, (la_Trigger_Mode_t)dins_t);
			}else{
				g_la_controller->setTrigger(i, LA_NONE);
			}
		}

		if (IS_NEW(dins_name[i]) || force){
			dins_name[i].Update();
		}

		if (IS_NEW(dins_position[i]) || force){
			dins_position[i].Update();
		}

	}

	for(size_t i = 0 ; i < 4; i++){
		if (IS_NEW(decoders[i]) || force){
			decoders[i].Update();
			auto decoder_config = decoders[i].Value();
			auto name = getNameFromConfig(decoder_config);
			if (name == ""){
				g_la_controller->removeDecoder(std::to_string(i));
				TRACE("Remove decoder %d",i)
			}else{
				if (g_la_controller->isDecoderExist(std::to_string(i))){
					auto curDecoder = getName(g_la_controller->getDecoderType(std::to_string(i)));
					if (curDecoder != name){
						g_la_controller->removeDecoder(std::to_string(i));
						g_la_controller->addDecoder(std::to_string(i),getType(name));
						g_la_controller->setDecoderEnable(std::to_string(i),decoders_enabled[i].Value());
						auto settings = getParamFromConfig(decoder_config);
						if (g_la_controller->setDecoderSettings(std::to_string(i),settings)){
							TRACE("Change decoder with config %s = %s",name.c_str(), settings.c_str())
						}else{
							ERROR_LOG("Error set config %s = %s",name.c_str(),settings.c_str())
							settings = g_la_controller->getDecoderSettings(std::to_string(i));
							auto newConfig = getConfig(name,settings);
							TRACE("Change decoder with config %s",newConfig.c_str())
							decoders[i].SendValue(newConfig);
						}
					}else{
						auto settings = getParamFromConfig(decoder_config);
						if (g_la_controller->setDecoderSettings(std::to_string(i),settings)){
							TRACE("Set new config from frontend %s = %s",name.c_str(), settings.c_str())
						}else{
							ERROR_LOG("Error set config %d : %s = %s",i, name.c_str(),settings.c_str())
							settings = g_la_controller->getDecoderSettings(std::to_string(i));
							TRACE("Wrong config. Get default")
							auto newConfig = getConfig(name,settings);
							TRACE("Send config back  %s",newConfig.c_str())
							decoders[i].SendValue(newConfig);
						}
					}
				}else{
					TRACE("Create decoder %d = %s",i,name.c_str())
					g_la_controller->addDecoder(std::to_string(i),getType(name));
					g_la_controller->setDecoderEnable(std::to_string(i),decoders_enabled[i].Value());
					auto settings = getParamFromConfig(decoder_config);
					if (settings == ""){
						settings = g_la_controller->getDecoderSettings(std::to_string(i));
						TRACE("Missing config. Get default")
					}else{
						if (g_la_controller->setDecoderSettings(std::to_string(i),settings)){
							TRACE("Set new config from frontend %s = %s",name.c_str(), settings.c_str())
						}else{
							ERROR_LOG("Error set config %s = %s",name.c_str(),settings.c_str())
							settings = g_la_controller->getDecoderSettings(std::to_string(i));
							TRACE("Missing config. Get default")
						}
					}
					auto newConfig = getConfig(name,settings);
					TRACE("Send config back  %s",newConfig.c_str())
					decoders[i].SendValue(newConfig);
				}
				needRedecode = true;
			}
		}

		if (IS_NEW(decoders_enabled[i]) || force){
			decoders_enabled[i].Update();
			if (g_la_controller->getDecoderType(std::to_string(i)) != LA_DECODER_NONE){
				g_la_controller->setDecoderEnable(std::to_string(i),decoders_enabled[i].Value());
			}else{
				decoders_enabled[i].SendValue(false);
			}
		}
	}


	bool needRunCapture = false;
	if (IS_NEW(inRun)){
		inRun.Update();
		if (inRun.Value() == 3){
			g_la_controller->loadFromFileAndDecode(FILE_NAME, true, 0);
			needRedecode = true;
		}
		if (inRun.Value() == 2){
			needRedecode = true;
		}
		if (inRun.Value() == 1){
			needRunCapture = true;
		}
		if (inRun.Value() == 0){
			g_la_controller->softwareTrigger();
			measureState.SendValue(LA_APP_STOP);
		}
	}

	if (needRunCapture){
		g_needUpdateSignals = false;
		for(int i = 0; i < 4; i++){
			auto freq = max_freq.Value() / g_la_controller->getDecimation();
			g_la_controller->setDecoderSettingsUInt(std::to_string(i),"acq_speed", freq);
		}
		g_la_controller->runAsync(0);
		measureState.SendValue(LA_APP_RUNNED);
	}else if (needRedecode){
		TRACE_SHORT("Decode async")
		g_needUpdateDecoders = false;
		for(int i = 0; i < 4; i++){
			g_la_controller->setDecoderSettingsUInt(std::to_string(i),"acq_speed", cur_freq.Value());
		}
		g_la_controller->decodeAsync();
	}
}

void OnNewParams(void) {

    if (IS_NEW(controlSettings)){
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
				saveCurrentSettingToStore(fileSettings.Value());
				controlSettings.SendValue(controlSettings::NONE);
				listFileSettings.SendValue(getListOfSettingsInStore());
			}

			if (controlSettings.NewValue() == controlSettings::LOAD){
				controlSettings.Update();
				fileSettings.Update();
				loadSettingsFromStore(fileSettings.Value());
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
    }

    if (!g_config_changed)
        g_config_changed = isChanged();
    updateFromFront(false);
}

void OnNewSignals(void){}
