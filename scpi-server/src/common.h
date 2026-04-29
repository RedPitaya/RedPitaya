/**
 * $Id: $
 *
 * @brief Red Pitaya Scpi server utils module interface
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */

#ifndef COMMON_H_
#define COMMON_H_

#include <syslog.h>

#include "rp.h"
#include "rp_hw-profiles.h"
#include "scpi/error.h"
#include "scpi/parser.h"
#include "scpi/types.h"

typedef enum { RP_SCPI_LOG_OFF, RP_SCPI_LOG_CONSOLE, RP_SCPI_LOG_SYSLOG } rp_scpi_log;

typedef enum {
    RP_SCPI_VOLTS,
    RP_SCPI_RAW,
} rp_scpi_acq_unit_t;

/* These structures are a direct API mirror
and should not be altered! */
const scpi_choice_def_t scpi_RpUnits[] = {{"VOLTS", 0}, {"RAW", 1}, SCPI_CHOICE_LIST_END};

const scpi_choice_def_t scpi_RpGain[] = {{"LV", 0}, {"HV", 1}, SCPI_CHOICE_LIST_END};

const scpi_choice_def_t scpi_RpAC_DC[] = {{"DC", 0}, {"AC", 1}, SCPI_CHOICE_LIST_END};

// const scpi_choice_def_t scpi_RpSmpRate[] = {
//     {"S_125MHz",   0}, //!< Sample rate 125Msps; Buffer time length 131us; Decimation 1
//     {"S_15_6MHz",  1}, //!< Sample rate 15.625Msps; Buffer time length 1.048ms; Decimation 8
//     {"S_1_9MHz",   2}, //!< Sample rate 1.953Msps; Buffer time length 8.388ms; Decimation 64
//     {"S_103_8kHz", 3}, //!< Sample rate 122.070ksps; Buffer time length 134.2ms; Decimation 1024
//     {"S_15_2kHz",  4}, //!< Sample rate 15.258ksps; Buffer time length 1.073s; Decimation 8192
//     {"S_1_9kHz",   5},
//     SCPI_CHOICE_LIST_END
// };

const scpi_choice_def_t scpi_RpTrigSrc[] = {{"DISABLED", RP_TRIG_SRC_DISABLED}, {"NOW", RP_TRIG_SRC_NOW},       {"CH1_PE", RP_TRIG_SRC_CHA_PE},
                                            {"CH1_NE", RP_TRIG_SRC_CHA_NE},     {"CH2_PE", RP_TRIG_SRC_CHB_PE}, {"CH2_NE", RP_TRIG_SRC_CHB_NE},
                                            {"EXT_PE", RP_TRIG_SRC_EXT_PE},     {"EXT_NE", RP_TRIG_SRC_EXT_NE}, {"AWG_PE", RP_TRIG_SRC_AWG_PE},
                                            {"AWG_NE", RP_TRIG_SRC_AWG_NE},     {"CH3_PE", RP_TRIG_SRC_CHC_PE}, {"CH3_NE", RP_TRIG_SRC_CHC_NE},
                                            {"CH4_PE", RP_TRIG_SRC_CHD_PE},     {"CH4_NE", RP_TRIG_SRC_CHD_NE}, {"CH1_AE", RP_TRIG_SRC_CHA_AE},
                                            {"CH2_AE", RP_TRIG_SRC_CHB_AE},     {"EXT_AE", RP_TRIG_SRC_EXT_AE}, {"AWG_AE", RP_TRIG_SRC_AWG_AE},
                                            {"CH3_AE", RP_TRIG_SRC_CHC_AE},     {"CH4_AE", RP_TRIG_SRC_CHD_AE}, SCPI_CHOICE_LIST_END};

const scpi_choice_def_t scpi_RpTrigStat[] = {{"TD", 0}, {"WAIT", 1}, SCPI_CHOICE_LIST_END};
const scpi_choice_def_t scpi_RpIntStat[] = {{"OK", RP_OK}, {"TIMEOUT", RP_ETIM}, {"ERROR", RP_EOP}, SCPI_CHOICE_LIST_END};

auto getCmdName(scpi_t* context) -> const char*;
auto rp_Log(scpi_t* context, int mode, int rp_err_code, const char* format, ...) -> void;
auto scpi_Log(scpi_t* context, int mode, int err_code, const char* format, ...) -> void;

#define SCPI_CMD_NUM 1
#define RP_F_NAME auto func = getCmdName(context);
#define RP_LOG_INFO(...)                                                        \
    {                                                                           \
        char error_msg[512];                                                    \
        snprintf(error_msg, 512, __VA_ARGS__);                                  \
        rp_Log(context, LOG_INFO, 0, "*%s %s", getCmdName(context), error_msg); \
    }
#define RP_LOG_ERR(...)                                                             \
    {                                                                               \
        char error_msg[512];                                                        \
        snprintf(error_msg, 512, __VA_ARGS__);                                      \
        rp_Log(context, LOG_ERR, result, "*%s %s", getCmdName(context), error_msg); \
    }
#define RP_LOG_CRIT(...)                                                             \
    {                                                                                \
        char error_msg[512];                                                         \
        snprintf(error_msg, 512, __VA_ARGS__);                                       \
        rp_Log(context, LOG_CRIT, result, "*%s %s", getCmdName(context), error_msg); \
    }

#define RP_LOG_INFO(...)                                                        \
    {                                                                           \
        char error_msg[512];                                                    \
        snprintf(error_msg, 512, __VA_ARGS__);                                  \
        rp_Log(context, LOG_INFO, 0, "*%s %s", getCmdName(context), error_msg); \
    }
#define RP_LOG_ERR(...)                                                             \
    {                                                                               \
        char error_msg[512];                                                        \
        snprintf(error_msg, 512, __VA_ARGS__);                                      \
        rp_Log(context, LOG_ERR, result, "*%s %s", getCmdName(context), error_msg); \
    }
#define RP_LOG_CRIT(...)                                                             \
    {                                                                                \
        char error_msg[512];                                                         \
        snprintf(error_msg, 512, __VA_ARGS__);                                       \
        rp_Log(context, LOG_CRIT, result, "*%s %s", getCmdName(context), error_msg); \
    }

#define SCPI_LOG_ERR(X, ...)                                                   \
    {                                                                          \
        char error_msg[512];                                                   \
        snprintf(error_msg, 512, __VA_ARGS__);                                 \
        rp_Log(context, LOG_ERR, X, "*%s %s", getCmdName(context), error_msg); \
    }
#define SCPI_LOG_CRIT(X, ...)                                                   \
    {                                                                           \
        char error_msg[512];                                                    \
        snprintf(error_msg, 512, __VA_ARGS__);                                  \
        rp_Log(context, LOG_CRIT, X, "*%s %s", getCmdName(context), error_msg); \
    }

int RP_ParseChArgvADC(scpi_t* context, rp_channel_t* channel);
int RP_ParseChArgvDAC(scpi_t* context, rp_channel_t* channel);

rp_scpi_log getLogMode();
scpi_result_t RP_SetLogMode(scpi_t* context);
scpi_result_t RP_SetRetOnError(scpi_t* context);
bool getRetOnError();

uint8_t getADCChannels(scpi_t* context);
uint8_t getDACChannels(scpi_t* context);
uint32_t getDACRate(scpi_t* context);
uint32_t getADCRate(scpi_t* context);
rp_HPeModels_t getModel(scpi_t* context);

scpi_result_t RP_Time(scpi_t* context);
scpi_result_t RP_TimeQ(scpi_t* context);

scpi_result_t RP_Date(scpi_t* context);
scpi_result_t RP_DateQ(scpi_t* context);

scpi_result_t RP_BoardID(scpi_t* context);
scpi_result_t RP_BoardName(scpi_t* context);

scpi_result_t RP_ExtTriggerDebouncerUs(scpi_t* context);
scpi_result_t RP_ExtTriggerDebouncerUsQ(scpi_t* context);

void requestSendNewLine(scpi_t* context);

void stopAllThreads(scpi_t* context);

scpi_result_t RP_ExtTriggerLevel(scpi_t* context);
scpi_result_t RP_ExtTriggerLevelQ(scpi_t* context);

#endif /* COMMON_H_ */
