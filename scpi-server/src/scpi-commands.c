/**
 * $Id: $
 *
 * @brief Red Pitaya Scpi server SCPI configuration and general command implementation
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

#include "utils.h"
#include "dpin.h"
#include "apin.h"
#include "generate.h"
#include "oscilloscopeApp.h"
#include "spectrometerApp.h"
#include "../3rdparty/libs/scpi-parser/libscpi/inc/scpi/error.h"
#include "../3rdparty/libs/scpi-parser/libscpi/inc/scpi/ieee488.h"
#include "../3rdparty/libs/scpi-parser/libscpi/inc/scpi/minimal.h"
#include "../3rdparty/libs/scpi-parser/libscpi/inc/scpi/units.h"
#include "../3rdparty/libs/scpi-parser/libscpi/inc/scpi/parser.h"

/**
 * Interface general commands
 */

size_t SCPI_Write(scpi_t * context, const char * data, size_t len) {

	size_t total = 0;

	if (context->user_context != NULL) {
        int fd = *(int *)(context->user_context);
        while (len > 0) {
        	size_t written =  write(fd, data, len);
        	if (written < 0) {
				syslog(LOG_ERR,
						"Failed to write into the socket. Should send %zu bytes. Could send only %zu bytes",
						len, written);
				return total;
        	}
            len -= written;
            data += written;
            total += written;
        }
    }
    return total;
}

scpi_result_t SCPI_Flush(scpi_t * context) {
    return SCPI_RES_OK;
}

int SCPI_Error(scpi_t * context, int_fast16_t err) {
	const char error[] = "ERR!";
    syslog(LOG_ERR, "**ERROR: %d, \"%s\"", (int32_t) err, SCPI_ErrorTranslate(err));
    SCPI_Write(context, error, strlen(error));
    return 0;
}

scpi_result_t SCPI_Control(scpi_t * context, scpi_ctrl_name_t ctrl, scpi_reg_val_t val) {
    if (SCPI_CTRL_SRQ == ctrl) {
        syslog(LOG_ERR, "**SRQ not implemented");
    } else {
    	 syslog(LOG_ERR, "**CTRL not implemented");
    }

    return SCPI_RES_ERR;
}

scpi_result_t SCPI_Test(scpi_t * context) {
	syslog(LOG_ERR, "**Test not implemented");
    return SCPI_RES_ERR;
}

scpi_result_t SCPI_Reset(scpi_t * context) {
	syslog(LOG_ERR, "**Reset not implemented");
    return SCPI_RES_ERR;
}

scpi_result_t SCPI_SystemCommTcpipControlQ(scpi_t * context) {
	syslog(LOG_ERR, "**SCPI_SystemCommTcpipControlQ not implemented");
	return SCPI_RES_ERR;
}

scpi_result_t SCPI_Echo(scpi_t * context) {
    syslog(LOG_ERR, "*ECHO");
    SCPI_ResultText(context, "ECHO?");
    return SCPI_RES_OK;
}

scpi_result_t SCPI_EchoVersion(scpi_t * context) {
    syslog(LOG_ERR, "*ECO:VERSION?");
    SCPI_ResultText(context, rp_GetVersion());
    return SCPI_RES_OK;
}




/**
 * SCPI Configuration
 */

static const scpi_command_t scpi_commands[] = {
        /* IEEE Mandated Commands (SCPI std V1999.0 4.1.1) */
        { .pattern = "*CLS", .callback = SCPI_CoreCls,},
        { .pattern = "*ESE", .callback = SCPI_CoreEse,},
        { .pattern = "*ESE?", .callback = SCPI_CoreEseQ,},
        { .pattern = "*ESR?", .callback = SCPI_CoreEsrQ,},
        { .pattern = "*IDN?", .callback = SCPI_CoreIdnQ,},
        { .pattern = "*OPC", .callback = SCPI_CoreOpc,},
        { .pattern = "*OPC?", .callback = SCPI_CoreOpcQ,},
        { .pattern = "*RST", .callback = SCPI_CoreRst,},
        { .pattern = "*SRE", .callback = SCPI_CoreSre,},
        { .pattern = "*SRE?", .callback = SCPI_CoreSreQ,},
        { .pattern = "*STB?", .callback = SCPI_CoreStbQ,},
        { .pattern = "*TST?", .callback = SCPI_CoreTstQ,},
        { .pattern = "*WAI", .callback = SCPI_CoreWai,},

        /* Required SCPI commands (SCPI std V1999.0 4.2.1) */
        {.pattern = "SYSTem:ERRor[:NEXT]?", .callback = SCPI_SystemErrorNextQ,},
        {.pattern = "SYSTem:ERRor:COUNt?", .callback = SCPI_SystemErrorCountQ,},
        {.pattern = "SYSTem:VERSion?", .callback = SCPI_SystemVersionQ,},

        {.pattern = "STATus:QUEStionable[:EVENt]?", .callback = SCPI_StatusQuestionableEventQ,},
        {.pattern = "STATus:QUEStionable:ENABle", .callback = SCPI_StatusQuestionableEnable,},
        {.pattern = "STATus:QUEStionable:ENABle?", .callback = SCPI_StatusQuestionableEnableQ,},

        {.pattern = "STATus:PRESet", .callback = SCPI_StatusPreset,},

        {.pattern = "SYSTem:COMMunication:TCPIP:CONTROL?", .callback = SCPI_SystemCommTcpipControlQ,},

        {.pattern = "ECHO?", .callback = SCPI_Echo,},
        {.pattern = "ECO:VERSION?", .callback = SCPI_EchoVersion,},

        /* RedPitaya */
        {.pattern = "DIG:RST", .callback = RP_DigitalPinReset,},
        {.pattern = "DIG:PIN", .callback = RP_DigitalPinSetState,},
        {.pattern = "DIG:PIN?", .callback = RP_DigitalPinGetStateQ,},
        {.pattern = "DIG:PIN:DIR", .callback = RP_DigitalPinSetDirection,},

        {.pattern = "ANALOG:RST", .callback = RP_AnalogPinReset,},
        {.pattern = "ANALOG:PIN", .callback = RP_AnalogPinSetValue,},
        {.pattern = "ANALOG:PIN?", .callback = RP_AnalogPinGetValue,},

        /* Acquire */
        {.pattern = "ACQ:START", .callback = RP_AcqStart,},
        {.pattern = "ACQ:STOP", .callback = RP_AcqStop,},
        {.pattern = "ACQ:RST", .callback = RP_AcqReset,},
        {.pattern = "ACQ:DEC", .callback = RP_AcqSetDecimation,},
        {.pattern = "ACQ:DEC?", .callback = RP_AcqGetDecimation,},
        {.pattern = "ACQ:SRAT", .callback = RP_AcqSetSamplingRate,},
        {.pattern = "ACQ:SRAT?", .callback = RP_AcqGetSamplingRate,},
        {.pattern = "ACQ:SRA:HZ?", .callback = RP_AcqGetSamplingRateHz,},
        {.pattern = "ACQ:AVG", .callback = RP_AcqSetAveraging,},
        {.pattern = "ACQ:AVG?", .callback = RP_AcqGetAveraging,},
        {.pattern = "ACQ:TRIG", .callback = RP_AcqSetTriggerSrc,},
        {.pattern = "ACQ:TRIG:STAT?", .callback = RP_AcqGetTrigger,},
        {.pattern = "ACQ:TRIG:DLY", .callback = RP_AcqSetTriggerDelay,},
        {.pattern = "ACQ:TRIG:DLY?", .callback = RP_AcqGetTriggerDelay,},
        {.pattern = "ACQ:TRIG:DLY:NS", .callback = RP_AcqSetTriggerDelayNs,},
        {.pattern = "ACQ:TRIG:DLY:NS?", .callback = RP_AcqGetTriggerDelayNs,},
        {.pattern = "ACQ:SOUR1:GAIN", .callback = RP_AcqSetChannel1Gain,},
        {.pattern = "ACQ:SOUR2:GAIN", .callback = RP_AcqSetChannel2Gain,},
        {.pattern = "ACQ:SOUR1:GAIN?", .callback = RP_AcqGetChannel1Gain,},
        {.pattern = "ACQ:SOUR2:GAIN?", .callback = RP_AcqGetChannel2Gain,},
        {.pattern = "ACQ:TRIG:LEV", .callback = RP_AcqSetTriggerLevel,},
        {.pattern = "ACQ:TRIG:LEV?", .callback = RP_AcqGetTriggerLevel,},
        {.pattern = "ACQ:WPOS?", .callback = RP_AcqGetWritePointer,},
        {.pattern = "ACQ:TPOS?", .callback = RP_AcqGetWritePointerAtTrig,},
        {.pattern = "ACQ:DATA:UNITS", .callback = RP_AcqScpiDataUnits,},
        {.pattern = "ACQ:DATA:FORMAT", .callback = RP_AcqSetDataFormat,},
        {.pattern = "ACQ:SOUR1:DATA:STA:END?", .callback = RP_AcqGetChanel1DataPos,},
        {.pattern = "ACQ:SOUR2:DATA:STA:END?", .callback = RP_AcqGetChanel2DataPos,},
        {.pattern = "ACQ:SOUR1:DATA:STA:N?", .callback = RP_AcqGetChanel1Data,},
        {.pattern = "ACQ:SOUR2:DATA:STA:N?", .callback = RP_AcqGetChanel2Data,},
        {.pattern = "ACQ:SOUR1:DATA?", .callback = RP_AcqGetChanel1OldestDataAll,},
        {.pattern = "ACQ:SOUR2:DATA?", .callback = RP_AcqGetChanel2OldestDataAll,},
        {.pattern = "ACQ:SOUR1:DATA:OLD:N?", .callback = RP_AcqGetChanel1OldestData,},
        {.pattern = "ACQ:SOUR2:DATA:OLD:N?", .callback = RP_AcqGetChanel2OldestData,},
        {.pattern = "ACQ:SOUR1:DATA:LAT:N?", .callback = RP_AcqGetChanel1LatestData,},
        {.pattern = "ACQ:SOUR2:DATA:LAT:N?", .callback = RP_AcqGetChanel2LatestData,},
        {.pattern = "ACQ:BUF:SIZE?", .callback = RP_AcqGetBufferSize,},

        /* Generate */
        {.pattern = "OUTPUT1:STATE", .callback = RP_GenChannel1SetState,},
        {.pattern = "OUTPUT2:STATE", .callback = RP_GenChannel2SetState,},
        {.pattern = "OUTPUT1:STATE?", .callback = RP_GenChannel1GetState,},
        {.pattern = "OUTPUT2:STATE?", .callback = RP_GenChannel2GetState,},
        {.pattern = "GEN:RST", .callback = RP_GenReset,},
        {.pattern = "SOUR1:FREQ:FIX", .callback = RP_GenChannel1SetFrequency,},
        {.pattern = "SOUR2:FREQ:FIX", .callback = RP_GenChannel2SetFrequency,},
        {.pattern = "SOUR1:FREQ:FIX?", .callback = RP_GenChannel1GetFrequency,},
        {.pattern = "SOUR2:FREQ:FIX?", .callback = RP_GenChannel2GetFrequency,},
        {.pattern = "SOUR1:FUNC", .callback = RP_GenChannel1SetWaveForm,},
        {.pattern = "SOUR2:FUNC", .callback = RP_GenChannel2SetWaveForm,},
        {.pattern = "SOUR1:FUNC?", .callback = RP_GenChannel1GetWaveForm,},
        {.pattern = "SOUR2:FUNC?", .callback = RP_GenChannel2GetWaveForm,},
        {.pattern = "SOUR1:VOLT", .callback = RP_GenChannel1SetAmplitude,},
        {.pattern = "SOUR2:VOLT", .callback = RP_GenChannel2SetAmplitude,},
        {.pattern = "SOUR1:VOLT?", .callback = RP_GenChannel1GetAmplitude,},
        {.pattern = "SOUR2:VOLT?", .callback = RP_GenChannel2GetAmplitude,},
        {.pattern = "SOUR1:VOLT:OFFS", .callback = RP_GenChannel1SetOffset,},
        {.pattern = "SOUR2:VOLT:OFFS", .callback = RP_GenChannel2SetOffset,},
        {.pattern = "SOUR1:VOLT:OFFS?", .callback = RP_GenChannel1GetOffset,},
        {.pattern = "SOUR2:VOLT:OFFS?", .callback = RP_GenChannel2GetOffset,},
        {.pattern = "SOUR1:PHAS", .callback = RP_GenChannel1SetPhase,},
        {.pattern = "SOUR2:PHAS", .callback = RP_GenChannel2SetPhase,},
        {.pattern = "SOUR1:PHAS?", .callback = RP_GenChannel1GetPhase,},
        {.pattern = "SOUR2:PHAS?", .callback = RP_GenChannel2GetPhase,},
        {.pattern = "SOUR1:DCYC", .callback = RP_GenChannel1SetDutyCycle,},
        {.pattern = "SOUR2:DCYC", .callback = RP_GenChannel2SetDutyCycle,},
        {.pattern = "SOUR1:DCYC?", .callback = RP_GenChannel1GetDutyCycle,},
        {.pattern = "SOUR2:DCYC?", .callback = RP_GenChannel2GetDutyCycle,},
        {.pattern = "SOUR1:TRAC:DATA:DATA", .callback = RP_GenChannel1SetArbitraryWaveForm,},
        {.pattern = "SOUR2:TRAC:DATA:DATA", .callback = RP_GenChannel2SetArbitraryWaveForm,},
        {.pattern = "SOUR1:TRAC:DATA:DATA?", .callback = RP_GenChannel1GetArbitraryWaveForm,},
        {.pattern = "SOUR2:TRAC:DATA:DATA?", .callback = RP_GenChannel2GetArbitraryWaveForm,},
        {.pattern = "SOUR1:BURS:STAT", .callback = RP_GenChannel1SetGenerateMode,},
        {.pattern = "SOUR2:BURS:STAT", .callback = RP_GenChannel2SetGenerateMode,},
        {.pattern = "SOUR1:BURS:STAT?", .callback = RP_GenChannel1GetGenerateMode,},
        {.pattern = "SOUR2:BURS:STAT?", .callback = RP_GenChannel2GetGenerateMode,},
        {.pattern = "SOUR1:BURS:NCYC", .callback = RP_GenChannel1SetBurstCount,},
        {.pattern = "SOUR2:BURS:NCYC", .callback = RP_GenChannel2SetBurstCount,},
        {.pattern = "SOUR1:BURS:NCYC?", .callback = RP_GenChannel1GetBurstCount,},
        {.pattern = "SOUR2:BURS:NCYC?", .callback = RP_GenChannel2GetBurstCount,},
        {.pattern = "SOUR1:BURS:INT:PER", .callback = RP_GenChannel1SetBurstPeriod,},
        {.pattern = "SOUR2:BURS:INT:PER", .callback = RP_GenChannel2SetBurstPeriod,},
        {.pattern = "SOUR1:BURS:INT:PER?", .callback = RP_GenChannel1GetBurstPeriod,},
        {.pattern = "SOUR2:BURS:INT:PER?", .callback = RP_GenChannel2GetBurstPeriod,},
        {.pattern = "SOUR1:BURS:NOR", .callback = RP_GenChannel1SetBurstRepetitions,},
        {.pattern = "SOUR2:BURS:NOR", .callback = RP_GenChannel2SetBurstRepetitions,},
        {.pattern = "SOUR1:BURS:NOR?", .callback = RP_GenChannel1GetBurstRepetitions,},
        {.pattern = "SOUR2:BURS:NOR?", .callback = RP_GenChannel2GetBurstRepetitions,},
        {.pattern = "SOUR1:TRIG:SOUR", .callback = RP_GenChannel1SetTriggerSource,},
        {.pattern = "SOUR2:TRIG:SOUR", .callback = RP_GenChannel2SetTriggerSource,},
        {.pattern = "SOUR1:TRIG:SOUR?", .callback = RP_GenChannel1GetTriggerSource,},
        {.pattern = "SOUR2:TRIG:SOUR?", .callback = RP_GenChannel2GetTriggerSource,},
        {.pattern = "SOUR1:TRIG:IMM", .callback = RP_GenChannel1Trigger,},
        {.pattern = "SOUR2:TRIG:IMM", .callback = RP_GenChannel2Trigger,},
        {.pattern = "TRIG:IMM", .callback = RP_GenChannelAllTrigger,},

        /* Oscilloscope */
        {.pattern = "OSC:RUN", .callback = RP_APP_OscRun,},
        {.pattern = "OSC:STOP", .callback = RP_APP_OscStop,},
        {.pattern = "OSC:RST", .callback = RP_APP_OscReset,},
        {.pattern = "OSC:AUTOSCALE", .callback = RP_APP_OscAutoscale,},
        {.pattern = "OSC:SINGLE", .callback = RP_APP_OscSingle,},
        {.pattern = "OSC:RUNNING", .callback = RP_APP_OscRunning,},
        {.pattern = "OSC:CH1:OFFSET", .callback = RP_APP_OscChannel1SetAmplitudeOffset,},
        {.pattern = "OSC:CH2:OFFSET", .callback = RP_APP_OscChannel2SetAmplitudeOffset,},
        {.pattern = "OSC:MATH:OFFSET", .callback = RP_APP_OscChannel3SetAmplitudeOffset,},
        {.pattern = "OSC:CH1:OFFSET?", .callback = RP_APP_OscChannel1GetAmplitudeOffset,},
        {.pattern = "OSC:CH2:OFFSET?", .callback = RP_APP_OscChannel2GetAmplitudeOffset,},
        {.pattern = "OSC:MATH:OFFSET?", .callback = RP_APP_OscChannel3GetAmplitudeOffset,},
        {.pattern = "OSC:CH1:SCALE", .callback = RP_APP_OscChannel1SetAmplitudeScale,},
        {.pattern = "OSC:CH2:SCALE", .callback = RP_APP_OscChannel2SetAmplitudeScale,},
        {.pattern = "OSC:MATH:SCALE", .callback = RP_APP_OscChannel3SetAmplitudeScale,},
        {.pattern = "OSC:CH1:SCALE?", .callback = RP_APP_OscChannel1GetAmplitudeScale,},
        {.pattern = "OSC:CH2:SCALE?", .callback = RP_APP_OscChannel2GetAmplitudeScale,},
        {.pattern = "OSC:MATH:SCALE?", .callback = RP_APP_OscChannel3GetAmplitudeScale,},
        {.pattern = "OSC:CH1:PROBE", .callback = RP_APP_OscChannel1SetProbeAtt,},
        {.pattern = "OSC:CH2:PROBE", .callback = RP_APP_OscChannel2SetProbeAtt,},
        {.pattern = "OSC:CH1:PROBE?", .callback = RP_APP_OscChannel1GetProbeAtt,},
        {.pattern = "OSC:CH2:PROBE?", .callback = RP_APP_OscChannel2GetProbeAtt,},
        {.pattern = "OSC:CH1:IN:GAIN", .callback = RP_APP_OscChannel1SetInputGain,},
        {.pattern = "OSC:CH2:IN:GAIN", .callback = RP_APP_OscChannel2SetInputGain,},
        {.pattern = "OSC:CH1:IN:GAIN?", .callback = RP_APP_OscChannel1GetInputGain,},
        {.pattern = "OSC:CH2:IN:GAIN?", .callback = RP_APP_OscChannel2GetInputGain,},
        {.pattern = "OSC:TIME:OFFSET", .callback = RP_APP_OscSetTimeOffset,},
        {.pattern = "OSC:TIME:OFFSET?", .callback = RP_APP_OscGetTimeOffset,},
        {.pattern = "OSC:TIME:SCALE", .callback = RP_APP_OscSetTimeScale,},
        {.pattern = "OSC:TIME:SCALE?", .callback = RP_APP_OscGetTimeScale,},
        {.pattern = "OSC:TRIG:SWEEP", .callback = RP_APP_OscSetTriggerSweep,},
        {.pattern = "OSC:TRIG:SWEEP?", .callback = RP_APP_OscGetTriggerSweep,},
        {.pattern = "OSC:TRIG:SOURCE", .callback = RP_APP_OscSetTriggerSource,},
        {.pattern = "OSC:TRIG:SOURCE?", .callback = RP_APP_OscGetTriggerSource,},
        {.pattern = "OSC:TRIG:SLOPE", .callback = RP_APP_OscSetTriggerSlope,},
        {.pattern = "OSC:TRIG:SLOPE?", .callback = RP_APP_OscGetTriggerSlope,},
        {.pattern = "OSC:TRIG:LEVEL", .callback = RP_APP_OscSetTriggerLevel,},
        {.pattern = "OSC:TRIG:LEVEL?", .callback = RP_APP_OscGetTriggerLevel,},
        {.pattern = "OSC:CH1:DATA?", .callback = RP_APP_OscChannel1GetViewData,},
        {.pattern = "OSC:CH2:DATA?", .callback = RP_APP_OscChannel2GetViewData,},
        {.pattern = "OSC:MATH:DATA?", .callback = RP_APP_OscChannel3GetViewData,},
        {.pattern = "OSC:DATA:SIZE", .callback = RP_APP_OscSetViewSize,},
        {.pattern = "OSC:DATA:SIZE?", .callback = RP_APP_OscGetViewSize,},
        {.pattern = "OSC:VIEW:POS?", .callback = RP_APP_OscGetViewPos,},
        {.pattern = "OSC:VIEW:PART?", .callback = RP_APP_OscGetViewPart,},
        {.pattern = "OSC:MEAS:CH1:VPP?", .callback = RP_APP_OscChannel1MeasureAmplitude,},
        {.pattern = "OSC:MEAS:CH2:VPP?", .callback = RP_APP_OscChannel2MeasureAmplitude,},
        {.pattern = "OSC:MEAS:MATH:VPP?", .callback = RP_APP_OscChannel3MeasureAmplitude,},
        {.pattern = "OSC:MEAS:CH1:VMEAN?", .callback = RP_APP_OscChannel1MeasureMeanVoltage,},
        {.pattern = "OSC:MEAS:CH2:VMEAN?", .callback = RP_APP_OscChannel2MeasureMeanVoltage,},
        {.pattern = "OSC:MEAS:MATH:VMEAN?", .callback = RP_APP_OscChannel3MeasureMeanVoltage,},
        {.pattern = "OSC:MEAS:CH1:VMAX?", .callback = RP_APP_OscChannel1MeasureAmplitudeMax,},
        {.pattern = "OSC:MEAS:CH2:VMAX?", .callback = RP_APP_OscChannel2MeasureAmplitudeMax,},
        {.pattern = "OSC:MEAS:MATH:VMAX?", .callback = RP_APP_OscChannel3MeasureAmplitudeMax,},
        {.pattern = "OSC:MEAS:CH1:VMIN?", .callback = RP_APP_OscChannel1MeasureAmplitudeMin,},
        {.pattern = "OSC:MEAS:CH2:VMIN?", .callback = RP_APP_OscChannel2MeasureAmplitudeMin,},
        {.pattern = "OSC:MEAS:MATH:VMIN?", .callback = RP_APP_OscChannel3MeasureAmplitudeMin,},
        {.pattern = "OSC:MEAS:CH1:FREQ?", .callback = RP_APP_OscChannel1MeasureFrequency,},
        {.pattern = "OSC:MEAS:CH2:FREQ?", .callback = RP_APP_OscChannel2MeasureFrequency,},
        {.pattern = "OSC:MEAS:MATH:FREQ?", .callback = RP_APP_OscChannel3MeasureFrequency,},
        {.pattern = "OSC:MEAS:CH1:T0?", .callback = RP_APP_OscChannel1MeasurePeriod,},
        {.pattern = "OSC:MEAS:CH2:T0?", .callback = RP_APP_OscChannel2MeasurePeriod,},
        {.pattern = "OSC:MEAS:MATH:T0?", .callback = RP_APP_OscChannel3MeasurePeriod,},
        {.pattern = "OSC:MEAS:CH1:DCYC?", .callback = RP_APP_OscChannel1MeasureDutyCycle,},
        {.pattern = "OSC:MEAS:CH2:DCYC?", .callback = RP_APP_OscChannel2MeasureDutyCycle,},
        {.pattern = "OSC:MEAS:MATH:DCYC?", .callback = RP_APP_OscChannel3MeasureDutyCycle,},
        {.pattern = "OSC:MEAS:CH1:RMS?", .callback = RP_APP_OscChannel1RMS,},
        {.pattern = "OSC:MEAS:CH2:RMS?", .callback = RP_APP_OscChannel2RMS,},
        {.pattern = "OSC:MEAS:MATH:RMS?", .callback = RP_APP_OscChannel3RMS,},
        {.pattern = "OSC:CUR:CH1:V?", .callback = RP_APP_OscChannel1GetCursorVoltage,},
        {.pattern = "OSC:CUR:CH2:V?", .callback = RP_APP_OscChannel2GetCursorVoltage,},
        {.pattern = "OSC:CUR:MATH:V?", .callback = RP_APP_OscChannel3GetCursorVoltage,},
        {.pattern = "OSC:CUR:T?", .callback = RP_APP_OscGetCursorTime,},
        {.pattern = "OSC:CUR:DT?", .callback = RP_APP_OscGetCursorDeltaTime,},
        {.pattern = "OSC:CUR:CH1:DV?", .callback = RP_APP_OscChannel1GetCursorDeltaAmplitude,},
        {.pattern = "OSC:CUR:CH2:DV?", .callback = RP_APP_OscChannel2GetCursorDeltaAmplitude,},
        {.pattern = "OSC:CUR:MATH:DV?", .callback = RP_APP_OscChannel3GetCursorDeltaAmplitude,},
        {.pattern = "OSC:CUR:DF?", .callback = RP_APP_OscGetCursorDeltaFrequency,},
        {.pattern = "OSC:MATH:OP", .callback = RP_APP_OscSetMathOperation,},
        {.pattern = "OSC:MATH:OP?", .callback = RP_APP_OscGetMathOperation,},
        {.pattern = "OSC:MATH:SOUR", .callback = RP_APP_OscSetMathSources,},
        {.pattern = "OSC:MATH:SOUR?", .callback = RP_APP_OscGetMathSources,},

/*
scpi_result_t RP_APP_SpecRun(scpi_t *context); // :RUN
scpi_result_t RP_APP_SpecStop(scpi_t *context); // :STOP
scpi_result_t RP_APP_SpecReset(scpi_t *context); // :RST
scpi_result_t RP_APP_SpecAutoscale(scpi_t *context); // :AUTOSCALE
scpi_result_t RP_APP_SpecRunning(scpi_t *context); // :RUNNING

scpi_result_t RP_APP_SpecChannel1GetViewData(scpi_t *context); // :CH1:DATA?
scpi_result_t RP_APP_SpecChannel2GetViewData(scpi_t *context); // :CH2:DATA?
scpi_result_t RP_APP_SpecGetViewSize(scpi_t *context); // :DATA:SIZE?
scpi_result_t RP_APP_SpecSetViewSize(scpi_t *context); // :DATA:SIZE

scpi_result_t RP_APP_SpecChannel1GetPeak(scpi_t *context); // :CH1:PEAK
scpi_result_t RP_APP_SpecChannel2GetPeak(scpi_t *context); // :CH2:PEAK
scpi_result_t RP_APP_SpecChannel1Freeze(scpi_t *context); // :CH1:FREEZE
scpi_result_t RP_APP_SpecChannel2Freeze(scpi_t *context); // :CH2:FREEZE

scpi_result_t RP_APP_SpecGetFreqMin(scpi_t *context); // :FREQ:MIN
scpi_result_t RP_APP_SpecGetFreqMax(scpi_t *context); // :FREQ:MAX
*/

        /* Spectrum */
        {.pattern = "SPEC:RUN", .callback = RP_APP_SpecRun,},
        {.pattern = "SPEC:STOP", .callback = RP_APP_SpecStop,},
        {.pattern = "SPEC:RST", .callback = RP_APP_OscReset,},
		{.pattern = "SPEC:RUNNING", .callback = RP_APP_SpecRunning,},

        {.pattern = "SPEC:CH1:DATA?", .callback = RP_APP_SpecChannel1GetViewData,},
        {.pattern = "SPEC:CH2:DATA?", .callback = RP_APP_SpecChannel2GetViewData,},

        {.pattern = "SPEC:DATA:SIZE?", .callback = RP_APP_SpecGetViewSize,},
        //{.pattern = "SPEC:DATA:SIZE", .callback = RP_APP_SpecSetViewSize,},

        {.pattern = "SPEC:CH1:PEAK?", .callback = RP_APP_SpecChannel1GetPeak,},
        {.pattern = "SPEC:CH2:PEAK?", .callback = RP_APP_SpecChannel2GetPeak,},
        {.pattern = "SPEC:CH1:PEAK:FREQ?", .callback = RP_APP_SpecChannel1GetPeakFreq,},
        {.pattern = "SPEC:CH2:PEAK:FREQ?", .callback = RP_APP_SpecChannel2GetPeakFreq,},

//TODO
/*
        {.pattern = "SPEC:CH1:FREEZE?", .callback = RP_APP_SpecChannel1Freeze,},
        {.pattern = "SPEC:CH2:FREEZE?", .callback = RP_APP_SpecChannel2Freeze,},

        {.pattern = "SPEC:CUR:CH1:DB?", .callback = RP_APP_OscChannel1SetProbeAtt,},
        {.pattern = "SPEC:CUR:CH2:DB?", .callback = RP_APP_OscChannel2SetProbeAtt,},

        {.pattern = "SPEC:CUR:CH1:FREQ", .callback = RP_APP_OscChannel1SetProbeAtt,},
        {.pattern = "SPEC:CUR:CH2:FREQ", .callback = RP_APP_OscChannel2SetProbeAtt,},
*/

    SCPI_CMD_LIST_END
};

static scpi_interface_t scpi_interface = {
    .error = SCPI_Error,
    .write = SCPI_Write,
    .control = SCPI_Control,
    .flush = SCPI_Flush,
    .reset = SCPI_Reset,
    .test = SCPI_Test,
};

#define SCPI_INPUT_BUFFER_LENGTH 262144
static char scpi_input_buffer[SCPI_INPUT_BUFFER_LENGTH];

static scpi_reg_val_t scpi_regs[SCPI_REG_COUNT];


scpi_t scpi_context = {
    .cmdlist = scpi_commands,
    .buffer = {
        .length = SCPI_INPUT_BUFFER_LENGTH,
        .data = scpi_input_buffer,
    },
    .interface = &scpi_interface,
    .registers = scpi_regs,
    .units = scpi_units_def,
    .special_numbers = scpi_special_numbers_def,
    .idn = {"REDPITAYA", "INSTR2014", NULL, "01-02"},
};
