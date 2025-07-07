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
    {.pattern = "*CLS",
     .callback = SCPI_CoreClsEx,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "*ESE",
     .callback = SCPI_CoreEse,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "*ESE?",
     .callback = SCPI_CoreEseQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "*ESR?",
     .callback = SCPI_CoreEsrQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "*IDN?",
     .callback = SCPI_CoreIdnQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "*OPC",
     .callback = SCPI_CoreOpc,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "*OPC?",
     .callback = SCPI_CoreOpcQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "*RST",
     .callback = SCPI_CoreRst,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "*SRE",
     .callback = SCPI_CoreSre,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "*SRE?",
     .callback = SCPI_CoreSreQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "*STB?",
     .callback = SCPI_CoreStbQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "*TST?",
     .callback = SCPI_CoreTstQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "*WAI",
     .callback = SCPI_CoreWai,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },

    /* Required SCPI commands (SCPI std V1999.0 4.2.1) */
    {.pattern = "SYSTem:ERRor[:NEXT]?",
     .callback = SCPI_SystemErrorNextQEx,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SYSTem:ERRor:COUNt?",
     .callback = SCPI_SystemErrorCountQEx,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SYSTem:VERSion?",
     .callback = RP_EcosystemVersionQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SYSTem:BRD:ID?",
     .callback = RP_BoardID,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SYSTem:BRD:Name?",
     .callback = RP_BoardName,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SYSTem:Help?",
     .callback = RP_CommandsList,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },

    {.pattern = "STATus:QUEStionable[:EVENt]?",
     .callback = SCPI_StatusQuestionableEventQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "STATus:QUEStionable:ENABle",
     .callback = SCPI_StatusQuestionableEnable,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "STATus:QUEStionable:ENABle?",
     .callback = SCPI_StatusQuestionableEnableQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "STATus:PRESet",
     .callback = SCPI_StatusPreset,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },

    {.pattern = "SYSTem:COMMunication:TCPIP:CONTROL?",
     .callback = SCPI_SystemCommTcpipControlQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },

    /* RedPitaya */

    {.pattern = "SYSTem:TIME",
     .callback = RP_Time,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SYSTem:TIME?",
     .callback = RP_TimeQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },

    {.pattern = "SYSTem:DATE",
     .callback = RP_Date,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SYSTem:DATE?",
     .callback = RP_DateQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },

    /* General commands */
    {.pattern = "RP:INit",
     .callback = RP_InitAll,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "RP:REset",
     .callback = RP_ResetAll,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "RP:RELease",
     .callback = RP_ReleaseAll,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "RP:DIg[:loop]",
     .callback = RP_EnableDigLoop,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "RP:LOGmode",
     .callback = RP_SetLogMode,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "RP:RET_ON_ERROR",
     .callback = RP_SetRetOnError,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },

    {.pattern = "RP:PLL:ENable",
     .callback = RP_PLL,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "RP:PLL:ENable?",
     .callback = RP_PLLQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "RP:PLL:STATE?",
     .callback = RP_PLLStateQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },

    {.pattern = "DIG:RST",
     .callback = RP_DigitalPinReset,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "DIG:PIN",
     .callback = RP_DigitalPinState,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "DIG:PIN?",
     .callback = RP_DigitalPinStateQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "DIG:PIN:DIR",
     .callback = RP_DigitalPinDirection,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "DIG:PIN:DIR?",
     .callback = RP_DigitalPinDirectionQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },

    {.pattern = "ANALOG:RST",
     .callback = RP_AnalogPinReset,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ANALOG:PIN",
     .callback = RP_AnalogPinValue,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ANALOG:PIN?",
     .callback = RP_AnalogPinValueQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },

    {.pattern = "DAISY:SYNC:TRig",
     .callback = RP_EnableDaisyChainTrigSync,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "DAISY:SYNC:TRig?",
     .callback = RP_EnableDaisyChainTrigSyncQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },

    {.pattern = "DAISY:SYNC:CLK",
     .callback = RP_EnableDaisyChainClockSync,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "DAISY:SYNC:CLK?",
     .callback = RP_EnableDaisyChainClockSyncQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },

    {.pattern = "DAISY:TRIG_O:ENable",
     .callback = RP_DpinEnableTrigOutput,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },  // Depraceted need remove after update all examples
    {.pattern = "DAISY:TRig:Out:ENable",
     .callback = RP_DpinEnableTrigOutput,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "DAISY:TRIG_O:ENable?",
     .callback = RP_DpinEnableTrigOutputQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },  // Depraceted need remove after update all examples
    {.pattern = "DAISY:TRig:Out:ENable?",
     .callback = RP_DpinEnableTrigOutputQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },

    {.pattern = "DAISY:TRIG_O:SOUR",
     .callback = RP_SourceTrigOutput,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },  // Depraceted need remove after update all examples
    {.pattern = "DAISY:TRig:Out:SOUR",
     .callback = RP_SourceTrigOutput,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "DAISY:TRIG_O:SOUR?",
     .callback = RP_SourceTrigOutputQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },  // Depraceted need remove after update all examples
    {.pattern = "DAISY:TRig:Out:SOUR?",
     .callback = RP_SourceTrigOutputQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },

    /* Acquire */
    {.pattern = "ACQ:START",
     .callback = RP_AcqStart,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:START:CH#",
     .callback = RP_AcqStartCh,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:STOP",
     .callback = RP_AcqStop,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:STOP:CH#",
     .callback = rp_AcqStopCh,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:RST",
     .callback = RP_AcqReset,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:RST:CH#",
     .callback = RP_AcqResetCh,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:SPLIT:TRig",
     .callback = RP_AcqSplitTrigger,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:SPLIT:TRig?",
     .callback = RP_AcqSplitTriggerQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },

    {.pattern = "ACQ:DEC",
     .callback = RP_AcqDecimation,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:DEC:CH#",
     .callback = RP_AcqDecimationCh,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:DEC?",
     .callback = RP_AcqDecimationQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:DEC:CH#?",
     .callback = RP_AcqDecimationChQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:DEC:Factor",
     .callback = RP_AcqDecimationFactor,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:DEC:Factor:CH#",
     .callback = RP_AcqDecimationFactorCh,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:DEC:Factor?",
     .callback = RP_AcqDecimationFactorQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:DEC:Factor:CH#?",
     .callback = RP_AcqDecimationFactorChQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },

    {.pattern = "ACQ:SRATe?",
     .callback = RP_AcqSamplingRateHzQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:SRATe:CH#?",
     .callback = RP_AcqSamplingRateHzChQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:AVG",
     .callback = RP_AcqAveraging,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:AVG?",
     .callback = RP_AcqAveragingQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:AVG:CH#",
     .callback = RP_AcqAveragingCh,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:AVG:CH#?",
     .callback = RP_AcqAveragingChQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:FILTER:BYPASS:CH#",
     .callback = RP_AcqBypassFilterCh,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:FILTER:BYPASS:CH#?",
     .callback = RP_AcqBypassFilterChQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:TRig",
     .callback = RP_AcqTriggerSrc,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:TRig:CH#",
     .callback = RP_AcqTriggerSrcCh,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:TRig:STAT?",
     .callback = RP_AcqTriggerStateQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:TRig:STAT:CH#?",
     .callback = RP_AcqTriggerStateChQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:TRig:DLY",
     .callback = RP_AcqTriggerDelay,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:TRig:DLY?",
     .callback = RP_AcqTriggerDelayQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:TRig:DLY:NS",
     .callback = RP_AcqTriggerDelayNs,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:TRig:DLY:NS?",
     .callback = RP_AcqTriggerDelayNsQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:TRig:DLY:CH#",
     .callback = RP_AcqTriggerDelayCh,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:TRig:DLY:CH#?",
     .callback = RP_AcqTriggerDelayChQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:TRig:DLY:NS:CH#",
     .callback = RP_AcqTriggerDelayNsCh,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:TRig:DLY:NS:CH#?",
     .callback = RP_AcqTriggerDelayNsChQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:TRig:HYST",
     .callback = RP_AcqTriggerHyst,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:TRig:HYST?",
     .callback = RP_AcqTriggerHystQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:TRig:FILL?",
     .callback = RP_AcqTriggerFillQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:TRig:FILL:CH#?",
     .callback = RP_AcqTriggerFillChQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:SOUR#:GAIN",
     .callback = RP_AcqGain,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:SOUR#:GAIN?",
     .callback = RP_AcqGainQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:TRig:LEV",
     .callback = RP_AcqTriggerLevel,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:TRig:LEV?",
     .callback = RP_AcqTriggerLevelQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:TRig:LEV:CH#",
     .callback = RP_AcqTriggerLevelCh,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:TRig:LEV:CH#?",
     .callback = RP_AcqTriggerLevelChQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:WPOS?",
     .callback = RP_AcqWritePointerQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:TPOS?",
     .callback = RP_AcqWritePointerAtTrigQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:WPOS:CH#?",
     .callback = RP_AcqWritePointerChQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:TPOS:CH#?",
     .callback = RP_AcqWritePointerAtTrigChQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:DATA:Units",
     .callback = RP_AcqScpiDataUnits,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:DATA:Units?",
     .callback = RP_AcqScpiDataUnitsQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:DATA:FORMAT",
     .callback = RP_AcqDataFormat,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:DATA:FORMAT?",
     .callback = RP_AcqDataFormatQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:SOUR#:DATA:STArt:End?",
     .callback = RP_AcqDataPosQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:SOUR#:DATA:Start:End?",
     .callback = RP_AcqDataPosQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:SOUR#:DATA:STArt:N?",
     .callback = RP_AcqDataQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:SOUR#:DATA:Start:N?",
     .callback = RP_AcqDataQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:SOUR#:DATA:Old:N?",
     .callback = RP_AcqOldestDataQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:SOUR#:DATA?",
     .callback = RP_AcqDataOldestAllQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:SOUR#:DATA:LATest:N?",
     .callback = RP_AcqLatestDataQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:SOUR#:DATA:TRig?",
     .callback = RP_AcqTriggerDataQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:BUF:SIZE?",
     .callback = RP_AcqBufferSizeQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },

    // DMA mode for ACQ
    {.pattern = "ACQ:AXI:DATA:Units",
     .callback = RP_AcqAxiScpiDataUnits,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:AXI:DATA:Units?",
     .callback = RP_AcqAxiScpiDataUnitsQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:AXI:DEC",
     .callback = RP_AcqAxiDecimation,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:AXI:DEC?",
     .callback = RP_AcqAxiDecimationQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:AXI:DEC:CH#",
     .callback = RP_AcqAxiDecimationCh,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:AXI:DEC:CH#?",
     .callback = RP_AcqAxiDecimationChQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:AXI:START?",
     .callback = RP_AcqAxiStartQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:AXI:SIZE?",
     .callback = RP_AcqAxiEndQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:AXI:SOUR#:Trig:Fill?",
     .callback = RP_AcqAxiTriggerFillQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:AXI:SOUR#:Trig:Dly",
     .callback = RP_AcqAxiTriggerDelay,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:AXI:SOUR#:Trig:Dly?",
     .callback = RP_AcqAxiTriggerDelayQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:AXI:SOUR#:Write:Pos?",
     .callback = RP_AcqAxiWritePointerQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:AXI:SOUR#:Trig:Pos?",
     .callback = RP_AcqAxiWritePointerAtTrigQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:AXI:SOUR#:ENable",
     .callback = RP_AcqAxiEnable,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:AXI:SOUR#:DATA:Start:N?",
     .callback = RP_AcqAxiDataQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:AXI:SOUR#:DATA:STArt:N?",
     .callback = RP_AcqAxiDataQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:AXI:SOUR#:SET:Buffer",
     .callback = RP_AcqAxiSetAddres,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },

    {.pattern = "ACQ:SOUR#:COUP",
     .callback = RP_AcqAC_DC,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:SOUR#:COUP?",
     .callback = RP_AcqAC_DCQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },

    {.pattern = "TRig:EXT:LEV",
     .callback = RP_ExtTriggerLevel,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "TRig:EXT:LEV?",
     .callback = RP_ExtTriggerLevelQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },

    {.pattern = "ACQ:TRig:EXT:DEBouncer[:US]",
     .callback = RP_ExtTriggerDebouncerUs,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "ACQ:TRig:EXT:DEBouncer[:US]?",
     .callback = RP_ExtTriggerDebouncerUsQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },

    /* Generate */
    {.pattern = "GEN:RST",
     .callback = RP_GenReset,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "PHAS:ALIGN",
     .callback = RP_GenSync,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "OUTPUT:STATE",
     .callback = RP_GenSyncState,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "OUTPUT#:STATE",
     .callback = RP_GenState,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "OUTPUT#:STATE?",
     .callback = RP_GenStateQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SOUR:TRig:INT",
     .callback = RP_GenTriggerBoth,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SOUR:TRig:INT:ONLY",
     .callback = RP_GenTriggerOnlyBoth,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SOUR#:FREQ:FIX",
     .callback = RP_GenFrequency,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SOUR#:FREQ:FIX?",
     .callback = RP_GenFrequencyQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SOUR#:FREQ:FIX:Direct",
     .callback = RP_GenFrequencyDirect,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SOUR#:FUNC",
     .callback = RP_GenWaveForm,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SOUR#:FUNC?",
     .callback = RP_GenWaveFormQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SOUR#:VOLT",
     .callback = RP_GenAmplitude,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SOUR#:VOLT?",
     .callback = RP_GenAmplitudeQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SOUR#:VOLT:OFFS",
     .callback = RP_GenOffset,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SOUR#:VOLT:OFFS?",
     .callback = RP_GenOffsetQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SOUR#:PHAS",
     .callback = RP_GenPhase,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SOUR#:PHAS?",
     .callback = RP_GenPhaseQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SOUR#:DCYC",
     .callback = RP_GenDutyCycle,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SOUR#:DCYC?",
     .callback = RP_GenDutyCycleQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SOUR#:LOAD",
     .callback = RP_GenLoad,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SOUR#:LOAD?",
     .callback = RP_GenLoadQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SOUR#:TRAC:DATA:DATA",
     .callback = RP_GenArbitraryWaveForm,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SOUR#:TRAC:DATA:DATA?",
     .callback = RP_GenArbitraryWaveFormQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SOUR#:BURS:STAT",
     .callback = RP_GenGenerateMode,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SOUR#:BURS:STAT?",
     .callback = RP_GenGenerateModeQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SOUR#:BURS:NCYC",
     .callback = RP_GenBurstCount,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SOUR#:BURS:NCYC?",
     .callback = RP_GenBurstCountQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SOUR#:BURS:NOR",
     .callback = RP_GenBurstRepetitions,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SOUR#:BURS:NOR?",
     .callback = RP_GenBurstRepetitionsQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SOUR#:BURS:INT:PER",
     .callback = RP_GenBurstPeriod,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SOUR#:BURS:INT:PER?",
     .callback = RP_GenBurstPeriodQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },

    {.pattern = "SOUR:SWeep:PAUSE",
     .callback = RP_GenSweepPause,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SOUR:SWeep:DEFault",
     .callback = RP_GenSweepDefault,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SOUR:SWeep:RESET",
     .callback = RP_GenSweepReset,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SOUR#:SWeep:STATE",
     .callback = RP_GenSweepState,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SOUR#:SWeep:STATE?",
     .callback = RP_GenSweepStateQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SOUR#:SWeep:FREQ:START",
     .callback = RP_GenSweepFreqStart,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SOUR#:SWeep:FREQ:START?",
     .callback = RP_GenSweepFreqStartQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SOUR#:SWeep:FREQ:STOP",
     .callback = RP_GenSweepFreqStop,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SOUR#:SWeep:FREQ:STOP?",
     .callback = RP_GenSweepFreqStopQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SOUR#:SWeep:TIME",
     .callback = RP_GenSweepTime,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SOUR#:SWeep:TIME?",
     .callback = RP_GenSweepTimeQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SOUR#:SWeep:MODE",
     .callback = RP_GenSweepMode,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SOUR#:SWeep:MODE?",
     .callback = RP_GenSweepModeQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SOUR#:SWeep:REP:INF",
     .callback = RP_GenSweepRepInf,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SOUR#:SWeep:REP:INF?",
     .callback = RP_GenSweepRepInfQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SOUR#:SWeep:REP:COUNT",
     .callback = RP_GenSweepRepCount,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SOUR#:SWeep:REP:COUNT?",
     .callback = RP_GenSweepRepCountQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SOUR#:SWeep:DIR",
     .callback = RP_GenSweepDir,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SOUR#:SWeep:DIR?",
     .callback = RP_GenSweepDirQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },

    {.pattern = "SOUR#:BURS:INITValue",
     .callback = RP_GenInitValue,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SOUR#:BURS:INITValue?",
     .callback = RP_GenInitValueQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },

    {.pattern = "SOUR#:BURS:LASTValue",
     .callback = RP_GenBurstLastValue,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SOUR#:BURS:LASTValue?",
     .callback = RP_GenBurstLastValueQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },

    {.pattern = "SOUR#:INITValue",
     .callback = RP_GenInitValue,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SOUR#:INITValue?",
     .callback = RP_GenInitValueQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },

    {.pattern = "SOUR#:TRig:SOUR",
     .callback = RP_GenTriggerSource,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SOUR#:TRig:SOUR?",
     .callback = RP_GenTriggerSourceQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SOUR#:TRig:INT",
     .callback = RP_GenTrigger,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SOUR#:TRig:INT:ONLY",
     .callback = RP_GenTriggerOnly,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SOUR:TRig:EXT:DEBouncer[:US]",
     .callback = RP_GenExtTriggerDebouncerUs,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SOUR:TRig:EXT:DEBouncer[:US]?",
     .callback = RP_GenExtTriggerDebouncerUsQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },

    /* uart */
    {.pattern = "UART:INIT",
     .callback = RP_Uart_Init,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "UART:RELEASE",
     .callback = RP_Uart_Release,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "UART:SETUP",
     .callback = RP_Uart_SetSettings,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "UART:BITS",
     .callback = RP_Uart_BIT_Size,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "UART:BITS?",
     .callback = RP_Uart_BIT_SizeQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "UART:SPEED",
     .callback = RP_Uart_Speed,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "UART:SPEED?",
     .callback = RP_Uart_SpeedQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "UART:STOPB",
     .callback = RP_Uart_STOP_Bit,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "UART:STOPB?",
     .callback = RP_Uart_STOP_BitQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "UART:PARITY",
     .callback = RP_Uart_PARITY,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "UART:PARITY?",
     .callback = RP_Uart_PARITYQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "UART:TIMEOUT",
     .callback = RP_Uart_Timeout,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "UART:TIMEOUT?",
     .callback = RP_Uart_TimeoutQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "UART:WRITE#",
     .callback = RP_Uart_SendBuffer,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "UART:READ#?",
     .callback = RP_Uart_ReadBufferQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },

    /* led */
    {.pattern = "LED:MMC",
     .callback = RP_LED_MMC,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "LED:MMC?",
     .callback = RP_LED_MMCQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "LED:HB",
     .callback = RP_LED_HB,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "LED:HB?",
     .callback = RP_LED_HBQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "LED:ETH",
     .callback = RP_LED_ETH,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "LED:ETH?",
     .callback = RP_LED_ETHQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },

    /* spi */
    {.pattern = "SPI:INIT",
     .callback = RP_SPI_Init,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SPI:INIT:DEV",
     .callback = RP_SPI_InitDev,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SPI:RELEASE",
     .callback = RP_SPI_Release,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SPI:SETtings:DEFault",
     .callback = RP_SPI_SetDefault,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SPI:SETtings:SET",
     .callback = RP_SPI_SetSettings,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SPI:SETtings:GET",
     .callback = RP_SPI_GetSettings,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },

    {.pattern = "SPI:SETtings:MODE",
     .callback = RP_SPI_SetMode,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SPI:SETtings:MODE?",
     .callback = RP_SPI_GetModeQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SPI:SETtings:CSMODE",
     .callback = RP_SPI_SetCSMode,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SPI:SETtings:CSMODE?",
     .callback = RP_SPI_GetCSModeQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SPI:SETtings:SPEED",
     .callback = RP_SPI_SetSpeed,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SPI:SETtings:SPEED?",
     .callback = RP_SPI_GetSpeedQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },

    {.pattern = "SPI:SETtings:WORD",
     .callback = RP_SPI_SetWord,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SPI:SETtings:WORD?",
     .callback = RP_SPI_GetWordQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },

    {.pattern = "SPI:MSG:CREATE",
     .callback = RP_SPI_CreateMessage,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SPI:MSG:DEL",
     .callback = RP_SPI_DestroyMessage,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SPI:MSG:SIZE?",
     .callback = RP_SPI_GetMessageLenQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },

    {.pattern = "SPI:MSG#:TX#",
     .callback = RP_SPI_SetTX,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SPI:MSG#:TX#:RX",
     .callback = RP_SPI_SetTXRX,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SPI:MSG#:RX#",
     .callback = RP_SPI_SetRX,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SPI:MSG#:TX#:CS",
     .callback = RP_SPI_SetTXCS,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SPI:MSG#:TX#:RX:CS",
     .callback = RP_SPI_SetTXRXCS,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SPI:MSG#:RX#:CS",
     .callback = RP_SPI_SetRXCS,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SPI:MSG#:RX?",
     .callback = RP_SPI_GetRXBufferQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SPI:MSG#:TX?",
     .callback = RP_SPI_GetTXBufferQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "SPI:MSG#:CS?",
     .callback = RP_SPI_GetCSChangeStateQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },

    {.pattern = "SPI:PASS",
     .callback = RP_SPI_Pass,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },

    /* i2c */
    {.pattern = "I2C:DEV#",
     .callback = RP_I2C_Dev,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "I2C:DEV?",
     .callback = RP_I2C_DevQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "I2C:FMODE",
     .callback = RP_I2C_ForceMode,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "I2C:FMODE?",
     .callback = RP_I2C_ForceModeQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },

    {.pattern = "I2C:Smbus:Read#?",
     .callback = RP_I2C_SMBUS_ReadQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "I2C:Smbus:Read#:Word?",
     .callback = RP_I2C_SMBUS_ReadWordQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "I2C:Smbus:Read#:Buffer#?",
     .callback = RP_I2C_SMBUS_ReadBufferQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "I2C:IOctl:Read:Buffer#?",
     .callback = RP_I2C_IOCTL_ReadBufferQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },

    {.pattern = "I2C:Smbus:Write#",
     .callback = RP_I2C_SMBUS_Write,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "I2C:Smbus:Write#:Word",
     .callback = RP_I2C_SMBUS_WriteWord,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "I2C:Smbus:Write#:Buffer#",
     .callback = RP_I2C_SMBUS_WriteBuffer,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "I2C:IOctl:Write:Buffer#",
     .callback = RP_I2C_IOCTL_WriteBuffer,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },

    /* can */
    {.pattern = "CAN:FPGA",
     .callback = RP_CAN_FpgaEnable,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "CAN:FPGA?",
     .callback = RP_CAN_FpgaEnableQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "CAN#:START",
     .callback = RP_CAN_Start,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "CAN#:STOP",
     .callback = RP_CAN_Stop,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "CAN#:RESTART",
     .callback = RP_CAN_Restart,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "CAN#:STATE?",
     .callback = RP_CAN_StateQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "CAN#:BITRate",
     .callback = RP_CAN_Bitrate,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "CAN#:BITRate:SP",
     .callback = RP_CAN_BitrateSamplePoint,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "CAN#:BITRate:SP?",
     .callback = RP_CAN_BitrateSamplePointQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "CAN#:BITTiming",
     .callback = RP_CAN_BitTiming,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "CAN#:BITTiming?",
     .callback = RP_CAN_BitTimingQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "CAN#:BITTiming:Limits?",
     .callback = RP_CAN_BitTimingLimitsQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "CAN#:CLOCK?",
     .callback = RP_CAN_ClockFreqQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "CAN#:BUS:ERROR?",
     .callback = RP_CAN_BusErrorCountersQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "CAN#:Restart:Time",
     .callback = RP_CAN_RestartTime,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "CAN#:Restart:Time?",
     .callback = RP_CAN_RestartTimeQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "CAN#:MODE",
     .callback = RP_CAN_ControllerMode,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "CAN#:MODE?",
     .callback = RP_CAN_ControllerModeQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "CAN#:OPEN",
     .callback = RP_CAN_Open,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "CAN#:CLOSE",
     .callback = RP_CAN_Close,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "CAN#:Send#",
     .callback = RP_CAN_Send,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "CAN#:Send#:Timeout#",
     .callback = RP_CAN_Send,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "CAN#:Send#:Ext",
     .callback = RP_CAN_Send,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "CAN#:Send#:Timeout#:Ext",
     .callback = RP_CAN_Send,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "CAN#:Send#:RTR",
     .callback = RP_CAN_Send,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "CAN#:Send#:Timeout#:RTR",
     .callback = RP_CAN_Send,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "CAN#:Send#:Ext:RTR",
     .callback = RP_CAN_Send,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "CAN#:Send#:Timeout#:Ext:RTR",
     .callback = RP_CAN_Send,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "CAN#:Read?",
     .callback = RP_CAN_ReadQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "CAN#:Read:Timeout#?",
     .callback = RP_CAN_ReadQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "CAN#:Filter:Add",
     .callback = RP_CAN_AddFilter,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "CAN#:Filter:Remove",
     .callback = RP_CAN_RemoveFilter,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "CAN#:Filter:Clear",
     .callback = RP_CAN_ClearFilter,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "CAN#:Filter:Set",
     .callback = RP_CAN_SetFilter,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "CAN#:SHOW:ERROR",
     .callback = RP_CAN_ShowErrorFrames,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },

    /* LCR */

    {.pattern = "LCR:START",
     .callback = RP_LCRStart,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "LCR:START:GEN",
     .callback = RP_LCRStartGen,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "LCR:STOP",
     .callback = RP_LCRStop,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "LCR:RESET",
     .callback = RP_LCRReset,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "LCR:MEASURE?",
     .callback = RP_LCRMeasureQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "LCR:FREQ",
     .callback = RP_LCRFrequency,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "LCR:FREQ?",
     .callback = RP_LCRFrequencyQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "LCR:VOLT",
     .callback = RP_LCRAmplitude,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "LCR:VOLT?",
     .callback = RP_LCRAmplitudeQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "LCR:VOLT:OFFS",
     .callback = RP_LCROffset,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "LCR:VOLT:OFFS?",
     .callback = RP_LCROffsetQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "LCR:SHUNT",
     .callback = RP_LCRShunt,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "LCR:SHUNT?",
     .callback = RP_LCRShuntQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "LCR:SHUNT:CUSTOM",
     .callback = RP_LCRCustomShunt,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "LCR:SHUNT:CUSTOM?",
     .callback = RP_LCRCustomShuntQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "LCR:SHUNT:MODE",
     .callback = RP_LCRShuntMode,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "LCR:SHUNT:MODE?",
     .callback = RP_LCRShuntModeQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "LCR:SHUNT:AUTO",
     .callback = RP_LCRShuntAuto,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "LCR:CIRCUIT",
     .callback = RP_LCRMeasSeries,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "LCR:CIRCUIT?",
     .callback = RP_LCRMeasSeriesQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },
    {.pattern = "LCR:EXT:MODULE?",
     .callback = RP_LCRCheckExtensionModuleConnectioQ,
#if USE_COMMAND_TAGS
     .tag = 0
#endif
    },

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
