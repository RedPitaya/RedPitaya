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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

#include "acquire.h"
#include "acquire_axi.h"
#include "api_cmd.h"
#include "apin.h"
#include "can.h"
#include "common.h"
#include "dpin.h"
#include "error.h"
#include "generate.h"
#include "generate_axi.h"
#include "i2c.h"
#include "lcr.h"
#include "led.h"
#include "spi.h"
#include "sweep.h"
#include "uart.h"

#include "scpi/error.h"
#include "scpi/ieee488.h"
#include "scpi/minimal.h"
#include "scpi/parser.h"
#include "scpi/units.h"

#include "scpi-commands.h"
#include "scpi-parser-ext.h"

#if USE_COMMAND_TAGS
#define SCPI_CMD(p, c) \
    { .pattern = p, .callback = c, .tag = 0 }
#else
#define SCPI_CMD(p, c) \
    { .pattern = p, .callback = c }
#endif

scpi_result_t RP_EcosystemVersionQ(scpi_t* context) {
    SCPI_ResultMnemonic(context, rp_GetVersion());
    return SCPI_RES_OK;
}

scpi_result_t RP_CommandsList(scpi_t* context) {
    std::string result;

    for (int i = 0; context->cmdlist[i].pattern != NULL; i++) {
        if (i != 0) {
            result += "\n";
        }
        result += std::string(context->cmdlist[i].pattern);
    }

    SCPI_ResultMnemonic(context, result.c_str());
    return SCPI_RES_OK;
}

/**
 * SCPI Configuration
 */

static const scpi_command_t scpi_commands[] = {
    /* IEEE Mandated Commands (SCPI std V1999.0 4.1.1) */
    SCPI_CMD("*CLS", SCPI_CoreClsEx),
    SCPI_CMD("*ESE", SCPI_CoreEse),
    SCPI_CMD("*ESE?", SCPI_CoreEseQ),
    SCPI_CMD("*ESR?", SCPI_CoreEsrQ),
    SCPI_CMD("*IDN?", SCPI_CoreIdnQ),
    SCPI_CMD("*OPC", SCPI_CoreOpc),
    SCPI_CMD("*OPC?", SCPI_CoreOpcQ),
    SCPI_CMD("*RST", SCPI_CoreRst),
    SCPI_CMD("*SRE", SCPI_CoreSre),
    SCPI_CMD("*SRE?", SCPI_CoreSreQ),
    SCPI_CMD("*STB?", SCPI_CoreStbQ),
    SCPI_CMD("*TST?", SCPI_CoreTstQ),
    SCPI_CMD("*WAI", SCPI_CoreWai),

    /* Required SCPI commands (SCPI std V1999.0 4.2.1) */
    SCPI_CMD("SYSTem:ERRor[:NEXT]?", SCPI_SystemErrorNextQEx),
    SCPI_CMD("SYSTem:ERRor:COUNt?", SCPI_SystemErrorCountQEx),
    SCPI_CMD("SYSTem:VERSion?", RP_EcosystemVersionQ),
    SCPI_CMD("SYSTem:BRD:ID?", RP_BoardID),
    SCPI_CMD("SYSTem:BRD:Name?", RP_BoardName),
    SCPI_CMD("SYSTem:Help?", RP_CommandsList),

    SCPI_CMD("STATus:QUEStionable[:EVENt]?", SCPI_StatusQuestionableEventQ),
    SCPI_CMD("STATus:QUEStionable:ENABle", SCPI_StatusQuestionableEnable),
    SCPI_CMD("STATus:QUEStionable:ENABle?", SCPI_StatusQuestionableEnableQ),
    SCPI_CMD("STATus:PRESet", SCPI_StatusPreset),

    SCPI_CMD("SYSTem:COMMunication:TCPIP:CONTROL?", SCPI_SystemCommTcpipControlQ),

    /* RedPitaya */
    SCPI_CMD("SYSTem:TIME", RP_Time),
    SCPI_CMD("SYSTem:TIME?", RP_TimeQ),
    SCPI_CMD("SYSTem:DATE", RP_Date),
    SCPI_CMD("SYSTem:DATE?", RP_DateQ),

    /* General commands */
    SCPI_CMD("RP:INit", RP_InitAll),
    SCPI_CMD("RP:REset", RP_ResetAll),
    SCPI_CMD("RP:RELease", RP_ReleaseAll),
    SCPI_CMD("RP:DIg[:loop]", RP_EnableDigLoop),
    SCPI_CMD("RP:LOGmode", RP_SetLogMode),
    SCPI_CMD("RP:RET_ON_ERROR", RP_SetRetOnError),

    SCPI_CMD("RP:PLL:ENable", RP_PLL),
    SCPI_CMD("RP:PLL:ENable?", RP_PLLQ),
    SCPI_CMD("RP:PLL:STATE?", RP_PLLStateQ),

    SCPI_CMD("DIG:RST", RP_DigitalPinReset),
    SCPI_CMD("DIG:PIN", RP_DigitalPinState),
    SCPI_CMD("DIG:PIN?", RP_DigitalPinStateQ),
    SCPI_CMD("DIG:PIN:DIR", RP_DigitalPinDirection),
    SCPI_CMD("DIG:PIN:DIR?", RP_DigitalPinDirectionQ),

    SCPI_CMD("ANALOG:RST", RP_AnalogPinReset),
    SCPI_CMD("ANALOG:PIN", RP_AnalogPinValue),
    SCPI_CMD("ANALOG:PIN?", RP_AnalogPinValueQ),

    SCPI_CMD("DAISY:SYNC:TRig", RP_EnableDaisyChainTrigSync),
    SCPI_CMD("DAISY:SYNC:TRig?", RP_EnableDaisyChainTrigSyncQ),
    SCPI_CMD("DAISY:SYNC:CLK", RP_EnableDaisyChainClockSync),
    SCPI_CMD("DAISY:SYNC:CLK?", RP_EnableDaisyChainClockSyncQ),
    SCPI_CMD("DAISY:TRig:Out:ENable", RP_DpinEnableTrigOutput),
    SCPI_CMD("DAISY:TRig:Out:ENable?", RP_DpinEnableTrigOutputQ),
    SCPI_CMD("DAISY:TRig:Out:SOUR", RP_SourceTrigOutput),
    SCPI_CMD("DAISY:TRig:Out:SOUR?", RP_SourceTrigOutputQ),

    /* Acquire */
    SCPI_CMD("ACQ:START", RP_AcqStart),
    SCPI_CMD("ACQ:START:CH#", RP_AcqStartCh),
    SCPI_CMD("ACQ:STOP", RP_AcqStop),
    SCPI_CMD("ACQ:STOP:CH#", rp_AcqStopCh),
    SCPI_CMD("ACQ:RST", RP_AcqReset),
    SCPI_CMD("ACQ:RST:CH#", RP_AcqResetCh),
    SCPI_CMD("ACQ:SPLIT:TRig", RP_AcqSplitTrigger),
    SCPI_CMD("ACQ:SPLIT:TRig?", RP_AcqSplitTriggerQ),

    SCPI_CMD("ACQ:DEC", RP_AcqDecimation),
    SCPI_CMD("ACQ:DEC:CH#", RP_AcqDecimationCh),
    SCPI_CMD("ACQ:DEC?", RP_AcqDecimationQ),
    SCPI_CMD("ACQ:DEC:CH#?", RP_AcqDecimationChQ),
    SCPI_CMD("ACQ:DEC:Factor", RP_AcqDecimationFactor),
    SCPI_CMD("ACQ:DEC:Factor:CH#", RP_AcqDecimationFactorCh),
    SCPI_CMD("ACQ:DEC:Factor?", RP_AcqDecimationFactorQ),
    SCPI_CMD("ACQ:DEC:Factor:CH#?", RP_AcqDecimationFactorChQ),

    SCPI_CMD("ACQ:SRATe?", RP_AcqSamplingRateHzQ),
    SCPI_CMD("ACQ:SRATe:CH#?", RP_AcqSamplingRateHzChQ),
    SCPI_CMD("ACQ:AVG", RP_AcqAveraging),
    SCPI_CMD("ACQ:AVG?", RP_AcqAveragingQ),
    SCPI_CMD("ACQ:AVG:CH#", RP_AcqAveragingCh),
    SCPI_CMD("ACQ:AVG:CH#?", RP_AcqAveragingChQ),
    SCPI_CMD("ACQ:FILTER:BYPASS:CH#", RP_AcqBypassFilterCh),
    SCPI_CMD("ACQ:FILTER:BYPASS:CH#?", RP_AcqBypassFilterChQ),
    SCPI_CMD("ACQ:TRig", RP_AcqTriggerSrc),
    SCPI_CMD("ACQ:TRig:CH#", RP_AcqTriggerSrcCh),
    SCPI_CMD("ACQ:TRig:STAT?", RP_AcqTriggerStateQ),
    SCPI_CMD("ACQ:TRig:STAT:CH#?", RP_AcqTriggerStateChQ),
    SCPI_CMD("ACQ:TRig:DLY", RP_AcqTriggerDelay),
    SCPI_CMD("ACQ:TRig:DLY?", RP_AcqTriggerDelayQ),
    SCPI_CMD("ACQ:TRig:DLY:NS", RP_AcqTriggerDelayNs),
    SCPI_CMD("ACQ:TRig:DLY:NS?", RP_AcqTriggerDelayNsQ),
    SCPI_CMD("ACQ:TRig:DLY:CH#", RP_AcqTriggerDelayCh),
    SCPI_CMD("ACQ:TRig:DLY:CH#?", RP_AcqTriggerDelayChQ),
    SCPI_CMD("ACQ:TRig:DLY:NS:CH#", RP_AcqTriggerDelayNsCh),
    SCPI_CMD("ACQ:TRig:DLY:NS:CH#?", RP_AcqTriggerDelayNsChQ),
    SCPI_CMD("ACQ:TRig:HYST", RP_AcqTriggerHyst),
    SCPI_CMD("ACQ:TRig:HYST?", RP_AcqTriggerHystQ),
    SCPI_CMD("ACQ:TRig:FILL?", RP_AcqTriggerFillQ),
    SCPI_CMD("ACQ:TRig:FILL:CH#?", RP_AcqTriggerFillChQ),
    SCPI_CMD("ACQ:SOUR#:GAIN", RP_AcqGain),
    SCPI_CMD("ACQ:SOUR#:GAIN?", RP_AcqGainQ),
    SCPI_CMD("ACQ:TRig:LEV", RP_AcqTriggerLevel),
    SCPI_CMD("ACQ:TRig:LEV?", RP_AcqTriggerLevelQ),
    SCPI_CMD("ACQ:TRig:LEV:CH#", RP_AcqTriggerLevelCh),
    SCPI_CMD("ACQ:TRig:LEV:CH#?", RP_AcqTriggerLevelChQ),
    SCPI_CMD("ACQ:WPOS?", RP_AcqWritePointerQ),
    SCPI_CMD("ACQ:TPOS?", RP_AcqWritePointerAtTrigQ),
    SCPI_CMD("ACQ:WPOS:CH#?", RP_AcqWritePointerChQ),
    SCPI_CMD("ACQ:TPOS:CH#?", RP_AcqWritePointerAtTrigChQ),
    SCPI_CMD("ACQ:DATA:Units", RP_AcqScpiDataUnits),
    SCPI_CMD("ACQ:DATA:Units?", RP_AcqScpiDataUnitsQ),
    SCPI_CMD("ACQ:DATA:FORMAT", RP_AcqDataFormat),
    SCPI_CMD("ACQ:DATA:FORMAT?", RP_AcqDataFormatQ),
    SCPI_CMD("ACQ:DATA:BYTE:ORDER", RP_AcqDataEndian),
    SCPI_CMD("ACQ:DATA:BYTE:ORDER?", RP_AcqDataEndianQ),
    SCPI_CMD("ACQ:SOUR#:DATA:STArt:End?", RP_AcqDataPosQ),
    SCPI_CMD("ACQ:SOUR#:DATA:Start:End?", RP_AcqDataPosQ),
    SCPI_CMD("ACQ:SOUR#:DATA:STArt:N?", RP_AcqDataQ),
    SCPI_CMD("ACQ:SOUR#:DATA:Start:N?", RP_AcqDataQ),
    SCPI_CMD("ACQ:SOUR#:DATA:Old:N?", RP_AcqOldestDataQ),
    SCPI_CMD("ACQ:SOUR#:DATA?", RP_AcqDataOldestAllQ),
    SCPI_CMD("ACQ:SOUR#:DATA:LATest:N?", RP_AcqLatestDataQ),
    SCPI_CMD("ACQ:SOUR#:DATA:TRig?", RP_AcqTriggerDataQ),
    SCPI_CMD("ACQ:BUF:SIZE?", RP_AcqBufferSizeQ),

    // DMA mode for ACQ
    SCPI_CMD("ACQ:AXI:DATA:Units", RP_AcqAxiScpiDataUnits),
    SCPI_CMD("ACQ:AXI:DATA:Units?", RP_AcqAxiScpiDataUnitsQ),
    SCPI_CMD("ACQ:AXI:DEC", RP_AcqAxiDecimation),
    SCPI_CMD("ACQ:AXI:DEC?", RP_AcqAxiDecimationQ),
    SCPI_CMD("ACQ:AXI:DEC:CH#", RP_AcqAxiDecimationCh),
    SCPI_CMD("ACQ:AXI:DEC:CH#?", RP_AcqAxiDecimationChQ),
    SCPI_CMD("ACQ:AXI:START?", RP_AcqAxiStartQ),
    SCPI_CMD("ACQ:AXI:SIZE?", RP_AcqAxiEndQ),
    SCPI_CMD("ACQ:AXI:SOUR#:Trig:Fill?", RP_AcqAxiTriggerFillQ),
    SCPI_CMD("ACQ:AXI:SOUR#:Trig:Dly", RP_AcqAxiTriggerDelay),
    SCPI_CMD("ACQ:AXI:SOUR#:Trig:Dly?", RP_AcqAxiTriggerDelayQ),
    SCPI_CMD("ACQ:AXI:SOUR#:Write:Pos?", RP_AcqAxiWritePointerQ),
    SCPI_CMD("ACQ:AXI:SOUR#:Trig:Pos?", RP_AcqAxiWritePointerAtTrigQ),
    SCPI_CMD("ACQ:AXI:SOUR#:ENable", RP_AcqAxiEnable),
    SCPI_CMD("ACQ:AXI:SOUR#:DATA:Start:N?", RP_AcqAxiDataQ),
    SCPI_CMD("ACQ:AXI:SOUR#:DATA:STArt:N?", RP_AcqAxiDataQ),
    SCPI_CMD("ACQ:AXI:SOUR#:SET:Buffer", RP_AcqAxiSetAddres),

    SCPI_CMD("ACQ:SOUR#:COUP", RP_AcqAC_DC),
    SCPI_CMD("ACQ:SOUR#:COUP?", RP_AcqAC_DCQ),

    SCPI_CMD("TRig:EXT:LEV", RP_ExtTriggerLevel),
    SCPI_CMD("TRig:EXT:LEV?", RP_ExtTriggerLevelQ),
    SCPI_CMD("ACQ:TRig:EXT:DEBouncer[:US]", RP_ExtTriggerDebouncerUs),
    SCPI_CMD("ACQ:TRig:EXT:DEBouncer[:US]?", RP_ExtTriggerDebouncerUsQ),

    /* Generate */
    SCPI_CMD("GEN:RST", RP_GenReset),
    SCPI_CMD("PHAS:ALIGN", RP_GenSync),
    SCPI_CMD("OUTPUT:STATE", RP_GenSyncState),
    SCPI_CMD("OUTPUT#:STATE", RP_GenState),
    SCPI_CMD("OUTPUT#:STATE?", RP_GenStateQ),
    SCPI_CMD("SOUR:TRig:INT", RP_GenTriggerBoth),
    SCPI_CMD("SOUR:TRig:INT:ONLY", RP_GenTriggerOnlyBoth),
    SCPI_CMD("SOUR#:FREQ:FIX", RP_GenFrequency),
    SCPI_CMD("SOUR#:FREQ:FIX?", RP_GenFrequencyQ),
    SCPI_CMD("SOUR#:FREQ:FIX:Direct", RP_GenFrequencyDirect),
    SCPI_CMD("SOUR#:FUNC", RP_GenWaveForm),
    SCPI_CMD("SOUR#:FUNC?", RP_GenWaveFormQ),
    SCPI_CMD("SOUR#:VOLT", RP_GenAmplitude),
    SCPI_CMD("SOUR#:VOLT?", RP_GenAmplitudeQ),
    SCPI_CMD("SOUR#:VOLT:OFFS", RP_GenOffset),
    SCPI_CMD("SOUR#:VOLT:OFFS?", RP_GenOffsetQ),
    SCPI_CMD("SOUR#:PHAS", RP_GenPhase),
    SCPI_CMD("SOUR#:PHAS?", RP_GenPhaseQ),
    SCPI_CMD("SOUR#:DCYC", RP_GenDutyCycle),
    SCPI_CMD("SOUR#:DCYC?", RP_GenDutyCycleQ),
    SCPI_CMD("SOUR#:RISE:TIME", RP_GenRiseTime),
    SCPI_CMD("SOUR#:RISE:TIME?", RP_GenRiseTimeQ),
    SCPI_CMD("SOUR#:FALL:TIME", RP_GenFallTime),
    SCPI_CMD("SOUR#:FALL:TIME?", RP_GenFallTimeQ),
    SCPI_CMD("SOUR#:LOAD", RP_GenLoad),
    SCPI_CMD("SOUR#:LOAD?", RP_GenLoadQ),
    SCPI_CMD("SOUR#:TRAC:DATA:DATA", RP_GenArbitraryWaveForm),
    SCPI_CMD("SOUR#:TRAC:DATA:DATA?", RP_GenArbitraryWaveFormQ),
    SCPI_CMD("SOUR#:BURS:STAT", RP_GenGenerateMode),
    SCPI_CMD("SOUR#:BURS:STAT?", RP_GenGenerateModeQ),
    SCPI_CMD("SOUR#:BURS:NCYC", RP_GenBurstCount),
    SCPI_CMD("SOUR#:BURS:NCYC?", RP_GenBurstCountQ),
    SCPI_CMD("SOUR#:BURS:NOR", RP_GenBurstRepetitions),
    SCPI_CMD("SOUR#:BURS:NOR?", RP_GenBurstRepetitionsQ),
    SCPI_CMD("SOUR#:BURS:INT:PER", RP_GenBurstPeriod),
    SCPI_CMD("SOUR#:BURS:INT:PER?", RP_GenBurstPeriodQ),

    /* DMA mode for Generate */
    SCPI_CMD("GEN:AXI:START?", RP_GenAxiStartQ),
    SCPI_CMD("GEN:AXI:SIZE?", RP_GenAxiEndQ),

    SCPI_CMD("SOUR#:AXI:RESERVE", RP_GenAxiReserveMemory),
    SCPI_CMD("SOUR#:AXI:RELEASE", RP_GenAxiReleaseMemory),

    SCPI_CMD("SOUR#:AXI:ENable", RP_GenAxiSetEnable),
    SCPI_CMD("SOUR#:AXI:ENable?", RP_GenAxiGetEnable),
    SCPI_CMD("SOUR#:AXI:DEC", RP_GenAxiSetDecimationFactor),
    SCPI_CMD("SOUR#:AXI:DEC?", RP_GenAxiGetDecimationFactor),
    SCPI_CMD("SOUR#:AXI:SET:CALIB", RP_GenSetAmplitudeAndOffsetOrigin),
    SCPI_CMD("SOUR#:AXI:OFFSET#:DATA#:", RP_GenAxiWriteWaveform),

    /* Sweep for Generate */
    SCPI_CMD("SOUR:SWeep:PAUSE", RP_GenSweepPause),
    SCPI_CMD("SOUR:SWeep:DEFault", RP_GenSweepDefault),
    SCPI_CMD("SOUR:SWeep:RESET", RP_GenSweepReset),
    SCPI_CMD("SOUR#:SWeep:STATE", RP_GenSweepState),
    SCPI_CMD("SOUR#:SWeep:STATE?", RP_GenSweepStateQ),
    SCPI_CMD("SOUR#:SWeep:FREQ:START", RP_GenSweepFreqStart),
    SCPI_CMD("SOUR#:SWeep:FREQ:START?", RP_GenSweepFreqStartQ),
    SCPI_CMD("SOUR#:SWeep:FREQ:STOP", RP_GenSweepFreqStop),
    SCPI_CMD("SOUR#:SWeep:FREQ:STOP?", RP_GenSweepFreqStopQ),
    SCPI_CMD("SOUR#:SWeep:TIME", RP_GenSweepTime),
    SCPI_CMD("SOUR#:SWeep:TIME?", RP_GenSweepTimeQ),
    SCPI_CMD("SOUR#:SWeep:MODE", RP_GenSweepMode),
    SCPI_CMD("SOUR#:SWeep:MODE?", RP_GenSweepModeQ),
    SCPI_CMD("SOUR#:SWeep:REP:INF", RP_GenSweepRepInf),
    SCPI_CMD("SOUR#:SWeep:REP:INF?", RP_GenSweepRepInfQ),
    SCPI_CMD("SOUR#:SWeep:REP:COUNT", RP_GenSweepRepCount),
    SCPI_CMD("SOUR#:SWeep:REP:COUNT?", RP_GenSweepRepCountQ),
    SCPI_CMD("SOUR#:SWeep:DIR", RP_GenSweepDir),
    SCPI_CMD("SOUR#:SWeep:DIR?", RP_GenSweepDirQ),

    SCPI_CMD("SOUR#:BURS:INITValue", RP_GenInitValue),
    SCPI_CMD("SOUR#:BURS:INITValue?", RP_GenInitValueQ),
    SCPI_CMD("SOUR#:BURS:LASTValue", RP_GenBurstLastValue),
    SCPI_CMD("SOUR#:BURS:LASTValue?", RP_GenBurstLastValueQ),
    SCPI_CMD("SOUR#:INITValue", RP_GenInitValue),
    SCPI_CMD("SOUR#:INITValue?", RP_GenInitValueQ),

    SCPI_CMD("SOUR#:TRig:SOUR", RP_GenTriggerSource),
    SCPI_CMD("SOUR#:TRig:SOUR?", RP_GenTriggerSourceQ),
    SCPI_CMD("SOUR#:TRig:INT", RP_GenTrigger),
    SCPI_CMD("SOUR#:TRig:INT:ONLY", RP_GenTriggerOnly),
    SCPI_CMD("SOUR:TRig:EXT:DEBouncer[:US]", RP_GenExtTriggerDebouncerUs),
    SCPI_CMD("SOUR:TRig:EXT:DEBouncer[:US]?", RP_GenExtTriggerDebouncerUsQ),

    /* uart */
    SCPI_CMD("UART:INIT", RP_Uart_Init),
    SCPI_CMD("UART:RELEASE", RP_Uart_Release),
    SCPI_CMD("UART:SETUP", RP_Uart_SetSettings),
    SCPI_CMD("UART:BITS", RP_Uart_BIT_Size),
    SCPI_CMD("UART:BITS?", RP_Uart_BIT_SizeQ),
    SCPI_CMD("UART:SPEED", RP_Uart_Speed),
    SCPI_CMD("UART:SPEED?", RP_Uart_SpeedQ),
    SCPI_CMD("UART:STOPB", RP_Uart_STOP_Bit),
    SCPI_CMD("UART:STOPB?", RP_Uart_STOP_BitQ),
    SCPI_CMD("UART:PARITY", RP_Uart_PARITY),
    SCPI_CMD("UART:PARITY?", RP_Uart_PARITYQ),
    SCPI_CMD("UART:TIMEOUT", RP_Uart_Timeout),
    SCPI_CMD("UART:TIMEOUT?", RP_Uart_TimeoutQ),
    SCPI_CMD("UART:WRITE#", RP_Uart_SendBuffer),
    SCPI_CMD("UART:READ#?", RP_Uart_ReadBufferQ),

    /* led */
    SCPI_CMD("LED:MMC", RP_LED_MMC),
    SCPI_CMD("LED:MMC?", RP_LED_MMCQ),
    SCPI_CMD("LED:HB", RP_LED_HB),
    SCPI_CMD("LED:HB?", RP_LED_HBQ),
    SCPI_CMD("LED:ETH", RP_LED_ETH),
    SCPI_CMD("LED:ETH?", RP_LED_ETHQ),

    /* spi */
    SCPI_CMD("SPI:INIT", RP_SPI_Init),
    SCPI_CMD("SPI:INIT:DEV", RP_SPI_InitDev),
    SCPI_CMD("SPI:RELEASE", RP_SPI_Release),
    SCPI_CMD("SPI:SETtings:DEFault", RP_SPI_SetDefault),
    SCPI_CMD("SPI:SETtings:SET", RP_SPI_SetSettings),
    SCPI_CMD("SPI:SETtings:GET", RP_SPI_GetSettings),

    SCPI_CMD("SPI:SETtings:MODE", RP_SPI_SetMode),
    SCPI_CMD("SPI:SETtings:MODE?", RP_SPI_GetModeQ),
    SCPI_CMD("SPI:SETtings:CSMODE", RP_SPI_SetCSMode),
    SCPI_CMD("SPI:SETtings:CSMODE?", RP_SPI_GetCSModeQ),
    SCPI_CMD("SPI:SETtings:SPEED", RP_SPI_SetSpeed),
    SCPI_CMD("SPI:SETtings:SPEED?", RP_SPI_GetSpeedQ),
    SCPI_CMD("SPI:SETtings:WORD", RP_SPI_SetWord),
    SCPI_CMD("SPI:SETtings:WORD?", RP_SPI_GetWordQ),

    SCPI_CMD("SPI:MSG:CREATE", RP_SPI_CreateMessage),
    SCPI_CMD("SPI:MSG:DEL", RP_SPI_DestroyMessage),
    SCPI_CMD("SPI:MSG:SIZE?", RP_SPI_GetMessageLenQ),

    SCPI_CMD("SPI:MSG#:TX#", RP_SPI_SetTX),
    SCPI_CMD("SPI:MSG#:TX#:RX", RP_SPI_SetTXRX),
    SCPI_CMD("SPI:MSG#:RX#", RP_SPI_SetRX),
    SCPI_CMD("SPI:MSG#:TX#:CS", RP_SPI_SetTXCS),
    SCPI_CMD("SPI:MSG#:TX#:RX:CS", RP_SPI_SetTXRXCS),
    SCPI_CMD("SPI:MSG#:RX#:CS", RP_SPI_SetRXCS),
    SCPI_CMD("SPI:MSG#:RX?", RP_SPI_GetRXBufferQ),
    SCPI_CMD("SPI:MSG#:TX?", RP_SPI_GetTXBufferQ),
    SCPI_CMD("SPI:MSG#:CS?", RP_SPI_GetCSChangeStateQ),

    SCPI_CMD("SPI:PASS", RP_SPI_Pass),

    /* i2c */
    SCPI_CMD("I2C:DEV#", RP_I2C_Dev),
    SCPI_CMD("I2C:DEV?", RP_I2C_DevQ),
    SCPI_CMD("I2C:FMODE", RP_I2C_ForceMode),
    SCPI_CMD("I2C:FMODE?", RP_I2C_ForceModeQ),

    SCPI_CMD("I2C:Smbus:Read#?", RP_I2C_SMBUS_ReadQ),
    SCPI_CMD("I2C:Smbus:Read#:Word?", RP_I2C_SMBUS_ReadWordQ),
    SCPI_CMD("I2C:Smbus:Read#:Buffer#?", RP_I2C_SMBUS_ReadBufferQ),
    SCPI_CMD("I2C:IOctl:Read:Buffer#?", RP_I2C_IOCTL_ReadBufferQ),

    SCPI_CMD("I2C:Smbus:Write#", RP_I2C_SMBUS_Write),
    SCPI_CMD("I2C:Smbus:Write#:Word", RP_I2C_SMBUS_WriteWord),
    SCPI_CMD("I2C:Smbus:Write#:Buffer#", RP_I2C_SMBUS_WriteBuffer),
    SCPI_CMD("I2C:IOctl:Write:Buffer#", RP_I2C_IOCTL_WriteBuffer),

    /* can */
    SCPI_CMD("CAN:FPGA", RP_CAN_FpgaEnable),
    SCPI_CMD("CAN:FPGA?", RP_CAN_FpgaEnableQ),
    SCPI_CMD("CAN#:START", RP_CAN_Start),
    SCPI_CMD("CAN#:STOP", RP_CAN_Stop),
    SCPI_CMD("CAN#:RESTART", RP_CAN_Restart),
    SCPI_CMD("CAN#:STATE?", RP_CAN_StateQ),
    SCPI_CMD("CAN#:BITRate", RP_CAN_Bitrate),
    SCPI_CMD("CAN#:BITRate:SP", RP_CAN_BitrateSamplePoint),
    SCPI_CMD("CAN#:BITRate:SP?", RP_CAN_BitrateSamplePointQ),
    SCPI_CMD("CAN#:BITTiming", RP_CAN_BitTiming),
    SCPI_CMD("CAN#:BITTiming?", RP_CAN_BitTimingQ),
    SCPI_CMD("CAN#:BITTiming:Limits?", RP_CAN_BitTimingLimitsQ),
    SCPI_CMD("CAN#:CLOCK?", RP_CAN_ClockFreqQ),
    SCPI_CMD("CAN#:BUS:ERROR?", RP_CAN_BusErrorCountersQ),
    SCPI_CMD("CAN#:Restart:Time", RP_CAN_RestartTime),
    SCPI_CMD("CAN#:Restart:Time?", RP_CAN_RestartTimeQ),
    SCPI_CMD("CAN#:MODE", RP_CAN_ControllerMode),
    SCPI_CMD("CAN#:MODE?", RP_CAN_ControllerModeQ),
    SCPI_CMD("CAN#:OPEN", RP_CAN_Open),
    SCPI_CMD("CAN#:CLOSE", RP_CAN_Close),
    SCPI_CMD("CAN#:Send#", RP_CAN_Send),
    SCPI_CMD("CAN#:Send#:Timeout#", RP_CAN_Send),
    SCPI_CMD("CAN#:Send#:Ext", RP_CAN_Send),
    SCPI_CMD("CAN#:Send#:Timeout#:Ext", RP_CAN_Send),
    SCPI_CMD("CAN#:Send#:RTR", RP_CAN_Send),
    SCPI_CMD("CAN#:Send#:Timeout#:RTR", RP_CAN_Send),
    SCPI_CMD("CAN#:Send#:Ext:RTR", RP_CAN_Send),
    SCPI_CMD("CAN#:Send#:Timeout#:Ext:RTR", RP_CAN_Send),
    SCPI_CMD("CAN#:Read?", RP_CAN_ReadQ),
    SCPI_CMD("CAN#:Read:Timeout#?", RP_CAN_ReadQ),
    SCPI_CMD("CAN#:Filter:Add", RP_CAN_AddFilter),
    SCPI_CMD("CAN#:Filter:Remove", RP_CAN_RemoveFilter),
    SCPI_CMD("CAN#:Filter:Clear", RP_CAN_ClearFilter),
    SCPI_CMD("CAN#:Filter:Set", RP_CAN_SetFilter),
    SCPI_CMD("CAN#:SHOW:ERROR", RP_CAN_ShowErrorFrames),

    /* LCR */
    SCPI_CMD("LCR:START", RP_LCRStart),
    SCPI_CMD("LCR:START:GEN", RP_LCRStartGen),
    SCPI_CMD("LCR:STOP", RP_LCRStop),
    SCPI_CMD("LCR:RESET", RP_LCRReset),
    SCPI_CMD("LCR:MEASURE?", RP_LCRMeasureQ),
    SCPI_CMD("LCR:FREQ", RP_LCRFrequency),
    SCPI_CMD("LCR:FREQ?", RP_LCRFrequencyQ),
    SCPI_CMD("LCR:VOLT", RP_LCRAmplitude),
    SCPI_CMD("LCR:VOLT?", RP_LCRAmplitudeQ),
    SCPI_CMD("LCR:VOLT:OFFS", RP_LCROffset),
    SCPI_CMD("LCR:VOLT:OFFS?", RP_LCROffsetQ),
    SCPI_CMD("LCR:SHUNT", RP_LCRShunt),
    SCPI_CMD("LCR:SHUNT?", RP_LCRShuntQ),
    SCPI_CMD("LCR:SHUNT:CUSTOM", RP_LCRCustomShunt),
    SCPI_CMD("LCR:SHUNT:CUSTOM?", RP_LCRCustomShuntQ),
    SCPI_CMD("LCR:SHUNT:MODE", RP_LCRShuntMode),
    SCPI_CMD("LCR:SHUNT:MODE?", RP_LCRShuntModeQ),
    SCPI_CMD("LCR:SHUNT:AUTO", RP_LCRShuntAuto),
    SCPI_CMD("LCR:CIRCUIT", RP_LCRMeasSeries),
    SCPI_CMD("LCR:CIRCUIT?", RP_LCRMeasSeriesQ),
    SCPI_CMD("LCR:EXT:MODULE?", RP_LCRCheckExtensionModuleConnectioQ),

    SCPI_CMD_LIST_END};

static scpi_interface_t scpi_interface = {
    .error = SCPI_Error,
    .write = SCPI_Write,
    .control = SCPI_Control,
    .flush = SCPI_Flush,
    .reset = SCPI_Reset,
};

static scpi_interface_t scpi_arduino_interface = {
    .error = SCPI_Error,
    .write = SCPI_WriteUartProtocol,
    .control = SCPI_Control,
    .flush = SCPI_Flush,
    .reset = SCPI_Reset,
};

#define SCPI_INPUT_BUFFER_LENGTH 1024 * 512
// static char scpi_input_buffer[SCPI_INPUT_BUFFER_LENGTH];

scpi_t* initContext(bool arduinoMode) {
    _scpi_t* ctx = NULL;
    char* buffer = NULL;
    try {
        ctx = new _scpi_t();
    } catch (const std::bad_alloc& err) {
        fprintf(stderr, "Failed allocate scpi_t: %s\n", err.what());
        return NULL;
    };

    try {
        buffer = new char[SCPI_INPUT_BUFFER_LENGTH];
    } catch (const std::bad_alloc&) {
        fprintf(stderr, "Failed allocate buffer for scpi_t\n");
        return NULL;
    };
    ctx->cmdlist = scpi_commands;
    ctx->buffer.data = buffer;
    ctx->buffer.length = SCPI_INPUT_BUFFER_LENGTH;
    ctx->interface = arduinoMode ? &scpi_arduino_interface : &scpi_interface;
    ctx->units = scpi_units_def;
    // user_context will be pointer to socket
    ctx->user_context = NULL;
    // ctx->binary_output = false;
    return ctx;
}
