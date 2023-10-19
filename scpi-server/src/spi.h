/**
 * $Id: $
 *
 * @brief Red Pitaya Scpi server SPI SCPI commands interface
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 */


#ifndef RPHW_SPI_H_
#define RPHW_SPI_H_

#include "scpi/types.h"

scpi_result_t RP_SPI_Init(scpi_t * context);
scpi_result_t RP_SPI_InitDev(scpi_t *context);
scpi_result_t RP_SPI_Release(scpi_t * context);
scpi_result_t RP_SPI_SetDefault(scpi_t * context);
scpi_result_t RP_SPI_SetSettings(scpi_t * context);
scpi_result_t RP_SPI_GetSettings(scpi_t * context);

scpi_result_t RP_SPI_CreateMessage(scpi_t * context);
scpi_result_t RP_SPI_DestroyMessage(scpi_t * context);
scpi_result_t RP_SPI_GetMessageLenQ(scpi_t * context);
scpi_result_t RP_SPI_GetRXBufferQ(scpi_t * context);
scpi_result_t RP_SPI_GetTXBufferQ(scpi_t * context);
scpi_result_t RP_SPI_GetCSChangeStateQ(scpi_t * context);


scpi_result_t RP_SPI_SetTX(scpi_t * context);
scpi_result_t RP_SPI_SetTXRX(scpi_t * context);
scpi_result_t RP_SPI_SetRX(scpi_t * context);
scpi_result_t RP_SPI_SetTXCS(scpi_t * context);
scpi_result_t RP_SPI_SetTXRXCS(scpi_t * context);
scpi_result_t RP_SPI_SetRXCS(scpi_t * context);

scpi_result_t RP_SPI_SetMode(scpi_t * context);
scpi_result_t RP_SPI_GetModeQ(scpi_t * context);

scpi_result_t RP_SPI_SetCSMode(scpi_t * context);
scpi_result_t RP_SPI_GetCSModeQ(scpi_t * context);

scpi_result_t RP_SPI_SetSpeed(scpi_t * context);
scpi_result_t RP_SPI_GetSpeedQ(scpi_t * context);

scpi_result_t RP_SPI_SetWord(scpi_t * context);
scpi_result_t RP_SPI_GetWordQ(scpi_t * context);

scpi_result_t RP_SPI_Pass(scpi_t * context);

#endif /* RPHW_UART_H_ */
