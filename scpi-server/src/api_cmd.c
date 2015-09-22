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

#include <syslog.h>
#include <stdio.h>
#include <string.h>

#include "utils.h"
#include "api_cmd.h"
#include "../3rdparty/libs/scpi-parser/libscpi/inc/scpi/parser.h"
#include "../../api/rpbase/src/common.h"


scpi_result_t RP_InitAll(scpi_t *context){

    int result = rp_Init();

    if(result != RP_OK){
        syslog(LOG_ERR, "*RP:INIT Failed to initialize Red Pitaya modules: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    syslog(LOG_INFO, "*RP:INIT Successfully inizitalized Red Pitaya modules.");
    return SCPI_RES_OK;
}

scpi_result_t RP_ResetAll(scpi_t *context){

    int result = rp_Reset();

    if(result != RP_OK){
        syslog(LOG_ERR, "*RP:RST Failed to reset Red Pitaya modules: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    syslog(LOG_INFO, "*RP:RST Successfully reset Red Pitaya modules.");
    return SCPI_RES_OK;
}

scpi_result_t RP_RealaseAll(scpi_t *context){

    int result = rp_Release();

    if(result != RP_OK){
        syslog(LOG_ERR, "*RP:RELEASE Failed to release Red Pitaya modules: %s", rp_GetError(result));
        return SCPI_RES_ERR;
    }

    syslog(LOG_INFO, "*RP:RELEASE Successfully released Red Pitaya modules.");
    return SCPI_RES_OK;
}

scpi_result_t RP_FpgaLoad(scpi_t *context){

    const char *param;
    size_t param_len;

    char fpga_image[100];
    char *fpga_dir = "/opt/redpitaya/fpga/";
    

    //Read first parameter(specific fpga image to load)
    if(!SCPI_ParamString(context, &param, &param_len, true)){
        syslog(LOG_ERR, "*RP:FPGA:RST Failed to reset load fpga image.\n");
        return SCPI_RES_ERR;
    }

    strncpy(fpga_image, &param[0], param_len);
    fpga_image[param_len] = '\0';

    char fpga_path[strlen(fpga_dir) + strlen(fpga_image) + 2];
    sprintf(fpga_path, "%s/%s", fpga_path, fpga_image);

    int result = rp_fpga_load(&fpga_path[0]);

    if(result != RP_OK){
        syslog(LOG_ERR, "*RP:FPGA:RST Failed to reset load fpga image.\n");
        return SCPI_RES_ERR;
    }

    return SCPI_RES_OK;
}