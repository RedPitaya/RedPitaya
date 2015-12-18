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

    int result = rp_Reset();

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

scpi_result_t RP_FpgaBitStream(scpi_t *context){

    const char *fpga_dir = "/opt/redpitaya/fpga/fpga_";
    const char *param;
    size_t param_len;

    int fo, fi, fpga_s, fpga_dir_s;
    struct stat st;

    /* Read first param(fpga bit file typ (0.93, 0.94)) fom context */
    if(!SCPI_ParamCharacters(context, &param, &param_len, true)){
        RP_LOG(LOG_ERR, "*RP:FPGA:BITSTR Failed to parse first parameter.\n");
        return SCPI_RES_ERR;
    }

    fpga_dir_s = strlen(fpga_dir) + param_len + strlen(".bit") + 2;

    /* Get fpga image path */
    char fpga_file[fpga_dir_s];
    char delim_param[param_len];
    strncpy(delim_param, param, param_len);
    delim_param[param_len] = '\0';

    snprintf(fpga_file, fpga_dir_s, "%s%s.bit", &fpga_dir[0], &delim_param[0]);
    fpga_file[fpga_dir_s - 1] = '\0';
    

    /* Load new fpga image into /dev/xdevcfg */
    fo = open("/dev/xdevcfg", O_WRONLY);
    if(fo < 0){
        RP_LOG(LOG_ERR, "*RP:FPGA:BITstr Failed to open output file.\n");
        return SCPI_RES_ERR;
    }

    fi = open(fpga_file, O_RDONLY);
    if(fi < 0){
        RP_LOG(LOG_ERR, "*RP:FPGA:BITstr Failed to open input file: %d\n", fi);
        return SCPI_RES_ERR;
    }

    /* Get FPGA size */
    stat(fpga_file, &st);
    fpga_s = st.st_size;
    char fi_buff[fpga_s];

    /* Read FPGA file into fi_buff */
    if(read(fi, &fi_buff, fpga_s) < 0){
        RP_LOG(LOG_ERR, "*RP:FPGA:BITstr Unable to read "
            "fpga bit stream into buffer: %d\n", fo);
        return SCPI_RES_ERR;
    }

    if(write(fo, &fi_buff, fpga_s) < 0){
        RP_LOG(LOG_ERR, "*RP:FPGA:BITstr Unable to write fpga "
            "bit stream: %d\n", fo);
        return SCPI_RES_ERR;
    }

    /* Close resources */
    close(fi);
    close(fo);

    RP_LOG(LOG_INFO, "*RP:FPGA:BITstr Successfully loaded FPGA bit stream.\n");
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
