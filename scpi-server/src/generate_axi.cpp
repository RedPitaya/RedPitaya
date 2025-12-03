/**
 * $Id: $
 *
 * @brief Red Pitaya Scpi server acquire SCPI commands implementation
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <new>
#include <span>
#include <vector>

#include "common.h"
#include "generate_axi.h"

#include "scpi-parser-ext.h"
#include "scpi/parser.h"
#include "scpi/units.h"

scpi_result_t RP_GenAxiStartQ(scpi_t* context) {
    uint32_t start = 0, size = 0;
    int result = rp_GenAxiGetMemoryRegion(&start, &size);
    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to get decimation: %s", rp_GetError(result));
        if (getRetOnError())
            requestSendNewLine(context);
        return SCPI_RES_ERR;
    }
    SCPI_ResultUInt32Base(context, start, 10);
    RP_LOG_INFO("%s", rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_GenAxiEndQ(scpi_t* context) {
    uint32_t start = 0, size = 0;
    auto result = rp_GenAxiGetMemoryRegion(&start, &size);
    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to get decimation: %s", rp_GetError(result));
        if (getRetOnError())
            requestSendNewLine(context);
        return SCPI_RES_ERR;
    }
    SCPI_ResultUInt32Base(context, size, 10);
    RP_LOG_INFO("%s", rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_GenAxiReserveMemory(scpi_t* context) {
    rp_channel_t channel = RP_CH_1;
    if (RP_ParseChArgvDAC(context, &channel) != RP_OK) {
        if (getRetOnError())
            requestSendNewLine(context);
        return SCPI_RES_ERR;
    }

    uint32_t start = 0, end = 0;
    if (!SCPI_ParamUInt32(context, &start, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing start parameter.");
        return SCPI_RES_ERR;
    }

    if (!SCPI_ParamUInt32(context, &end, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing end parameter.");
        return SCPI_RES_ERR;
    }

    auto result = rp_GenAxiReserveMemory(channel, start, end);
    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to reserve memory: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s", rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_GenAxiReleaseMemory(scpi_t* context) {
    rp_channel_t channel = RP_CH_1;
    if (RP_ParseChArgvDAC(context, &channel) != RP_OK) {
        if (getRetOnError())
            requestSendNewLine(context);
        return SCPI_RES_ERR;
    }

    auto result = rp_GenAxiReleaseMemory(channel);
    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to release memory: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s", rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_GenAxiSetEnable(scpi_t* context) {
    rp_channel_t channel = RP_CH_1;
    if (RP_ParseChArgvDAC(context, &channel) != RP_OK) {
        if (getRetOnError())
            requestSendNewLine(context);
        return SCPI_RES_ERR;
    }

    scpi_bool_t value = FALSE;
    // read first parameter (OFF,ON)
    if (!SCPI_ParamBool(context, &value, false)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing first parameter.");
        return SCPI_RES_ERR;
    }

    auto result = rp_GenAxiSetEnable(channel, value);
    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to set AXI gen state: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s", rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_GenAxiGetEnable(scpi_t* context) {
    rp_channel_t channel = RP_CH_1;
    if (RP_ParseChArgvDAC(context, &channel) != RP_OK) {
        if (getRetOnError())
            requestSendNewLine(context);
        return SCPI_RES_ERR;
    }

    bool value = false;
    auto result = rp_GenAxiGetEnable(channel, &value);
    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to get AXI gen state: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    SCPI_ResultMnemonic(context, value ? "ON" : "OFF");
    RP_LOG_INFO("%s", rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_GenAxiSetDecimationFactor(scpi_t* context) {
    rp_channel_t channel = RP_CH_1;
    if (RP_ParseChArgvDAC(context, &channel) != RP_OK) {
        return SCPI_RES_ERR;
    }
    uint32_t value = 0;
    /* Read DECIMATION parameter */
    if (!SCPI_ParamUInt32(context, &value, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Missing first parameter.");
        return SCPI_RES_ERR;
    }
    auto result = rp_GenAxiSetDecimationFactor(channel, value);
    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to set decimation: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s", rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_GenAxiGetDecimationFactor(scpi_t* context) {
    rp_channel_t channel = RP_CH_1;
    if (RP_ParseChArgvDAC(context, &channel) != RP_OK) {
        if (getRetOnError())
            requestSendNewLine(context);
        return SCPI_RES_ERR;
    }
    uint32_t decimation = 0;
    auto result = rp_GenAxiGetDecimationFactor(channel, &decimation);
    if (RP_OK != result) {
        RP_LOG_CRIT("Failed to get decimation: %s", rp_GetError(result));
        if (getRetOnError())
            requestSendNewLine(context);
        return SCPI_RES_ERR;
    }
    SCPI_ResultUInt32Base(context, decimation, 10);
    RP_LOG_INFO("%s", rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_GenSetAmplitudeAndOffsetOrigin(scpi_t* context) {
    rp_channel_t channel = RP_CH_1;
    if (RP_ParseChArgvDAC(context, &channel) != RP_OK) {
        if (getRetOnError())
            requestSendNewLine(context);
        return SCPI_RES_ERR;
    }

    auto result = rp_GenSetAmplitudeAndOffsetOrigin(channel);
    if (RP_OK != result) {
        RP_LOG_CRIT("Failed set amplitude and offset for AXI gen: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s", rp_GetError(result))
    return SCPI_RES_OK;
}

scpi_result_t RP_GenAxiWriteWaveform(scpi_t* context) {

    rp_channel_t channel = RP_CH_1;
    size_t size = 0;
    uint32_t offset = 0;
    float* buffer = NULL;
    int32_t cmd[3] = {0, 0, 0};
    if (!SCPI_CommandNumbers(context, cmd, 3, -1)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Failed to get parameters.")
        return SCPI_RES_ERR;
    }
    if (cmd[0] == -1) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Failed to get channel.")
        return SCPI_RES_ERR;
    }
    if (cmd[1] == -1) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Failed to get offset.")
        return SCPI_RES_ERR;
    }
    if (cmd[2] == -1) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Failed to get size of buffer.")
        return SCPI_RES_ERR;
    }
    if (cmd[0] <= 0 || cmd[0] > getDACChannels(context)) {
        SCPI_LOG_ERR(SCPI_ERROR_MISSING_PARAMETER, "Failed to get channel.")
        return SCPI_RES_ERR;
    }
    channel = (rp_channel_t)(cmd[0] - 1);
    offset = cmd[1];
    size = cmd[2];
    buffer = (float*)malloc(size * sizeof(float));
    if (!buffer) {
        SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR, "Failed allocate buffer with size: %d.", size)
        return SCPI_RES_ERR;
    }

    size_t buf_size = size;
    if (!SCPI_ParamBufferFloat(context, buffer, &buf_size, true)) {
        SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR, "Failed get data for buffer.")
        free(buffer);
        return SCPI_RES_ERR;
    }
    if (buf_size != size) {
        SCPI_LOG_ERR(SCPI_ERROR_EXECUTION_ERROR, "Wrong data length.")
        free(buffer);
        return SCPI_RES_ERR;
    }

    auto result = rp_GenAxiWriteWaveformOffset(channel, offset, buffer, size);
    if (result != RP_OK) {
        RP_LOG_CRIT("Failed to set AXI gen waveform data: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }
    RP_LOG_INFO("%s", rp_GetError(result))
    return SCPI_RES_OK;
}
