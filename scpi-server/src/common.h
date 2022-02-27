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

#define CH_NUM		4

#define SCPI_CMD_NUM 	1

#ifdef SCPI_DEBUG
#define RP_LOG(...) \
syslog(__VA_ARGS__);

#define RP_F_NAME(X) X

#else
#define RP_LOG(X,...)
#define RP_F_NAME(X)
#endif

int RP_ParseChArgv(scpi_t *context, rp_channel_t *channel);


#endif /* COMMON_H_ */
