/**
 * $Id: $
 *
 * @brief Red Pitaya Scpi server SCPI
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */

#ifndef SCPI_PARSER_EXT_H_
#define SCPI_PARSER_EXT_H_

#include "scpi/scpi.h"
#include "uart_protocol.h"

struct user_context_t {
    int fd = -1;
    bool binary_format = false;
};

int SCPI_Error(scpi_t* context, int_fast16_t err);

size_t writeDataEx(scpi_t* context, const char* data, size_t len);
size_t SCPI_Write(scpi_t* context, const char* data, size_t len);
size_t SCPI_WriteUartProtocol(scpi_t* context, const char* data, size_t len);

scpi_result_t SCPI_Reset(scpi_t* context);
scpi_result_t SCPI_Flush(scpi_t* context);
scpi_result_t SCPI_Control(scpi_t* context, scpi_ctrl_name_t ctrl, scpi_reg_val_t val);
scpi_result_t SCPI_SystemCommTcpipControlQ(scpi_t* context);
scpi_result_t SCPI_SystemErrorNextQEx(scpi_t* context);
scpi_result_t SCPI_SystemErrorCountQEx(scpi_t* context);
scpi_result_t SCPI_CoreClsEx(scpi_t* context);

scpi_bool_t SCPI_ParamUInt8(scpi_t* context, uint8_t* value, scpi_bool_t mandatory);
scpi_bool_t SCPI_ParamBufferFloat(scpi_t* context, float* data, uint32_t* size, scpi_bool_t mandatory);
scpi_bool_t SCPI_ParamBufferUInt8(scpi_t* context, uint8_t* data, uint32_t* size, scpi_bool_t mandatory);

size_t SCPI_ResultBufferInt16(scpi_t* context, const int16_t* data, size_t size);
size_t SCPI_ResultBufferUInt8(scpi_t* context, const uint8_t* data, size_t size);
size_t SCPI_ResultBufferFloat(scpi_t* context, const float* data, uint32_t size);

size_t writeBinHeader(scpi_t* context, uint32_t numElems, size_t sizeOfElem);

UARTProtocol* getUARTProtocol();

#endif /* SCPI_PARSER_EXT_H_ */
