/**
 * $Id: $
 *
 * @brief Red Pitaya Scpi server SCPI commands implementation
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
#include <stdlib.h>

#include "common.h"
#include "spi.h"
#include "scpi/parser.h"
#include "rp_hw.h"


const scpi_choice_def_t scpi_mode[] = {
    {"LISL", RP_SPI_MODE_LISL},
    {"LIST", RP_SPI_MODE_LIST},
    {"HISL", RP_SPI_MODE_HISL},
    {"HIST", RP_SPI_MODE_HIST},
    SCPI_CHOICE_LIST_END
};

const scpi_choice_def_t scpi_order_bit[] = {
    {"MSB", RP_SPI_ORDER_BIT_MSB},
    {"LSB", RP_SPI_ORDER_BIT_LSB},
    SCPI_CHOICE_LIST_END
};

const scpi_choice_def_t scpi_cs_mode[] = {
    {"NORMAL", RP_SPI_CS_NORMAL},
    {"HIGH", RP_SPI_CS_HIGH},
    SCPI_CHOICE_LIST_END
};


scpi_result_t RP_SPI_Init(scpi_t * context){
    int result = rp_SPI_Init();
    if (RP_HW_OK != result) {
        RP_LOG(context,LOG_ERR, "SPI:INIT Failed to init Red Pitaya spi: %d" , result);
        return SCPI_RES_ERR;
    }

    RP_LOG(context,LOG_INFO, "*SPI:INIT Successfully init spi.");
    return SCPI_RES_OK;
}

scpi_result_t RP_SPI_InitDev(scpi_t *context){
    char  dev[255];
    const char * param = NULL;
    size_t param_len = 0;
    memset(dev,0,255);

    if (!SCPI_ParamCharacters(context, &param, &param_len, true)) {
        RP_LOG(context,LOG_ERR, "*SPI:INIT:DEV is missing first parameter.");
        return SCPI_RES_ERR;
    }

    if (param_len == 0) {
        RP_LOG(context,LOG_ERR, "*SPI:INIT:DEV is missing first parameter.");
        return SCPI_RES_ERR;
    }

    strncpy(dev,param,param_len);
    int result = rp_SPI_InitDevice(dev);
    if (RP_HW_OK != result) {
        RP_LOG(context,LOG_ERR, "SPI:INIT:DEV Failed to init Red Pitaya spi: %d" , result);
        return SCPI_RES_ERR;
    }

    RP_LOG(context,LOG_INFO, "*SPI:INIT:DEV Successfully init spi.");
    return SCPI_RES_OK;
}

scpi_result_t RP_SPI_Release(scpi_t * context){
    int result = rp_SPI_Release();
    if (RP_HW_OK != result) {
        RP_LOG(context,LOG_ERR, "SPI:RELEASE Failed to release spi: %d" , result);
        return SCPI_RES_ERR;
    }

    RP_LOG(context,LOG_INFO, "*SPI:RELEASE Successfully release spi.");
    return SCPI_RES_OK;
}

scpi_result_t RP_SPI_SetDefault(scpi_t * context){
    int result = rp_SPI_SetDefaultSettings();
    if (RP_HW_OK != result) {
        RP_LOG(context,LOG_ERR, "*SPI:SETtings:DEF Failed to set default settings for spi: %d" , result);
        return SCPI_RES_ERR;
    }

    RP_LOG(context,LOG_INFO, "*SPI:SETtings:DEF Successfully set default settings for spi.");
    return SCPI_RES_OK;
}

scpi_result_t RP_SPI_SetSettings(scpi_t * context){
    int result = rp_SPI_SetSettings();
    if (RP_HW_OK != result) {
        RP_LOG(context,LOG_ERR, "SPI:SETtings:GET Failed to set settings for spi: %d" , result);
        return SCPI_RES_ERR;
    }

    RP_LOG(context,LOG_INFO, "*SPI:SETtings:GET Successfully set settings for spi.");
    return SCPI_RES_OK;
}

scpi_result_t RP_SPI_GetSettings(scpi_t * context){
    int result = rp_SPI_GetSettings();
    if (RP_HW_OK != result) {
        RP_LOG(context,LOG_ERR, "*SPI:SETtings:GET Failed to get settings for spi: %d" , result);
        return SCPI_RES_ERR;
    }

    RP_LOG(context,LOG_INFO, "*SPI:SETtings:GET Successfully get settings for spi.");
    return SCPI_RES_OK;
}

scpi_result_t RP_SPI_CreateMessage(scpi_t * context){
    uint32_t value = 0;

    if (!SCPI_ParamUInt32(context, &value, true)) {
        RP_LOG(context,LOG_ERR, "*SPI:MSG:CREATE is missing first parameter.");
        return SCPI_RES_ERR;
    }

    int result = rp_SPI_CreateMessage(value);
    if (RP_HW_OK != result) {
        RP_LOG(context,LOG_ERR, "*SPI:MSG:CREATE Failed to create message: %d", result);
        return SCPI_RES_ERR;
    }

    RP_LOG(context,LOG_INFO, "*SPI:MSG:CREATE Successfully create message.");
    return SCPI_RES_OK;
}

scpi_result_t RP_SPI_DestroyMessage(scpi_t * context){
    int result = rp_SPI_DestoryMessage();
    if (RP_HW_OK != result) {
        RP_LOG(context,LOG_ERR, "SPI:MSG:DEL Failed to delete message: %d" , result);
        return SCPI_RES_ERR;
    }

    RP_LOG(context,LOG_INFO, "*SPI:MSG:DEL Successfully delete message.");
    return SCPI_RES_OK;
}

scpi_result_t RP_SPI_GetMessageLen(scpi_t * context){
    size_t len;
    int result = rp_SPI_GetMessageLen(&len);

    if (RP_OK != result) {
        RP_LOG(context,LOG_ERR, "*SPI:MSG:SIZE? Failed to get spi message size: %d", result);
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultUInt32Base(context, len, 10);

    RP_LOG(context,LOG_INFO, "*SPI:MSG:SIZE? Successfully returned message size.");
    return SCPI_RES_OK;
}

scpi_result_t RP_SPI_GetRXBuffer(scpi_t * context){
    const uint8_t *buffer = 0;
    size_t index = 0;
    size_t size = 0;
    int result;
    int32_t cmd[1] = {0};

    if (!SCPI_CommandNumbers(context,cmd,1,-1)){
        RP_LOG(context,LOG_ERR, "*SPI:MGS#:RX? Failed to get parameters.");
        return SCPI_RES_ERR;
    }

    if (cmd[0] == -1){
        RP_LOG(context,LOG_ERR, "*SPI:MGS#:RX? Failed to get index of message.");
        return SCPI_RES_ERR;
    }

    index = cmd[0];

    result = rp_SPI_GetRxBuffer(index,&buffer, &size);
    if(result != RP_HW_OK){
        RP_LOG(context,LOG_ERR, "*SPI:MGS#:RX? Failed rx buffer: %d", result);
        return SCPI_RES_ERR;
    }

    if (!buffer){
        RP_LOG(context,LOG_ERR, "*SPI:MGS#:RX? Buffer is null");
        return SCPI_RES_ERR;
    }

    SCPI_ResultBufferUInt8(context, buffer, size);
    RP_LOG(context,LOG_INFO, "*SPI:MGS#:RX? Successfully returned rx buffer to client.");
    return SCPI_RES_OK;
}

scpi_result_t RP_SPI_GetTXBuffer(scpi_t * context){
    const uint8_t *buffer = 0;
    size_t index = 0;
    size_t size = 0;
    int result;
    int32_t cmd[1] = {0};

    if (!SCPI_CommandNumbers(context,cmd,1,-1)){
        RP_LOG(context,LOG_ERR, "*SPI:MGS#:TX? Failed to get parameters.");
        return SCPI_RES_ERR;
    }

    if (cmd[0] == -1){
        RP_LOG(context,LOG_ERR, "*SPI:MGS#:TX? Failed to get index of message.");
        return SCPI_RES_ERR;
    }

    index = cmd[0];

    result = rp_SPI_GetTxBuffer(index,&buffer, &size);
    if(result != RP_HW_OK){
        RP_LOG(context,LOG_ERR, "*SPI:MGS#:TX? Failed tx buffer: %d", result);
        return SCPI_RES_ERR;
    }

    if (!buffer){
        RP_LOG(context,LOG_ERR, "*SPI:MGS#:TX? Buffer is null");
        return SCPI_RES_ERR;
    }

    SCPI_ResultBufferUInt8(context, buffer, size);
    RP_LOG(context,LOG_INFO, "*SPI:MGS#:TX? Successfully returned tx buffer to client.");
    return SCPI_RES_OK;
}

scpi_result_t RP_SPI_GetCSChangeState(scpi_t * context){
    size_t index = 0;
    bool cs_state = false;
    int32_t cmd[1] = {0};

    if (!SCPI_CommandNumbers(context,cmd,1,-1)){
        RP_LOG(context,LOG_ERR, "*SPI:MSG#:CS? Failed to get parameters.");
        return SCPI_RES_ERR;
    }

    if (cmd[0] == -1){
        RP_LOG(context,LOG_ERR, "*SPI:MSG#:CS? Failed to get index of message.");
        return SCPI_RES_ERR;
    }

    index = cmd[0];

    int result = rp_SPI_GetCSChangeState(index,&cs_state);
    if (RP_OK != result) {
        RP_LOG(context,LOG_ERR, "*SPI:MSG#:CS? Failed to get cs state: %d", result);
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultBool(context, cs_state);

    RP_LOG(context,LOG_INFO, "*SPI:MSG#:CS? Successfully returned cs state.");
    return SCPI_RES_OK;
}

scpi_result_t RP_SPI_Set(scpi_t * context,bool cs_state,bool init_rx,const char *func){
    size_t index = 0;
    size_t size = 0;
    uint8_t *buffer = NULL;
    int32_t cmd[2] = {0,0};


    if (!SCPI_CommandNumbers(context,cmd,2,-1)){
        RP_LOG(context,LOG_ERR, "*%s Failed to get parameters.",func);
        return SCPI_RES_ERR;
    }

    if (cmd[0] == -1){
        RP_LOG(context,LOG_ERR, "*%s Failed to get index of message.",func);
        return SCPI_RES_ERR;
    }

    if (cmd[1] == -1){
        RP_LOG(context,LOG_ERR, "*%s Failed to get size of buffer.",func);
        return SCPI_RES_ERR;
    }

    index = cmd[0];
    size = cmd[1];

    buffer = (uint8_t*)malloc(size * sizeof(uint8_t));
    if (!buffer){
        RP_LOG(context,LOG_ERR, "*%s Failed allocate buffer with size: %d.",func,size);
        return SCPI_RES_ERR;
    }
    size_t buf_size = size;
    if(!SCPI_ParamBufferUInt8(context, buffer, &buf_size, true)){
        RP_LOG(context,LOG_ERR, "*%s Failed get data for buffer.",func);
        free(buffer);
        return SCPI_RES_ERR;
    }

    if (buf_size != size){
        RP_LOG(context,LOG_ERR, "*%s Wrong data length.",func);
        free(buffer);
        return SCPI_RES_ERR;
    }

    int result = rp_SPI_SetBufferForMessage(index,buffer,init_rx,buf_size,cs_state);
    if (RP_OK != result) {
        RP_LOG(context,LOG_ERR, "*%s Failed to set TX buffer: %d",func, result);
        free(buffer);
        return SCPI_RES_ERR;
    }
    free(buffer);
    RP_LOG(context,LOG_INFO, "*%s Successfully set TX buffer.",func);
    return SCPI_RES_OK;
}

scpi_result_t RP_SPI_SetTX(scpi_t * context){
     const char func[] = "SPI:MSG#:TX#";
    return RP_SPI_Set(context,false,false,func);
}

scpi_result_t RP_SPI_SetTXCS(scpi_t * context){
    const char func[] = "SPI:MSG#:TX#:CS";
    return RP_SPI_Set(context,true,false,func);
}

scpi_result_t RP_SPI_SetTXRX(scpi_t * context){
    const char func[] = "SPI:MSG#:TX#:RX";
    return RP_SPI_Set(context,false,true,func);
}

scpi_result_t RP_SPI_SetTXRXCS(scpi_t * context){
    const char func[] = "SPI:MSG#:TX#:RX:CS";
    return RP_SPI_Set(context,true,true,func);
}

scpi_result_t RP_SPI_SetRX2(scpi_t * context,bool cs_state,const char func[]){
    size_t index = 0;
    size_t size = 0;
    int32_t cmd[2] = {0,0};


    if (!SCPI_CommandNumbers(context,cmd,2,-1)){
        RP_LOG(context,LOG_ERR, "*%s Failed to get parameters.",func);
        return SCPI_RES_ERR;
    }

    if (cmd[0] == -1){
        RP_LOG(context,LOG_ERR, "*%s Failed to get index of message.",func);
        return SCPI_RES_ERR;
    }

    if (cmd[1] == -1){
        RP_LOG(context,LOG_ERR, "*%s Failed to get size of buffer.",func);
        return SCPI_RES_ERR;
    }

    index = cmd[0];
    size = cmd[1];

    int result = rp_SPI_SetBufferForMessage(index,NULL,true,size,cs_state);
    if (RP_OK != result) {
        RP_LOG(context,LOG_ERR, "*%s Failed to set RX buffer: %d",func, result);
        return SCPI_RES_ERR;
    }
    RP_LOG(context,LOG_INFO, "*%s Successfully set RX buffer.",func);
    return SCPI_RES_OK;
}

scpi_result_t RP_SPI_SetRX(scpi_t * context){
    const char func[] = "SPI:MSG#:RX#";
    return RP_SPI_SetRX2(context,false,func);
}

scpi_result_t RP_SPI_SetRXCS(scpi_t * context){
    const char func[] = "SPI:MSG#:RX#:CS";
    return RP_SPI_SetRX2(context,true,func);
}

scpi_result_t RP_SPI_SetMode(scpi_t * context){
    int32_t value;

    if (!SCPI_ParamChoice(context, scpi_mode, &value, true)) {
        RP_LOG(context,LOG_ERR, "*SPI:SETtings:MODE is missing first parameter.");
        return SCPI_RES_ERR;
    }

    rp_spi_mode_t par = (rp_spi_mode_t)value;

    int result = rp_SPI_SetMode(par);
    if (RP_HW_OK != result) {
        RP_LOG(context,LOG_ERR, "*SPI:SETtings:MODE Failed to set mode: %d", result);
        return SCPI_RES_ERR;
    }

    RP_LOG(context,LOG_INFO, "*SPI:SETtings:MODE Successfully set spi mode.");
    return SCPI_RES_OK;
}

scpi_result_t RP_SPI_GetMode(scpi_t * context){
    const char *_name;

    rp_spi_mode_t value;
    int result = rp_SPI_GetMode(&value);

    if (RP_OK != result) {
        RP_LOG(context,LOG_ERR, "*SPI:SETtings:MODE? Failed to get spi mode: %d", result);
        return SCPI_RES_ERR;
    }

    if(!SCPI_ChoiceToName(scpi_mode, value, &_name)){
        RP_LOG(context,LOG_ERR, "*SPI:SETtings:MODE? Failed to parse mode.");
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultMnemonic(context, _name);


    RP_LOG(context,LOG_INFO, "*SPI:SETtings:MODE? Successfully returned character size.");
    return SCPI_RES_OK;
}

scpi_result_t RP_SPI_SetCSMode(scpi_t * context){
    int32_t value;

    if (!SCPI_ParamChoice(context, scpi_cs_mode, &value, true)) {
        RP_LOG(context,LOG_ERR, "*SPI:SETtings:CSMODE is missing first parameter.");
        return SCPI_RES_ERR;
    }

    rp_spi_cs_mode_t par = (rp_spi_cs_mode_t)value;

    int result = rp_SPI_SetCSMode(par);
    if (RP_HW_OK != result) {
        RP_LOG(context,LOG_ERR, "*SPI:SETtings:CSMODE Failed to set cs mode: %d", result);
        return SCPI_RES_ERR;
    }

    RP_LOG(context,LOG_INFO, "*SPI:SETtings:CSMODE Successfully set spi cs mode.");
    return SCPI_RES_OK;
}

scpi_result_t RP_SPI_GetCSMode(scpi_t * context){
    const char *_name;

    rp_spi_cs_mode_t value;
    int result = rp_SPI_GetCSMode(&value);

    if (RP_OK != result) {
        RP_LOG(context,LOG_ERR, "*SPI:SETtings:CSMODE? Failed to get spi cs mode: %d", result);
        return SCPI_RES_ERR;
    }

    if(!SCPI_ChoiceToName(scpi_cs_mode, value, &_name)){
        RP_LOG(context,LOG_ERR, "*SPI:SETtings:CSMODE? Failed to parse cs mode.");
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultMnemonic(context, _name);


    RP_LOG(context,LOG_INFO, "*SPI:SETtings:CSMODE? Successfully returned character size.");
    return SCPI_RES_OK;
}

scpi_result_t RP_SPI_SetSpeed(scpi_t * context){
    uint32_t value;

    if (!SCPI_ParamUInt32(context, &value, true)) {
        RP_LOG(context,LOG_ERR, "*SPI:SETtings:SPEED is missing first parameter.");
        return SCPI_RES_ERR;
    }

    int result = rp_SPI_SetSpeed(value);
    if (RP_HW_OK != result) {
        RP_LOG(context,LOG_ERR, "*SPI:SETtings:SPEED Failed to set spi speed: %d", result);
        return SCPI_RES_ERR;
    }

    RP_LOG(context,LOG_INFO, "*SPI:SETtings:SPEED Successfully set spi speed.");
    return SCPI_RES_OK;
}

scpi_result_t RP_SPI_GetSpeed(scpi_t * context){
    int32_t speed;
    int result = rp_SPI_GetSpeed(&speed);

    if (RP_OK != result) {
        RP_LOG(context,LOG_ERR, "*SPI:SETtings:SPEED? Failed to get spi speed: %d", result);
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultUInt32Base(context, speed, 10);

    RP_LOG(context,LOG_INFO, "*SPI:SETtings:SPEED? Successfully returned speed.");
    return SCPI_RES_OK;
}

scpi_result_t RP_SPI_SetWord(scpi_t * context){
    uint32_t value;

    if (!SCPI_ParamUInt32(context, &value, true)) {
        RP_LOG(context,LOG_ERR, "*SPI:SETtings:WORD is missing first parameter.");
        return SCPI_RES_ERR;
    }

    int result = rp_SPI_SetWordLen(value);
    if (RP_HW_OK != result) {
        RP_LOG(context,LOG_ERR, "*SPI:SETtings:WORD Failed to set word length: %d", result);
        return SCPI_RES_ERR;
    }

    RP_LOG(context,LOG_INFO, "*SPI:SETtings:WORD Successfully set word length.");
    return SCPI_RES_OK;
}

scpi_result_t RP_SPI_GetWord(scpi_t * context){
    int32_t len;
    int result = rp_SPI_GetWordLen(&len);

    if (RP_OK != result) {
        RP_LOG(context,LOG_ERR, "*SPI:SETtings:WORD? Failed to get word length: %d", result);
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultUInt32Base(context, len, 10);

    RP_LOG(context,LOG_INFO, "*SPI:SETtings:WORD? Successfully returned speed.");
    return SCPI_RES_OK;
}

scpi_result_t RP_SPI_Pass(scpi_t * context){
    int result = rp_SPI_ReadWrite();
    if (RP_HW_OK != result) {
        RP_LOG(context,LOG_ERR, "*SPI:PASS Failed pass message to spi: %d" , result);
        return SCPI_RES_ERR;
    }

    RP_LOG(context,LOG_INFO, "*SPI:PASS Successfully pass message to spi.");
    return SCPI_RES_OK;
}
