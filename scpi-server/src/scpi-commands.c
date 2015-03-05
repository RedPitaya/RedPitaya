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
#include "scpi/scpi.h"

#include "utils.h"
#include "dpin.h"
#include "apin.h"
#include "generate.h"

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
        {.pattern = "ACQ:RST", .callback = RP_AcqReset,},
        {.pattern = "ACQ:DEC", .callback = RP_AcqSetDecimation,},
        {.pattern = "ACQ:DEC?", .callback = RP_AcqGetDecimation,},
        {.pattern = "ACQ:SRAT", .callback = RP_AcqSetSamplingRate,},
        {.pattern = "ACQ:SRAT?", .callback = RP_AcqGetSamplingRate,},
        {.pattern = "ACQ:SRA:HZ?", .callback = RP_AcqGetSamplingRateHz,},
        {.pattern = "ACQ:AVG", .callback = RP_AcqSetAveraging,},
        {.pattern = "ACQ:AVG?", .callback = RP_AcqGetAveraging,},
        {.pattern = "ACQ:TRIG", .callback = RP_AcqSetTriggerSrc,},
        {.pattern = "ACQ:TRIG:STAT?", .callback = RP_AcqGetTriggerSrc,},
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
        {.pattern = "ACQ:DATA:FORMAT", .callback = RP_AcqScpiDataFormat,},
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

        /* Deep averagning acquire */
        {.pattern = "ACQ:DP:START", .callback = RP_AcqDPAvgStart,                      },
        {.pattern = "ACQ:DP:COUNT?", .callback = RP_AcqDPAvgSetCount,                  },
        {.pattern = "ACQ:DP:SHIFT?", .callback = RP_AcqDPAvgSetShift,                  },
        {.pattern = "ACQ:DP:LEN?", .callback = RP_AcqDPAvgSetSeqLen,                   },
        {.pattern = "ACQ:DP:DEBTIM?", .callback = RP_AcqDPAvgSetDebTim,                },
        {.pattern = "ACQ:DP:COUNT", .callback = RP_AcqDPAvgGetCount,                   },
        {.pattern = "ACQ:DP:SHIFT", .callback = RP_AcqDPAvgGetShift,                   },
        {.pattern = "ACQ:DP:LEN", .callback = RP_AcqDPAvgGetSeqLen,                    },
        {.pattern = "ACQ:DP:DEBTIM", .callback = RP_AcqDPAvgGetDebTim,                 },
        {.pattern = "ACQ:DP:RUN?", .callback = RP_AcqDPAvgGetRunState,                  },
        {.pattern = "ACQ:DP:SOUR1:DATA?", .callback = RP_AcqDPAvgGetRawDataCh1,        },
        {.pattern = "ACQ:DP:SOUR2:DATA?", .callback = RP_AcqDPAvgGetRawDataCh2,        },


        /* Generate */
        {.pattern = "OUTPUT1:STATE", .callback = RP_GenChannel1SetState,},
        {.pattern = "OUTPUT2:STATE", .callback = RP_GenChannel2SetState,},
        {.pattern = "GEN:RST", .callback = RP_GenReset,},
        {.pattern = "SOUR1:FREQ:FIX", .callback = RP_GenChannel1SetFrequency,},
        {.pattern = "SOUR2:FREQ:FIX", .callback = RP_GenChannel2SetFrequency,},
        {.pattern = "SOUR1:FUNC", .callback = RP_GenChannel1SetWaveForm,},
        {.pattern = "SOUR2:FUNC", .callback = RP_GenChannel2SetWaveForm,},
        {.pattern = "SOUR1:VOLT", .callback = RP_GenChannel1SetAmplitude,},
        {.pattern = "SOUR2:VOLT", .callback = RP_GenChannel2SetAmplitude,},
        {.pattern = "SOUR1:VOLT:OFFS", .callback = RP_GenChannel1SetOffset,},
        {.pattern = "SOUR2:VOLT:OFFS", .callback = RP_GenChannel2SetOffset,},
        {.pattern = "SOUR1:PHAS", .callback = RP_GenChannel1SetPhase,},
        {.pattern = "SOUR2:PHAS", .callback = RP_GenChannel2SetPhase,},
        {.pattern = "SOUR1:DCYC", .callback = RP_GenChannel1SetDutyCycle,},
        {.pattern = "SOUR2:DCYC", .callback = RP_GenChannel2SetDutyCycle,},
        {.pattern = "SOUR1:TRAC:DATA:DATA", .callback = RP_GenChannel1SetArbitraryWaveForm,},
        {.pattern = "SOUR2:TRAC:DATA:DATA", .callback = RP_GenChannel2SetArbitraryWaveForm,},
        {.pattern = "SOUR1:BURS:STAT", .callback = RP_GenChannel1SetGenerateMode,},
        {.pattern = "SOUR2:BURS:STAT", .callback = RP_GenChannel2SetGenerateMode,},
        {.pattern = "SOUR1:BURS:NCYC", .callback = RP_GenChannel1SetBurstCount,},
        {.pattern = "SOUR2:BURS:NCYC", .callback = RP_GenChannel2SetBurstCount,},
        {.pattern = "SOUR1:BURS:INT:PER", .callback = RP_GenChannel1SetBurstPeriod,},
        {.pattern = "SOUR2:BURS:INT:PER", .callback = RP_GenChannel2SetBurstPeriod,},
        {.pattern = "SOUR1:BURS:NOR", .callback = RP_GenChannel1SetBurstRepetitions,},
        {.pattern = "SOUR2:BURS:NOR", .callback = RP_GenChannel2SetBurstRepetitions,},
        {.pattern = "SOUR1:TRIG:SOUR", .callback = RP_GenChannel1SetTriggerSource,},
        {.pattern = "SOUR2:TRIG:SOUR", .callback = RP_GenChannel2SetTriggerSource,},
        {.pattern = "SOUR1:TRIG:IMM", .callback = RP_GenChannel1SetTrigger,},
        {.pattern = "SOUR2:TRIG:IMM", .callback = RP_GenChannel2SetTrigger,},
        {.pattern = "TRIG:IMM", .callback = RP_GenChannel3SetTrigger,},

    //{.pattern = "MEASure:PERiod?", .callback = SCPI_StubQ,},

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
