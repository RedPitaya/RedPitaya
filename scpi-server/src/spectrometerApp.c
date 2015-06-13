/**
 * $Id: $
 *
 * @brief Red Pitaya Scpi server generate SCPI commands implementation
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#include <syslog.h>
#include "spectrometerApp.h"
#include <string.h>
#include <stdlib.h>

#include "../../api-mockup/rpApplications/src/rpApp.h"
#include "../3rdparty/libs/scpi-parser/libscpi/inc/scpi/parser.h"
#include "utils.h"


scpi_result_t RP_APP_SpecGetViewData(rpApp_osc_source source, scpi_t *context) {
    uint32_t viewSize = 0;
    rpApp_SpecGetViewSize(&viewSize);

	static float* data[3] = {0};
	if (data[0] == 0)
	{
		size_t i;
		for (i = 0; i < 3; ++i)
			data[i] = malloc(sizeof(float)*viewSize);
	}

    int result = rpApp_SpecGetViewData(data, viewSize);
    if (RP_OK != result) {
        syslog(LOG_ERR, "*SPEC:CH<n>:DATA? Failed to get: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    SCPI_ResultBufferFloat(context, data[source], viewSize);
    syslog(LOG_INFO, "*SPEC:CH<n>:DATA? get successfully.");
    return SCPI_RES_OK;
}


scpi_result_t RP_APP_SpecRun(scpi_t *context) {
    syslog(LOG_INFO, "*SPEC:RUN start.");
    int result = rpApp_SpecRun(NULL);
    if (RP_OK != result) {
        syslog(LOG_ERR, "*SPEC:RUN Failed: %s.", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    syslog(LOG_INFO, "*SPEC:RUN Successfully.");
    return SCPI_RES_OK;
}

scpi_result_t RP_APP_SpecStop(scpi_t *context) {
    int result = rpApp_SpecStop();
    if (RP_OK != result) {
        syslog(LOG_ERR, "*SPEC:STOP Failed: %s.", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    syslog(LOG_INFO, "*SPEC:STOP Successfully.");
    return SCPI_RES_OK;
}

scpi_result_t RP_APP_SpecReset(scpi_t *context) {
    int result = rpApp_SpecReset();
    if (RP_OK != result) {
        syslog(LOG_ERR, "*SPEC:RESET Failed: %s.", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    syslog(LOG_INFO, "*SPEC:RESET Successfully.");
    return SCPI_RES_OK;
}

scpi_result_t RP_APP_SpecRunning(scpi_t *context) {
    int running = rpApp_SpecRunning();

    SCPI_ResultInt(context, running);
    syslog(LOG_INFO, "*SPEC:RUNNING Successfully.");
    return SCPI_RES_OK;
}

scpi_result_t RP_APP_SpecChannel1GetViewData(scpi_t *context) {
    return RP_APP_SpecGetViewData(RPAPP_OSC_SOUR_CH1, context);
}

scpi_result_t RP_APP_SpecChannel2GetViewData(scpi_t *context) {
    return RP_APP_SpecGetViewData(RPAPP_OSC_SOUR_CH2, context);
}

scpi_result_t RP_APP_SpecGetViewSize(scpi_t *context) {
    uint32_t viewSize = 2048;
    int result = rpApp_SpecGetViewSize(&viewSize);
    if (RP_OK != result) {
        syslog(LOG_ERR, "*SPEC:DATA:SIZE? Failed to get: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    SCPI_ResultUInt(context, viewSize);
    syslog(LOG_INFO, "*SPEC:DATA:SIZE? get successfully.");
    return SCPI_RES_OK;
}

scpi_result_t RP_APP_SpecSetViewSize(scpi_t *context) {
    uint32_t viewSize = 2048;
    int result = rpApp_SpecGetViewSize(&viewSize);
    if (RP_OK != result) {
        syslog(LOG_ERR, "*SPEC:DATA:SIZE Failed to get: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    SCPI_ResultUInt(context, viewSize);
    syslog(LOG_INFO, "*SPEC:DATA:SIZE get successfully.");
    return SCPI_RES_OK;
}

scpi_result_t RP_APP_SpecChannel1GetPeak(scpi_t *context) {
	float power;
    int result = rpApp_SpecGetPeakPower(0, &power);
    if (RP_OK != result) {
        syslog(LOG_ERR, "*SPEC:CH1:PEAK? Failed to get: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    SCPI_ResultDouble(context, power);
    syslog(LOG_INFO, "*SPEC:CH1:PEAK? get successfully.");
    return SCPI_RES_OK;
}

scpi_result_t RP_APP_SpecChannel2GetPeak(scpi_t *context) {
	float power;
    int result = rpApp_SpecGetPeakPower(1, &power);
    if (RP_OK != result) {
        syslog(LOG_ERR, "*SPEC:CH2:PEAK? Failed to get: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    SCPI_ResultDouble(context, power);
    syslog(LOG_INFO, "*SPEC:CH2:PEAK? get successfully.");
    return SCPI_RES_OK;
}

scpi_result_t RP_APP_SpecChannel1GetPeakFreq(scpi_t *context) {
	float freq;
    int result = rpApp_SpecGetPeakFreq(0, &freq);
    if (RP_OK != result) {
        syslog(LOG_ERR, "*SPEC:CH1:PEAK:FREQ? Failed to get: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    SCPI_ResultDouble(context, freq);
    syslog(LOG_INFO, "*SPEC:CH1:PEAK:FREQ? get successfully.");
    return SCPI_RES_OK;
}

scpi_result_t RP_APP_SpecChannel2GetPeakFreq(scpi_t *context) {
	float freq;
    int result = rpApp_SpecGetPeakFreq(1, &freq);
    if (RP_OK != result) {
        syslog(LOG_ERR, "*SPEC:CH2:PEAK:FREQ? Failed to get: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    SCPI_ResultDouble(context, freq);
    syslog(LOG_INFO, "*SPEC:CH2:PEAK:FREQ? get successfully.");
    return SCPI_RES_OK;
}

scpi_result_t RP_APP_SpecGetFreqMin(scpi_t *context) {
	float freq;
    int result = rpApp_SpecGetFreqMin(&freq);
    if (RP_OK != result) {
        syslog(LOG_ERR, "*SPEC:FREQ:MIN? Failed to get: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    SCPI_ResultDouble(context, freq);
    syslog(LOG_INFO, "*SPEC:FREQ:MIN? get successfully.");
    return SCPI_RES_OK;
}

scpi_result_t RP_APP_SpecGetFreqMax(scpi_t *context) { 
	float freq;
    int result = rpApp_SpecGetFreqMax(&freq);
    if (RP_OK != result) {
        syslog(LOG_ERR, "*SPEC:FREQ:MAX? Failed to get: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    SCPI_ResultDouble(context, freq);
    syslog(LOG_INFO, "*SPEC:FREQ:MAX? get successfully.");
    return SCPI_RES_OK;
}

scpi_result_t RP_APP_SpecSetFreqMin(scpi_t *context) {
	/*
	float freq;
    int result = rpApp_SpecGetPeakFreq(1, &freq); // TODO
    if (RP_OK != result) {
        syslog(LOG_ERR, "*SPEC:FREQ:MIN Failed to get: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    SCPI_ResultDouble(context, freq);
    syslog(LOG_INFO, "*SPEC:FREQ:MIN get successfully.");
	*/
    return SCPI_RES_OK;
}

scpi_result_t RP_APP_SpecSetFreqMax(scpi_t *context) {
    double freq;
    if (!SCPI_ParamDouble(context, &freq, true)) {
        syslog(LOG_ERR, "*SPEC:FREQ:MAX is missing first parameter.");
        return SCPI_RES_ERR;
    }

	int result = rpApp_SpecSetFreqMax(freq);
    if (RP_OK != result) {
        syslog(LOG_ERR, "*SPEC:FREQ:MAX Failed to get: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    SCPI_ResultDouble(context, freq);
    syslog(LOG_INFO, "*SPEC:FREQ:MAX get successfully.");
    return SCPI_RES_OK;
}

scpi_result_t RP_APP_SpecGetFpgaFreq(scpi_t *context) {
	float freq;
    int result = rpApp_SpecGetFpgaFreq(&freq);
    if (RP_OK != result) {
        syslog(LOG_ERR, "*SPEC:FPGA:FREQ? Failed to get: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    SCPI_ResultDouble(context, freq);
    syslog(LOG_INFO, "*SPEC:FREQ:FREQ? get successfully.");
    return SCPI_RES_OK;
}


