/**
 * $Id: $
 *
 * @brief Red Pitaya Scpi server utils module implementation
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#include <stdio.h>

#include "common.h"

/* Parse channel */
int RP_ParseChArgv(scpi_t *context, rp_channel_t *channel){

    int32_t ch_usr[1];
    SCPI_CommandNumbers(context, ch_usr, 1, SCPI_CMD_NUM);
    if (!((ch_usr[0] > 0) && (ch_usr[0] <= CH_NUM))) {
        RP_LOG(LOG_ERR, "ERROR: Invalid channel number: %.*s\n", 50, context->param_list.cmd_raw.data);
        return RP_EOOR;
    }
    *channel = ch_usr[0] - 1;
    
    return RP_OK;
}
