#include <string>
#include <fstream>
#include <sstream>
#include <math.h>
#include <algorithm>
#include <istream>
#include <iterator>
#include <math.h>

#include "main.h"
#include "settings.h"

#include "common/version.h"
#include <sys/sysinfo.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/stat.h>

#include "neon_asm.h"

#include "common.h"
#include "sig_gen_logic.h"
#include "osc_logic.h"
#include "math_logic.h"


/* -------------------------  debug parameter  --------------------------------- */
CIntParameter signalPeriod("DEBUG_SIGNAL_PERIOD", CBaseParameter::RW, 20, 0, 1, 10000);
CIntParameter parameterPeriod("DEBUG_PARAM_PERIOD", CBaseParameter::RW, 50, 0, 1, 10000);
CBooleanParameter digitalLoop("DIGITAL_LOOP", CBaseParameter::RW, false, 0);



/***************************************************************************************
*                                   SYSTEM STATUS                                      *
****************************************************************************************/
long double cpu_values[4] = {0, 0, 0, 0}; /* reading only user, nice, system, idle */
CFloatParameter cpuLoad("CPU_LOAD", CBaseParameter::RW, 0, 0, 0, 100);

CFloatParameter memoryTotal("TOTAL_RAM", CBaseParameter::RW, 0, 0, 0, 1e15);
CFloatParameter memoryFree ("FREE_RAM", CBaseParameter::RW, 0, 0, 0, 1e15);
CStringParameter redpitaya_model("RP_MODEL_STR", CBaseParameter::RO, getModelName(), 0);

CIntParameter resetSettings("RESET_CONFIG_SETTINGS", CBaseParameter::RW, 0, 0, 0, 10);


bool g_config_changed = false;
uint16_t g_save_counter = 0; // By default, a save check every 40 ticks. One tick 50 ms.

const std::vector<std::string> g_savedParams = {"OSC_CH1_IN_GAIN","OSC_CH2_IN_GAIN","OSC_CH3_IN_GAIN","OSC_CH4_IN_GAIN",
                                                "OSC_CH1_IN_AC_DC","OSC_CH2_IN_AC_DC","OSC_CH3_IN_AC_DC","OSC_CH4_IN_AC_DC"};

void updateParametersByConfig(){
    configGet(getHomeDirectory() + "/.config/redpitaya/apps/scopegenpro/config.json");

    initOscAfterLoad();

    if (rp_HPIsFastDAC_PresentOrDefault()){
        initGenAfterLoad();
        updateGeneratorParameters(true);
        rp_GenSynchronise();
    }

    updateOscParams(true);
    updateMathParams(true);
    updateTriggerLimit(true);
}

auto createDir(const std::string dir) -> bool
{
    mkdir(dir.c_str(), 0777);
    return true;
}

auto createDirTree(const std::string full_path) -> bool
{
    char ch = '/';

    size_t pos = 0;
    bool ret_val = true;

    while(ret_val == true && pos != std::string::npos)
    {
        pos = full_path.find(ch, pos + 1);
        ret_val = createDir(full_path.substr(0, pos));
    }

    return ret_val;
}

const char *rp_app_desc(void) {
    return (const char *)"Red Pitaya osciloscope application.\n";
}

int rp_app_init(void) {
    fprintf(stderr, "Loading scope version %s-%s.\n", VERSION_STR, REVISION_STR);

    CDataManager::GetInstance()->SetParamInterval(parameterPeriod.Value());
    CDataManager::GetInstance()->SetSignalInterval(signalPeriod.Value());

    rpApp_Init();
    rpApp_OscRun();
    // Need run after init parameters
    updateParametersByConfig();
    createDirTree("/tmp/scopegenpro");
    rpApp_OscRunMainThread();
    return 0;
}

int rp_app_exit(void) {
    fprintf(stderr, "Unloading scope version %s-%s.\n", VERSION_STR, REVISION_STR);

    deleteSweepController();

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



void UpdateParams(void) {

    // fprintf(stderr,"UpdateParams \n");
    auto is_osc_running = getOscRunState();

    resumeSweepController(!is_osc_running);

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

    updateGenTempProtection();


#ifdef DIGITAL_LOOP
	rp_EnableDigitalLoop(digitalLoop.Value());
#else
	static bool inited_loop = false;
	if (!inited_loop) {
		rp_EnableDigitalLoop(digitalLoop.Value());
		inited_loop = true;
	}
#endif

    auto is_auto_scale = isAutoScale();
    updateOscParametersToWEB();
    updateMathParametersToWEB(is_auto_scale);
    sendFreqInSweepMode();


    if (g_config_changed && (g_save_counter++ % 40 == 0)){
        g_config_changed = false;
        // Save the configuration file
        configSet(getHomeDirectory() + "/.config/redpitaya/apps/scopegenpro", "config.json");
    }
}


void UpdateSignals(void) {
    updateOscSignal();
    updateMathSignal();
    float tscale = getOSCTimeScale();
    generateOutSignalForWeb(tscale);
}


void OnNewParams(void) {

    if (resetSettings.IsNewValue()){
        if (resetSettings.NewValue() == 1){
            deleteConfig(getHomeDirectory() + "/.config/redpitaya/apps/scopegenpro/config.json");
            configSetWithList(getHomeDirectory() + "/.config/redpitaya/apps/scopegenpro", "config.json",g_savedParams);
            resetSettings.Update();
            resetSettings.SendValue(2);
            return;
        }
    }

    // fprintf(stderr,"OnNewParams \n");
    g_config_changed = isChanged();

    checkMathScale();
    updateGeneratorParameters(false);
    updateOscParams(false);
    updateMathParams(false);


/* ------ UPDATE DEBUG PARAMETERS ------*/

//    signalPeriiod.Update();
//    parameterPeriiod.Update();
//    digitalLoop.Update();
//    fprintf(stderr,"OnNewParams End \n");
}

void OnNewSignals(void){}

void PostUpdateSignals(void){}