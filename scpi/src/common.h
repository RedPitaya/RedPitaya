/**
 * @brief Red Pitaya Scpi server utils module interface
 * @Author Red Pitaya
 * (c) Red Pitaya  http://www.redpitaya.com
 */

#ifndef COMMON_H_
#define COMMON_H_

#include <syslog.h>

#include "scpi/parser.h"
#include "redpitaya/rp1.h"

#define SET_OK(cont) \
    	SCPI_ResultString(cont, "OK"); \
    	return SCPI_RES_OK;

#define RPSCPI_CMD_NUM 	1

#ifdef SCPI_DEBUG
#define RP_LOG(...) syslog(__VA_ARGS__);
#else
#define RP_LOG(...)
#endif

// Red Pitaya specific contct inside SCPI context
typedef struct {
    int connfd;
    bool binary_output;
    // API
    int unsigned gen_num;
    int unsigned osc_num;
//    rp_clb_t clb;
    rp_gen_t *gen;
//    rp_osc_t *osc;
    // generator
    int32_t gen_waveform_tag;
    float  *gen_waveform;
} rpscpi_context_t;

#endif /* COMMON_H_ */
