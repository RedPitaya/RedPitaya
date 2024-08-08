/**
 * $Id$
 *
 * @brief Red Pitaya Web module
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */


#ifndef __RP_CLIENT_H__
#define __RP_CLIENT_H__

#include <stdint.h>

typedef enum {
    RP_WC_NONE          = 0,
    RP_WC_REQUEST_ID    = 0x1,
    RP_WC_DELETE_ID     = 0x2
} rp_client_request_t;


void rp_WC_Init();
void rp_WC_SetPingInterval(uint32_t ms);
void rp_WC_UpdateParameters(bool force);
void rp_WC_PauseSend(bool state);
void rp_WC_OnNewParam();

#endif

