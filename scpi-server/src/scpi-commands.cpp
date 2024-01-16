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
#include <stdio.h>

#include "api_cmd.h"
#include "common.h"
#include "dpin.h"
#include "error.h"
#include "apin.h"
#include "uart.h"
#include "led.h"
#include "spi.h"
#include "i2c.h"
#include "can.h"
#include "acquire.h"
#include "acquire_axi.h"
#include "generate.h"

#include "scpi/error.h"
#include "scpi/ieee488.h"
#include "scpi/minimal.h"
#include "scpi/units.h"
#include "scpi/parser.h"

bool RST_executed = FALSE;

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
    syslog(LOG_ERR, "**ERROR: %d, \"%s\"", (int32_t) err, SCPI_ErrorTranslate(err));
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
    /* Terminating all scpi operations */
    RP_LOG_INFO("Sucsessfuly reset scpi server.")
    return SCPI_RES_OK;
}

scpi_result_t SCPI_SystemCommTcpipControlQ(scpi_t * context) {
    syslog(LOG_ERR, "**SCPI_SystemCommTcpipControlQ not implemented");
    return SCPI_RES_ERR;
}

scpi_result_t SCPI_SystemErrorNextQEx(scpi_t * context) {
    if (rp_errorCount(context)){
        auto err = rp_popError(context);
        SCPI_ResultInt32(context, err.baseCode + err.errorCode);
        SCPI_ResultText(context, err.msg.c_str());
        return SCPI_RES_OK;
    }

    return SCPI_SystemErrorNextQ(context);
}

scpi_result_t SCPI_SystemErrorCountQEx(scpi_t * context) {
    SCPI_ResultInt32(context, SCPI_ErrorCount(context) + rp_errorCount(context));
    return SCPI_RES_OK;
}

scpi_result_t SCPI_CoreClsEx(scpi_t * context) {
    rp_resetErrorList(context);
    return SCPI_CoreCls(context);
}

scpi_result_t RP_EcosystemVersionQ(scpi_t * context) {
    SCPI_ResultMnemonic(context, rp_GetVersion());
    return SCPI_RES_OK;
}

/**
 * SCPI Configuration
 */

static const scpi_command_t scpi_commands[] = {
    /* IEEE Mandated Commands (SCPI std V1999.0 4.1.1) */
    { .pattern = "*CLS" , .callback = SCPI_CoreClsEx,},
    { .pattern = "*ESE" , .callback = SCPI_CoreEse,},
    { .pattern = "*ESE?", .callback = SCPI_CoreEseQ,},
    { .pattern = "*ESR?", .callback = SCPI_CoreEsrQ,},
    { .pattern = "*IDN?", .callback = SCPI_CoreIdnQ,},
    { .pattern = "*OPC" , .callback = SCPI_CoreOpc,},
    { .pattern = "*OPC?", .callback = SCPI_CoreOpcQ,},
    { .pattern = "*RST" , .callback = SCPI_CoreRst,},
    { .pattern = "*SRE" , .callback = SCPI_CoreSre,},
    { .pattern = "*SRE?", .callback = SCPI_CoreSreQ,},
    { .pattern = "*STB?", .callback = SCPI_CoreStbQ,},
    { .pattern = "*TST?", .callback = SCPI_CoreTstQ,},
    { .pattern = "*WAI" , .callback = SCPI_CoreWai,},

    /* Required SCPI commands (SCPI std V1999.0 4.2.1) */
    {.pattern = "SYSTem:ERRor[:NEXT]?", .callback = SCPI_SystemErrorNextQEx,},
    {.pattern = "SYSTem:ERRor:COUNt?",  .callback = SCPI_SystemErrorCountQEx,},
    {.pattern = "SYSTem:VERSion?",      .callback = RP_EcosystemVersionQ,},
    {.pattern = "SYSTem:BRD:ID?",       .callback = RP_BoardID,},
    {.pattern = "SYSTem:BRD:Name?",     .callback = RP_BoardName,},

    {.pattern = "STATus:QUEStionable[:EVENt]?", .callback = SCPI_StatusQuestionableEventQ,},
    {.pattern = "STATus:QUEStionable:ENABle",   .callback = SCPI_StatusQuestionableEnable,},
    {.pattern = "STATus:QUEStionable:ENABle?",  .callback = SCPI_StatusQuestionableEnableQ,},
    {.pattern = "STATus:PRESet",                .callback = SCPI_StatusPreset,},

    {.pattern = "SYSTem:COMMunication:TCPIP:CONTROL?", .callback = SCPI_SystemCommTcpipControlQ,},

    /* RedPitaya */

    {.pattern = "SYSTem:TIME", .callback                = RP_Time,},
    {.pattern = "SYSTem:TIME?", .callback               = RP_TimeQ,},

    {.pattern = "SYSTem:DATE", .callback                = RP_Date,},
    {.pattern = "SYSTem:DATE?", .callback               = RP_DateQ,},

    /* General commands */
    {.pattern = "RP:INit", .callback                    = RP_InitAll,},
    {.pattern = "RP:REset", .callback                   = RP_ResetAll,},
    {.pattern = "RP:RELease", .callback                 = RP_ReleaseAll,},
    {.pattern = "RP:DIg[:loop]", .callback              = RP_EnableDigLoop,},
    {.pattern = "RP:LOGmode", .callback                 = RP_SetLogMode,},

    {.pattern = "DIG:RST", .callback                    = RP_DigitalPinReset,},
    {.pattern = "DIG:PIN", .callback                    = RP_DigitalPinState,},
    {.pattern = "DIG:PIN?", .callback                   = RP_DigitalPinStateQ,},
    {.pattern = "DIG:PIN:DIR", .callback                = RP_DigitalPinDirection,},
    {.pattern = "DIG:PIN:DIR?", .callback               = RP_DigitalPinDirectionQ,},

    {.pattern = "ANALOG:RST", .callback                 = RP_AnalogPinReset,},
    {.pattern = "ANALOG:PIN", .callback                 = RP_AnalogPinValue,},
    {.pattern = "ANALOG:PIN?", .callback                = RP_AnalogPinValueQ,},

    {.pattern = "DAISY:SYNC:TRig", .callback            = RP_EnableDaisyChainTrigSync,},
    {.pattern = "DAISY:SYNC:TRig?", .callback           = RP_EnableDaisyChainTrigSyncQ,},

    {.pattern = "DAISY:SYNC:CLK", .callback             = RP_EnableDaisyChainClockSync,},
    {.pattern = "DAISY:SYNC:CLK?", .callback            = RP_EnableDaisyChainClockSyncQ,},

    {.pattern = "DAISY:TRIG_O:ENable", .callback        = RP_DpinEnableTrigOutput,},
    {.pattern = "DAISY:TRIG_O:ENable?", .callback       = RP_DpinEnableTrigOutputQ,},

    {.pattern = "DAISY:TRIG_O:SOUR", .callback          = RP_SourceTrigOutput,},
    {.pattern = "DAISY:TRIG_O:SOUR?", .callback         = RP_SourceTrigOutputQ,},

    /* Acquire */
    {.pattern = "ACQ:START", .callback                  = RP_AcqStart,},
    {.pattern = "ACQ:STOP", .callback                   = RP_AcqStop,},
    {.pattern = "ACQ:RST", .callback                    = RP_AcqReset,},
    {.pattern = "ACQ:DEC", .callback                    = RP_AcqDecimation,},
    {.pattern = "ACQ:DEC?", .callback                   = RP_AcqDecimationQ,},
    {.pattern = "ACQ:DEC:Factor", .callback             = RP_AcqDecimationFactor,},
    {.pattern = "ACQ:DEC:Factor?", .callback            = RP_AcqDecimationFactorQ,},

    {.pattern = "ACQ:SRATe?", .callback                 = RP_AcqSamplingRateHzQ,},
    {.pattern = "ACQ:AVG", .callback                    = RP_AcqAveraging,},
    {.pattern = "ACQ:AVG?", .callback                   = RP_AcqAveragingQ,},
    {.pattern = "ACQ:TRig", .callback                   = RP_AcqTriggerSrc,},
    {.pattern = "ACQ:TRig:STAT?", .callback             = RP_AcqTriggerSrcQ,},
    {.pattern = "ACQ:TRig:DLY", .callback               = RP_AcqTriggerDelay,},
    {.pattern = "ACQ:TRig:DLY?", .callback              = RP_AcqTriggerDelayQ,},
    {.pattern = "ACQ:TRig:DLY:NS", .callback            = RP_AcqTriggerDelayNs,},
    {.pattern = "ACQ:TRig:DLY:NS?", .callback           = RP_AcqTriggerDelayNsQ,},
    {.pattern = "ACQ:TRig:HYST", .callback              = RP_AcqTriggerHyst,},
    {.pattern = "ACQ:TRig:HYST?", .callback             = RP_AcqTriggerHystQ,},
    {.pattern = "ACQ:TRig:FILL?", .callback             = RP_AcqTriggerFillQ,},
    {.pattern = "ACQ:SOUR#:GAIN", .callback             = RP_AcqGain,},
    {.pattern = "ACQ:SOUR#:GAIN?", .callback            = RP_AcqGainQ,},
    {.pattern = "ACQ:TRig:LEV", .callback               = RP_AcqTriggerLevel,},
    {.pattern = "ACQ:TRig:LEV?", .callback              = RP_AcqTriggerLevelQ,},
    {.pattern = "ACQ:WPOS?", .callback                  = RP_AcqWritePointerQ,},
    {.pattern = "ACQ:TPOS?", .callback                  = RP_AcqWritePointerAtTrigQ,},
    {.pattern = "ACQ:DATA:Units", .callback             = RP_AcqScpiDataUnits,},
    {.pattern = "ACQ:DATA:Units?", .callback            = RP_AcqScpiDataUnitsQ,},
    {.pattern = "ACQ:DATA:FORMAT", .callback            = RP_AcqDataFormat,},
    {.pattern = "ACQ:DATA:FORMAT?", .callback           = RP_AcqDataFormatQ,},
    {.pattern = "ACQ:SOUR#:DATA:Start:End?", .callback  = RP_AcqDataPosQ,},
    {.pattern = "ACQ:SOUR#:DATA:Start:N?", .callback    = RP_AcqDataQ,},
    {.pattern = "ACQ:SOUR#:DATA:Old:N?", .callback      = RP_AcqOldestDataQ,},
    {.pattern = "ACQ:SOUR#:DATA?", .callback            = RP_AcqDataOldestAllQ,},
    {.pattern = "ACQ:SOUR#:DATA:Last:N?", .callback     = RP_AcqLatestDataQ,},
    {.pattern = "ACQ:SOUR#:DATA:TRig?", .callback       = RP_AcqTriggerDataQ,},
    {.pattern = "ACQ:BUF:SIZE?", .callback              = RP_AcqBufferSizeQ,},

    // DMA mode for ACQ
    {.pattern = "ACQ:AXI:DATA:Units", .callback         = RP_AcqAxiScpiDataUnits,},
    {.pattern = "ACQ:AXI:DATA:Units?", .callback        = RP_AcqAxiScpiDataUnitsQ,},
    {.pattern = "ACQ:AXI:DEC", .callback                = RP_AcqAxiDecimation,},
    {.pattern = "ACQ:AXI:DEC?", .callback               = RP_AcqAxiDecimationQ,},
    {.pattern = "ACQ:AXI:START?", .callback             = RP_AcqAxiStartQ,},
    {.pattern = "ACQ:AXI:SIZE?", .callback              = RP_AcqAxiEndQ,},
    {.pattern = "ACQ:AXI:SOUR#:Trig:Fill?", .callback   = RP_AcqAxiTriggerFillQ,},
    {.pattern = "ACQ:AXI:SOUR#:Trig:Dly", .callback     = RP_AcqAxiTriggerDelay,},
    {.pattern = "ACQ:AXI:SOUR#:Trig:Dly?", .callback    = RP_AcqAxiTriggerDelayQ,},
    {.pattern = "ACQ:AXI:SOUR#:Write:Pos?", .callback   = RP_AcqAxiWritePointerQ,},
    {.pattern = "ACQ:AXI:SOUR#:Trig:Pos?", .callback    = RP_AcqAxiWritePointerAtTrigQ,},
    {.pattern = "ACQ:AXI:SOUR#:ENable", .callback       = RP_AcqAxiEnable,},
    {.pattern = "ACQ:AXI:SOUR#:DATA:Start:N?",.callback = RP_AcqAxiDataQ,},
    {.pattern = "ACQ:AXI:SOUR#:SET:Buffer", .callback   = RP_AcqAxiSetAddres,},


    {.pattern = "ACQ:SOUR#:COUP", .callback             = RP_AcqAC_DC,},
    {.pattern = "ACQ:SOUR#:COUP?", .callback            = RP_AcqAC_DCQ,},
    {.pattern = "ACQ:TRig:EXT:LEV", .callback           = RP_AcqExtTriggerLevel,},
    {.pattern = "ACQ:TRig:EXT:LEV?", .callback          = RP_AcqExtTriggerLevelQ,},

    {.pattern = "ACQ:TRig:EXT:DEBouncer[:US]", .callback   = RP_AcqExtTriggerDebouncerUs,},
    {.pattern = "ACQ:TRig:EXT:DEBouncer[:US]?", .callback  = RP_AcqExtTriggerDebouncerUsQ,},

    /* Generate */
    {.pattern = "GEN:RST", .callback                    = RP_GenReset,},
    {.pattern = "PHAS:ALIGN", .callback                 = RP_GenSync,},
    {.pattern = "OUTPUT:STATE", .callback               = RP_GenSyncState,},
    {.pattern = "OUTPUT#:STATE", .callback              = RP_GenState,},
    {.pattern = "OUTPUT#:STATE?", .callback             = RP_GenStateQ,},
    {.pattern = "SOUR:TRig:INT", .callback              = RP_GenTriggerBoth,},
    {.pattern = "SOUR#:FREQ:FIX", .callback             = RP_GenFrequency,},
    {.pattern = "SOUR#:FREQ:FIX?", .callback            = RP_GenFrequencyQ,},
    {.pattern = "SOUR#:FREQ:FIX:Direct", .callback      = RP_GenFrequencyDirect,},
    {.pattern = "SOUR#:FUNC", .callback                 = RP_GenWaveForm,},
    {.pattern = "SOUR#:FUNC?", .callback                = RP_GenWaveFormQ,},
    {.pattern = "SOUR#:VOLT", .callback                 = RP_GenAmplitude,},
    {.pattern = "SOUR#:VOLT?", .callback                = RP_GenAmplitudeQ,},
    {.pattern = "SOUR#:VOLT:OFFS", .callback            = RP_GenOffset,},
    {.pattern = "SOUR#:VOLT:OFFS?", .callback           = RP_GenOffsetQ,},
    {.pattern = "SOUR#:PHAS", .callback                 = RP_GenPhase,},
    {.pattern = "SOUR#:PHAS?", .callback                = RP_GenPhaseQ,},
    {.pattern = "SOUR#:DCYC", .callback                 = RP_GenDutyCycle,},
    {.pattern = "SOUR#:DCYC?", .callback                = RP_GenDutyCycleQ,},
    {.pattern = "SOUR#:TRAC:DATA:DATA", .callback       = RP_GenArbitraryWaveForm,},
    {.pattern = "SOUR#:TRAC:DATA:DATA?", .callback      = RP_GenArbitraryWaveFormQ,},
    {.pattern = "SOUR#:BURS:STAT", .callback            = RP_GenGenerateMode,},
    {.pattern = "SOUR#:BURS:STAT?", .callback           = RP_GenGenerateModeQ,},
    {.pattern = "SOUR#:BURS:NCYC", .callback            = RP_GenBurstCount,},
    {.pattern = "SOUR#:BURS:NCYC?", .callback           = RP_GenBurstCountQ,},
    {.pattern = "SOUR#:BURS:NOR", .callback             = RP_GenBurstRepetitions,},
    {.pattern = "SOUR#:BURS:NOR?", .callback            = RP_GenBurstRepetitionsQ,},
    {.pattern = "SOUR#:BURS:INT:PER", .callback         = RP_GenBurstPeriod,},
    {.pattern = "SOUR#:BURS:INT:PER?", .callback        = RP_GenBurstPeriodQ,},

    {.pattern = "SOUR#:BURS:LASTValue", .callback       = RP_GenBurstLastValue,},
    {.pattern = "SOUR#:BURS:LASTValue?", .callback      = RP_GenBurstLastValueQ,},

    {.pattern = "SOUR#:INITValue", .callback            = RP_GenInitValue,},
    {.pattern = "SOUR#:INITValue?", .callback           = RP_GenInitValueQ,},

    {.pattern = "SOUR#:TRig:SOUR", .callback            = RP_GenTriggerSource,},
    {.pattern = "SOUR#:TRig:SOUR?", .callback           = RP_GenTriggerSourceQ,},
    {.pattern = "SOUR#:TRig:INT", .callback             = RP_GenTrigger,},

    {.pattern = "SOUR:TRig:EXT:DEBouncer[:US]", .callback  = RP_GenExtTriggerDebouncerUs,},
    {.pattern = "SOUR:TRig:EXT:DEBouncer[:US]?", .callback = RP_GenExtTriggerDebouncerUsQ,},

    /* uart */
    {.pattern = "UART:INIT", .callback                  = RP_Uart_Init,},
    {.pattern = "UART:RELEASE", .callback               = RP_Uart_Release,},
    {.pattern = "UART:SETUP", .callback                 = RP_Uart_SetSettings,},
    {.pattern = "UART:BITS", .callback                  = RP_Uart_BIT_Size,},
    {.pattern = "UART:BITS?", .callback                 = RP_Uart_BIT_SizeQ,},
    {.pattern = "UART:SPEED", .callback                 = RP_Uart_Speed,},
    {.pattern = "UART:SPEED?", .callback                = RP_Uart_SpeedQ,},
    {.pattern = "UART:STOPB", .callback                 = RP_Uart_STOP_Bit,},
    {.pattern = "UART:STOPB?", .callback                = RP_Uart_STOP_BitQ,},
    {.pattern = "UART:PARITY", .callback                = RP_Uart_PARITY,},
    {.pattern = "UART:PARITY?", .callback               = RP_Uart_PARITYQ,},
    {.pattern = "UART:TIMEOUT", .callback               = RP_Uart_Timeout,},
    {.pattern = "UART:TIMEOUT?", .callback              = RP_Uart_TimeoutQ,},
    {.pattern = "UART:WRITE#", .callback                = RP_Uart_SendBuffer,},
    {.pattern = "UART:READ#?", .callback                = RP_Uart_ReadBufferQ,},

    /* led */
    {.pattern = "LED:MMC", .callback                    = RP_LED_MMC,},
    {.pattern = "LED:MMC?", .callback                   = RP_LED_MMCQ,},
    {.pattern = "LED:HB", .callback                     = RP_LED_HB,},
    {.pattern = "LED:HB?", .callback                    = RP_LED_HBQ,},
    {.pattern = "LED:ETH", .callback                    = RP_LED_ETH,},
    {.pattern = "LED:ETH?", .callback                   = RP_LED_ETHQ,},

    /* spi */
    {.pattern = "SPI:INIT", .callback                   = RP_SPI_Init,},
    {.pattern = "SPI:INIT:DEV", .callback               = RP_SPI_InitDev,},
    {.pattern = "SPI:RELEASE", .callback                = RP_SPI_Release,},
    {.pattern = "SPI:SETtings:DEF", .callback           = RP_SPI_SetDefault,},
    {.pattern = "SPI:SETtings:SET", .callback           = RP_SPI_SetSettings,},
    {.pattern = "SPI:SETtings:GET", .callback           = RP_SPI_GetSettings,},

    {.pattern = "SPI:SETtings:MODE", .callback          = RP_SPI_SetMode,},
    {.pattern = "SPI:SETtings:MODE?", .callback         = RP_SPI_GetModeQ,},
    {.pattern = "SPI:SETtings:CSMODE", .callback        = RP_SPI_SetCSMode,},
    {.pattern = "SPI:SETtings:CSMODE?", .callback       = RP_SPI_GetCSModeQ,},
    {.pattern = "SPI:SETtings:SPEED", .callback         = RP_SPI_SetSpeed,},
    {.pattern = "SPI:SETtings:SPEED?", .callback        = RP_SPI_GetSpeedQ,},

    {.pattern = "SPI:SETtings:WORD", .callback          = RP_SPI_SetWord,},
    {.pattern = "SPI:SETtings:WORD?", .callback         = RP_SPI_GetWordQ,},


    {.pattern = "SPI:MSG:CREATE", .callback             = RP_SPI_CreateMessage,},
    {.pattern = "SPI:MSG:DEL", .callback                = RP_SPI_DestroyMessage,},
    {.pattern = "SPI:MSG:SIZE?", .callback              = RP_SPI_GetMessageLenQ,},

    {.pattern = "SPI:MSG#:TX#", .callback               = RP_SPI_SetTX,},
    {.pattern = "SPI:MSG#:TX#:RX", .callback            = RP_SPI_SetTXRX,},
    {.pattern = "SPI:MSG#:RX#", .callback               = RP_SPI_SetRX,},
    {.pattern = "SPI:MSG#:TX#:CS", .callback            = RP_SPI_SetTXCS,},
    {.pattern = "SPI:MSG#:TX#:RX:CS", .callback         = RP_SPI_SetTXRXCS,},
    {.pattern = "SPI:MSG#:RX#:CS", .callback            = RP_SPI_SetRXCS,},
    {.pattern = "SPI:MSG#:RX?", .callback               = RP_SPI_GetRXBufferQ,},
    {.pattern = "SPI:MSG#:TX?", .callback               = RP_SPI_GetTXBufferQ,},
    {.pattern = "SPI:MSG#:CS?", .callback               = RP_SPI_GetCSChangeStateQ,},

    {.pattern = "SPI:PASS", .callback                   = RP_SPI_Pass,},

     /* i2c */
    {.pattern = "I2C:DEV#", .callback                  = RP_I2C_Dev,},
    {.pattern = "I2C:DEV?", .callback                  = RP_I2C_DevQ,},
    {.pattern = "I2C:FMODE", .callback                 = RP_I2C_ForceMode,},
    {.pattern = "I2C:FMODE?", .callback                = RP_I2C_ForceModeQ,},

    {.pattern = "I2C:Smbus:Read#?", .callback          = RP_I2C_SMBUS_ReadQ,},
    {.pattern = "I2C:Smbus:Read#:Word?", .callback     = RP_I2C_SMBUS_ReadWordQ,},
    {.pattern = "I2C:Smbus:Read#:Buffer#?", .callback  = RP_I2C_SMBUS_ReadBufferQ,},
    {.pattern = "I2C:IOctl:Read:Buffer#?", .callback   = RP_I2C_IOCTL_ReadBufferQ,},

    {.pattern = "I2C:Smbus:Write#", .callback          = RP_I2C_SMBUS_Write,},
    {.pattern = "I2C:Smbus:Write#:Word", .callback     = RP_I2C_SMBUS_WriteWord,},
    {.pattern = "I2C:Smbus:Write#:Buffer#", .callback  = RP_I2C_SMBUS_WriteBuffer,},
    {.pattern = "I2C:IOctl:Write:Buffer#", .callback   = RP_I2C_IOCTL_WriteBuffer,},

    /* can */
    {.pattern = "CAN:FPGA", .callback                   = RP_CAN_FpgaEnable,},
    {.pattern = "CAN:FPGA?", .callback                  = RP_CAN_FpgaEnableQ,},
    {.pattern = "CAN#:START", .callback                 = RP_CAN_Start,},
    {.pattern = "CAN#:STOP", .callback                  = RP_CAN_Stop,},
    {.pattern = "CAN#:RESTART", .callback               = RP_CAN_Restart,},
    {.pattern = "CAN#:STATE?", .callback                = RP_CAN_StateQ,},
    {.pattern = "CAN#:BITRate", .callback               = RP_CAN_Bitrate,},
    {.pattern = "CAN#:BITRate:SP", .callback            = RP_CAN_BitrateSamplePoint,},
    {.pattern = "CAN#:BITRate:SP?", .callback           = RP_CAN_BitrateSamplePointQ,},
    {.pattern = "CAN#:BITTiming", .callback             = RP_CAN_BitTiming,},
    {.pattern = "CAN#:BITTiming?", .callback            = RP_CAN_BitTimingQ,},
    {.pattern = "CAN#:BITTiming:Limits?", .callback     = RP_CAN_BitTimingLimitsQ,},
    {.pattern = "CAN#:CLOCK?", .callback                = RP_CAN_ClockFreqQ,},
    {.pattern = "CAN#:BUS:ERROR?", .callback            = RP_CAN_BusErrorCountersQ,},
    {.pattern = "CAN#:Restart:Time", .callback          = RP_CAN_RestartTime,},
    {.pattern = "CAN#:Restart:Time?", .callback         = RP_CAN_RestartTimeQ,},
    {.pattern = "CAN#:MODE", .callback                  = RP_CAN_ControllerMode,},
    {.pattern = "CAN#:MODE?", .callback                 = RP_CAN_ControllerModeQ,},
    {.pattern = "CAN#:OPEN", .callback                  = RP_CAN_Open,},
    {.pattern = "CAN#:CLOSE", .callback                 = RP_CAN_Close,},
    {.pattern = "CAN#:Send#", .callback                 = RP_CAN_Send,},
    {.pattern = "CAN#:Send#:Timeout#", .callback        = RP_CAN_Send,},
    {.pattern = "CAN#:Send#:Ext", .callback             = RP_CAN_Send,},
    {.pattern = "CAN#:Send#:Timeout#:Ext", .callback    = RP_CAN_Send,},
    {.pattern = "CAN#:Send#:RTR", .callback             = RP_CAN_Send,},
    {.pattern = "CAN#:Send#:Timeout#:RTR", .callback    = RP_CAN_Send,},
    {.pattern = "CAN#:Send#:Ext:RTR", .callback         = RP_CAN_Send,},
    {.pattern = "CAN#:Send#:Timeout#:Ext:RTR", .callback= RP_CAN_Send,},
    {.pattern = "CAN#:Read?", .callback                 = RP_CAN_ReadQ,},
    {.pattern = "CAN#:Read:Timeout#?", .callback        = RP_CAN_ReadQ,},
    {.pattern = "CAN#:Filter:Add", .callback            = RP_CAN_AddFilter,},
    {.pattern = "CAN#:Filter:Remove", .callback         = RP_CAN_RemoveFilter,},
    {.pattern = "CAN#:Filter:Clear", .callback          = RP_CAN_ClearFilter,},
    {.pattern = "CAN#:Filter:Set", .callback            = RP_CAN_SetFilter,},
    {.pattern = "CAN#:SHOW:ERROR", .callback            = RP_CAN_ShowErrorFrames,},

    SCPI_CMD_LIST_END
};

static scpi_interface_t scpi_interface = {
    .error   = SCPI_Error,
    .write   = SCPI_Write,
    .control = SCPI_Control,
    .flush   = SCPI_Flush,
    .reset   = SCPI_Reset,
};

#define SCPI_INPUT_BUFFER_LENGTH 538688
static char scpi_input_buffer[SCPI_INPUT_BUFFER_LENGTH];



scpi_t scpi_context = {
    .cmdlist = scpi_commands,
    .buffer = {
        .length = SCPI_INPUT_BUFFER_LENGTH,
        .data = scpi_input_buffer,
    },
    .interface = &scpi_interface,
    .units = scpi_units_def,
    .idn = {"REDPITAYA", "INSTR2023", NULL, "05-03"},
};
