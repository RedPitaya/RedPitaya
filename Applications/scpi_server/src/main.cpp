#include "main.h"

#include <limits.h>
#include <math.h>
#include <stdio.h>
#include "version.h"
#include <sys/types.h>
#include <sys/sysinfo.h>

/* --------------------------------  OUT PARAMETERS  ------------------------------ */
CBooleanParameter inRun("OSC_RUN", CBaseParameter::RW, false, 0);
CBooleanParameter isRunning("OSC_RUNNING", CBaseParameter::RWSA, false, 0);

static const float DEF_MIN_SCALE = 1.f/1000.f;
static const float DEF_MAX_SCALE = 5.f;

const char *rp_app_desc(void) {
    return (const char *)"Red Pitaya SCPI server application.\n";
}

int rp_app_init(void) {
    fprintf(stderr, "Loading SCPI server version %s-%s.\n", VERSION_STR, REVISION_STR);
    CDataManager::GetInstance()->SetParamInterval(1000);

    rpApp_Init();
    rpApp_OscRun();
    return 0;
}

int rp_app_exit(void) {
    fprintf(stderr, "Unloading SCPI server version %s-%s.\n", VERSION_STR, REVISION_STR);
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
    isRunning.Value() = IsRunning();
}

bool check_params(const rp_calib_params_t& current_params, int step) {

	return false;
}

int IsRunning(void) {
    FILE* pop = popen("/usr/bin/pgrep scpi-server", "r");
    char buf[1024];
    fgets(buf, 1024, pop);
    pclose(pop);

    return atoi(buf);
}

void OnNewParams(void) {
/* ------ SEND OSCILLOSCOPE PARAMETERS TO API ------*/
	fprintf(stderr, "-----LOG SCPI SERVER OnNewParams()------\n");
    //IF_VALUE_CHANGED_BOOL(inRun, rpApp_OscRun(), rpApp_OscStop())

    if(IsRunning() && inRun.NewValue() == false)
    {
    	system("killall scpi-server");
    	inRun.Value() = false;
    }
    else if(!IsRunning() && inRun.NewValue() == true)
    {
    	system("export LD_LIBRARY_PATH=/opt/redpitaya/lib/ && /opt/redpitaya/bin/scpi-server &");
    	inRun.Value() = true;
    }
}
