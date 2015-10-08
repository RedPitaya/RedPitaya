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

#include "api_cmd.h"
#include "utils.h"
#include "dpin.h"
#include "apin.h"
#include "generate.h"
#include "scpi/error.h"
#include "scpi/ieee488.h"
#include "scpi/minimal.h"
#include "scpi/units.h"
#include "scpi/parser.h"

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

        /* General commands */
        {.pattern = "RP:INit", .callback            = RP_InitAll,},
        {.pattern = "RP:REset", .callback           = RP_ResetAll,},
        {.pattern = "RP:RELease", .callback         = RP_ReleaseAll,},
        {.pattern = "RP:FPGA:BITstr", .callback     = RP_FpgaBitStream,},
        {.pattern = "RP:DIG:LOop", .callback        = RP_EnableDigLoop,},

        {.pattern = "DIG:RST", .callback            = RP_DigitalPinReset,},
        {.pattern = "DIG:PIN", .callback            = RP_DigitalPinState,},
        {.pattern = "DIG:PIN?", .callback           = RP_DigitalPinStateQ,},
        {.pattern = "DIG:PIN:DIR", .callback        = RP_DigitalPinDirection,},

        {.pattern = "ANALOG:RST", .callback         = RP_AnalogPinReset,},
        {.pattern = "ANALOG:PIN", .callback         = RP_AnalogPinValue,},
        {.pattern = "ANALOG:PIN?", .callback        = RP_AnalogPinValueQ,},

        /* Acquire */
        {.pattern = "ACQ:START", .callback          = RP_AcqStart,},
        {.pattern = "ACQ:STOP", .callback           = RP_AcqStop,},
        {.pattern = "ACQ:RST", .callback            = RP_AcqReset,},
        {.pattern = "ACQ:DEC", .callback            = RP_AcqDecimation,},
        {.pattern = "ACQ:DEC?", .callback           = RP_AcqDecimationQ,},
        {.pattern = "ACQ:SRAT", .callback           = RP_AcqSamplingRate,},
        {.pattern = "ACQ:SRAT?", .callback          = RP_AcqSamplingRateQ,},
        {.pattern = "ACQ:SRA:HZ?", .callback        = RP_AcqSamplingRateHzQ,},
        {.pattern = "ACQ:AVG", .callback            = RP_AcqAveraging,},
        {.pattern = "ACQ:AVG?", .callback           = RP_AcqAveragingQ,},
        {.pattern = "ACQ:TRIG", .callback           = RP_AcqTriggerSrc,},
        {.pattern = "ACQ:TRIG:STAT?", .callback     = RP_AcqTriggerQ,},
        {.pattern = "ACQ:TRIG:DLY", .callback       = RP_AcqTriggerDelay,},
        {.pattern = "ACQ:TRIG:DLY?", .callback      = RP_AcqTriggerDelayQ,},
        {.pattern = "ACQ:TRIG:DLY:NS", .callback    = RP_AcqTriggerDelayNs,},
        {.pattern = "ACQ:TRIG:DLY:NS?", .callback   = RP_AcqTriggerDelayNsQ,},
        {.pattern = "ACQ:SOUR#:GAIN", .callback     = RP_AcqGain,},
        {.pattern = "ACQ:SOUR#:GAIN?", .callback    = RP_AcqGainQ,},
        {.pattern = "ACQ:TRIG:LEV", .callback       = RP_AcqTriggerLevel,},
        {.pattern = "ACQ:TRIG:LEV?", .callback      = RP_AcqTriggerLevel,},
        {.pattern = "ACQ:WPOS?", .callback          = RP_AcqWritePointerQ,},
        {.pattern = "ACQ:TPOS?", .callback          = RP_AcqWritePointerAtTrigQ,},
        {.pattern = "ACQ:DATA:UNITS", .callback     = RP_AcqScpiDataUnits,},
        {.pattern = "ACQ:DATA:UNITS?", .callback     = RP_AcqScpiDataUnitsQ,},
        {.pattern = "ACQ:DATA:FORMAT", .callback    = RP_AcqSetDataFormat,},
        {.pattern = "ACQ:SOUR#:DATA:STA:END?", .callback    = RP_AcqDataPosQ,},
        {.pattern = "ACQ:SOUR#:DATA:STA:N?", .callback      = RP_AcqDataQ,},
        {.pattern = "ACQ:SOUR#:DATA:OLD:N?", .callback      = RP_AcqOldestDataQ,},
        {.pattern = "ACQ:SOUR#:DATA?", .callback            = RP_AcqDataOldestAllQ,},
        {.pattern = "ACQ:SOUR#:DATA:LAT:N?", .callback      = RP_AcqLatestDataQ,},
        {.pattern = "ACQ:BUF:SIZE?", .callback              = RP_AcqBufferSizeQ,},

        /* Generate */
        {.pattern = "GEN:RST", .callback                = RP_GenReset,},
        {.pattern = "OUTPUT#:STATE", .callback          = RP_GenState,},
        {.pattern = "OUTPUT#:STATE?", .callback         = RP_GenStateQ,},
        {.pattern = "SOUR#:FREQ:FIX", .callback         = RP_GenFrequency,},
        {.pattern = "SOUR#:FREQ:FIX?", .callback        = RP_GenFrequencyQ,},
        {.pattern = "SOUR1:FUNC", .callback             = RP_GenChannel1WaveForm,},
        {.pattern = "SOUR2:FUNC", .callback             = RP_GenChannel2WaveForm,},
        {.pattern = "SOUR1:FUNC?", .callback            = RP_GenChannel1WaveFormQ,},
        {.pattern = "SOUR2:FUNC?", .callback            = RP_GenChannel2WaveFormQ,},
        {.pattern = "SOUR1:VOLT", .callback             = RP_GenChannel1Amplitude,},
        {.pattern = "SOUR2:VOLT", .callback             = RP_GenChannel2Amplitude,},
        {.pattern = "SOUR1:VOLT?", .callback            = RP_GenChannel1AmplitudeQ,},
        {.pattern = "SOUR2:VOLT?", .callback            = RP_GenChannel2AmplitudeQ,},
        {.pattern = "SOUR1:VOLT:OFFS", .callback        = RP_GenChannel1Offset,},
        {.pattern = "SOUR2:VOLT:OFFS", .callback        = RP_GenChannel2Offset,},
        {.pattern = "SOUR1:VOLT:OFFS?", .callback       = RP_GenChannel1OffsetQ,},
        {.pattern = "SOUR2:VOLT:OFFS?", .callback       = RP_GenChannel2OffsetQ,},
        {.pattern = "SOUR1:PHAS", .callback             = RP_GenChannel1Phase,},
        {.pattern = "SOUR2:PHAS", .callback             = RP_GenChannel2Phase,},
        {.pattern = "SOUR1:PHAS?", .callback            = RP_GenChannel1PhaseQ,},
        {.pattern = "SOUR2:PHAS?", .callback            = RP_GenChannel2PhaseQ,},
        {.pattern = "SOUR1:DCYC", .callback             = RP_GenChannel1DutyCycle,},
        {.pattern = "SOUR2:DCYC", .callback             = RP_GenChannel2DutyCycle,},
        {.pattern = "SOUR1:DCYC?", .callback            = RP_GenChannel1DutyCycleQ,},
        {.pattern = "SOUR2:DCYC?", .callback            = RP_GenChannel2DutyCycleQ,},
        {.pattern = "SOUR1:TRAC:DATA:DATA", .callback   = RP_GenChannel1ArbitraryWaveForm,},
        {.pattern = "SOUR2:TRAC:DATA:DATA", .callback   = RP_GenChannel2ArbitraryWaveForm,},
        {.pattern = "SOUR1:TRAC:DATA:DATA?", .callback  = RP_GenChannel1ArbitraryWaveFormQ,},
        {.pattern = "SOUR2:TRAC:DATA:DATA?", .callback  = RP_GenChannel2ArbitraryWaveFormQ,},
        {.pattern = "SOUR1:BURS:STAT", .callback        = RP_GenChannel1GenerateMode,},
        {.pattern = "SOUR2:BURS:STAT", .callback        = RP_GenChannel2GenerateMode,},
        {.pattern = "SOUR1:BURS:STAT?", .callback       = RP_GenChannel1GenerateModeQ,},
        {.pattern = "SOUR2:BURS:STAT?", .callback       = RP_GenChannel2GenerateModeQ,},
        {.pattern = "SOUR1:BURS:NCYC", .callback        = RP_GenChannel1BurstCount,},
        {.pattern = "SOUR2:BURS:NCYC", .callback        = RP_GenChannel2BurstCount,},
        {.pattern = "SOUR1:BURS:NCYC?", .callback       = RP_GenChannel1BurstCountQ,},
        {.pattern = "SOUR2:BURS:NCYC?", .callback       = RP_GenChannel2BurstCountQ,},
        {.pattern = "SOUR1:BURS:INT:PER", .callback     = RP_GenChannel1BurstPeriod,},
        {.pattern = "SOUR2:BURS:INT:PER", .callback     = RP_GenChannel2BurstPeriod,},
        {.pattern = "SOUR1:BURS:INT:PER?", .callback    = RP_GenChannel1BurstPeriodQ,},
        {.pattern = "SOUR2:BURS:INT:PER?", .callback    = RP_GenChannel2BurstPeriodQ,},
        {.pattern = "SOUR1:BURS:NOR", .callback         = RP_GenChannel1BurstRepetitions,},
        {.pattern = "SOUR2:BURS:NOR", .callback         = RP_GenChannel2BurstRepetitions,},
        {.pattern = "SOUR1:BURS:NOR?", .callback        = RP_GenChannel1BurstRepetitionsQ,},
        {.pattern = "SOUR2:BURS:NOR?", .callback        = RP_GenChannel2BurstRepetitionsQ,},
        {.pattern = "SOUR1:TRIG:SOUR", .callback        = RP_GenChannel1TriggerSource,},
        {.pattern = "SOUR2:TRIG:SOUR", .callback        = RP_GenChannel2TriggerSource,},
        {.pattern = "SOUR1:TRIG:SOUR?", .callback       = RP_GenChannel1TriggerSourceQ,},
        {.pattern = "SOUR2:TRIG:SOUR?", .callback       = RP_GenChannel2TriggerSourceQ,},
        {.pattern = "SOUR1:TRIG:IMM", .callback         = RP_GenChannel1Trigger,},
        {.pattern = "SOUR2:TRIG:IMM", .callback         = RP_GenChannel2Trigger,},
        {.pattern = "TRIG:IMM", .callback               = RP_GenChannelAllTrigger,},

    SCPI_CMD_LIST_END
};

static scpi_interface_t scpi_interface = {
    .error = SCPI_Error,
    .write = SCPI_Write,
    .control = SCPI_Control,
    .flush = SCPI_Flush,
    .reset = SCPI_Reset,
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
    .idn = {"REDPITAYA", "INSTR2014", NULL, "01-02"},
};

