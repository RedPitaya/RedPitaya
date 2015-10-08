/**
 * $Id: $
 *
 * @brief Red Pitaya Scpi server utils module interface
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <stdbool.h>
#include <syslog.h>
#include "../../api/rpbase/src/rp.h"
#include "acquire.h"
#include "../../api/rpApplications/src/rpApp.h"

#define SET_OK(cont) \
    	SCPI_ResultString(cont, "OK"); \
    	return SCPI_RES_OK;

#define MIN_CH			0
#define MAX_CH			3

#define SCPI_CMD_NUM 	1
#define SCPI_DEBUG 		1
#define CNV_STR(x) 		#x

/* rp scpi log */
#ifdef SCPI_DEBUG
#define RP_ERR(msg, param) \
(CNV_STR(param) == NULL) ? \
syslog(LOG_ERR, "%s\n", msg): \
syslog(LOG_ERR,"%s: %s\n", msg, CNV_STR(param));
#define RP_INFO(msg) \
syslog(LOG_INFO, "%s\n", msg);
#else
#define RP_ERR(msg, param)
#define RP_INFO(msg)
#endif

int getRpWaveform(const char *waveformString, rp_waveform_t *waveform);
int getRpWaveformString(rp_waveform_t waveform, char *waveformString);
int getRpGenTriggerSource(const char *triggerSourceString, rp_trig_src_t *triggerSource);
int getRpGenTriggerSourceString(rp_trig_src_t triggerSource, char *string);

int getRpAppTrigSlope(const char *string, rpApp_osc_trig_slope_t *slope);
int getRpAppTrigSlopeString(rpApp_osc_trig_slope_t slope, char *string);
int getRpAppTrigSweep(const char *string, rpApp_osc_trig_sweep_t *sweep);
int getRpAppTrigSweepString(rpApp_osc_trig_sweep_t sweep, char *string);
int getRpAppMathOperation(const char *string, rpApp_osc_math_oper_t *op);
int getRpAppMathOperationString(rpApp_osc_math_oper_t op, char *string);

int getRpInfinityInteger(const char *string, int32_t *value);
int getRpInfinityIntegerString(int32_t value, char *string);
int getRpUnit(const char *unitString, rp_scpi_acq_unit_t *unit);
int getRpStateIntegerString(int32_t value, char *string);

#endif /* UTILS_H_ */
