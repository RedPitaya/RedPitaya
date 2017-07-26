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

#include "scpi/error.h"
#include "scpi/ieee488.h"
#include "scpi/minimal.h"
#include "scpi/units.h"
#include "scpi/parser.h"

#include "redpitaya/rp1.h"

#include "scpi-commands.h"
#include "api_cmd.h"
#include "common.h"
#include "scpi_gen.h"

/**
 * SCPI Configuration
 */

const scpi_command_t scpi_commands[] = {
    /* IEEE Mandated Commands (SCPI std V1999.0 4.1.1) */
    { .pattern = "*CLS" , .callback = SCPI_CoreCls,},
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
    {.pattern = ":SYSTem:ERRor[:NEXT]?", .callback = SCPI_SystemErrorNextQ,},
    {.pattern = ":SYSTem:ERRor:COUNt?",  .callback = SCPI_SystemErrorCountQ,},
    {.pattern = ":SYSTem:VERSion?",      .callback = SCPI_SystemVersionQ,},

    {.pattern = ":STATus:QUEStionable[:EVENt]?", .callback = SCPI_StatusQuestionableEventQ,},
    {.pattern = ":STATus:QUEStionable:ENABle",   .callback = SCPI_StatusQuestionableEnable,},
    {.pattern = ":STATus:QUEStionable:ENABle?",  .callback = SCPI_StatusQuestionableEnableQ,},
    {.pattern = ":STATus:PRESet",                .callback = SCPI_StatusPreset,},

    {.pattern = ":SYSTem:COMMunication:TCPIP:CONTROL?", .callback = SCPI_SystemCommTcpipControlQ,},

    /* RedPitaya */

    /* General commands */
//    {.pattern = "RP:INit", .callback                    = RP_InitAll,},
//    {.pattern = "RP:REset", .callback                   = RP_ResetAll,},
//    {.pattern = "RP:RELease", .callback                 = RP_ReleaseAll,},
//    {.pattern = "RP:FPGABITREAM", .callback             = RP_FpgaBitStream,},
//    {.pattern = "RP:DIg[:loop]", .callback              = RP_EnableDigLoop,},

//    /* Acquire */
//    {.pattern = "ACQ:START", .callback                  = RP_AcqStart,},
//    {.pattern = "ACQ:STOP", .callback                   = RP_AcqStop,},
//    {.pattern = "ACQ:RST", .callback                    = RP_AcqReset,},
//    {.pattern = "ACQ:DEC", .callback                    = RP_AcqDecimation,},
//    {.pattern = "ACQ:DEC?", .callback                   = RP_AcqDecimationQ,},
//    {.pattern = "ACQ:SRAT?", .callback                  = RP_AcqSamplingRateHzQ,},
//    {.pattern = "ACQ:AVG", .callback                    = RP_AcqAveraging,},
//    {.pattern = "ACQ:AVG?", .callback                   = RP_AcqAveragingQ,},
//    {.pattern = "ACQ:TRIGger", .callback                = RP_AcqTriggerSrc,},
//    {.pattern = "ACQ:TRIGger:STAT?", .callback          = RP_AcqTriggerSrcQ,},
//    {.pattern = "ACQ:TRIGger:DLY", .callback            = RP_AcqTriggerDelay,},
//    {.pattern = "ACQ:TRIGger:DLY?", .callback           = RP_AcqTriggerDelayQ,},
//    {.pattern = "ACQ:TRIGger:DLY:NS", .callback         = RP_AcqTriggerDelayNs,},
//    {.pattern = "ACQ:TRIGger:DLY:NS?", .callback        = RP_AcqTriggerDelayNsQ,},
//    {.pattern = "ACQ:TRIGger:HYST", .callback           = RP_AcqTriggerHyst,},
//    {.pattern = "ACQ:TRIGger:HYST?", .callback          = RP_AcqTriggerHystQ,},
//    {.pattern = "ACQ:SOURce#:GAIN", .callback           = RP_AcqGain,},
//    {.pattern = "ACQ:SOURce#:GAIN?", .callback          = RP_AcqGainQ,},
//    {.pattern = "ACQ:TRIG:LEV", .callback               = RP_AcqTriggerLevel,},
//    {.pattern = "ACQ:TRIG:LEV?", .callback              = RP_AcqTriggerLevelQ,},
//    {.pattern = "ACQ:WPOS?", .callback                  = RP_AcqWritePointerQ,},
//    {.pattern = "ACQ:TPOS?", .callback                  = RP_AcqWritePointerAtTrigQ,},
//    {.pattern = "ACQ:DATA:UNITS", .callback             = RP_AcqScpiDataUnits,},
//    {.pattern = "ACQ:DATA:UNITS?", .callback            = RP_AcqScpiDataUnitsQ,},
//    {.pattern = "ACQ:DATA:FORMAT", .callback            = RP_AcqSetDataFormat,},
//    {.pattern = "ACQ:SOURce#:DATA:STA:END?", .callback  = RP_AcqDataPosQ,},
//    {.pattern = "ACQ:SOURce#:DATA:STA:N?", .callback    = RP_AcqDataQ,},
//    {.pattern = "ACQ:SOURce#:DATA:OLD:N?", .callback    = RP_AcqOldestDataQ,},
//    {.pattern = "ACQ:SOURce#:DATA?", .callback          = RP_AcqDataOldestAllQ,},
//    {.pattern = "ACQ:SOURce#:DATA:LAT:N?", .callback    = RP_AcqLatestDataQ,},
//    {.pattern = "ACQ:BUF:SIZE?", .callback              = RP_AcqBufferSizeQ,},

    /* Generate */
    {.pattern = ":SOURce#:RESET",                              .callback = rpscpi_gen_reset,},
    {.pattern = ":SOURce#:START",                              .callback = rpscpi_gen_start,},
    {.pattern = ":SOURce#:STOP",                               .callback = rpscpi_gen_stop,},
    {.pattern = ":SOURce#:TRIGger",                            .callback = rpscpi_gen_trigger,},
    {.pattern = ":SOURce#:RUN?",                               .callback = rpscpi_gen_status_run,},      // TODO: use something standard
    {.pattern = ":SOURce#:TRIGger?",                           .callback = rpscpi_gen_status_trigger,},  // TODO: use something standard
    {.pattern = ":SOURce#:EVENT:SYNChronization[:SOURce]",     .callback = rpscpi_gen_set_sync_src,},    // TODO: use something standard
    {.pattern = ":SOURce#:EVENT:SYNChronization[:SOURce]?",    .callback = rpscpi_gen_get_sync_src,},    // TODO: use something standard
    {.pattern = ":SOURce#:EVENT:TRIGger[:SOURce]",             .callback = rpscpi_gen_set_trig_src,},    // TODO: use something standard
    {.pattern = ":SOURce#:EVENT:TRIGger[:SOURce]?",            .callback = rpscpi_gen_get_trig_src,},    // TODO: use something standard
    {.pattern = "[:SOURce#]:MODE",                             .callback = rpscpi_gen_set_mode,},
    {.pattern = "[:SOURce#]:MODE?",                            .callback = rpscpi_gen_get_mode,},
    {.pattern = "[:SOURce#]:FREQuency[:FIXed]",                .callback = rpscpi_gen_set_frequency,},
    {.pattern = "[:SOURce#]:FREQuency[:FIXed]?",               .callback = rpscpi_gen_get_frequency,},
    {.pattern = "[:SOURce#]:FUNCtion[:SHAPe]",                 .callback = rpscpi_gen_set_waveform_shape,},
    {.pattern = "[:SOURce#]:FUNCtion[:SHAPe]?",                .callback = rpscpi_gen_get_waveform_shape,},
    {.pattern = "[:SOURce#]:PHASe[:ADJust]",                   .callback = rpscpi_gen_set_phase,},
    {.pattern = "[:SOURce#]:PHASe[:ADJust]?",                  .callback = rpscpi_gen_get_phase,},
//  {.pattern = "[:SOURce#]:PHASe:INITiate",                   .callback = RP_GenPhaseInit,},  // use software trigger instead
    {.pattern = "[:SOURce#]:TRACe:DATA[:DATA]",                .callback = rpscpi_gen_set_waveform_data,},
    {.pattern = "[:SOURce#]:TRACe:DATA[:DATA]?",               .callback = rpscpi_gen_get_waveform_data,},
    {.pattern = "[:SOURce#]:TRACe:DATA:RAW",                   .callback = rpscpi_gen_set_waveform_raw,},
    {.pattern = "[:SOURce#]:TRACe:DATA:RAW?",                  .callback = rpscpi_gen_get_waveform_raw,},
    {.pattern = "[:SOURce#]:BURSt[:MODE]",                     .callback = rpscpi_gen_set_burst_mode,},
    {.pattern = "[:SOURce#]:BURSt[:MODE]",                     .callback = rpscpi_gen_set_burst_mode,},
    {.pattern = "[:SOURce#]:BURSt[:MODE]?",                    .callback = rpscpi_gen_get_burst_mode,},
    {.pattern = "[:SOURce#]:BURSt:DATA:REPetitions",           .callback = rpscpi_gen_set_data_repetitions,},
    {.pattern = "[:SOURce#]:BURSt:DATA:REPetitions?",          .callback = rpscpi_gen_get_data_repetitions,},
    {.pattern = "[:SOURce#]:BURSt:DATA:LENgth",                .callback = rpscpi_gen_set_data_length,},
    {.pattern = "[:SOURce#]:BURSt:DATA:LENgth?",               .callback = rpscpi_gen_get_data_length,},
    {.pattern = "[:SOURce#]:BURSt:PERiod:LENgth",              .callback = rpscpi_gen_set_period_length,},
    {.pattern = "[:SOURce#]:BURSt:PERiod:LENgth?",             .callback = rpscpi_gen_get_period_length,},
    {.pattern = "[:SOURce#]:BURSt:PERiod:NUMber",              .callback = rpscpi_gen_set_period_number,},
    {.pattern = "[:SOURce#]:BURSt:PERiod:NUMber?",             .callback = rpscpi_gen_get_period_number,},
//    {.pattern = ":SOURce#:TRIGger:SOUR", .callback       = RP_GenTriggerSource,},
//    {.pattern = ":SOURce#:TRIGger:SOUR?", .callback      = RP_GenTriggerSourceQ,},
//    {.pattern = ":SOURce#:TRIGger:IMM", .callback        = RP_GenTrigger,},
    {.pattern = "[:SOURce#]:VOLTage[:IMMediate][:AMPlitude]",  .callback = rpscpi_gen_set_amplitude,},
    {.pattern = "[:SOURce#]:VOLTage[:IMMediate][:AMPlitude]?", .callback = rpscpi_gen_get_amplitude,},
    {.pattern = "[:SOURce#]:VOLTage[:IMMediate]:OFFSet",       .callback = rpscpi_gen_set_offset,},
    {.pattern = "[:SOURce#]:VOLTage[:IMMediate]:OFFSet?",      .callback = rpscpi_gen_get_offset,},
    {.pattern = ":OUTPUT#[:STATe]",                            .callback = rpscpi_gen_set_enable,},
    {.pattern = ":OUTPUT#[:STATe]?",                           .callback = rpscpi_gen_get_enable,},

    SCPI_CMD_LIST_END
};

// TODO
// SHAPe should provide some kind of enumerated value parser

scpi_interface_t scpi_interface = {
    .error = SCPI_Error,
    .write = SCPI_Write,
    .control = SCPI_Control,
    .flush = SCPI_Flush,
    .reset = SCPI_Reset,
};

char scpi_input_buffer[SCPI_INPUT_BUFFER_LENGTH];
scpi_error_t scpi_error_queue_data[SCPI_ERROR_QUEUE_SIZE];

scpi_t scpi_context;

