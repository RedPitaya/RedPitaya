/**
 * $Id: $
 *
 * @brief Red Pitaya SPI Module.
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 */

#ifndef __SPI_H
#define __SPI_H


#include "rp_hw.h"

int spi_Init();
int spi_InitDevice(const char *_device);
int spi_SetDefaultSettings();
int spi_GetSettings();
int spi_SetSettings();
int spi_Release();

int spi_CreateMessage(size_t len);
int spi_GetMessageLen(size_t *len);
int spi_GetRxBuffer(size_t msg,const uint8_t **buffer,size_t *len);
int spi_GetTxBuffer(size_t msg,const uint8_t **buffer,size_t *len);
int spi_GetCSChangeState(size_t msg,bool *cs_change);
int spi_SetBufferForMessage(size_t msg,const uint8_t *tx_buffer,bool init_rx_buffer,size_t len, bool cs_change);
int spi_DestoryMessage();

int spi_GetMode(rp_spi_mode_t *mode);
int spi_SetMode(rp_spi_mode_t mode);

int spi_GetState(rp_spi_state_t *state);
int spi_SetState(rp_spi_state_t state);

int spi_GetCSMode(rp_spi_cs_mode_t  *mode);
int spi_SetCSMode(rp_spi_cs_mode_t mode);

int spi_GetOrderBit(rp_spi_order_bit_t *order);
int spi_SetOrderBit(rp_spi_order_bit_t order);

int spi_GetSpeed(int *speed);
int spi_SetSpeed(int speed);

int spi_GetWordLen(int *len);
int spi_SetWordLen(int len);

int spi_ReadWrite();

#endif
