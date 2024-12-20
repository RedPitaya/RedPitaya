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



CBooleanParameter 	inRun			("LA_RUN", CBaseParameter::RW, false, 0);
CIntParameter 		measureState	("LA_MEASURE_STATE", CBaseParameter::RW, 1, 0, 1, 4);
CIntParameter 		measureSelect	("LA_MEASURE_MODE", CBaseParameter::RW, 1, 0, 1, 2, CONFIG_VAR);
CIntParameter 		controlSettings ("CONTROL_CONFIG_SETTINGS", CBaseParameter::RW, 0, 0, 0, 10);
CStringParameter 	fileSettings	("FILE_SATTINGS", CBaseParameter::RW, "", 0);
CStringParameter 	listFileSettings("LIST_FILE_SATTINGS", CBaseParameter::RW, "", 0);

CIntParameter 		max_freq		("LA_MAX_FREQ", CBaseParameter::RO, getMAXFreq(), 0, 1, 1e9);
CIntParameter 		cur_freq		("LA_CUR_FREQ", CBaseParameter::RO, getMAXFreq(), 0, 1, 1e9);

CByteBase64Signal	data_rle		("data_rle", CBaseParameter::RO, 0, 0);

CBooleanParameter 	dins_enable[8] 	= INIT8("LA_DIN_","", CBaseParameter::RW, true, 0, CONFIG_VAR);
CStringParameter 	dins_name[8] 	= INIT8("LA_DIN_NAME_","", CBaseParameter::RW, "", 0, CONFIG_VAR);
CIntParameter	 	dins_trigger[8] = INIT8("LA_DIN_","_TRIGGER", CBaseParameter::RW, 0, 0, 0 , 5 , CONFIG_VAR);
CFloatParameter	 	dins_position[8] = INIT8("LA_DIN_","_POS", CBaseParameter::RW, -1, 0, 0 , 9 , CONFIG_VAR);

CIntParameter 		decimate		("LA_DECIMATE", CBaseParameter::RW, 1, 0, 0, 1024, CONFIG_VAR);
CDoubleParameter    timeScale		("LA_SCALE", CBaseParameter::RW, 1, 0,0.005,100,CONFIG_VAR);
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


CIntParameter 		display_radix	("LA_DISPLAY_RADIX", CBaseParameter::RW, 1, 0, 1, 20, CONFIG_VAR);

CDecodedSignal		decoder_signal[4] = INIT4("DECODER_SIGNAL_","", CBaseParameter::RO, 0, rp_la::OutputPacket());

// CStringParameter 	createDecoder("CREATE_DECODER", CBaseParameter::RW, "", 1);
// CStringParameter 	destroyDecoder("DESTROY_DECODER", CBaseParameter::RW, "", 1);
// CStringParameter 	decoderName("DECODER_NAME", CBaseParameter::RW, "", 1);


// std::vector<RP_DIGITAL_CHANNEL_DIRECTIONS> triggers;

pthread_t g_tid2;

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
		if (numSamples)
			controller->saveCaptureDataToFile("/tmp/logicData.bin");
		g_needUpdateSignals = true;
		preSampleCount.SendValue(preSamples);
		postSampleCount.SendValue(postSamples);
		sampleCount.SendValue(numSamples);
		measureState.SendValue(LA_APP_DONE);
		view_port_pos.SendValue(std::to_string((double)preSamples/(double)numSamples));
		controller->printRLE(false);
		TRACE("Done")
	}else{
		measureState.SendValue(LA_APP_TIMEOUT);
		TRACE("Timeout")
	}
	inRun.SendValue(false);
}

void CLACallbackHandler::decodeDone(rp_la::CLAController* controller, std::string name){
	g_needUpdateDecoders = true;
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

void PostUpdateSignals(void)
{
	// ch1.Update();
	// for (auto& decoder : g_decoders)
	// {
	// 	decoder.second->UpdateSignals();
	// }
}

// void* trigAcq(void *) {
// 	bool isTimeout = false;
// 	g_la_controller->wait(2000, &isTimeout);
// 	if (isTimeout){
// 		measureState.SendValue(LA_APP_TIMEOUT);
// 	}
//     return NULL;
// }


// void DoDecode(bool fpgaDataReceived, uint8_t*) {
// 	for (auto& decoder : g_decoders)
//     {
//         if (decoder.second->IsParametersChanged() || fpgaDataReceived)
//         {
//             if(ch1.GetSize())
//             {
//             	uint8_t buffer[ch1.GetSize()];
//             	for(int i=0; i<ch1.GetSize(); i++)
//             		buffer[i] = ch1[i];
//             	decoder.second->Decode(buffer, ch1.GetSize());
//             }
//             else
//             {
//             	decoder.second->UpdateParameters();
//             }
//         }
//     }
// }

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

	bool reqRecalcPresample = false;

	if (IS_NEW(decimate) || force){
		decimate.Update();
		g_la_controller->setDecimation(decimate.Value());
		auto freq = max_freq.Value() / decimate.Value();
		cur_freq.SendValue(freq);
		reqRecalcPresample = true;
	}

	if (IS_NEW(timeScale) || force){
		timeScale.Update();
	}

	if (IS_NEW(preSampleBufMs) || IS_NEW(postSampleBufMs) || force || reqRecalcPresample){
		preSampleBufMs.Update();
		postSampleBufMs.Update();
        double val = preSampleBufMs.Value() / 1000.0; // to sec convert
		double maxValue = (MAX_SAMPLES * 1000.0) / (double)cur_freq.Value();
		preSampleBufMs.SetMax(maxValue);
		postSampleBufMs.SetMax(maxValue);
        val *= cur_freq.Value();
        if (val > MAX_SAMPLES) {
			val = MAX_SAMPLES;
            uint32_t newVal = (MAX_SAMPLES * 1000.0) / (double)cur_freq.Value();
			preSampleBufMs.SendValue(newVal);
		}

        double valPost = postSampleBufMs.Value() / 1000.0; // to sec convert
        valPost *= cur_freq.Value();
        if (valPost > MAX_SAMPLES) {
			valPost = MAX_SAMPLES;
            uint32_t newVal = (MAX_SAMPLES * 1000.0) / (double)cur_freq.Value();
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
					TRACE("Crate decoder %d = %s",i,name.c_str())
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
		if (inRun.Value()){
			needRunCapture = true;
		}else{
			g_la_controller->softwareTrigger();
			measureState.SendValue(LA_APP_STOP);
		}
	}

	if (needRunCapture){
		g_needUpdateSignals = false;
		for(int i = 0; i < 4; i++){
			if (decoders_enabled[i].Value()){
				g_la_controller->setDecoderSettingsUInt(std::to_string(i),"acq_speed", cur_freq.Value());
			}
		}
		g_la_controller->runAsync(0);
		measureState.SendValue(LA_APP_RUNNED);
	}else if (needRedecode){
		g_needUpdateDecoders = false;
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
				// configSet();
				saveCurrentSettingToStore(fileSettings.Value());
				controlSettings.SendValue(controlSettings::NONE);
				listFileSettings.SendValue(getListOfSettingsInStore());
			}

			if (controlSettings.NewValue() == controlSettings::LOAD){
				controlSettings.Update();
				fileSettings.Update();
				loadSettingsFromStore(fileSettings.Value());
				// configGet();
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

	return;



	// ch1.Update();

	// // buffers for file
	// const size_t file_size = 102400;
	// static uint8_t file_buf[file_size];

	// if (!inRun.NewValue() && inRun.Value())
	// {
	// 	inRun.Update();
	// 	rp_Stop();
	// 	sleep(2);
	// 	measureState.SendValue(1);
	// }
	// else if (inRun.NewValue() && !inRun.Value()) // FPGA mode
	// {
	// 	inRun.Update();
	// 	measureState.SendValue(2);

	// 	std::thread([&]{
	// 		size_t BUF_SIZE = 1024*1024;
	// 		int pre = preSampleBuf.Value();
	// 		uint32_t POST = BUF_SIZE - pre;
	// 		uint32_t samples = 0;
    //         int s;
	// 		// buffers for fpga
	// 		auto buf = new uint8_t[BUF_SIZE*2];
	// 		auto buf1 = new int16_t[BUF_SIZE];

	// 		rp_Stop();
	// 		uint8_t decimateRate = decimate.Value();


	// 		double timeIndisposedMs;
	// 		TRACE("pre = %d post = %d buf_size = %zu", pre, POST, BUF_SIZE);
	// 		// run non blocking
	// 		s = rp_Run(pre, POST, decimateRate, &timeIndisposedMs);

	// 		if (triggers.empty()){
	// 			rp_SoftwareTrigger();
	// 		}

	// 		s = rp_WaitData(0);

	// 		s = rp_SetDataBuffer(buf1, BUF_SIZE);
	// 		samples = BUF_SIZE;
	// 		s = rp_GetValues(&samples);

	// 		uint32_t trigPos = 0;
	// 		uint32_t sum = 0;
	// 		rp_GetTrigPosition(&trigPos);

	// 		for (size_t i = 0; i < samples; ++i)
	// 		{
	// 			buf[i*2] = buf1[i] >> 8;
	// 			buf[i*2 + 1] = buf1[i];

	// 			if(i < trigPos)
	// 				sum += buf[i*2] + 1;
	// 		}

	// 		TRACE("ch1 samples %d sum %d", samples, trigPos);
	// 		TRACE("%x\n-->%x\n%x", buf[pre*2-1], buf[pre*2+1], buf[pre*2+3]);
	// 		ch1.Set(buf, samples*2);

	// 		uint8_t fileBuf[ch1.GetSize()];
	//         for(int i=0; i<ch1.GetSize(); i++)
	//         	fileBuf[i] = ch1[i];
	//         writeToFile("/tmp/logicData.bin", fileBuf, ch1.GetSize());

	// 		g_fpgaDataReceived = true;
	// 		inRun.SendValue(false); // send always
	// 		samplesSum.SendValue(sum); // send always
	// 		measureState.SendValue(3);
	// 		// DoDecode(g_fpgaDataReceived, NULL);

	// 	    delete[] buf;
	// 	    delete[] buf1;

    //         if (s != RP_OK){
    //             ERROR_LOG("Error api2 %d",s);
    //         }
	// 	}).detach();
	// }
	// else if (!createDecoder.IsNewValue() && !decoderName.IsNewValue() && !destroyDecoder.IsNewValue() && !g_decoders.size() && !ch1.GetSize()) // nothing
	// {
	// 	inRun.Update();
	// 	return;
	// }

    // Create decoders
	// if (createDecoder.IsNewValue() && decoderName.IsNewValue())
	// {
	// 	TRACE("CREATE DECODER...");

	// 	createDecoder.Update();
	// 	decoderName.Update();

	// 	const auto name = decoderName.Value();
	// 	if (createDecoder.Value() == "i2c")
	// 	{
	// 		g_decoders[name] = std::make_shared<I2CDecoder>(name);
    //         TRACE("createDecoder: %s", createDecoder.Value().c_str());
	// 	}
	// 	else if (createDecoder.Value() == "spi")
	// 	{
	// 		g_decoders[name] = std::make_shared<SpiDecoder>(name);
    //         TRACE("createDecoder: %s", createDecoder.Value().c_str());
	// 	}
	// 	else if (createDecoder.Value() == "can")
	// 	{
	// 		g_decoders[name] = std::make_shared<CANDecoder>(name);
    //         TRACE("createDecoder: %s", createDecoder.Value().c_str());
	// 	}
	// 	else if (createDecoder.Value() == "uart")
	// 	{
	// 		g_decoders[name] = std::make_shared<UARTDecoder>(name);
    //         TRACE("createDecoder: %s", createDecoder.Value().c_str());
	// 	}

	// 	createDecoder.Value() = name;
    //     TRACE("Value: %s", name.c_str());
	// }

	// // Delete decoders
	// if (destroyDecoder.IsNewValue())
	// {
	// 	destroyDecoder.Update();
    //     TRACE("destroyDecoder: %s", destroyDecoder.Value().c_str());
	// 	g_decoders.erase(destroyDecoder.Value());
	// }

	// DoDecode(false, file_buf);


}

void OnNewSignals(void)
{

}
