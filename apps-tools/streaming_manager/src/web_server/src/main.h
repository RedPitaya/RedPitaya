#pragma once

#include <sys/syslog.h> //Add custom RP_LCR LOG system

#include <DataManager.h>
#include <CustomParameters.h>

#include <fstream>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <sys/syslog.h>
#include <complex.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <ctime>
#include <stdlib.h>

#include <vector>
#include <algorithm>
#include <thread>
#include <mutex>

#include <unistd.h>
#include <signal.h>

#include "rp.h"
#include "rp_hw-calib.h"
#include "rp_hw-profiles.h"
#include "api250-12/rp-spi.h"
#include "api250-12/rp-gpio-power.h"
#include "api250-12/rp-i2c-max7311.h"

#include "common/version.h"

#include "uio_lib/oscilloscope.h"
#include "uio_lib/generator.h"
#include "data_lib/thread_cout.h"
#include "streaming_lib/streaming_net.h"
#include "streaming_lib/streaming_fpga.h"
#include "streaming_lib/streaming_buffer_cached.h"
#include "streaming_lib/streaming_file.h"
#include "dac_streaming_lib/dac_streaming_application.h"
#include "dac_streaming_lib/dac_net_controller.h"
#include "dac_streaming_lib/dac_streaming_manager.h"
#include "config_net_lib/server_net_config_manager.h"

#define EXEC_CHECK_MUTEX(x, mutex){ \
 		int retval = (x); \
 		if(retval != RP_OK) { \
            pthread_mutex_unlock((&mutex)); \
 			return retval; \
 		} \
}

#define IF_VALUE_CHANGED(X, ACTION) \
if (X.Value() != X.NewValue()) { \
    int res = ACTION;\
    if (res == RP_OK) { \
        X.Update(); \
    } \
}

#define IF_VALUE_CHANGED_BOOL(X, ACTION1, ACTION2) \
if (X.Value() != X.NewValue()) { \
    if (X.NewValue()) { \
        ACTION1;    X.Update(); \
    } else { \
        ACTION2;    X.Update(); }}

#define IS_NEW(X) X.Value() != X.NewValue()


#ifdef __cplusplus
extern "C" {
#endif


/* Parameters description structure - must be the same for all RP controllers */
typedef struct rp_app_params_s {
    char  *name;
    float  value;
    int    fpga_update;
    int    read_only;
    float  min_val;
    float  max_val;
} rp_app_params_t;


/* Signal measurement results structure - filled in worker and updated when
 * also measurement signal is stored from worker
 */
typedef struct rp_osc_meas_res_s {
    float min;
    float max;
    float amp;
    float avg;
    float freq;
    float period;
} rp_osc_meas_res_t;


//Rp app functions
const char *rp_app_desc(void);
int rp_app_init(void);
int rp_app_exit(void);
int rp_set_params(rp_app_params_t *p, int len);
int rp_get_params(rp_app_params_t **p);
int rp_get_signals(float ***s, int *sig_num, int *sig_len);

auto getADCChannels() -> uint8_t;
auto getDACChannels() -> uint8_t;
auto getDACRate() -> uint32_t;
auto getADCRate() -> uint32_t;
auto getModel() -> broadcast_lib::EModel;
auto getModelS() -> std::string;
auto getAC_DC() -> bool;

#ifdef __cplusplus
}
#endif
