#include "main.h"


void startServer(bool testMode);
void stopServer(ServerNetConfigManager::EStopReason x);
void stopNonBlocking(ServerNetConfigManager::EStopReason x);

auto startDACServer(bool testMode) -> void;
auto stopDACNonBlocking(dac_streaming_lib::CDACStreamingManager::NotifyResult x) -> void;
auto stopDACServer(dac_streaming_lib::CDACStreamingManager::NotifyResult x) -> void;
void startADC();

void setConfig(bool _force);
void updateUI();

static std::mutex g_adc_mutex;
static constexpr char config_file[] = "/root/.config/redpitaya/apps/streaming/streaming_config.json";


#define SERVER_CONFIG_PORT "8901"
#define SERVER_BROADCAST_PORT "8902"


//#define DEBUG_MODE


//Parameters

CBooleanParameter 	ss_start(			"SS_START", 			CBaseParameter::RW, false,0);

CBooleanParameter 	ss_dac_start(		"SS_DAC_START", 		CBaseParameter::RW, false,0);


CBooleanParameter 	ss_use_localfile(	"SS_USE_FILE", 	        CBaseParameter::RW, false,0);
CIntParameter 		ss_is_master(		"SS_IS_MASTER",	        CBaseParameter::RO, 0, 0, 0, 2);

CIntParameter		ss_port(  			"SS_PORT_NUMBER", 		CBaseParameter::RW, 8900,0,	1,65535);
CStringParameter    ss_ip_addr(			"SS_IP_ADDR",			CBaseParameter::RW, "127.0.0.1",0);
CIntParameter		ss_protocol(  		"SS_PROTOCOL", 			CBaseParameter::RW, 0 ,0,	0,1);
CIntParameter		ss_samples(  		"SS_SAMPLES", 			CBaseParameter::RW, 20000000 ,0,	-1,2000000000);
CIntParameter		ss_channels(  		"SS_CHANNEL", 			CBaseParameter::RW, 0 ,0,	0, 256);
CIntParameter		ss_resolution(  	"SS_RESOLUTION", 		CBaseParameter::RW, 0 ,0,	0, 256);
CIntParameter		ss_calib( 	 		"SS_USE_CALIB", 		CBaseParameter::RW, 1 ,0,	0,1);
CIntParameter		ss_save_mode(  		"SS_SAVE_MODE", 		CBaseParameter::RW, 0 ,0,	0,1);
CIntParameter		ss_rate(  			"SS_RATE", 				CBaseParameter::RW, 4 ,0,	1,65536);
CIntParameter		ss_format( 			"SS_FORMAT", 			CBaseParameter::RW, 0 ,0,	0, 2);
CIntParameter		ss_status( 			"SS_STATUS", 			CBaseParameter::RW, 1 ,0,	0,100);
CBooleanParameter 	ss_adc_data_pass(	"SS_ADC_DATA_PASS",		CBaseParameter::RW, false,0);
CIntParameter		ss_acd_max(			"SS_ACD_MAX", 			CBaseParameter::RO, getADCRate() ,0,	0, getADCRate());
CIntParameter		ss_attenuator( 		"SS_ATTENUATOR",		CBaseParameter::RW, 0 ,0,	0, 256);
CIntParameter		ss_ac_dc( 			"SS_AC_DC",				CBaseParameter::RW, 0 ,0,	0, 256);
CStringParameter 	redpitaya_model(	"RP_MODEL_STR", 		CBaseParameter::RO, getModelS(), 10);

CStringParameter    ss_dac_file(		"SS_DAC_FILE",			CBaseParameter::RW, "", 0);
CIntParameter    	ss_dac_file_type(	"SS_DAC_FILE_TYPE",		CBaseParameter::RW,  0 ,0, 0, 1);
CIntParameter    	ss_dac_gain(		"SS_DAC_GAIN",			CBaseParameter::RW,  0 ,0, 0, 256);
CIntParameter    	ss_dac_mode(		"SS_DAC_MODE",			CBaseParameter::RW,  0 ,0, 0, 1);
CIntParameter		ss_dac_speed(		"SS_DAC_HZ", 			CBaseParameter::RW, getDACRate() ,0,	1.0 / (65536.0 /getDACRate()) + 1.0, getDACRate());
CIntParameter		ss_dac_max_speed(	"SS_DAC_MAX_HZ",		CBaseParameter::RO, getDACRate() ,0,	0, getDACRate());
CIntParameter    	ss_dac_repeat(		"SS_DAC_REPEAT",		CBaseParameter::RW, -1 ,0, -2, 0);
CIntParameter    	ss_dac_rep_count(	"SS_DAC_REPEAT_COUNT",	CBaseParameter::RW, 1 ,0, 1, 2000000000);
CIntParameter		ss_dac_port(		"SS_DAC_PORT_NUMBER", 	CBaseParameter::RW, 8903,0,	1,65535);
CIntParameter		ss_dac_memory(		"SS_DAC_MEMORYCACHE", 	CBaseParameter::RW, 1024 * 1024,0,	0, 1024 * 1024 * 64);
CIntParameter		ss_dac_status( 		"SS_DAC_STATUS",		CBaseParameter::RW, 1 ,0,	0,100);

CIntParameter    	ss_lb_mode(			"SS_LB_MODE",		    CBaseParameter::RW,  0,0,   0,100);
CIntParameter		ss_lb_speed(		"SS_LB_SPEED", 			CBaseParameter::RW, -1,0,	-1,getDACRate());
CIntParameter		ss_lb_timeout(		"SS_LB_TIMEOUT",	 	CBaseParameter::RW,  1,0,	0,1);
CIntParameter		ss_lb_channels(		"SS_LB_CHANNELS",		CBaseParameter::RW,  1,0,	0,1);

CIntParameter		ss_adc_channels(	"SS_ADC_CHANNELS", 		CBaseParameter::RO, getADCChannels() ,0,	0,10);
CIntParameter		ss_dac_channels(  	"SS_DAC_CHANNELS", 		CBaseParameter::RO, getDACChannels() ,0,	0,10);
CBooleanParameter   ss_ac_dc_enable(  	"SS_IS_AC_DC", 			CBaseParameter::RO, getAC_DC() ,0);


uio_lib::COscilloscope::Ptr g_osc = nullptr;
uio_lib::CGenerator::Ptr    g_gen = nullptr;

streaming_lib::CStreamingFPGA::Ptr   		g_s_fpga = nullptr;
streaming_lib::CStreamingBufferCached::Ptr 	g_s_buffer = nullptr;
streaming_lib::CStreamingNet::Ptr    		g_s_net = nullptr;
streaming_lib::CStreamingFile::Ptr   		g_s_file = nullptr;

dac_streaming_lib::CDACStreamingApplication::Ptr g_dac_app = nullptr;
dac_streaming_lib::CDACStreamingManager::Ptr     g_dac_manger = nullptr;
ServerNetConfigManager::Ptr                     g_serverNetConfig = nullptr;

uio_lib::BoardMode g_isMaster = uio_lib::BoardMode::UNKNOWN;

std::atomic_bool g_serverRun(false);
std::atomic_bool g_dac_serverRun(false);

auto getADCChannels() -> uint8_t{
    uint8_t c = 0;
    if (rp_HPGetFastADCChannelsCount(&c) != RP_HP_OK){
        ERROR("Can't get fast ADC channels count");
    }
    return c;
}

auto getDACChannels() -> uint8_t{
    uint8_t c = 0;
    if (rp_HPGetFastDACChannelsCount(&c) != RP_HP_OK){
        ERROR("Can't get fast DAC channels count");
    }
    return c;
}

auto getAC_DC() -> bool{
    bool c = false;
    if (rp_HPGetFastADCIsAC_DC(&c) != RP_HP_OK){
        ERROR("Can't get fast AC/DC mode");
    }
    return c;
}

auto getDACRate() -> uint32_t{
    uint32_t c = 0;
    if (rp_HPGetBaseFastDACSpeedHz(&c) != RP_HP_OK){
        ERROR("Can't get fast DAC channels count");
		return 1;
    }
    return c;
}

auto getADCRate() -> uint32_t{
    uint32_t c = 0;
    if (rp_HPGetBaseFastADCSpeedHz(&c) != RP_HP_OK){
        ERROR("Can't get fast ADC channels count");
    }
    return c;
}

auto getADCBits() -> uint8_t{
    uint8_t c = 0;
    if (rp_HPGetFastADCBits(&c) != RP_HP_OK){
        ERROR("Can't get fast ADC bits");
    }
    return c;
}

auto getModel() -> broadcast_lib::EModel{
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
			return broadcast_lib::EModel::RP_125_14;
        case STEM_125_14_Z7020_v1_0:
        case STEM_125_14_Z7020_LN_v1_1:
			return broadcast_lib::EModel::RP_125_14_Z20;

        case STEM_122_16SDR_v1_0:
        case STEM_122_16SDR_v1_1:
            return broadcast_lib::EModel::RP_122_16;

        case STEM_125_14_Z7020_4IN_v1_0:
        case STEM_125_14_Z7020_4IN_v1_2:
        case STEM_125_14_Z7020_4IN_v1_3:
            return broadcast_lib::EModel::RP_125_4CH;

        case STEM_250_12_v1_0:
        case STEM_250_12_v1_1:
        case STEM_250_12_v1_2:
		case STEM_250_12_v1_2a:
            return broadcast_lib::EModel::RP_250_12;
		case STEM_250_12_120:
			return broadcast_lib::EModel::RP_250_12;
        default:
            ERROR("Can't get board model");
            exit(-1);
    }
    return broadcast_lib::EModel::RP_125_14;
}

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
            return "Z20_250_12";
		case STEM_250_12_120:
            return "Z20_250_12_120";
        default:
            ERROR("Can't get board model");
            exit(-1);
    }
    return "Z10";
}


//Application description
const char *rp_app_desc(void)
{
	return (const char *)"Red Pitaya Stream server application.\n";
}

auto termSignalHandler(int) -> void {
}

auto installTermSignalHandler() -> void {
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = termSignalHandler;
    sigaction(SIGINT, &action, NULL);
}

//Application init
auto rp_app_init(void) -> int {
	fprintf(stderr, "Loading stream server version %s-%s.\n", VERSION_STR, REVISION_STR);
	installTermSignalHandler();
	CDataManager::GetInstance()->SetParamInterval(100);
	g_serverRun = false;
	try {
		try {
	        auto uioList = uio_lib::GetUioList();
			for (auto &uio : uioList){
				if (uio.nodeName == "rp_oscilloscope")
				{
					WARNING("Check master/slave");
					auto osc = uio_lib::COscilloscope::create(uio,1,true,getADCRate(),false,getADCBits(),getADCChannels());
					g_isMaster = osc->isMaster();
					WARNING("Detected %s mode",g_isMaster == uio_lib::BoardMode::MASTER ? "Master" : (g_isMaster == uio_lib::BoardMode::SLAVE ? "Slave" : "Unknown"));
					break;
				}
			}
			TRACE("ss_ip_addr %s",ss_ip_addr.Value().c_str());
			g_serverNetConfig = std::make_shared<ServerNetConfigManager>(config_file,g_isMaster != uio_lib::BoardMode::SLAVE ? broadcast_lib::EMode::AB_SERVER_MASTER
																							    : broadcast_lib::EMode::AB_SERVER_SLAVE,
																								ss_ip_addr.Value(),
																								SERVER_CONFIG_PORT);
            g_serverNetConfig->getNewSettingsNofiy.connect([](){
				updateUI();
			});

            g_serverNetConfig->startStreamingNofiy.connect([](){
                startServer(false);
			});

            g_serverNetConfig->startStreamingTestNofiy.connect([](){
                startServer(true);
			});

            g_serverNetConfig->stopStreamingNofiy.connect([](){
                stopNonBlocking(ServerNetConfigManager::EStopReason::NORMAL);
			});

            g_serverNetConfig->startDacStreamingNofiy.connect([](){
				startDACServer(false);
			});

            g_serverNetConfig->startDacStreamingTestNofiy.connect([](){
				startDACServer(true);
			});

            g_serverNetConfig->stopDacStreamingNofiy.connect([](){
                stopDACNonBlocking(dac_streaming_lib::CDACStreamingManager::NR_STOP);
			});

            g_serverNetConfig->startADCNofiy.connect([](){
            	startADC();
        	});

            g_serverNetConfig->getNewSettingsNofiy.connect([](){
                WARNING("Info: Get new settigns from client\n");
            });


		}catch (std::exception& e)
			{
				WARNING("Init ServerNetConfigManager() %s\n",e.what());
			}
		ss_is_master.SendValue(g_isMaster);
		ss_status.SendValue(0);
		ss_adc_data_pass.SendValue(0);
		if (g_serverNetConfig->getSettingsRef().isSetted()){
			updateUI();
		}else{
			setConfig(true);
		}

        streaming_lib::CStreamingFile::makeEmptyDir(FILE_PATH);
		if (rp_HPGetIsAttenuatorControllerPresentOrDefault() && rp_HPGetFastADCIsAC_DCOrDefault() && rp_HPGetIsGainDACx5OrDefault()){
	    	rp_max7311::rp_initController();
		}

	}catch (std::exception& e)
	{
        ERROR("Error: rp_app_init() %s",e.what());
	}
	return 0;
}

//Application exit
auto rp_app_exit(void) -> int {
	g_serverNetConfig->stop();
    stopServer(ServerNetConfigManager::EStopReason::NORMAL);
    stopDACServer(dac_streaming_lib::CDACStreamingManager::NR_STOP);
    aprintf(stderr, "Unloading stream server version %s-%s.\n", VERSION_STR, REVISION_STR);
	return 0;
}

//Set parameters
auto rp_set_params(rp_app_params_t *, int) -> int {
    return 0;
}

//Get parameters
auto rp_get_params(rp_app_params_t **) -> int{
    return 0;
}

//Get signals
auto rp_get_signals(float ***, int *, int *) -> int {
    return 0;
}

//Update signals
auto UpdateSignals(void) -> void {
}

auto saveConfigInFile() -> void {
	if (!g_serverNetConfig->getSettingsRef().writeToFile(config_file)){
        ERROR("Error save to file (%s)",config_file);
	}
}

void updateUI(){
	ss_port.SendValue(std::atoi(g_serverNetConfig->getSettingsRef().getPort().c_str()));
	ss_dac_file.SendValue(g_serverNetConfig->getSettingsRef().getDACFile());
	ss_dac_port.SendValue(std::atoi(g_serverNetConfig->getSettingsRef().getDACPort().c_str()));

    ss_use_localfile.SendValue(g_serverNetConfig->getSettingsRef().getSaveType());
	ss_protocol.SendValue(g_serverNetConfig->getSettingsRef().getProtocol());
	ss_channels.SendValue(g_serverNetConfig->getSettingsRef().getChannels());
	ss_resolution.SendValue(g_serverNetConfig->getSettingsRef().getResolution());
    ss_save_mode.SendValue(g_serverNetConfig->getSettingsRef().getType());
    ss_format.SendValue(g_serverNetConfig->getSettingsRef().getFormat());
    ss_attenuator.SendValue(g_serverNetConfig->getSettingsRef().getAttenuator());
	ss_ac_dc.SendValue(g_serverNetConfig->getSettingsRef().getAC_DC());
    ss_dac_file_type.SendValue(g_serverNetConfig->getSettingsRef().getDACFileType());
    ss_dac_gain.SendValue(g_serverNetConfig->getSettingsRef().getDACGain());
    ss_dac_mode.SendValue(g_serverNetConfig->getSettingsRef().getDACMode());

	ss_dac_repeat.SendValue(g_serverNetConfig->getSettingsRef().getDACRepeat());
	ss_dac_rep_count.SendValue(g_serverNetConfig->getSettingsRef().getDACRepeatCount());
	ss_dac_memory.SendValue(g_serverNetConfig->getSettingsRef().getDACMemoryUsage());
	ss_dac_speed.SendValue(g_serverNetConfig->getSettingsRef().getDACHz());
	ss_rate.SendValue(g_serverNetConfig->getSettingsRef().getDecimation());
	ss_samples.SendValue(g_serverNetConfig->getSettingsRef().getSamples());
    ss_calib.SendValue(g_serverNetConfig->getSettingsRef().getCalibration());

	ss_lb_channels.SendValue(g_serverNetConfig->getSettingsRef().getLoopbackChannels());
	ss_lb_mode.SendValue(g_serverNetConfig->getSettingsRef().getLoopbackMode());
	ss_lb_timeout.SendValue(g_serverNetConfig->getSettingsRef().getLoopbackTimeout());
	ss_lb_speed.SendValue(g_serverNetConfig->getSettingsRef().getLoopbackSpeed());
}

void setConfig(bool _force){
	bool needUpdate = false;

	if (ss_port.IsNewValue() || _force)
	{
		ss_port.Update();
		g_serverNetConfig->getSettingsRef().setPort(std::to_string(ss_port.Value()));
		needUpdate = true;
	}

	if (ss_dac_file.IsNewValue() || _force)
	{
		ss_dac_file.Update();
		g_serverNetConfig->getSettingsRef().setDACFile(ss_dac_file.Value());
		needUpdate = true;
	}

	if (ss_dac_port.IsNewValue() || _force)
	{
		ss_dac_port.Update();
		g_serverNetConfig->getSettingsRef().setDACPort(std::to_string(ss_dac_port.Value()));
		needUpdate = true;
	}

	if (ss_ip_addr.IsNewValue() || _force)
	{
		ss_ip_addr.Update();
		g_serverNetConfig->startServer(ss_ip_addr.Value(),SERVER_CONFIG_PORT);
		g_serverNetConfig->startBroadcast(getModel(), ss_ip_addr.Value(),SERVER_BROADCAST_PORT);
	}

	if (ss_use_localfile.IsNewValue() || _force)
	{
		ss_use_localfile.Update();
		g_serverNetConfig->getSettingsRef().setSaveType((CStreamSettings::SaveType)ss_use_localfile.Value());
		needUpdate = true;
	}

	if (ss_protocol.IsNewValue() || _force)
	{
		ss_protocol.Update();
		g_serverNetConfig->getSettingsRef().setProtocol((CStreamSettings::Protocol)ss_protocol.Value());
		needUpdate = true;
	}

	if (ss_channels.IsNewValue() || _force)
	{
		ss_channels.Update();
		g_serverNetConfig->getSettingsRef().setChannels(ss_channels.Value());
		needUpdate = true;
	}

	if (ss_resolution.IsNewValue() || _force)
	{
		ss_resolution.Update();
		g_serverNetConfig->getSettingsRef().setResolution((CStreamSettings::Resolution)ss_resolution.Value());
		needUpdate = true;
	}

	if (ss_save_mode.IsNewValue() || _force)
	{
		ss_save_mode.Update();
		g_serverNetConfig->getSettingsRef().setType((CStreamSettings::DataType)ss_save_mode.Value());
		needUpdate = true;
	}

	if (ss_rate.IsNewValue() || _force)
	{
		ss_rate.Update();
		g_serverNetConfig->getSettingsRef().setDecimation(ss_rate.Value());
		needUpdate = true;
	}

	if (ss_format.IsNewValue() || _force)
	{
		ss_format.Update();
		g_serverNetConfig->getSettingsRef().setFormat((CStreamSettings::DataFormat)ss_format.Value());
		needUpdate = true;
	}

	if (ss_samples.IsNewValue() || _force)
	{
		ss_samples.Update();
		g_serverNetConfig->getSettingsRef().setSamples(ss_samples.Value());
		needUpdate = true;
	}

	if (rp_HPGetFastADCIsLV_HVOrDefault()){
		if (ss_attenuator.IsNewValue() || _force)
		{
			ss_attenuator.Update();
			g_serverNetConfig->getSettingsRef().setAttenuator(ss_attenuator.Value());
			needUpdate = true;
		}
	}else{
		g_serverNetConfig->getSettingsRef().setAttenuator(0);
	}

	if (ss_calib.IsNewValue() || _force)
	{
		ss_calib.Update();
		g_serverNetConfig->getSettingsRef().setCalibration(ss_calib.Value());
		needUpdate = true;
	}

	if (rp_HPGetFastADCIsAC_DCOrDefault()){
		if (ss_ac_dc.IsNewValue() || _force)
		{
			ss_ac_dc.Update();
			g_serverNetConfig->getSettingsRef().setAC_DC(ss_ac_dc.Value());
			needUpdate = true;
		}
	}
	else
		g_serverNetConfig->getSettingsRef().setAC_DC(0xF);


	if (ss_dac_file_type.IsNewValue() || _force)
	{
		ss_dac_file_type.Update();
		g_serverNetConfig->getSettingsRef().setDACFileType((CStreamSettings::DataFormat)ss_dac_file_type.Value());
		needUpdate = true;
	}

	if (rp_HPGetIsGainDACx5OrDefault()){
		if (ss_dac_gain.IsNewValue() || _force)
		{
			ss_dac_gain.Update();
			g_serverNetConfig->getSettingsRef().setDACGain(ss_dac_gain.Value());
			needUpdate = true;
		}
	}
	else
		g_serverNetConfig->getSettingsRef().setDACGain(0);


	if (ss_dac_mode.IsNewValue() || _force)
	{
		ss_dac_mode.Update();
		g_serverNetConfig->getSettingsRef().setDACMode((CStreamSettings::DACType)ss_dac_mode.Value());
		needUpdate = true;
	}

	if (ss_dac_repeat.IsNewValue() || _force)
	{
		ss_dac_repeat.Update();
		g_serverNetConfig->getSettingsRef().setDACRepeat((CStreamSettings::DACRepeat)ss_dac_repeat.Value());
		needUpdate = true;
	}

	if (ss_dac_rep_count.IsNewValue() || _force)
	{
		ss_dac_rep_count.Update();
		g_serverNetConfig->getSettingsRef().setDACRepeatCount(ss_dac_rep_count.Value());
		needUpdate = true;
	}

	if (ss_dac_memory.IsNewValue() || _force)
	{
		ss_dac_memory.Update();
		g_serverNetConfig->getSettingsRef().setDACMemoryUsage(ss_dac_memory.Value());
		needUpdate = true;
	}

	if (ss_dac_speed.IsNewValue() || _force)
	{
		ss_dac_speed.Update();
		g_serverNetConfig->getSettingsRef().setDACHz(ss_dac_speed.Value());
		needUpdate = true;
	}

	if (ss_lb_channels.IsNewValue() || _force)
	{
		ss_lb_channels.Update();
		g_serverNetConfig->getSettingsRef().setLoopbackChannels((CStreamSettings::LOOPBACKChannels)ss_lb_channels.Value());
		needUpdate = true;
	}

	if (ss_lb_mode.IsNewValue() || _force)
	{
		ss_lb_mode.Update();
		g_serverNetConfig->getSettingsRef().setLoopbackMode((CStreamSettings::LOOPBACKMode)ss_lb_mode.Value());
		needUpdate = true;
	}

	if (ss_lb_speed.IsNewValue() || _force)
	{
		ss_lb_speed.Update();
		g_serverNetConfig->getSettingsRef().setLoopbackSpeed(ss_lb_speed.Value());
		needUpdate = true;
	}

	if (ss_lb_timeout.IsNewValue() || _force)
	{
		ss_lb_timeout.Update();
		g_serverNetConfig->getSettingsRef().setLoopbackTimeout(ss_lb_timeout.Value());
		needUpdate = true;
	}

	if (needUpdate){
		saveConfigInFile();
	}
}

//Update parameters
void UpdateParams(void) {
	try{
		setConfig(false);
		if (ss_start.IsNewValue())
		{
			ss_start.Update();
			if (ss_start.Value() == 1){
                startServer(false);
				startADC();
			}else{
                stopNonBlocking(ServerNetConfigManager::EStopReason::NORMAL);
			}
		}

		if (ss_dac_start.IsNewValue())
		{
			ss_dac_start.Update();
			if (ss_dac_start.Value() == 1){
				startDACServer(false);
			}else{
                stopDACNonBlocking(dac_streaming_lib::CDACStreamingManager::NR_STOP);
			}
		}
	}catch (std::exception& e)
	{
        ERROR("UpdateParams() %s",e.what());
	}
}

void PostUpdateSignals() {}

void OnNewParams(void) {
	//Update parameters
	UpdateParams();
}

void OnNewSignals(void) {
	UpdateSignals();
}



void startServer(bool testMode) {
	// Search oscilloscope

	try{
        std::lock_guard<std::mutex> guard(g_adc_mutex);

        g_s_file = nullptr;
        g_s_net = nullptr;
        g_s_buffer = nullptr;
        g_s_fpga = nullptr;
        g_osc = nullptr;

		CStreamSettings settings = testMode ? g_serverNetConfig->getTempSettings() : g_serverNetConfig->getSettings();
		if (!settings.isSetted()) return;

		auto resolution   = settings.getResolution();
		auto format       = settings.getFormat();
		auto sock_port    = settings.getPort();
		auto use_file     = settings.getSaveType();
		auto protocol     = settings.getProtocol();
		auto channel      = settings.getChannels();
		auto rate         = settings.getDecimation();
		auto ip_addr_host = ss_ip_addr.Value();
		auto samples      = settings.getSamples();
		auto save_mode    = settings.getType();

		auto use_calib    = settings.getCalibration();
		auto attenuator   = settings.getAttenuator();
		auto ac_dc        = settings.getAC_DC();
 		auto max_channels = getADCChannels();

		if (rp_CalibInit() != RP_HW_CALIB_OK){
	        ERROR("Error init calibration");
    	}

        auto uioList = uio_lib::GetUioList();
		int32_t ch_off[4] = {0,0,0,0};
		double  ch_gain[4] = {1,1,1,1};
		bool  filterBypass = true;
		uint32_t aa_ch[4] = {0,0,0,0};
		uint32_t bb_ch[4] = {0,0,0,0};
		uint32_t kk_ch[4] = {0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF};
		uint32_t pp_ch[4] = {0,0,0,0};

		filterBypass = !rp_HPGetFastADCIsFilterPresentOrDefault();
		if (use_calib) {
            for(uint8_t ch = 0; ch < max_channels; ++ch){
				rp_acq_ac_dc_mode_calib_t  mode = ( CStreamSettings::checkChannel(ac_dc, ch) ? RP_DC_CALIB : RP_AC_CALIB);
                if (CStreamSettings::checkChannel(attenuator,ch) == false){
                    if (rp_CalibGetFastADCCalibValue((rp_channel_calib_t)ch,mode,&ch_gain[ch],&ch_off[ch]) != RP_HW_CALIB_OK){
                        ERROR("Error get calibration channel: %d",ch);
                    }

                    if (!filterBypass){
                        channel_filter_t f;
                        if (rp_CalibGetFastADCFilter((rp_channel_calib_t)ch,&f) != RP_HW_CALIB_OK){
                            ERROR("Error get filter value: %d",ch);
                        }
                        aa_ch[ch] = f.aa;
                        bb_ch[ch] = f.bb;
                        kk_ch[ch] = f.kk;
                        pp_ch[ch] = f.pp;
                    }
                }
				else{
                    if (rp_CalibGetFastADCCalibValue_1_20((rp_channel_calib_t)ch,mode,&ch_gain[ch],&ch_off[ch]) != RP_HW_CALIB_OK){
                        ERROR("Error get calibration channel: %d",ch);
                    }

                    if (!filterBypass){
                        channel_filter_t f;
                        if (rp_CalibGetFastADCFilter_1_20((rp_channel_calib_t)ch,&f) != RP_HW_CALIB_OK){
                            ERROR("Error get filter value: %d",ch);
                        }
                        aa_ch[ch] = f.aa;
                        bb_ch[ch] = f.bb;
                        kk_ch[ch] = f.kk;
                        pp_ch[ch] = f.pp;
                    }
                }
			}
		}

        if (rp_HPGetIsAttenuatorControllerPresentOrDefault()){
		    rp_max7311::rp_setAttenuator(RP_MAX7311_IN1, attenuator & CStreamSettings::CH1 ? RP_ATTENUATOR_1_20 : RP_ATTENUATOR_1_1);
		    rp_max7311::rp_setAttenuator(RP_MAX7311_IN2, attenuator & CStreamSettings::CH2 ? RP_ATTENUATOR_1_20 : RP_ATTENUATOR_1_1);
        }

        if (rp_HPGetFastADCIsAC_DCOrDefault()){
	    	rp_max7311::rp_setAC_DC(RP_MAX7311_IN1, ac_dc & CStreamSettings::CH1 ? RP_DC_MODE : RP_AC_MODE);
	    	rp_max7311::rp_setAC_DC(RP_MAX7311_IN2, ac_dc & CStreamSettings::CH2 ? RP_DC_MODE : RP_AC_MODE);
        }

        for (auto &uio : uioList)
		{
			if (uio.nodeName == "rp_oscilloscope")
			{
				TRACE("COscilloscope::Create rate %d",rate);
                g_osc = uio_lib::COscilloscope::create(uio,rate,g_isMaster != uio_lib::BoardMode::SLAVE,getADCRate(),!filterBypass,getADCBits(),max_channels);
				for(uint8_t ch = 0; ch < max_channels; ++ch){
					g_osc->setCalibration(ch,ch_off[ch],ch_gain[ch]);
                	g_osc->setFilterCalibration(ch,aa_ch[ch],bb_ch[ch],kk_ch[ch],pp_ch[ch]);
				}
                g_osc->setFilterBypass(filterBypass);
				g_osc->set8BitMode(resolution == CStreamSettings::BIT_8);
				break;
			}
		}

		g_s_buffer = streaming_lib::CStreamingBufferCached::create();
		auto g_s_buffer_w = std::weak_ptr<streaming_lib::CStreamingBufferCached>(g_s_buffer);
        if (use_file == CStreamSettings::NET) {
            auto proto = protocol == CStreamSettings::TCP ? net_lib::EProtocol::P_TCP : net_lib::EProtocol::P_UDP;
            g_s_net = streaming_lib::CStreamingNet::create(ip_addr_host,sock_port,proto);


            g_s_net->getBuffer = [g_s_buffer_w]() -> DataLib::CDataBuffersPack::Ptr{
                auto obj = g_s_buffer_w.lock();
                if (obj) {
                    return obj->readBuffer();
                }
                return nullptr;
            };

			g_s_net->unlockBufferF = [g_s_buffer_w](){
				auto obj = g_s_buffer_w.lock();
				if (obj){
					obj->unlockBufferRead();
				}
				return nullptr;
        	};
        }

        if (use_file == CStreamSettings::FILE) {
            auto f_path = std::string(FILE_PATH);
            g_s_file = streaming_lib::CStreamingFile::create(format,f_path,samples, save_mode == CStreamSettings::VOLT, testMode);
            g_s_file->stopNotify.connect([](streaming_lib::CStreamingFile::EStopReason r){
                switch (r) {
                    case streaming_lib::CStreamingFile::EStopReason::NORMAL:{
                        stopNonBlocking(ServerNetConfigManager::EStopReason::NORMAL);
                        break;
                    }

                    case streaming_lib::CStreamingFile::EStopReason::OUT_SPACE:{
                        stopNonBlocking(ServerNetConfigManager::EStopReason::SD_FULL);
                        break;
                    }

                    case streaming_lib::CStreamingFile::EStopReason::REACH_LIMIT:{
                        stopNonBlocking(ServerNetConfigManager::EStopReason::DONE);
                        break;
                    }
                }
            });
        }


        g_s_fpga = std::make_shared<streaming_lib::CStreamingFPGA>(g_osc,16);
        uint8_t resolution_val = (resolution == CStreamSettings::BIT_8 ? 8 : 16);
		TRACE("Set channels resolution %d",resolution_val);

        for(int i = 0; i < max_channels; i++){
            if(CStreamSettings::checkChannel(channel,i)){
                g_s_fpga->addChannel((DataLib::EDataBuffersPackChannel)i, CStreamSettings::checkChannel(attenuator,i) ? DataLib::CDataBuffer::ATT_1_20 : DataLib::CDataBuffer::ATT_1_1,resolution_val);
                g_s_buffer->addChannel((DataLib::EDataBuffersPackChannel)i,uio_lib::osc_buf_size,resolution_val);
            }
        }

		g_s_buffer->generateBuffers();
        g_s_fpga->setTestMode(testMode);

		auto weak_obj = std::weak_ptr<streaming_lib::CStreamingBufferCached>(g_s_buffer);
        g_s_fpga->getBuffF = [weak_obj](uint64_t lostFPGA) -> DataLib::CDataBuffersPack::Ptr {
            auto obj = weak_obj.lock();
            if (obj){
                return obj->getFreeBuffer(lostFPGA);
            }
            return nullptr;
        };

        g_s_fpga->unlockBuffF = [weak_obj](){
            auto obj = weak_obj.lock();
            if (obj){
                obj->unlockBufferWrite();
            }
            return nullptr;
        };

        auto g_s_file_w = std::weak_ptr<streaming_lib::CStreamingFile>(g_s_file);
		auto g_s_net_w = std::weak_ptr<streaming_lib::CStreamingNet>(g_s_net);
        g_s_fpga->oscNotify.connect([g_s_net_w,g_s_file_w,rate,g_s_buffer_w](DataLib::CDataBuffersPack::Ptr pack) {

			auto f_obj = g_s_file_w.lock();
			auto b_obj = g_s_buffer_w.lock();
            if (f_obj && b_obj){
				auto p = b_obj->readBuffer();
				if (p){
                	f_obj->passBuffers(p);
					b_obj->unlockBufferRead();
				}
            }
        });

        char time_str[40];
        struct tm *timenow;
        time_t now = time(nullptr);
        timenow = gmtime(&now);
        strftime(time_str, sizeof(time_str), "%Y-%m-%d_%H-%M-%S", timenow);
        std::string filenameDate = time_str;

        if (g_s_net){
            g_s_net->run();
			usleep(1000);
            if (g_s_net->getProtocol() == net_lib::EProtocol::P_TCP){
                g_serverNetConfig->sendServerStartedTCP();
            }
            if (g_s_net->getProtocol() == net_lib::EProtocol::P_UDP){
                g_serverNetConfig->sendServerStartedUDP();
            }
        }

        if (g_s_file){
            g_s_file->run(filenameDate);
            g_serverNetConfig->sendServerStartedSD();
        }
        ss_status.SendValue(1);

		TRACE("Start server");

	}catch (std::exception& e)
	{
		TRACE("Error: StartServer() %s",e.what());
	}
}

void startADC(){
	try{
        if (g_s_fpga){
            g_s_fpga->runNonBlock();
            ss_adc_data_pass.SendValue(1);
            g_serverNetConfig->sendADCStarted();
        }
	}catch (std::exception& e)
	{
        ERROR("Error: startADC() %s",e.what());
	}
}

void stopNonBlocking(ServerNetConfigManager::EStopReason x){
	try{
        std::thread th(stopServer ,x);
		th.detach();
	}catch (std::exception& e)
	{
        ERROR("Error: stopNonBlocking() %s",e.what());
	}
}


void stopServer(ServerNetConfigManager::EStopReason x){
	try{
        std::lock_guard<std::mutex> guard(g_adc_mutex);
		ss_adc_data_pass.SendValue(0);
		switch (x)
		{
            case ServerNetConfigManager::EStopReason::NORMAL:
                g_serverNetConfig->sendServerStopped();
				ss_status.SendValue(0);
                break;
            case ServerNetConfigManager::EStopReason::SD_FULL:
                g_serverNetConfig->sendServerStoppedSDFull();
				ss_status.SendValue(2);
                break;
            case ServerNetConfigManager::EStopReason::DONE:
                g_serverNetConfig->sendServerStoppedDone();
				ss_status.SendValue(3);
                break;
            default:
                throw runtime_error("Unknown state");
                break;
		}
		if (g_s_buffer) g_s_buffer->notifyToDestory();
		if (g_s_file) g_s_file->disableNotify();
		g_s_net = nullptr;
        g_s_file = nullptr;
        g_s_buffer = nullptr;
        g_s_fpga = nullptr;
        TRACE("Stop server");
	}catch (std::exception& e)
	{
        ERROR("%s",e.what());
	}
}


auto startDACServer(bool testMode) -> void{
    if (!g_serverNetConfig) return;

    g_gen = nullptr;
    g_dac_app = nullptr;
    g_dac_manger = nullptr;

	try{
		CStreamSettings settings = testMode ? g_serverNetConfig->getTempSettings() : g_serverNetConfig->getSettings();
		if (!settings.isSetted()) return;
		// g_dac_serverRun = true;
		auto use_file     =  settings.getDACMode();
		auto sock_port    =  settings.getDACPort();
		auto dac_speed    =  settings.getDACHz();
		auto ip_addr_host = "127.0.0.1";

		auto use_calib    = settings.getCalibration();
		if (rp_CalibInit() != RP_HW_CALIB_OK){
	        ERROR("Error init calibration");
    	}
		auto dac_gain = settings.getDACGain();
		auto max_channels = getDACChannels();

        auto uioList = uio_lib::GetUioList();
		int32_t ch_off[4] = { 0 , 0 , 0 , 0 };
		double  ch_gain[4] = { 1 , 1 , 1 ,1 };

		if (use_calib) {
			for(uint8_t ch = 0; ch < max_channels; ++ch){
				rp_gen_gain_calib_t  mode = dac_gain == CStreamSettings::X1 ? RP_GAIN_CALIB_1X : RP_GAIN_CALIB_5X;
				if (rp_CalibGetFastDACCalibValue((rp_channel_calib_t)ch,mode,&ch_gain[ch],&ch_off[ch]) != RP_HW_CALIB_OK){
					ERROR("Error get calibration channel: %d",ch);
				}
			}
		}

		if (rp_HPGetIsGainDACx5OrDefault()){
        	rp_max7311::rp_setGainOut(RP_MAX7311_OUT1, dac_gain == CStreamSettings::X1 ? RP_GAIN_2V : RP_GAIN_10V);
        	rp_max7311::rp_setGainOut(RP_MAX7311_OUT2, dac_gain == CStreamSettings::X1 ? RP_GAIN_2V : RP_GAIN_10V);
		}

        for (auto uio : uioList)
		{
			if (uio.nodeName == "rp_dac")
			{
                g_gen = uio_lib::CGenerator::create(uio, true , true ,dac_speed,getDACRate());
                g_gen->setCalibration(ch_off[0],ch_gain[0],ch_off[1],ch_gain[1]);
                g_gen->setDacHz(dac_speed);
			}
		}

        if (!g_gen){
            ERROR("Error init generator module");
        	return;
		}

        if (use_file == CStreamSettings::DAC_NET) {
            g_dac_manger = dac_streaming_lib::CDACStreamingManager::Create(ip_addr_host,sock_port);
        }

        if (use_file == CStreamSettings::DAC_FILE) {
            auto format = settings.getDACFileType();
            auto filePath = settings.getDACFile();
            auto dacRepeatMode = settings.getDACRepeat();
            auto dacRepeatCount = settings.getDACRepeatCount();
            auto dacMemory = settings.getDACMemoryUsage();
            if (format == CStreamSettings::WAV) {
                g_dac_manger = dac_streaming_lib::CDACStreamingManager::Create(dac_streaming_lib::CDACStreamingManager::WAV_TYPE,filePath,dacRepeatMode,dacRepeatCount,dacMemory);
            }else if (format == CStreamSettings::TDMS) {
                g_dac_manger = dac_streaming_lib::CDACStreamingManager::Create(dac_streaming_lib::CDACStreamingManager::TDMS_TYPE,filePath,dacRepeatMode,dacRepeatCount,dacMemory);
            }else{
                g_serverNetConfig->sendDACServerStoppedSDBroken();
                return;
            }
            g_dac_manger->notifyStop.connect([](dac_streaming_lib::CDACStreamingManager::NotifyResult status)
            {
                stopDACNonBlocking(status);
            });

        }

        g_dac_app = std::make_shared<dac_streaming_lib::CDACStreamingApplication>(g_dac_manger, g_gen);
        g_dac_app->setTestMode(testMode);

        g_dac_app->runNonBlock();
        if (g_dac_manger->isLocalMode()){
            g_serverNetConfig->sendDACServerStartedSD();
        }else{
            g_serverNetConfig->sendDACServerStarted();
        }

        TRACE("Start dac server");
	}catch (std::exception& e)
	{
        ERROR("Error: startDACServer() %s",e.what());
	}
}

auto stopDACNonBlocking(dac_streaming_lib::CDACStreamingManager::NotifyResult x) -> void{
	try{
		std::thread th(stopDACServer ,x);
		th.detach();
	}catch (std::exception& e)
	{
        ERROR("Error: stopDACNonBlocking() %s",e.what());
	}
}


auto stopDACServer(dac_streaming_lib::CDACStreamingManager::NotifyResult x) -> void{
	try{
        if (g_dac_app)
        {
            g_dac_app->stop();
            g_dac_app = nullptr;
        }
        if (g_serverNetConfig){
            switch (x)
            {
                case dac_streaming_lib::CDACStreamingManager::NR_STOP:
					g_serverNetConfig->sendDACServerStopped();
					break;
                case dac_streaming_lib::CDACStreamingManager::NR_ENDED:
					g_serverNetConfig->sendDACServerStoppedSDDone();
					break;
                case dac_streaming_lib::CDACStreamingManager::NR_EMPTY:
					g_serverNetConfig->sendDACServerStoppedSDEmpty();
					break;
                case dac_streaming_lib::CDACStreamingManager::NR_BROKEN:
					g_serverNetConfig->sendDACServerStoppedSDBroken();
					break;
                case dac_streaming_lib::CDACStreamingManager::NR_MISSING_FILE:
					g_serverNetConfig->sendDACServerStoppedSDMissingFile();
					break;
				default:
					throw runtime_error("Unknown state");
					break;
            }
        }
        TRACE("Stop dac server");
	}catch (std::exception& e)
	{
        ERROR("%s",e.what());
	}
}
