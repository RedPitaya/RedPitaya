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
static constexpr char config_file[] = "/root/.streaming_config";


#define SS_NET		0
#define SS_FILE  	1

#define SS_TCP		1
#define SS_UDP  	2

#define SS_CH1  	1
#define SS_CH2  	2
#define SS_BOTH 	3

#define SS_8BIT		1
#define SS_16BIT	2

#define SS_RAW		1
#define SS_VOLT  	2

#define SS_WAV		0
#define SS_TDMS  	1
#define SS_BIN  	2

#define SS_A_1_1  	1
#define SS_A_1_20  	2

#define SS_DAC_GAIN_1   0
#define SS_DAC_GAIN_5  	1

#define SS_CALIB_OFF    1
#define SS_CALIB_ON     2

#define SERVER_CONFIG_PORT "8901"
#define SERVER_BROADCAST_PORT "8902"


//#define DEBUG_MODE


//Parameters

CBooleanParameter 	ss_start(			"SS_START", 			CBaseParameter::RW, false,0);

CBooleanParameter 	ss_dac_start(		"SS_DAC_START", 		CBaseParameter::RW, false,0);


CBooleanParameter 	ss_use_localfile(	"SS_USE_FILE", 	        CBaseParameter::RW, false,0);
CIntParameter		ss_port(  			"SS_PORT_NUMBER", 		CBaseParameter::RW, 8900,0,	1,65535);
CStringParameter    ss_ip_addr(			"SS_IP_ADDR",			CBaseParameter::RW, "127.0.0.1",0);
CIntParameter		ss_protocol(  		"SS_PROTOCOL", 			CBaseParameter::RW, 1 ,0,	1,2);
CIntParameter		ss_samples(  		"SS_SAMPLES", 			CBaseParameter::RW, 20000000 ,0,	-1,2000000000);
CIntParameter		ss_channels(  		"SS_CHANNEL", 			CBaseParameter::RW, 1 ,0,	1,3);
CIntParameter		ss_resolution(  	"SS_RESOLUTION", 		CBaseParameter::RW, 1 ,0,	1,2);
CIntParameter		ss_calib( 	 		"SS_USE_CALIB", 		CBaseParameter::RW, 2 ,0,	1,2);
CIntParameter		ss_save_mode(  		"SS_SAVE_MODE", 		CBaseParameter::RW, 1 ,0,	1,2);
CIntParameter		ss_rate(  			"SS_RATE", 				CBaseParameter::RW, 4 ,0,	1,65536);
CIntParameter		ss_format( 			"SS_FORMAT", 			CBaseParameter::RW, 0 ,0,	0, 2);
CIntParameter		ss_status( 			"SS_STATUS", 			CBaseParameter::RW, 1 ,0,	0,100);
CBooleanParameter 	ss_adc_data_pass(	"SS_ADC_DATA_PASS",		CBaseParameter::RW, false,0);
CIntParameter		ss_acd_max(			"SS_ACD_MAX", 			CBaseParameter::RO, getADCRate() ,0,	0, getADCRate());
CIntParameter		ss_attenuator( 		"SS_ATTENUATOR",		CBaseParameter::RW, 1 ,0,	1, 2);
CIntParameter		ss_ac_dc( 			"SS_AC_DC",				CBaseParameter::RW, 1 ,0,	1, 2);
CStringParameter 	redpitaya_model(	"RP_MODEL_STR", 		CBaseParameter::ROSA, getModelS(), 10);

CStringParameter    ss_dac_file(		"SS_DAC_FILE",			CBaseParameter::RW, "", 0);
CIntParameter    	ss_dac_file_type(	"SS_DAC_FILE_TYPE",		CBaseParameter::RW,  0 ,0, 0, 1);
CIntParameter    	ss_dac_gain(		"SS_DAC_GAIN",			CBaseParameter::RW,  0 ,0, 0, 1);
CIntParameter    	ss_dac_mode(		"SS_DAC_MODE",			CBaseParameter::RW,  0 ,0, 0, 1);
CIntParameter		ss_dac_speed(		"SS_DAC_HZ", 			CBaseParameter::RW, getDACRate() ,0,	1 / (65536 /getDACRate()) + 1, getDACRate());
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


uio_lib::COscilloscope::Ptr g_osc = nullptr;
uio_lib::CGenerator::Ptr    g_gen = nullptr;

streaming_lib::CStreamingFPGA::Ptr   		g_s_fpga = nullptr;
streaming_lib::CStreamingBufferCached::Ptr 	g_s_buffer = nullptr;
streaming_lib::CStreamingNet::Ptr    		g_s_net = nullptr;
streaming_lib::CStreamingFile::Ptr   		g_s_file = nullptr;

dac_streaming_lib::CDACStreamingApplication::Ptr g_dac_app = nullptr;
dac_streaming_lib::CDACStreamingManager::Ptr     g_dac_manger = nullptr;
ServerNetConfigManager::Ptr                     g_serverNetConfig = nullptr;

std::atomic_bool g_serverRun(false);
std::atomic_bool g_dac_serverRun(false);

auto getADCChannels() -> uint8_t{
    uint8_t c = 0;
    if (rp_HPGetFastADCChannelsCount(&c) != RP_HP_OK){
        fprintf(stderr,"[Error] Can't get fast ADC channels count\n");
    }
    return c;
}

auto getDACChannels() -> uint8_t{
    uint8_t c = 0;
    if (rp_HPGetFastDACChannelsCount(&c) != RP_HP_OK){
        fprintf(stderr,"[Error] Can't get fast DAC channels count\n");
    }
    return c;
}

auto getDACRate() -> uint32_t{
    uint32_t c = 0;
    if (rp_HPGetBaseFastDACSpeedHz(&c) != RP_HP_OK){
        fprintf(stderr,"[Error] Can't get fast DAC channels count\n");
    }
    return c;
}

auto getADCRate() -> uint32_t{
    uint32_t c = 0;
    if (rp_HPGetBaseFastADCSpeedHz(&c) != RP_HP_OK){
        fprintf(stderr,"[Error] Can't get fast ADC channels count\n");
    }
    return c;
}

auto getModel() -> broadcast_lib::EModel{
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

        case STEM_250_12_v1_1:
        case STEM_250_12_v1_2:
            return broadcast_lib::EModel::RP_250_12;
        default:
            fprintf(stderr,"[Error] Can't get board model\n");
            exit(-1);
    }
    return broadcast_lib::EModel::RP_125_14;
}

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

        case STEM_250_12_v1_1:
        case STEM_250_12_v1_2:
            return "Z20_250_12";
        default:
            fprintf(stderr,"[Error] Can't get board model\n");
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

			g_serverNetConfig = std::make_shared<ServerNetConfigManager>(config_file,broadcast_lib::EMode::AB_SERVER_MASTER,ss_ip_addr.Value(),SERVER_CONFIG_PORT);
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

		}catch (std::exception& e)
			{
				fprintf(stderr, "Error: Init ServerNetConfigManager() %s\n",e.what());
			}

		ss_status.SendValue(0);
		ss_adc_data_pass.SendValue(0);
		ss_acd_max.SendValue(getADCRate());
		ss_dac_max_speed.SendValue(getDACRate());
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
        aprintf(stderr, "Error: rp_app_init() %s\n",e.what());
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
        aprintf(stderr, "Error save to file (%s)\n",config_file);
	}
}

void updateUI(){
	ss_port.SendValue(std::atoi(g_serverNetConfig->getSettingsRef().getPort().c_str()));
	ss_dac_file.SendValue(g_serverNetConfig->getSettingsRef().getDACFile());
	ss_dac_port.SendValue(std::atoi(g_serverNetConfig->getSettingsRef().getDACPort().c_str()));

	switch (g_serverNetConfig->getSettingsRef().getSaveType())
	{
		case CStreamSettings::NET:
            ss_use_localfile.SendValue(SS_NET);
			break;
		case CStreamSettings::FILE:
            ss_use_localfile.SendValue(SS_FILE);
			break;
	}

	switch (g_serverNetConfig->getSettingsRef().getProtocol())
	{
		case CStreamSettings::TCP:
			ss_protocol.SendValue(SS_TCP);
			break;
		case CStreamSettings::UDP:
			ss_protocol.SendValue(SS_UDP);
			break;
	}

	switch (g_serverNetConfig->getSettingsRef().getChannels())
	{
		case CStreamSettings::CH1:
            ss_channels.SendValue(SS_CH1);
			break;
		case CStreamSettings::CH2:
            ss_channels.SendValue(SS_CH2);
			break;
		case CStreamSettings::BOTH:
            ss_channels.SendValue(SS_BOTH);
			break;
	}

	switch (g_serverNetConfig->getSettingsRef().getResolution())
	{
		case CStreamSettings::BIT_8:
			ss_resolution.SendValue(SS_8BIT);
			break;
		case CStreamSettings::BIT_16:
			ss_resolution.SendValue(SS_16BIT);
			break;
	}

	switch (g_serverNetConfig->getSettingsRef().getType())
	{
		case CStreamSettings::RAW:
            ss_save_mode.SendValue(SS_RAW);
			break;
		case CStreamSettings::VOLT:
            ss_save_mode.SendValue(SS_VOLT);
			break;
	}

	switch (g_serverNetConfig->getSettingsRef().getFormat())
	{
		case CStreamSettings::WAV:
            ss_format.SendValue(SS_WAV);
			break;
		case CStreamSettings::TDMS:
            ss_format.SendValue(SS_TDMS);
			break;
        case CStreamSettings::BIN:
            ss_format.SendValue(SS_BIN);
        default:
            aprintf(stderr,"Error format type in settings\n");
			break;
	}

	switch (g_serverNetConfig->getSettingsRef().getAttenuator())
	{
		case CStreamSettings::A_1_1:
            ss_attenuator.SendValue(SS_A_1_1);
			break;
		case CStreamSettings::A_1_20:
            ss_attenuator.SendValue(SS_A_1_20);
			break;
	}

	switch (g_serverNetConfig->getSettingsRef().getAC_DC())
	{
		case CStreamSettings::AC:
			ss_ac_dc.SendValue(1);
			break;
		case CStreamSettings::DC:
			ss_ac_dc.SendValue(2);
			break;
	}

	switch (g_serverNetConfig->getSettingsRef().getDACFileType())
	{
		case CStreamSettings::WAV:
            ss_dac_file_type.SendValue(SS_WAV);
			break;
		case CStreamSettings::TDMS:
            ss_dac_file_type.SendValue(SS_TDMS);
			break;
        default:
            aprintf(stderr,"Error format type for dac in settings\n");
	}

	switch (g_serverNetConfig->getSettingsRef().getDACGain())
	{
		case CStreamSettings::X1:
            ss_dac_gain.SendValue(SS_DAC_GAIN_1);
			break;
		case CStreamSettings::X5:
            ss_dac_gain.SendValue(SS_DAC_GAIN_5);
			break;
	}

	switch (g_serverNetConfig->getSettingsRef().getDACMode())
	{
		case CStreamSettings::DAC_NET:
            ss_dac_mode.SendValue(SS_NET);
			break;
		case CStreamSettings::DAC_FILE:
            ss_dac_mode.SendValue(SS_FILE);
			break;
	}


	ss_dac_repeat.SendValue(g_serverNetConfig->getSettingsRef().getDACRepeat());
	ss_dac_rep_count.SendValue(g_serverNetConfig->getSettingsRef().getDACRepeatCount());
	ss_dac_memory.SendValue(g_serverNetConfig->getSettingsRef().getDACMemoryUsage());
	ss_dac_speed.SendValue(g_serverNetConfig->getSettingsRef().getDACHz());
	ss_rate.SendValue(g_serverNetConfig->getSettingsRef().getDecimation());
	ss_samples.SendValue(g_serverNetConfig->getSettingsRef().getSamples());
    ss_calib.SendValue(g_serverNetConfig->getSettingsRef().getCalibration() ? SS_CALIB_ON : SS_CALIB_OFF);

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
        if (ss_use_localfile.Value() == SS_FILE)
			g_serverNetConfig->getSettingsRef().setSaveType(CStreamSettings::FILE);
        if (ss_use_localfile.Value() == SS_NET)
			g_serverNetConfig->getSettingsRef().setSaveType(CStreamSettings::NET);
		needUpdate = true;
	}

	if (ss_protocol.IsNewValue() || _force)
	{
		ss_protocol.Update();
		if (ss_protocol.Value() == SS_TCP)
			g_serverNetConfig->getSettingsRef().setProtocol(CStreamSettings::TCP);
		if (ss_protocol.Value() == SS_UDP)
			g_serverNetConfig->getSettingsRef().setProtocol(CStreamSettings::UDP);
		needUpdate = true;
	}

	if (ss_channels.IsNewValue() || _force)
	{
		ss_channels.Update();
        if (ss_channels.Value() == SS_CH1)
			g_serverNetConfig->getSettingsRef().setChannels(CStreamSettings::CH1);
        if (ss_channels.Value() == SS_CH2)
			g_serverNetConfig->getSettingsRef().setChannels(CStreamSettings::CH2);
        if (ss_channels.Value() == SS_BOTH)
			g_serverNetConfig->getSettingsRef().setChannels(CStreamSettings::BOTH);
		needUpdate = true;
	}

	if (ss_resolution.IsNewValue() || _force)
	{
		ss_resolution.Update();
		if (ss_resolution.Value() == SS_8BIT)
			g_serverNetConfig->getSettingsRef().setResolution(CStreamSettings::BIT_8);
		if (ss_resolution.Value() == SS_16BIT)
			g_serverNetConfig->getSettingsRef().setResolution(CStreamSettings::BIT_16);
		needUpdate = true;
	}

	if (ss_save_mode.IsNewValue() || _force)
	{
		ss_save_mode.Update();
        if (ss_save_mode.Value() == SS_RAW)
			g_serverNetConfig->getSettingsRef().setType(CStreamSettings::RAW);
        if (ss_save_mode.Value() == SS_VOLT)
			g_serverNetConfig->getSettingsRef().setType(CStreamSettings::VOLT);
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
        if (ss_format.Value() == SS_WAV)
			g_serverNetConfig->getSettingsRef().setFormat(CStreamSettings::WAV);
        if (ss_format.Value() == SS_TDMS)
			g_serverNetConfig->getSettingsRef().setFormat(CStreamSettings::TDMS);
        if (ss_format.Value() == SS_BIN)
            g_serverNetConfig->getSettingsRef().setFormat(CStreamSettings::BIN);
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
			if (ss_attenuator.Value() == 1)
				g_serverNetConfig->getSettingsRef().setAttenuator(CStreamSettings::A_1_1);
			if (ss_attenuator.Value() == 2)
				g_serverNetConfig->getSettingsRef().setAttenuator(CStreamSettings::A_1_20);
			needUpdate = true;
		}

		if (ss_calib.IsNewValue() || _force)
		{
			ss_calib.Update();
			g_serverNetConfig->getSettingsRef().setCalibration(ss_calib.Value() == SS_CALIB_ON);
			needUpdate = true;
		}
	}else{
		g_serverNetConfig->getSettingsRef().setAttenuator(CStreamSettings::A_1_1);
		g_serverNetConfig->getSettingsRef().setCalibration(false);
	}

	if (rp_HPGetFastADCIsAC_DCOrDefault()){
		if (ss_ac_dc.IsNewValue() || _force)
		{
			ss_ac_dc.Update();
			if (ss_ac_dc.Value() == 1)
				g_serverNetConfig->getSettingsRef().setAC_DC(CStreamSettings::AC);
			if (ss_ac_dc.Value() == 2)
				g_serverNetConfig->getSettingsRef().setAC_DC(CStreamSettings::DC);
			needUpdate = true;
		}
	}
	else
		g_serverNetConfig->getSettingsRef().setAC_DC(CStreamSettings::AC);


	if (ss_dac_file_type.IsNewValue() || _force)
	{
		ss_dac_file_type.Update();
        if (ss_dac_file_type.Value() == SS_WAV)
			g_serverNetConfig->getSettingsRef().setDACFileType(CStreamSettings::WAV);
        if (ss_dac_file_type.Value() == SS_TDMS)
			g_serverNetConfig->getSettingsRef().setDACFileType(CStreamSettings::TDMS);
		needUpdate = true;
	}

	if (rp_HPGetIsGainDACx5OrDefault()){
		if (ss_dac_gain.IsNewValue() || _force)
		{
			ss_dac_gain.Update();
			if (ss_dac_gain.Value() == SS_DAC_GAIN_1)
				g_serverNetConfig->getSettingsRef().setDACGain(CStreamSettings::X1);
			if (ss_dac_gain.Value() == SS_DAC_GAIN_5)
				g_serverNetConfig->getSettingsRef().setDACGain(CStreamSettings::X5);
			needUpdate = true;
		}
	}
	else
		g_serverNetConfig->getSettingsRef().setDACGain(CStreamSettings::X1);


	if (ss_dac_mode.IsNewValue() || _force)
	{
		ss_dac_mode.Update();
        if (ss_dac_mode.Value() == SS_NET)
			g_serverNetConfig->getSettingsRef().setDACMode(CStreamSettings::DAC_NET);
        if (ss_dac_mode.Value() == SS_FILE)
			g_serverNetConfig->getSettingsRef().setDACMode(CStreamSettings::DAC_FILE);
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
        aprintf(stderr, "Error: UpdateParams() %s\n",e.what());
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
 		auto channels     = getADCChannels();
		auto is_master    = settings.getBoardMode();

		if (rp_CalibInit() != RP_HW_CALIB_OK){
	        fprintf(stderr,"Error init calibration\n");
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
            for(uint8_t ch = 0; ch < channels; ++ch){
				rp_acq_ac_dc_mode_calib_t  mode = ac_dc == CStreamSettings::AC ? RP_AC_CALIB : RP_DC_CALIB;
                if (attenuator == CStreamSettings::A_1_1){
                    if (rp_CalibGetFastADCCalibValue((rp_channel_calib_t)ch,mode,&ch_gain[ch],&ch_off[ch]) != RP_HW_CALIB_OK){
                        fprintf(stderr,"Error get calibration channel: %d\n",ch);
                    }

                    if (!filterBypass){
                        channel_filter_t f;
                        if (rp_CalibGetFastADCFilter((rp_channel_calib_t)ch,&f) != RP_HW_CALIB_OK){
                            fprintf(stderr,"Error get filter value: %d\n",ch);
                        }
                        aa_ch[ch] = f.aa;
                        bb_ch[ch] = f.bb;
                        kk_ch[ch] = f.kk;
                        pp_ch[ch] = f.pp;
                    }
                }

                if (attenuator == CStreamSettings::A_1_20){
                    if (rp_CalibGetFastADCCalibValue_1_20((rp_channel_calib_t)ch,mode,&ch_gain[ch],&ch_off[ch]) != RP_HW_CALIB_OK){
                        fprintf(stderr,"Error get calibration channel: %d\n",ch);
                    }

                    if (!filterBypass){
                        channel_filter_t f;
                        if (rp_CalibGetFastADCFilter_1_20((rp_channel_calib_t)ch,&f) != RP_HW_CALIB_OK){
                            fprintf(stderr,"Error get filter value: %d\n",ch);
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
		    rp_max7311::rp_setAttenuator(RP_MAX7311_IN1, attenuator == CStreamSettings::A_1_1  ? RP_ATTENUATOR_1_1 : RP_ATTENUATOR_1_20);
		    rp_max7311::rp_setAttenuator(RP_MAX7311_IN2, attenuator == CStreamSettings::A_1_1  ? RP_ATTENUATOR_1_1 : RP_ATTENUATOR_1_20);
        }

        if (rp_HPGetFastADCIsAC_DCOrDefault()){
	    	rp_max7311::rp_setAC_DC(RP_MAX7311_IN1, ac_dc == CStreamSettings::AC ? RP_AC_MODE : RP_DC_MODE);
    		rp_max7311::rp_setAC_DC(RP_MAX7311_IN2, ac_dc == CStreamSettings::AC ? RP_AC_MODE : RP_DC_MODE);
        }

        for (auto &uio : uioList)
		{
			if (uio.nodeName == "rp_oscilloscope")
			{
				fprintf(stderr,"COscilloscope::Create rate %d\n",rate);
                g_osc = uio_lib::COscilloscope::create(uio,rate,is_master == CStreamSettings::MASTER,getADCRate());
                g_osc->setCalibration(ch_off[0],ch_gain[0],ch_off[1],ch_gain[1]);
                g_osc->setFilterCalibrationCh1(aa_ch[0],bb_ch[0],kk_ch[0],pp_ch[0]);
                g_osc->setFilterCalibrationCh2(aa_ch[1],bb_ch[1],kk_ch[1],pp_ch[1]);
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
		aprintf(stderr,"[Streaming] Set channels resolution %d\n",resolution_val);
        auto att = attenuator == CStreamSettings::A_1_1 ? DataLib::CDataBuffer::ATT_1_1 :  DataLib::CDataBuffer::ATT_1_20;
        if(channel == CStreamSettings::CH1 || channel == CStreamSettings::BOTH){
            g_s_fpga->addChannel(DataLib::CH1,att,resolution_val);
			g_s_buffer->addChannel(DataLib::CH1,uio_lib::osc_buf_size, resolution_val);
        }
        if(channel == CStreamSettings::CH2 || channel == CStreamSettings::BOTH){
            g_s_fpga->addChannel(DataLib::CH2,att,resolution_val);
			g_s_buffer->addChannel(DataLib::CH2,uio_lib::osc_buf_size, resolution_val);
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

		fprintf(stderr,"[Streaming] Start server\n");

	}catch (std::exception& e)
	{
		fprintf(stderr, "Error: StartServer() %s\n",e.what());
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
        aprintf(stderr, "Error: startADC() %s\n",e.what());
        syslog (LOG_ERR,"Error: startADC() %s\n",e.what());
	}
}

void stopNonBlocking(ServerNetConfigManager::EStopReason x){
	try{
        std::thread th(stopServer ,x);
		th.detach();
	}catch (std::exception& e)
	{
        aprintf(stderr, "Error: stopNonBlocking() %s\n",e.what());
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
        aprintf(stderr,"[Streaming] Stop server\n");
	}catch (std::exception& e)
	{
        aprintf(stderr, "Error: stopServer() %s\n",e.what());
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
	        fprintf(stderr,"Error init calibration\n");
    	}
		auto dac_gain = settings.getDACGain();
		auto channels = getDACChannels();

        auto uioList = uio_lib::GetUioList();
		int32_t ch_off[4] = { 0 , 0 , 0 , 0 };
		double  ch_gain[4] = { 1 , 1 , 1 ,1 };

		if (use_calib) {
			for(uint8_t ch = 0; ch < channels; ++ch){
				rp_gen_gain_calib_t  mode = dac_gain == CStreamSettings::X1 ? RP_GAIN_CALIB_1X : RP_GAIN_CALIB_5X;
				if (rp_CalibGetFastDACCalibValue((rp_channel_calib_t)ch,mode,&ch_gain[ch],&ch_off[ch]) != RP_HW_CALIB_OK){
					fprintf(stderr,"Error get calibration channel: %d\n",ch);
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
            aprintf(stdout,"[Streaming] Error init generator module\n");
        	syslog (LOG_NOTICE, "[Streaming] Error init generator module\n");
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

        aprintf(stdout,"[Streaming] Start dac server\n");
        syslog (LOG_NOTICE, "[Streaming] Start dac server\n");
	}catch (std::exception& e)
	{
        aprintf(stderr, "Error: startDACServer() %s\n",e.what());
        syslog (LOG_ERR,"Error: startDACServer() %s\n",e.what());
	}
}

auto stopDACNonBlocking(dac_streaming_lib::CDACStreamingManager::NotifyResult x) -> void{
	try{
		std::thread th(stopDACServer ,x);
		th.detach();
	}catch (std::exception& e)
	{
        aprintf(stderr, "Error: stopDACNonBlocking() %s\n",e.what());
        syslog (LOG_ERR,"Error: stopDACNonBlocking() %s\n",e.what());
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
        aprintf(stdout,"[Streaming] Stop dac server\n");
        syslog (LOG_NOTICE, "[Streaming] Stop dac server\n");
	}catch (std::exception& e)
	{
        aprintf(stderr, "Error: stopDACServer() %s\n",e.what());
        syslog (LOG_ERR,"Error: stopDACServer() %s\n",e.what());
	}
}
