#include <stdio.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <unistd.h>

#include <string>
#include <map>
#include <list>
#include <memory>
#include <thread>

#include <DataManager.h>
#include <CustomParameters.h>

#include "decoder.h"
#include "i2c_decoder.h"
#include "spi_decoder.h"
#include "uart_decoder.h"
#include "can_decoder.h"
#include "rp_hw-profiles.h"
#include "rp_log.h"

#include "web/rp_client.h"

extern "C" {
#include "la_acq.h"
#include "rp_api.h"


typedef struct rp_app_params_s {
	char  *name;
	float  value;
	int    fpga_update;
	int    read_only;
	float  min_val;
	float  max_val;
} rp_app_params_t;

int rp_set_params(rp_app_params_t *p, int len);
int rp_get_params(rp_app_params_t **p);
int rp_get_signals(float ***s, int *sig_num, int *sig_len);
const char *rp_app_desc(void);
int rp_app_init(void);
int rp_app_exit(void);
}

 auto getModelS() -> std::string;


namespace {


enum LA_MODE {
//	DEMO,
	BASIC_ONLY,
	PRO_ONLY
//	BOTH
};

enum MEASURE_MODE {
	NONE,
	BASIC,
	PRO
};

CBooleanParameter 	inRun("LA_RUN", CBaseParameter::RW, false, 0);
CIntParameter 		measureState("MEASURE_STATE", CBaseParameter::RW, 1, 0, 1, 4);
CIntParameter 		laMode("LA_MODE", CBaseParameter::RW, 0, 0, 0, 3);
CIntParameter 		measureMode("MEASURE_MODE", CBaseParameter::RW, 0, 0, 0, 2);
CIntParameter 		measureSelect("MEASURE_SELECT", CBaseParameter::RW, 1, 0, 1, 2);

CByteSignal 		ch1("ch1", CBaseParameter::RO, 1024*1024, 0); // FIXME opt
CIntParameter 		decimate("DECIMATE", CBaseParameter::RW, 1, 0, 0, 128);

CIntParameter 		signalPeriiod("DEBUG_SIGNAL_PERIOD", CBaseParameter::RW, 300, 0, 0, 10000);
CIntParameter 		parameterPeriiod("DEBUG_PARAM_PERIOD", CBaseParameter::RW, 300, 0, 0, 10000);
long double cpu_values[4] = {0, 0, 0, 0}; /* reading only user, nice, system, idle */
CFloatParameter 	cpuLoad("CPU_LOAD", CBaseParameter::RW, 0, 0, 0, 100);
CFloatParameter 	memoryTotal("TOTAL_RAM", CBaseParameter::RW, 0, 0, 0, 1e15);
CFloatParameter 	memoryFree ("FREE_RAM", CBaseParameter::RW, 0, 0, 0, 1e15);

CStringParameter 	createDecoder("CREATE_DECODER", CBaseParameter::RW, "", 1);
CStringParameter 	destroyDecoder("DESTROY_DECODER", CBaseParameter::RW, "", 1);
CStringParameter 	decoderName("DECODER_NAME", CBaseParameter::RW, "", 1);
CFloatParameter 	scale("SCALE", CBaseParameter::RW, 1, 0, -100, 100);
CIntParameter 		preSampleBuf("PRE_SAMPLE_BUFFER", CBaseParameter::RW, 200000, 0, 0, 500000000);
CIntParameter 		samplesSum("SAMPLES_SUM", CBaseParameter::RW, 512, 0, 0, 2000000000);
CStringParameter 	dins("DINS", CBaseParameter::RW, "", 1);

CStringParameter redpitaya_model("RP_MODEL_STR", CBaseParameter::RO, getModelS(), 10);

std::vector<RP_DIGITAL_CHANNEL_DIRECTIONS> triggers;

pthread_t g_tid2;

bool g_fpgaDataReceived;

const float DEF_MIN_SCALE = 1.f/1000.f;
const float DEF_MAX_SCALE = 5.f;

std::map<std::string, std::shared_ptr<Decoder> > g_decoders;
} // end namespace



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
        case STEM_125_14_LN_BO_v1_1:
        case STEM_125_14_LN_CE1_v1_1:
        case STEM_125_14_LN_CE2_v1_1:
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



// Set measure mode
void set_measure_mode(MEASURE_MODE mode)
{
	fprintf(stderr, "Set measure mode: ");
	if (mode == BASIC)
	{
		fprintf(stderr, "Basic\n");
		rp_SetPolarity(0);
	}
	else if (mode == PRO)
	{
		fprintf(stderr, "Pro\n");
		rp_SetPolarity(0xffff);
	}
	measureMode.SendValue(mode);
}

// Set application mode
void set_application_mode(LA_MODE mode)
{
	if (mode == BASIC_ONLY)
	{
		set_measure_mode(BASIC);
		laMode.SendValue(mode);
	}
	else if (mode == PRO_ONLY)
	{
		set_measure_mode(PRO);
		laMode.SendValue(mode);
	}
}

// Execute system command
std::string exec(const char* cmd) {
    char buffer[128];
    std::string result = "";
    FILE* pipe = popen(cmd, "r");
    if (!pipe) throw std::runtime_error("popen() failed!");
    try {
        while (!feof(pipe)) {
            if (fgets(buffer, 128, pipe) != NULL)
                result += buffer;
        }
    } catch (...) {
        pclose(pipe);
        throw;
    }
    pclose(pipe);
    return result;
}

int rp_set_params(rp_app_params_t *p, int len) {
	return 0;
}

int rp_get_params(rp_app_params_t **p) {
	return 0;
}

int rp_get_signals(float ***s, int *sig_num, int *sig_len) {
	return 0;
}

const char *rp_app_desc(void) {
	return (const char *)"Red Pitaya osciloscope application.\n";
}

int rp_app_init(void) {

	int s = rp_OpenUnit();
	TRACE("openunit %d", s);
	set_application_mode(BASIC_ONLY);

    rp_WC_Init();
    rp_WC_UpdateParameters(true);

	time_t t;
	srand((unsigned) time(&t));
	fprintf(stderr, "Loading logic analyzer version %s-%s.\n", "0.00-0000", "unknow");

	CDataManager::GetInstance()->SetParamInterval(parameterPeriiod.Value());
	CDataManager::GetInstance()->SetSignalInterval(signalPeriiod.Value());

	return 0;
}

int rp_app_exit(void) {
	fprintf(stderr, "Unloading scope version %s-%s.\n", "0.00-0000", "unknow");

	rp_CloseUnit();

	return 0;
}

void rpReadyCallback(RP_STATUS status, void * pParameter)
{
	TRACE("ACQ_CALLBACK");
}

static size_t readFile(const char* _fname, uint8_t* _buf, size_t _buf_size)
{
	FILE* f = fopen(_fname, "rb");
	fseek(f, 0, SEEK_END);
	size_t flen = std::min<size_t>(ftell(f), _buf_size);
	fseek(f, 0, SEEK_SET);

	flen = fread(_buf, sizeof(*_buf), flen, f);
	fclose(f);

	return flen;
}

static void writeToFile(const char* _fname, uint8_t* _buf, size_t _buf_size)
{
	TRACE("Data was got, writing to file: %s", _fname);
	FILE* f = fopen(_fname, "wb");
	fwrite(_buf, sizeof(uint8_t), _buf_size, f);
	fclose(f);
}

void UpdateParams(void)
{
	CDataManager::GetInstance()->SetParamInterval(parameterPeriiod.Value());
	CDataManager::GetInstance()->SetSignalInterval(signalPeriiod.Value());

    static int times = 0;

	if (times == 3)
	{
	    FILE *fp = fopen("/proc/stat","r");
	    if(fp)
	    {
	    	long double a[4];
	    	fscanf(fp,"%*s %Lf %Lf %Lf %Lf",&a[0],&a[1],&a[2],&a[3]);
	    	fclose(fp);

	    	long double divider = ((a[0]+a[1]+a[2]+a[3]) - (cpu_values[0]+cpu_values[1]+cpu_values[2]+cpu_values[3]));
	    	long double loadavg = 100;
	    	if(divider > 0.01)
			{
				loadavg = ((a[0]+a[1]+a[2]) - (cpu_values[0]+cpu_values[1]+cpu_values[2])) / divider;
			}
			cpuLoad.Value() = (float)(loadavg * 100);
			cpu_values[0]=a[0];cpu_values[1]=a[1];cpu_values[2]=a[2];cpu_values[3]=a[3];
	    }
	    times = 0;

		struct sysinfo memInfo;
	    sysinfo (&memInfo);
    	memoryTotal.Value() = (float)memInfo.totalram;
    	memoryFree.Value() = (float)memInfo.freeram;
	}
	times++;

	rp_WC_UpdateParameters(false);
}


void UpdateSignals(void)
{
    CDataManager::GetInstance()->SetSignalInterval(signalPeriiod.Value());
}

void PostUpdateSignals(void)
{
	ch1.Update();
	for (auto& decoder : g_decoders)
	{
		decoder.second->UpdateSignals();
	}
}

void* trigAcq(void *arg)
{
    sleep(2);
    if(rp_SoftwareTrigger() != RP_API_OK){
    	rp_Stop();
		measureState.SendValue(4);
    }
    return NULL;
}


void DoDecode(bool fpgaDataReceived, uint8_t* file_buf)
{
	for (auto& decoder : g_decoders)
    {
        if (decoder.second->IsParametersChanged() || fpgaDataReceived)
        {
            if(ch1.GetSize())
            {
            	uint8_t buffer[ch1.GetSize()];
            	for(int i=0; i<ch1.GetSize(); i++)
            		buffer[i] = ch1[i];
            	decoder.second->Decode(buffer, ch1.GetSize());
            }
            else
            {
            	decoder.second->UpdateParameters();
            }
        }
    }
}

void OnNewParams(void)
{

	// Update measure select
	if (measureSelect.IsNewValue())
	{
		measureSelect.Update();
		if (measureSelect.Value() == 1)
			set_application_mode(BASIC_ONLY);
		if (measureSelect.Value() == 2)
			set_application_mode(PRO_ONLY);
	}

	g_fpgaDataReceived = false;

	// collect data from JS
	if (dins.IsNewValue())
	{
		dins.Update();
		triggers.clear();
		for (size_t i = 0; i < dins.Value().size()/2; ++i)
		{
			int pin = dins.Value()[i*2] - '0';
			int dir = dins.Value()[i*2 + 1] - '0' + 1;

			RP_DIGITAL_CHANNEL_DIRECTIONS d;
			d.channel = (RP_DIGITAL_CHANNEL)pin;
			d.direction = (RP_DIGITAL_DIRECTION)dir;
			triggers.push_back(d);
		}
	}

	preSampleBuf.Update();
	decimate.Update();
	ch1.Update();

	static bool need_reload = true;
	// buffers for file
	const size_t file_size = 102400;
	static uint8_t file_buf[file_size];

	if (!inRun.NewValue() && inRun.Value())
	{
		inRun.Update();
		rp_Stop();
		sleep(2);
		measureState.SendValue(1);
	}
	else if (inRun.NewValue() && !inRun.Value()) // FPGA mode
	{
		inRun.Update();
		measureState.SendValue(2);
		std::thread([&]{
			size_t BUF_SIZE = 1024*1024;
			int pre = preSampleBuf.Value();
			uint32_t POST = BUF_SIZE - pre;
			uint32_t samples = 0;
            RP_STATUS s;
			// buffers for fpga
			auto buf = new uint8_t[BUF_SIZE*2];
			auto buf1 = new int16_t[BUF_SIZE];

			rp_Stop();
			uint8_t decimateRate = decimate.Value();

			if (triggers.empty())
			{
				RP_DIGITAL_CHANNEL_DIRECTIONS d = {RP_DIGITAL_CHANNEL_0, RP_DIGITAL_DONT_CARE};
				s = rp_SetTriggerDigitalPortProperties(&d, 0);
				pthread_create(&g_tid2, NULL, &trigAcq, NULL);
			}
			else
			{
				s = rp_SetTriggerDigitalPortProperties(triggers.data(), triggers.size());
			}

			s = rp_EnableDigitalPortDataRLE(1);
			double timeIndisposedMs;
			TRACE("pre = %d post = %d buf_size = %zu", pre, POST, BUF_SIZE);
			s = rp_RunBlock(pre, POST, decimateRate, &timeIndisposedMs, &rpReadyCallback, NULL);
			s = rp_SetDataBuffer(buf1, BUF_SIZE, RP_RATIO_MODE_NONE);
			samples = BUF_SIZE;
			s = rp_GetValues(0, &samples, 1, RP_RATIO_MODE_NONE, NULL);

			uint32_t trigPos = 0;
			uint32_t sum = 0;
			rp_GetTrigPosition(&trigPos);

			for (size_t i = 0; i < samples; ++i)
			{
				buf[i*2] = buf1[i] >> 8;
				buf[i*2 + 1] = buf1[i];

				if(i < trigPos)
					sum += buf[i*2] + 1;
			}

			TRACE("ch1 samples %d sum %d", samples, trigPos);
			TRACE("%x\n-->%x\n%x", buf[pre*2-1], buf[pre*2+1], buf[pre*2+3]);
			ch1.Set(buf, samples*2);

			uint8_t fileBuf[ch1.GetSize()];
	        for(int i=0; i<ch1.GetSize(); i++)
	        	fileBuf[i] = ch1[i];
	        writeToFile("/tmp/logicData.bin", fileBuf, ch1.GetSize());

			g_fpgaDataReceived = true;
			inRun.SendValue(false); // send always
			samplesSum.SendValue(sum); // send always
			measureState.SendValue(3);
			DoDecode(g_fpgaDataReceived, NULL);

		    delete[] buf;
		    delete[] buf1;

            if (s != RP_API_OK){
                ERROR("Error api2 %d",s);
            }
		}).detach();
	}
	else if (!createDecoder.IsNewValue() && !decoderName.IsNewValue() && !destroyDecoder.IsNewValue() && !g_decoders.size() && !ch1.GetSize()) // nothing
	{
		inRun.Update();
		return;
	}

    // Create decoders
	if (createDecoder.IsNewValue() && decoderName.IsNewValue())
	{
		TRACE("CREATE DECODER...");

		createDecoder.Update();
		decoderName.Update();

		const auto name = decoderName.Value();
		if (createDecoder.Value() == "i2c")
		{
			g_decoders[name] = std::make_shared<I2CDecoder>(name);
            TRACE("createDecoder: %s", createDecoder.Value().c_str());
		}
		else if (createDecoder.Value() == "spi")
		{
			g_decoders[name] = std::make_shared<SpiDecoder>(name);
            TRACE("createDecoder: %s", createDecoder.Value().c_str());
		}
		else if (createDecoder.Value() == "can")
		{
			g_decoders[name] = std::make_shared<CANDecoder>(name);
            TRACE("createDecoder: %s", createDecoder.Value().c_str());
		}
		else if (createDecoder.Value() == "uart")
		{
			g_decoders[name] = std::make_shared<UARTDecoder>(name);
            TRACE("createDecoder: %s", createDecoder.Value().c_str());
		}

		createDecoder.Value() = name;
        TRACE("Value: %s", name.c_str());
	}

	// Delete decoders
	if (destroyDecoder.IsNewValue())
	{
		destroyDecoder.Update();
        TRACE("destroyDecoder: %s", destroyDecoder.Value().c_str());
		g_decoders.erase(destroyDecoder.Value());
	}

	DoDecode(false, file_buf);

    signalPeriiod.Update();
    parameterPeriiod.Update();

	rp_WC_OnNewParam();
}

void OnNewSignals(void)
{

}
