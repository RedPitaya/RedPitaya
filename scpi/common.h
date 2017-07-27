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

#define RPSCPI_CMD_NUM 	1

// Red Pitaya specific contct inside SCPI context
typedef struct {
    int connfd;
    // API
    int unsigned gen_num;
    int unsigned osc_num;
//    rp_clb_t clb;
    rp_gen_t *gen;
//    rp_osc_t *osc;
    // generator
    int32_t gen_waveform_tag[2];
    float   gen_waveform_opt[2];
} rpscpi_context_t;

#endif /* COMMON_H_ */
