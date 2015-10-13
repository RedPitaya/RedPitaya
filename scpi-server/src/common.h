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
#include "../../api/rpbase/src/rp.h"

#define SET_OK(cont) \
    	SCPI_ResultString(cont, "OK"); \
    	return SCPI_RES_OK;

#define MIN_CH			0
#define MAX_CH			3

#define SCPI_CMD_NUM 	1
#define SCPI_DEBUG 		1


#define CNV_STR(x) #x
    	
/* rp scpi log */
#ifdef SCPI_DEBUG
#define RP_ERR(msg, param) \
(CNV_STR(param) == NULL) ? \
syslog(LOG_INFO, "%s\n", msg): \
syslog(LOG_INFO,"%s: %s\n", msg, CNV_STR(param));
#define RP_INFO(msg) \
syslog(LOG_INFO, "%s\n", msg);
#else
#define RP_ERR(msg, param)
#define RP_INFO(msg)
#endif

#ifdef SCPI_DEBUG
#define RP_LOG(...) \
syslog(__VA_ARGS__);
#else
#define RP_LOG(...) pass;
#endif

int RP_ParseChArgv(scpi_t *context, rp_channel_t *channel);

#endif /* COMMON_H_ */
