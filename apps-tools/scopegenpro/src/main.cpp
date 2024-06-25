#include <string>
#include <fstream>
#include <sstream>
#include <math.h>
#include <algorithm>
#include <istream>

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
#include "x_y_logic.h"
#include "web/rp_system.h"
#include "web/rp_client.h"





CStringParameter redpitaya_model("RP_MODEL_STR", CBaseParameter::RO, getModelName(), 0);

CIntParameter resetSettings("RESET_CONFIG_SETTINGS", CBaseParameter::RW, 0, 0, 0, 10);

CFloatParameter slow_dac0 ("OSC_SLOW_OUT_1", CBaseParameter::RW, 0, 0, 0, rp_HPGetSlowDACFullScaleOrDefault(0),CONFIG_VAR);
CFloatParameter slow_dac1 ("OSC_SLOW_OUT_2", CBaseParameter::RW, 0, 0, 0, rp_HPGetSlowDACFullScaleOrDefault(1),CONFIG_VAR);
CFloatParameter slow_dac2 ("OSC_SLOW_OUT_3", CBaseParameter::RW, 0, 0, 0, rp_HPGetSlowDACFullScaleOrDefault(2),CONFIG_VAR);
CFloatParameter slow_dac3 ("OSC_SLOW_OUT_4", CBaseParameter::RW, 0, 0, 0, rp_HPGetSlowDACFullScaleOrDefault(3),CONFIG_VAR);

bool g_config_changed = false;
uint16_t g_save_counter = 0; // By default, a save check every 40 ticks. One tick 50 ms.

const std::vector<std::string> g_savedParams = {"OSC_CH1_IN_GAIN","OSC_CH2_IN_GAIN","OSC_CH3_IN_GAIN","OSC_CH4_IN_GAIN",
                                                "OSC_CH1_IN_AC_DC","OSC_CH2_IN_AC_DC","OSC_CH3_IN_AC_DC","OSC_CH4_IN_AC_DC"};

void updateParametersByConfig(){

    initExtTriggerLimits();
    initGenBeforeLoadConfig();
    initOscBeforeLoadConfig();

    configGet(getHomeDirectory() + "/.config/redpitaya/apps/scopegenpro/config.json");

    initOscAfterLoad();

    if (rp_HPIsFastDAC_PresentOrDefault()){
        initGenAfterLoad();
        updateGeneratorParameters(true);
        rp_GenSynchronise();
    }

    updateOscParams(true);
    updateMathParams(true);
    updateXYParams(true);
    updateTriggerLimit(true);
    updateSlowDAC(true);
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

    rp_WS_Init();
    rp_WS_SetInterval(RP_WS_RAM,5000);
    rp_WS_SetInterval(RP_WS_SLOW_DAC,500);
    rp_WS_SetMode((rp_system_mode_t)(RP_WS_ALL & (~RP_WS_DISK_SIZE) & ~RP_WS_SENSOR_VOLT));
    rp_WS_UpdateParameters(true);

    rp_WC_Init();
    rp_WC_UpdateParameters(true);

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
    updateGenTempProtection();

    auto is_auto_scale = isAutoScale();
    // Update calculated values on web
    updateOscParametersToWEB();
    updateMathParametersToWEB(is_auto_scale);
    updateXYParametersToWEB();
    sendFreqInSweepMode();
    updateSlowDAC(false);

    rp_WS_UpdateParameters(false);
    rp_WC_UpdateParameters(false);

    if (g_config_changed && (g_save_counter++ % 40 == 0)){
        g_config_changed = false;
        // Save the configuration file
        configSet(getHomeDirectory() + "/.config/redpitaya/apps/scopegenpro", "config.json");
    }
}

void updateSlowDAC(bool force){
    if (slow_dac0.IsNewValue() || force){
        if (rp_AOpinSetValue(0,slow_dac0.NewValue()) == RP_OK)
            slow_dac0.Update();
    }
    if (slow_dac1.IsNewValue() || force){
        if (rp_AOpinSetValue(1,slow_dac1.NewValue()) == RP_OK)
            slow_dac1.Update();
    }
    if (slow_dac2.IsNewValue() || force){
        if (rp_AOpinSetValue(2,slow_dac2.NewValue()) == RP_OK)
            slow_dac2.Update();
    }
    if (slow_dac3.IsNewValue() || force){
        if (rp_AOpinSetValue(3,slow_dac3.NewValue()) == RP_OK)
            slow_dac3.Update();
    }
}


void UpdateSignals(void) {
    updateOscSignal();
    updateMathSignal();
    updateXYSignal();
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
    if (!g_config_changed)
        g_config_changed = isChanged();
    checkMathScale();
    updateGeneratorParameters(false);
    updateOscParams(false);
    updateMathParams(false);
    updateXYParams(false);
    rp_WC_OnNewParam();
}

void OnNewSignals(void){}

void PostUpdateSignals(void){}