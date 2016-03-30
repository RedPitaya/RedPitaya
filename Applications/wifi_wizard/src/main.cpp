#include "main.h"

#include <string>
#include <sstream>
#include <cstdlib>
#include <fstream>
#include <vector>

#include <limits.h>
#include <math.h>
#include <stdio.h>
#include "version.h"
#include <sys/types.h>
#include <sys/sysinfo.h>

/* --------------------------------  OUT PARAMETERS  ------------------------------ */
CBooleanParameter inRun("OSC_RUN", CBaseParameter::RW, false, 0);
// CIntParameter qualitySignal("QUALITY", CBaseParameter::RW, 200, 0, 0, 2);

std::vector<WIFINode> g_ListWIFI;

static const float DEF_MIN_SCALE = 1.f/1000.f;
static const float DEF_MAX_SCALE = 5.f;

const char *rp_app_desc(void) {
    return (const char *)"Red Pitaya Wi-Fi wizard application.\n";
}

int rp_app_init(void) {
    fprintf(stderr, "Loading Wi-Fi wizard version %s-%s.\n", VERSION_STR, REVISION_STR);
    // GetListOfWIFI(std::string("wlan0"));
    // std::string res = ParseLineOfESSID(std::string("                    ESSID:\"IntegraSources\""));
    // fprintf(stderr, "rerererererere %d\n", ParseLineEncPass(std::string("                    Encryption key:off")));
    CDataManager::GetInstance()->SetParamInterval(1000);

    rpApp_Init();
    rpApp_OscRun();
    return 0;
}

int rp_app_exit(void) {
    fprintf(stderr, "Unloading Wi-Fi wizard version %s-%s.\n", VERSION_STR, REVISION_STR);
    rpApp_Release();
    return 0;
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

double roundUpTo1(double data) {
    double power = ceil(log(data) / log(10)) - 1;       // calculate normalization factor
    double dataNorm = data / pow(10, power);            // normalize data, so that 1 < data < 10
    dataNorm = 10;
    return (dataNorm * pow(10, power));         // unnormalize data
}

void UpdateSignals(void) {

}

void UpdateParams(void) {
    // qualitySignal.Value() = // function with grepping
}

bool check_params(const rp_calib_params_t& current_params, int step) {

	return false;
}

std::string GetListOfWIFI(std::string wlanInterfaceName) {
	std::stringstream command;
	std::stringstream grepResult;
	std::string tmpFileName = "scanning.result";
	std::string rmTmpFile = "rm -f " + tmpFileName;
	std::string lineFromResult;

	command << "cat /opt/redpitaya/www/apps/wifi_wizard/scan_wlan0_res | grep '\\(ESSID\\|Encryption\\|Quality\\)' > " << tmpFileName;

	system(command.str().c_str());

	std::ifstream infile(tmpFileName);

	
	{
		WIFINode wifi;
		std::getline(infile, lineFromResult);
		wifi.essid = ParseLineOfESSID(lineFromResult);
		std::getline(infile, lineFromResult);
		wifi.keyEn = ParseLineEncPass(lineFromResult);
		std::getline(infile, lineFromResult);
		wifi.quality = ParseLineQuality(lineFromResult);
		std::getline(infile, lineFromResult);
		wifi.sigLevel = ParseLineSiglevel(lineFromResult);
		g_ListWIFI.push_back(wifi);
	}

	infile.close();

	grepResult << std::ifstream(tmpFileName).rdbuf();
	system(rmTmpFile.c_str());

	return grepResult.str();
}

std::string ParseLineOfESSID(std::string str) {
	size_t start = 0;
	size_t stop = 0;

	for(size_t i=0; i<str.size(); i++)
	{
		if(str[i] == '\"' && start == 0)
			start = i;
		if(str[i] == '\"' && start != 0)
			stop = i;
	}

	return str.substr(start+1, stop-start-1);
}

bool ParseLineEncPass(std::string str) {
	if(str.substr(str.size()-2, 2) == "on")
		return true;
	return false;
}

int ParseLineQuality(std::string str) {
	size_t start = 0;

	for(size_t i=0; i<str.size(); i++)
	{
		if(str[i] == '=' && start == 0)
			start = i;
	}

	std::string result = str.substr(start+1, 2);
	return atoi(result.c_str());
}

int ParseLineSiglevel(std::string str) {
	size_t start = 0;
	size_t stop = 0;

	for(size_t i=0; i<str.size(); i++)
	{
		if(str[i] == '=' && start == 0)
			start = i;
		if(str[i] == '=' && start != 0)
			stop = i;
	}

	std::string result = str.substr(stop+1, 2);
	return atoi(result.c_str());
}

void OnNewParams(void) {
// /* ------ SEND OSCILLOSCOPE PARAMETERS TO API ------*/
// 	fprintf(stderr, "-----LOG SCPI SERVER OnNewParams()------\n");
//     //IF_VALUE_CHANGED_BOOL(inRun, rpApp_OscRun(), rpApp_OscStop())

//     if(inRun.NewValue() == false)
//     {
//     	system("killall scpi-server");
//     	inRun.Value() = false;
//     }
//     else if(inRun.NewValue() == true)
//     {
//     	system("export LD_LIBRARY_PATH=/opt/redpitaya/lib/ && /opt/redpitaya/bin/scpi-server &");
//     	inRun.Value() = true;
//     }
}
