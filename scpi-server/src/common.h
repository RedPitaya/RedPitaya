/**
 * $Id: $
 *
 * @brief Red Pitaya Scpi server utils module interface
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef COMMON_H_
#define COMMON_H_

#include <syslog.h>

#include "scpi/parser.h"
#include "rp.h"


typedef enum {
    RP_SCPI_LOG_OFF,
    RP_SCPI_LOG_CONSOLE,
    RP_SCPI_LOG_SYSLOG
} rp_scpi_log;


#define SCPI_CMD_NUM 	1

#define RP_F_NAME(X) X


int RP_ParseChArgv(scpi_t *context, rp_channel_t *channel);

rp_scpi_log getLogMode();
scpi_result_t RP_SetLogMode(scpi_t *context);

void RP_LOG(int mode,const char * format, ...);


#endif /* COMMON_H_ */
