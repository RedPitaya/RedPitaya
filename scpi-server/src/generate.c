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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "generate.h"
#include "../../api-mockup/rpbase/src/generate.h"

#include "utils.h"
#include "../3rdparty/libs/scpi-parser/libscpi/inc/scpi/parser.h"
#include "../../api-mockup/rpbase/src/common.h"


scpi_result_t RP_GenReset(scpi_t *context) {
    int result = rp_GenReset();
    if (RP_OK != result) {
        syslog(LOG_ERR, "*GEN:RST Failed to: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    syslog(LOG_INFO, "*GEN:RST Successfully");

    return SCPI_RES_OK;
}

enum _scpi_result_t RP_GenChannel1SetState(scpi_t *context) {
    return RP_GenSetState(RP_CH_1, context);
}

enum _scpi_result_t RP_GenChannel2SetState(scpi_t *context) {
    return RP_GenSetState(RP_CH_2, context);
}

enum _scpi_result_t RP_GenChannel1SetFrequency(scpi_t *context) {
    return RP_GenSetFrequency(RP_CH_1, context);
}

enum _scpi_result_t RP_GenChannel2SetFrequency(scpi_t *context) {
    return RP_GenSetFrequency(RP_CH_2, context);
}

enum _scpi_result_t RP_GenChannel1SetWaveForm(scpi_t *context) {
    return RP_GenSetWaveForm(RP_CH_1, context);
}

enum _scpi_result_t RP_GenChannel2SetWaveForm(scpi_t *context) {
    return RP_GenSetWaveForm(RP_CH_2, context);
}

enum _scpi_result_t RP_GenChannel1SetAmplitude(scpi_t *context) {
    return RP_GenSetAmplitude(RP_CH_1, context);
}

enum _scpi_result_t RP_GenChannel2SetAmplitude(scpi_t *context) {
    return RP_GenSetAmplitude(RP_CH_2, context);
}

enum _scpi_result_t RP_GenChannel1SetOffset(scpi_t *context) {
    return RP_GenSetOffset(RP_CH_1, context);
}

enum _scpi_result_t RP_GenChannel2SetOffset(scpi_t *context) {
    return RP_GenSetOffset(RP_CH_2, context);
}

enum _scpi_result_t RP_GenChannel1SetPhase(scpi_t *context) {
    return RP_GenSetPhase(RP_CH_1, context);
}

enum _scpi_result_t RP_GenChannel2SetPhase(scpi_t *context) {
    return RP_GenSetPhase(RP_CH_2, context);
}

enum _scpi_result_t RP_GenChannel1SetDutyCycle(scpi_t *context) {
    return RP_GenSetDutyCycle(RP_CH_1, context);
}

enum _scpi_result_t RP_GenChannel2SetDutyCycle(scpi_t *context) {
    return RP_GenSetDutyCycle(RP_CH_2, context);
}

enum _scpi_result_t RP_GenChannel1SetArbitraryWaveForm(scpi_t *context) {
    return RP_GenSetArbitraryWaveForm(RP_CH_1, context);
}

enum _scpi_result_t RP_GenChannel2SetArbitraryWaveForm(scpi_t *context) {
    return RP_GenSetArbitraryWaveForm(RP_CH_2, context);
}

enum _scpi_result_t RP_GenChannel1SetGenerateMode(scpi_t *context) {
    return RP_GenSetGenerateMode(RP_CH_1, context);
}

enum _scpi_result_t RP_GenChannel2SetGenerateMode(scpi_t *context) {
    return RP_GenSetGenerateMode(RP_CH_2, context);
}

enum _scpi_result_t RP_GenChannel1SetBurstCount(scpi_t *context) {
    return RP_GenSetBurstCount(RP_CH_1, context);
}

enum _scpi_result_t RP_GenChannel2SetBurstCount(scpi_t *context) {
    return RP_GenSetBurstCount(RP_CH_2, context);
}

scpi_result_t RP_GenChannel1SetBurstRepetitions(scpi_t *context) {
    return RP_GenSetBurstRepetitions(RP_CH_1, context);
}

scpi_result_t RP_GenChannel2SetBurstRepetitions(scpi_t *context) {
    return RP_GenSetBurstRepetitions(RP_CH_2, context);
}

scpi_result_t RP_GenChannel1SetBurstPeriod(scpi_t *context) {
    return RP_GenSetBurstPeriod(RP_CH_1, context);
}

scpi_result_t RP_GenChannel2SetBurstPeriod(scpi_t *context) {
    return RP_GenSetBurstPeriod(RP_CH_2, context);
}

enum _scpi_result_t RP_GenChannel1SetTriggerSource(scpi_t *context) {
    return RP_GenSetTriggerSource(RP_CH_1, context);
}

enum _scpi_result_t RP_GenChannel2SetTriggerSource(scpi_t *context) {
    return RP_GenSetTriggerSource(RP_CH_2, context);
}

enum _scpi_result_t RP_GenChannel1SetTrigger(scpi_t *context) {
    return RP_GenSetTrigger(1, context);
}

enum _scpi_result_t RP_GenChannel2SetTrigger(scpi_t *context) {
    return RP_GenSetTrigger(2, context);
}

enum _scpi_result_t RP_GenChannel3SetTrigger(scpi_t *context) {
    return RP_GenSetTrigger(3, context);
}

enum _scpi_result_t RP_GenSetState(rp_channel_t channel, scpi_t *context) {
    bool state;
    // read first parameter STATE (ON, OFF)
    if (!SCPI_ParamBool(context, &state, true)) {
        syslog(LOG_ERR, "*OUTPUT<n>:STATE is missing first parameter.");
        return SCPI_RES_ERR;
    }

    int result;
    if (state) {
        result = rp_GenOutEnable(channel);
    }
    else {
        result = rp_GenOutDisable(channel);
    }
    
    
    if (RP_OK != result) {
        syslog(LOG_ERR, "*OUTPUT<n>:STATE Failed to %s channel: %s", state ? "enable" : "disable", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    syslog(LOG_INFO, "*OUTPUT<n>:STATE Successfully %s channel.", state ? "enabled" : "disabled");
    
   return SCPI_RES_OK;
}

enum _scpi_result_t RP_GenSetFrequency(rp_channel_t channel, scpi_t *context) {
    double value;
    // read first parameter FREQUENCY (value in Hz)
    if (!SCPI_ParamDouble(context, &value, true)) {
        syslog(LOG_ERR, "*SOUR<n>:FREQ:FIX is missing first parameter.");
        return SCPI_RES_ERR;
    }

    int result = rp_GenFreq(channel, (float) value);

    if (RP_OK != result) {
        syslog(LOG_ERR, "*SOUR<n>:FREQ:FIX Failed to set frequancy: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    syslog(LOG_INFO, "*SOUR<n>:FREQ:FIX Successfully set frequancy to %.2f Hz.", value);

    return SCPI_RES_OK;
}

enum _scpi_result_t RP_GenSetWaveForm(rp_channel_t channel, scpi_t *context) {
    const char * param;
    size_t param_len;
    char waveformString[15];

    // read first parameter SAMPLING_RATE (125MHz,15_6MHz, 1_9MHz,103_8kHz, 15_2kHz, 1_9kHz)
    if (!SCPI_ParamString(context, &param, &param_len, true)) {
        syslog(LOG_ERR, "*SOUR<n>:FUNC is missing first parameter.");
        return SCPI_RES_ERR;
    }
    strncpy(waveformString, param, param_len);
    waveformString[param_len] = '\0';

    // Convert waveform
    rp_waveform_t waveform;
    if (getRpWaveform(waveformString, &waveform)) {
        syslog(LOG_ERR, "*SOUR<n>:FUNC parameter waveform is invalid.");
        return SCPI_RES_ERR;
    }

    int result = rp_GenWaveform(channel, waveform);

    if (RP_OK != result) {
        syslog(LOG_ERR, "*SOUR<n>:FUNC Failed to set waveform: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    syslog(LOG_INFO, "*SOUR<n>:FUNC Successfully set waveform to %s.", waveformString);

    return SCPI_RES_OK;
}

enum _scpi_result_t RP_GenSetAmplitude(rp_channel_t channel, scpi_t *context) {
    double value;
    // read first parameter AMPLITUDE (value in V)
    if (!SCPI_ParamDouble(context, &value, true)) {
        syslog(LOG_ERR, "*SOUR<n>:VOLT is missing first parameter.");
        return SCPI_RES_ERR;
    }

    int result = rp_GenAmp(channel, (float) value);

    if (RP_OK != result) {
        syslog(LOG_ERR, "*SOUR<n>:VOLT Failed to set amplitude: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    syslog(LOG_INFO, "*SOUR<n>:VOLT Successfully set amplitude to %.2f V.", value);

    return SCPI_RES_OK;
}

enum _scpi_result_t RP_GenSetOffset(rp_channel_t channel, scpi_t *context) {
    double value;
    // read first parameter OFFSET (value in V)
    if (!SCPI_ParamDouble(context, &value, true)) {
        syslog(LOG_ERR, "*SOUR<n>:VOLT:OFFS is missing first parameter.");
        return SCPI_RES_ERR;
    }

    int result = rp_GenOffset(channel, (float) value);

    if (RP_OK != result) {
        syslog(LOG_ERR, "*SOUR<n>:VOLT:OFFS Failed to set offset: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    syslog(LOG_INFO, "*SOUR<n>:VOLT:OFFS Successfully set offset to %.2f V.", value);

    return SCPI_RES_OK;
}

enum _scpi_result_t RP_GenSetPhase(rp_channel_t channel, scpi_t *context) {
    double value;
    // read first parameter PHASE (value in degrees)
    if (!SCPI_ParamDouble(context, &value, true)) {
        syslog(LOG_ERR, "*SOUR<n>:PHAS is missing first parameter.");
        return SCPI_RES_ERR;
    }

    int result = rp_GenPhase(channel, (float) value);

    if (RP_OK != result) {
        syslog(LOG_ERR, "*SOUR<n>:PHAS Failed to set phase: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    syslog(LOG_INFO, "*SOUR<n>:PHAS Successfully set phase to %.2f deg.", value);

    return SCPI_RES_OK;
}

enum _scpi_result_t RP_GenSetDutyCycle(rp_channel_t channel, scpi_t *context) {
    double value;
    // read first parameter DUTY CYCLE (value in percentage)
    if (!SCPI_ParamDouble(context, &value, true)) {
        syslog(LOG_ERR, "*SOUR<n>:DCYC is missing first parameter.");
        return SCPI_RES_ERR;
    }

    int result = rp_GenDutyCycle(channel, (float) (value/100));

    if (RP_OK != result) {
        syslog(LOG_ERR, "*SOUR<n>:DCYC Failed to set duty cycle: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    syslog(LOG_INFO, "*SOUR<n>:DCYC Successfully set duty cycle to %.2f.", value);

    return SCPI_RES_OK;
}

enum _scpi_result_t RP_GenSetArbitraryWaveForm(rp_channel_t channel, scpi_t *context) {
    float buffer[BUFFER_LENGTH];
    uint32_t size;
    // read first parameter ARBITRARY WAVEFORM (float array form -1 to 1)
    if (!SCPI_ParamBufferFloat(context, buffer, &size, true)) {
        syslog(LOG_ERR, "*SOUR<n>:TRAC:DATA:DATA is missing first parameter.");
        return SCPI_RES_ERR;
    }

    int result = rp_GenArbWaveform(channel, buffer, size);

    if (RP_OK != result) {
        syslog(LOG_ERR, "*SOUR<n>:TRAC:DATA:DATA Failed to set arbitrary waveform: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    syslog(LOG_INFO, "*SOUR<n>:TRAC:DATA:DATA Successfully set arbitrary waveform");

    return SCPI_RES_OK;
}

enum _scpi_result_t RP_GenSetGenerateMode(rp_channel_t channel, scpi_t *context) {
    bool burst;
    // read first parameter BURST MODE (ON, OFF)
    if (!SCPI_ParamBool(context, &burst, true)) {
        syslog(LOG_ERR, "*SOUR<n>:BURS:STAT is missing first parameter.");
        return SCPI_RES_ERR;
    }

    int result;
    if (burst) {
        result = rp_GenMode(channel, RP_GEN_MODE_BURST);
    }
    else {
        result = rp_GenMode(channel, RP_GEN_MODE_CONTINUOUS);
    }

    if (RP_OK != result) {
        syslog(LOG_ERR, "*SOUR<n>:BURS:STAT Failed to set generate mode: %s",rp_GetError(result));
        return SCPI_RES_ERR;
    }

    syslog(LOG_INFO, "*SOUR<n>:BURS:STAT Successfully set generate mode");

    return SCPI_RES_OK;
}

enum _scpi_result_t RP_GenSetBurstCount(rp_channel_t channel, scpi_t *context) {
    const char * param;
    size_t param_len;
    char string[15];

    // read first parameter NUMBER OF CYCLES (integer value)
    if (!SCPI_ParamString(context, &param, &param_len, true)) {
        syslog(LOG_ERR, "*SOUR<n>:BURS:NCYC is missing first parameter.");
        return SCPI_RES_ERR;
    }
    strncpy(string, param, param_len);
    string[param_len] = '\0';

    // Convert String to int
    int32_t value;
    if (getRpInfinityInteger(string, &value)) {
        syslog(LOG_ERR, "*SOUR<n>:BURS:NCYC parameter cycles is invalid.");
        return SCPI_RES_ERR;
    }

    int result = rp_GenBurstCount(channel, value);

    if (RP_OK != result) {
        syslog(LOG_ERR, "*SOUR<n>:BURS:NCYC Failed to set burst count: %s",rp_GetError(result));
        return SCPI_RES_ERR;
    }

    syslog(LOG_INFO, "*SOUR<n>:BURS:NCYC Successfully set burst count");

    return SCPI_RES_OK;
}


scpi_result_t RP_GenSetBurstRepetitions(rp_channel_t channel, scpi_t *context) {
    int32_t value;
    // read first parameter NUMBER OF REPETITIONS (integer value)
    if (!SCPI_ParamInt(context, &value, true)) {
        syslog(LOG_ERR, "*SOUR<n>:BURS:NOR is missing first parameter.");
        return SCPI_RES_ERR;
    }

    int result = rp_GenBurstRepetitions(channel, value);

    if (RP_OK != result) {
        syslog(LOG_ERR, "*SOUR<n>:BURS:NOR Failed to set burst repetitions: %s",rp_GetError(result));
        return SCPI_RES_ERR;
    }

    syslog(LOG_INFO, "*SOUR<n>:BURS:NOR Successfully set burst repetitions");

    return SCPI_RES_OK;
}

scpi_result_t RP_GenSetBurstPeriod(rp_channel_t channel, scpi_t *context) {
    uint32_t value;

    // read first parameter PERIOD TIME (unsigned integer value)
    if (!SCPI_ParamUInt(context, &value, true)) {
        syslog(LOG_ERR, "*SOUR<n>:BURS:INT:PER is missing first parameter.");
        return SCPI_RES_ERR;
    }

    int result = rp_GenBurstPeriod(channel, value);

    if (RP_OK != result) {
        syslog(LOG_ERR, "*SOUR<n>:BURS:INT:PER Failed to set burst period: %s",rp_GetError(result));
        return SCPI_RES_ERR;
    }

    syslog(LOG_INFO, "*SOUR<n>:BURS:INT:PER Successfully set burst period");

    return SCPI_RES_OK;
}

enum _scpi_result_t RP_GenSetTriggerSource(rp_channel_t channel, scpi_t *context) {
    const char * param;
    size_t param_len;
    char triggerSourceString[15];

    // read first parameter TRIGGER SOURCE (EXT, INT)
    if (!SCPI_ParamString(context, &param, &param_len, true)) {
        syslog(LOG_ERR, "*SOUR<n>:TRIG:SOUR is missing first parameter.");
        return SCPI_RES_ERR;
    }
    strncpy(triggerSourceString, param, param_len);
    triggerSourceString[param_len] = '\0';

    // Convert triggerSource to rp_trig_src_t
    rp_trig_src_t triggerSource;
    if (getRpGenTriggerSource(triggerSourceString, &triggerSource)) {
        syslog(LOG_ERR, "*SOUR<n>:TRIG:SOUR parameter triggerSource is invalid.");
        return SCPI_RES_ERR;
    }

    int result = rp_GenTriggerSource(channel, triggerSource);

    if (RP_OK != result) {
        syslog(LOG_ERR, "*SOUR<n>:TRIG:SOUR Failed to set trigger source: %s",rp_GetError(result));
        return SCPI_RES_ERR;
    }

    syslog(LOG_INFO, "*SOUR<n>:TRIG:SOUR Successfully set trigger source");

    return SCPI_RES_OK;
}

enum _scpi_result_t RP_GenSetTrigger(int channel, scpi_t *context) {
    int result = rp_GenTrigger(channel);

    if (RP_OK != result) {
        syslog(LOG_ERR, "%s Failed to set triggert: %s", channel==3 ? "TRIG:IMM" : "SOUR<n>:TRIG:IMM", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    syslog(LOG_INFO, "*%s Successfully set trigger", channel==3 ? "TRIG:IMM" : "SOUR<n>:TRIG:IMM");

    return SCPI_RES_OK;
}
