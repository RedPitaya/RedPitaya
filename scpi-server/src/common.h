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

#include "scpi/parser.h"
#include "rp.h"
#include "rp_hw-profiles.h"

typedef enum {
    RP_SCPI_LOG_OFF,
    RP_SCPI_LOG_CONSOLE,
    RP_SCPI_LOG_SYSLOG
} rp_scpi_log;


#define SCPI_CMD_NUM 	1

#define RP_F_NAME(X) X


int RP_ParseChArgvADC(scpi_t *context, rp_channel_t *channel);
int RP_ParseChArgvDAC(scpi_t *context, rp_channel_t *channel);

rp_scpi_log getLogMode();
scpi_result_t RP_SetLogMode(scpi_t *context);

void RP_LOG(scpi_t *context,int mode, const char * format, ...);

uint8_t getADCChannels(scpi_t *context);
uint8_t getDACChannels(scpi_t *context);
uint32_t getDACRate(scpi_t *context);
uint32_t getADCRate(scpi_t *context);
rp_HPeModels_t getModel(scpi_t *context);


scpi_result_t RP_Time(scpi_t *context);
scpi_result_t RP_TimeQ(scpi_t *context);

scpi_result_t RP_Date(scpi_t *context);
scpi_result_t RP_DateQ(scpi_t *context);

#endif /* COMMON_H_ */
