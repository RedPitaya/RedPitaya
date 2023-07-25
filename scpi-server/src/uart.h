/**
 * $Id: $
 *
 * @brief Red Pitaya Scpi server UART SCPI commands interface
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */


#ifndef SCPI_UART_H_
#define SCPI_UART_H_

#include "scpi/types.h"

scpi_result_t RP_Uart_Init(scpi_t * context);
scpi_result_t RP_Uart_Release(scpi_t * context);
scpi_result_t RP_Uart_SetSettings(scpi_t * context);
scpi_result_t RP_Uart_BIT_Size(scpi_t *context);
scpi_result_t RP_Uart_BIT_SizeQ(scpi_t *context);
scpi_result_t RP_Uart_Speed(scpi_t *context);
scpi_result_t RP_Uart_SpeedQ(scpi_t *context);
scpi_result_t RP_Uart_STOP_Bit(scpi_t *context);
scpi_result_t RP_Uart_STOP_BitQ(scpi_t *context);
scpi_result_t RP_Uart_PARITY(scpi_t *context);
scpi_result_t RP_Uart_PARITYQ(scpi_t *context);
scpi_result_t RP_Uart_Timeout(scpi_t *context);
scpi_result_t RP_Uart_TimeoutQ(scpi_t *context);
scpi_result_t RP_Uart_SendBuffer(scpi_t * context);
scpi_result_t RP_Uart_ReadBuffer(scpi_t * context);

#endif /* SCPI_UART_H_ */
