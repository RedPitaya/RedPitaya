/**
 * $Id: $
 *
 * @brief Red Pitaya Scpi server apin SCPI commands implementation
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
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "common.h"
#include "api_cmd.h"
#include "scpi/parser.h"
#include "acquire.h"
#include "dpin.h"
#include "apin.h"

#include "generate.h"

scpi_result_t RP_InitAll(scpi_t *context){

    int result = rp_Init();

    if(result != RP_OK){
        RP_LOG(LOG_ERR, "*RP:INIT Failed to initialize Red "
            "Pitaya modules: %s\n", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    RP_LOG(LOG_INFO, "*RP:INIT Successfully inizitalized Red Pitaya modules.\n");
    return SCPI_RES_OK;
}

scpi_result_t RP_ResetAll(scpi_t *context){

    int result = RP_AcqReset(context);

    if(result != RP_OK){
        RP_LOG(LOG_ERR, "*RP:RST Failed to reset Red "
            "Pitaya modules: %s\n", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    result = RP_AnalogPinReset(context);

    if(result != RP_OK){
        RP_LOG(LOG_ERR, "*RP:RST Failed to reset Red "
            "Pitaya modules: %s\n", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    result = RP_DigitalPinReset(context);

    if(result != RP_OK){
        RP_LOG(LOG_ERR, "*RP:RST Failed to reset Red "
            "Pitaya modules: %s\n", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    result = RP_GenReset(context);

    if(result != RP_OK){
        RP_LOG(LOG_ERR, "*RP:RST Failed to reset Red "
            "Pitaya modules: %s\n", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    RP_LOG(LOG_INFO, "*RP:RST Successfully reset Red Pitaya modules.\n");
    return SCPI_RES_OK;
}

scpi_result_t RP_ReleaseAll(scpi_t *context){

    int result = rp_Release();

    if(result != RP_OK){
        RP_LOG(LOG_ERR, "*RP:RELEASE Failed to release Red "
            "Pitaya modules: %s\n", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    RP_LOG(LOG_INFO, "*RP:RELEASE Successfully released Red Pitaya modules.\n");
    return SCPI_RES_OK;
}


scpi_result_t RP_EnableDigLoop(scpi_t *context){

    int result = rp_EnableDigitalLoop(true);

    if(result != RP_OK){
        RP_LOG(LOG_ERR, "*RP:DIG:LOop Failed to initialize Red Pitaya"
            " digital loop: %s\n", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    RP_LOG(LOG_INFO, "*RP:DIG:LOop Successfully initialize Red Pitaya"
        " digital loop.\n");

    return SCPI_RES_OK;
}
