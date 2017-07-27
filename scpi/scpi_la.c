/**
 * @brief Red Pitaya SCPI server, logic analyzer commands implementation
 * @Author Red Pitaya
 * (c) Red Pitaya  http://www.redpitaya.com
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "redpitaya/rp1.h"
#include "common.h"
#include "scpi_la.h"

#include "scpi/parser.h"
#include "scpi/units.h"
#include "scpi/error.h"

/**
 * initialization of context and API
 */
scpi_result_t rpscpi_la_init(rpscpi_context_t *rp, int unsigned channels) {
    // run API initialization
    if (rp_la_init(&rp->la) !=0) {
        syslog(LOG_ERR, "Failed to initialize logic analyzer.");
        return(SCPI_RES_ERR);
    }
    return(SCPI_RES_OK);
}

/**
 * release of context and API
 */
scpi_result_t rpscpi_la_release(rpscpi_context_t *rp) {
    if (rp_la_release(&rp->la) !=0) {
        syslog(LOG_ERR, "Failed to release logic analyzer.");
        return (SCPI_RES_OK);
    }
    return(SCPI_RES_OK);
}

/**
 * helper function for detecting selected channel
 */
static scpi_bool_t rpcspi_la_channels(scpi_t *context, rp_la_t *la) {
    *la = ((rpscpi_context_t *) context->user_context)->la;
    return TRUE;
}

//scpi_result_t rpscpi_la_trg_set_edge_pos (scpi_t *context);
//scpi_result_t rpscpi_la_trg_get_edge_pos (scpi_t *context);
//scpi_result_t rpscpi_la_trg_set_edge_neg (scpi_t *context);
//scpi_result_t rpscpi_la_trg_get_edge_neg (scpi_t *context);

scpi_result_t rpscpi_la_trg_set_mask(scpi_t *context) {
    rp_la_t la;
    if (!rpcspi_la_channels(context, &la)) {
        return SCPI_RES_ERR;
    }

    uint32_t value;
    if (!SCPI_ParamUInt32(context, &value, true)) {
        return SCPI_RES_ERR;
    }
    if (rp_la_trg_set_mask(&la.trg, value)) {
        SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
        return SCPI_RES_ERR;
    }
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_la_trg_get_mask(scpi_t *context) {
    rp_la_t la;
    if (!rpcspi_la_channels(context, &la)) {
        return SCPI_RES_ERR;
    }

    uint32_t value = rp_la_trg_get_mask(&la.trg);
    SCPI_ResultUInt32(context, value);
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_la_trg_set_value(scpi_t *context) {
    rp_la_t la;
    if (!rpcspi_la_channels(context, &la)) {
        return SCPI_RES_ERR;
    }

    uint32_t value;
    if (!SCPI_ParamUInt32(context, &value, true)) {
        return SCPI_RES_ERR;
    }
    if (rp_la_trg_set_value(&la.trg, value)) {
        SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
        return SCPI_RES_ERR;
    }
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_la_trg_get_value(scpi_t *context) {
    rp_la_t la;
    if (!rpcspi_la_channels(context, &la)) {
        return SCPI_RES_ERR;
    }

    uint32_t value = rp_la_trg_get_value(&la.trg);
    SCPI_ResultUInt32(context, value);
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_la_trg_set_edge_pos(scpi_t *context) {
    rp_la_t la;
    if (!rpcspi_la_channels(context, &la)) {
        return SCPI_RES_ERR;
    }

    uint32_t value;
    if (!SCPI_ParamUInt32(context, &value, true)) {
        return SCPI_RES_ERR;
    }
    if (rp_la_trg_set_edge_pos(&la.trg, value)) {
        SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
        return SCPI_RES_ERR;
    }
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_la_trg_get_edge_pos(scpi_t *context) {
    rp_la_t la;
    if (!rpcspi_la_channels(context, &la)) {
        return SCPI_RES_ERR;
    }

    uint32_t value = rp_la_trg_get_edge_pos(&la.trg);
    SCPI_ResultUInt32(context, value);
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_la_trg_set_edge_neg(scpi_t *context) {
    rp_la_t la;
    if (!rpcspi_la_channels(context, &la)) {
        return SCPI_RES_ERR;
    }

    uint32_t value;
    if (!SCPI_ParamUInt32(context, &value, true)) {
        return SCPI_RES_ERR;
    }
    if (rp_la_trg_set_edge_neg(&la.trg, value)) {
        SCPI_ErrorPush(context, SCPI_ERROR_DATA_OUT_OF_RANGE);
        return SCPI_RES_ERR;
    }
    return SCPI_RES_OK;
}

scpi_result_t rpscpi_la_trg_get_edge_neg(scpi_t *context) {
    rp_la_t la;
    if (!rpcspi_la_channels(context, &la)) {
        return SCPI_RES_ERR;
    }

    uint32_t value = rp_la_trg_get_edge_neg(&la.trg);
    SCPI_ResultUInt32(context, value);
    return SCPI_RES_OK;
}

