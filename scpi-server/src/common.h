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

#include "scpi/types.h"
#include "scpi/parser.h"
#include "scpi/error.h"
#include "rp.h"
#include "rp_hw-profiles.h"

typedef enum {
    RP_SCPI_LOG_OFF,
    RP_SCPI_LOG_CONSOLE,
    RP_SCPI_LOG_SYSLOG
} rp_scpi_log;

typedef enum {
    RP_SCPI_VOLTS,
    RP_SCPI_RAW,
} rp_scpi_acq_unit_t;

/* These structures are a direct API mirror
and should not be altered! */
const scpi_choice_def_t scpi_RpUnits[] = {
    {"VOLTS", 0},
    {"RAW", 1},
    SCPI_CHOICE_LIST_END
};

const scpi_choice_def_t scpi_RpGain[] = {
    {"LV", 0},
    {"HV", 1},
    SCPI_CHOICE_LIST_END
};

const scpi_choice_def_t scpi_RpAC_DC[] = {
    {"DC", 0},
    {"AC", 1},
    SCPI_CHOICE_LIST_END
};

// const scpi_choice_def_t scpi_RpSmpRate[] = {
//     {"S_125MHz",   0}, //!< Sample rate 125Msps; Buffer time length 131us; Decimation 1
//     {"S_15_6MHz",  1}, //!< Sample rate 15.625Msps; Buffer time length 1.048ms; Decimation 8
//     {"S_1_9MHz",   2}, //!< Sample rate 1.953Msps; Buffer time length 8.388ms; Decimation 64
//     {"S_103_8kHz", 3}, //!< Sample rate 122.070ksps; Buffer time length 134.2ms; Decimation 1024
//     {"S_15_2kHz",  4}, //!< Sample rate 15.258ksps; Buffer time length 1.073s; Decimation 8192
//     {"S_1_9kHz",   5},
//     SCPI_CHOICE_LIST_END
// };

const scpi_choice_def_t scpi_RpTrigSrc[] = {
    {"DISABLED",    0},
    {"NOW",         1},
    {"CH1_PE",      2},
    {"CH1_NE",      3},
    {"CH2_PE",      4},
    {"CH2_NE",      5},
    {"EXT_PE",      6},
    {"EXT_NE",      7},
    {"AWG_PE",      8},
    {"AWG_NE",      9},
    {"CH3_PE",      10},
    {"CH3_NE",      11},
    {"CH4_PE",      12},
    {"CH4_NE",      13},
    SCPI_CHOICE_LIST_END
};

const scpi_choice_def_t scpi_RpTrigStat[] = {
    {"TD",   0},
    {"WAIT", 1},
    {"WAIT", 2},
    {"WAIT", 3},
    {"WAIT", 4},
    {"WAIT", 5},
    {"WAIT", 6},
    {"WAIT", 7},
    {"WAIT", 8},
    {"WAIT", 9},
    {"WAIT", 10},
    {"WAIT", 11},
    {"WAIT", 12},
    {"WAIT", 13},
    SCPI_CHOICE_LIST_END
};

auto getCmdName(scpi_t *context) -> const char *;
auto rp_Log(scpi_t *context, int mode, int rp_err_code, const char * format, ...) -> void;
auto scpi_Log(scpi_t *context, int mode, int err_code, const char * format, ...) -> void;

#define SCPI_CMD_NUM 	1
#define RP_F_NAME auto func = getCmdName(context);
#define RP_LOG_INFO(...)  { char error_msg[512]; \
                            snprintf(error_msg,512,__VA_ARGS__); \
                            rp_Log(context,LOG_INFO, 0, "*%s %s", getCmdName(context),error_msg); }
#define RP_LOG_ERR(...)  { char error_msg[512]; \
                            snprintf(error_msg,512,__VA_ARGS__); \
                            rp_Log(context,LOG_ERR, result, "*%s %s", getCmdName(context),error_msg); }
#define RP_LOG_CRIT(...)  { char error_msg[512]; \
                            snprintf(error_msg,512,__VA_ARGS__); \
                            rp_Log(context,LOG_CRIT, result, "*%s %s", getCmdName(context),error_msg); }

#define RP_LOG_INFO(...)  { char error_msg[512]; \
                            snprintf(error_msg,512,__VA_ARGS__); \
                            rp_Log(context,LOG_INFO, 0, "*%s %s", getCmdName(context),error_msg); }
#define RP_LOG_ERR(...)  { char error_msg[512]; \
                            snprintf(error_msg,512,__VA_ARGS__); \
                            rp_Log(context,LOG_ERR, result, "*%s %s", getCmdName(context),error_msg); }
#define RP_LOG_CRIT(...)  { char error_msg[512]; \
                            snprintf(error_msg,512,__VA_ARGS__); \
                            rp_Log(context,LOG_CRIT, result, "*%s %s", getCmdName(context),error_msg); }

#define SCPI_LOG_ERR(X,...)  { char error_msg[512]; \
                            snprintf(error_msg,512,__VA_ARGS__); \
                            rp_Log(context,LOG_ERR, X, "*%s %s", getCmdName(context),error_msg); }
#define SCPI_LOG_CRIT(X,...)  { char error_msg[512]; \
                            snprintf(error_msg,512,__VA_ARGS__); \
                            rp_Log(context,LOG_CRIT, X, "*%s %s", getCmdName(context),error_msg); }


int RP_ParseChArgvADC(scpi_t *context, rp_channel_t *channel);
int RP_ParseChArgvDAC(scpi_t *context, rp_channel_t *channel);

rp_scpi_log getLogMode();
scpi_result_t RP_SetLogMode(scpi_t *context);


uint8_t getADCChannels(scpi_t *context);
uint8_t getDACChannels(scpi_t *context);
uint32_t getDACRate(scpi_t *context);
uint32_t getADCRate(scpi_t *context);
rp_HPeModels_t getModel(scpi_t *context);


scpi_result_t RP_Time(scpi_t *context);
scpi_result_t RP_TimeQ(scpi_t *context);

scpi_result_t RP_Date(scpi_t *context);
scpi_result_t RP_DateQ(scpi_t *context);

scpi_result_t RP_BoardID(scpi_t *context);
scpi_result_t RP_BoardName(scpi_t *context);

scpi_result_t RP_ExtTriggerDebouncerUs(scpi_t *context);
scpi_result_t RP_ExtTriggerDebouncerUsQ(scpi_t *context);

#endif /* COMMON_H_ */
