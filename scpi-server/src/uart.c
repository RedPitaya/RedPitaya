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
#include "uart.h"
#include "scpi/parser.h"
#include "rp_hw.h"

const scpi_choice_def_t scpi_BIT_Size[] = {
    {"CS6", RP_UART_CS6},
    {"CS7", RP_UART_CS7},
    {"CS8", RP_UART_CS8},
    SCPI_CHOICE_LIST_END
};

const scpi_choice_def_t scpi_STOP_bit[] = {
    {"STOP1", RP_UART_STOP1},
    {"STOP2", RP_UART_STOP2},
    SCPI_CHOICE_LIST_END
};

const scpi_choice_def_t scpi_PARITY[] = {
    {"NONE", RP_UART_NONE},
    {"EVEN", RP_UART_EVEN},
    {"ODD",  RP_UART_ODD},
    {"MARK", RP_UART_MARK},
    {"SPACE",RP_UART_SPACE},
    SCPI_CHOICE_LIST_END
};

scpi_result_t RP_Uart_Init(scpi_t * context){
    int result = rp_UartInit();
    if (RP_HW_OK != result) {
        RP_LOG(LOG_ERR, "UART:INIT Failed to init Red "
            "Pitaya uart: %d\n" , result);
        return SCPI_RES_ERR;
    }

    RP_LOG(LOG_INFO, "*UART:INIT Successfully init uart.\n");
    return SCPI_RES_OK;
}

scpi_result_t RP_Uart_Release(scpi_t * context){
    int result = rp_UartRelease();
    if (RP_HW_OK != result) {
        RP_LOG(LOG_ERR, "UART:RELEASE Failed to release uart: %d\n" , result);
        return SCPI_RES_ERR;
    }

    RP_LOG(LOG_INFO, "*UART:RELEASE Successfully release uart.\n");
    return SCPI_RES_OK;
}

scpi_result_t RP_Uart_SetSettings(scpi_t * context){
    int result = rp_UartSetSettings();
    if (RP_HW_OK != result) {
        RP_LOG(LOG_ERR, "UART:SETUP Failed set settings uart: %d\n" , result);
        return SCPI_RES_ERR;
    }

    RP_LOG(LOG_INFO, "*UART:SETUP Successfully set settings uart.\n");
    return SCPI_RES_OK;
}


scpi_result_t RP_Uart_BIT_Size(scpi_t *context) {
    int32_t value;

    if (!SCPI_ParamChoice(context, scpi_BIT_Size, &value, true)) {
        RP_LOG(LOG_ERR, "*UART:BITS is missing first parameter.\n");
        return SCPI_RES_ERR;
    }

    rp_uart_bits_size_t par = (rp_uart_bits_size_t)value;

    int result = rp_UartSetBits(par);
    if (RP_HW_OK != result) {
        RP_LOG(LOG_ERR, "*UART:BITS Failed to set character size: %d\n", result);
        return SCPI_RES_ERR;
    }

    RP_LOG(LOG_INFO, "*UART:BITS Successfully set character size.\n");
    return SCPI_RES_OK;
}

scpi_result_t RP_Uart_BIT_SizeQ(scpi_t *context) {
    const char *_name;

    rp_uart_bits_size_t value;
    int result = rp_UartGetBits(&value);

    if (RP_OK != result) {
        RP_LOG(LOG_ERR, "*UART:BITS? Failed to get character size: %d", result);
        return SCPI_RES_ERR;
    }

    if(!SCPI_ChoiceToName(scpi_BIT_Size, value, &_name)){
        RP_LOG(LOG_ERR, "*UART:BITS? Failed to parse character size.\n");
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultMnemonic(context, _name);


    RP_LOG(LOG_INFO, "*UART:BITS? Successfully returned character size.\n");
    return SCPI_RES_OK;
}

scpi_result_t RP_Uart_Speed(scpi_t *context){
    uint32_t value;

    if (!SCPI_ParamUInt32(context, &value, true)) {
        RP_LOG(LOG_ERR, "*UART:SPEED is missing first parameter.\n");
        return SCPI_RES_ERR;
    }

    int result = rp_UartSetSpeed(value);
    if (RP_HW_OK != result) {
        RP_LOG(LOG_ERR, "*UART:SPEED Failed to set uart speed: %d\n", result);
        return SCPI_RES_ERR;
    }

    RP_LOG(LOG_INFO, "*UART:SPEED Successfully set uart speed.\n");
    return SCPI_RES_OK;
}

scpi_result_t RP_Uart_SpeedQ(scpi_t *context){
    int32_t speed;
    int result = rp_UartGetSpeed(&speed);

    if (RP_OK != result) {
        RP_LOG(LOG_ERR, "*UART:SPEED? Failed to get uart speed: %d", result);
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultUInt32Base(context, speed, 10);

    RP_LOG(LOG_INFO, "*UART:SPEED? Successfully returned speed.\n");
    return SCPI_RES_OK;
}

scpi_result_t RP_Uart_STOP_Bit(scpi_t *context) {
    int32_t value;

    if (!SCPI_ParamChoice(context, scpi_STOP_bit, &value, true)) {
        RP_LOG(LOG_ERR, "*UART:STOPB is missing first parameter.\n");
        return SCPI_RES_ERR;
    }

    rp_uart_stop_bits_t par = (rp_uart_stop_bits_t)value;

    int result = rp_UartSetStopBits(par);
    if (RP_HW_OK != result) {
        RP_LOG(LOG_ERR, "*UART:STOPB Failed to set stop bit size: %d\n", result);
        return SCPI_RES_ERR;
    }

    RP_LOG(LOG_INFO, "*UART:STOPB Successfully set stop bit size.\n");
    return SCPI_RES_OK;
}

scpi_result_t RP_Uart_STOP_BitQ(scpi_t *context) {
    const char *_name;

    rp_uart_stop_bits_t value;
    int result = rp_UartGetStopBits(&value);

    if (RP_HW_OK != result) {
        RP_LOG(LOG_ERR, "*UART:STOPB? Failed to get stop bit size: %d", result);
        return SCPI_RES_ERR;
    }

    if(!SCPI_ChoiceToName(scpi_STOP_bit, value, &_name)){
        RP_LOG(LOG_ERR, "*UART:STOPB? Failed to get stop bit size.\n");
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultMnemonic(context, _name);


    RP_LOG(LOG_INFO, "*UART:STOPB? Successfully returned stop bit size.\n");
    return SCPI_RES_OK;
}

scpi_result_t RP_Uart_PARITY(scpi_t *context) {
    int32_t value;

    if (!SCPI_ParamChoice(context, scpi_PARITY, &value, true)) {
        RP_LOG(LOG_ERR, "*UART:PARITY is missing first parameter.\n");
        return SCPI_RES_ERR;
    }

    rp_uart_parity_t par = (rp_uart_parity_t)value;

    int result = rp_UartSetParityMode(par);
    if (RP_HW_OK != result) {
        RP_LOG(LOG_ERR, "*UART:PARITY Failed to set parity mode: %d\n", result);
        return SCPI_RES_ERR;
    }

    RP_LOG(LOG_INFO, "*UART:PARITY Successfully set parity mode.\n");
    return SCPI_RES_OK;
}

scpi_result_t RP_Uart_PARITYQ(scpi_t *context) {
    const char *_name;

    rp_uart_parity_t value;
    int result = rp_UartGetParityMode(&value);

    if (RP_HW_OK != result) {
        RP_LOG(LOG_ERR, "*UART:PARITY? Failed to get parity mode: %d", result);
        return SCPI_RES_ERR;
    }

    if(!SCPI_ChoiceToName(scpi_PARITY, value, &_name)){
        RP_LOG(LOG_ERR, "*UART:PARITY? Failed to get parity mode.\n");
        return SCPI_RES_ERR;
    }

    // Return back result
    SCPI_ResultMnemonic(context, _name);


    RP_LOG(LOG_INFO, "*UART:PARITY? Successfully returned parity mode.\n");
    return SCPI_RES_OK;
}

scpi_result_t RP_Uart_Timeout(scpi_t *context){
    uint8_t value;
    if (!SCPI_ParamUInt8(context, &value, true)) {
        RP_LOG(LOG_ERR, "*UART:TIMEOUT is missing first parameter.\n");
        return SCPI_RES_ERR;
    }

    int result = rp_UartSetTimeout(value);
    if (RP_HW_OK != result) {
        RP_LOG(LOG_ERR, "*UART:TIMEOUT Failed to set uart timeout: %d\n", result);
        return SCPI_RES_ERR;
    }

    RP_LOG(LOG_INFO, "*UART:TIMEOUT Successfully set uart timeout.\n");
    return SCPI_RES_OK;
}

scpi_result_t RP_Uart_TimeoutQ(scpi_t *context){
    uint8_t speed;
    int result = rp_UartGetTimeout(&speed);

    if (RP_OK != result) {
        RP_LOG(LOG_ERR, "*UART:TIMEOUT? Failed to get uart timeout: %d", result);
        return SCPI_RES_ERR;
    }
    // Return back result
    SCPI_ResultUInt32Base(context, speed, 10);

    RP_LOG(LOG_INFO, "*UART:TIMEOUT? Successfully returned timeout.\n");
    return SCPI_RES_OK;
}

scpi_result_t RP_Uart_SendBuffer(scpi_t * context){

    uint8_t *buffer;
    size_t size;
    uint32_t buf_size;
    int result;
    int32_t cmd[1] = {0};

    if (!SCPI_CommandNumbers(context,cmd,1,-1)){
        RP_LOG(LOG_ERR, "*UART:WRITE Failed to get parameters.\n");
        return SCPI_RES_ERR;
    }

    if (cmd[0] == -1){
        RP_LOG(LOG_ERR, "*UART:WRITE Failed to get size.\n");
        return SCPI_RES_ERR;
    }

    size = cmd[0];

    buffer = malloc(size * sizeof(uint8_t));
    if (!buffer){
        RP_LOG(LOG_ERR, "*UART:WRITE Failed allocate buffer with size: %d.\n",size);
        return SCPI_RES_ERR;
    }
    buf_size = size;
    if(!SCPI_ParamBufferUInt8(context, buffer, &buf_size, true)){
        RP_LOG(LOG_ERR, "*UART:WRITE Failed get data for buffer.\n");
        free(buffer);
        return SCPI_RES_ERR;
    }

    if (buf_size != size){
        RP_LOG(LOG_ERR, "*UART:WRITE Wrong data length.\n");
        free(buffer);
        return SCPI_RES_ERR;
    }

    result = rp_UartWrite(buffer, buf_size);
    if(result != RP_HW_OK){
        RP_LOG(LOG_ERR, "*UART:WRITE Failed send data: %d\n", result);
        free(buffer);
        return SCPI_RES_ERR;
    }
    free(buffer);
    RP_LOG(LOG_INFO, "*UART:WRITE Successfully send data to uart.\n");
    return SCPI_RES_OK;
}

scpi_result_t RP_Uart_ReadBuffer(scpi_t * context){
    uint8_t *buffer = 0;
    size_t size = 0;
    int32_t read_size = 0;
    int result;
    int32_t cmd[1] = {0};

    if (!SCPI_CommandNumbers(context,cmd,1,-1)){
        RP_LOG(LOG_ERR, "*UART:READ# Failed to get parameters.\n");
        return SCPI_RES_ERR;
    }

    if (cmd[0] == -1){
        RP_LOG(LOG_ERR, "*UART:READ# Failed to get size.\n");
        return SCPI_RES_ERR;
    }

    size = cmd[0];

    buffer = malloc(size * sizeof(uint8_t));
    if (!buffer){
        RP_LOG(LOG_ERR,"*UART:READ# Failed allocate buffer with size: %d.\n",size);
        return SCPI_RES_ERR;
    }
    read_size = size;
    result = rp_UartRead(buffer, &read_size);
    if(result != RP_HW_OK){
        RP_LOG(LOG_ERR, "*UART:READ# Failed read data: %d\n", result);
        free(buffer);
        return SCPI_RES_ERR;
    }

    SCPI_ResultBufferUInt8(context, buffer, read_size);
    free(buffer);
    RP_LOG(LOG_INFO, "*UART:READ# Successfully returned uart data to client.\n");
    return SCPI_RES_OK;
}