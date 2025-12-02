/**
 * $Id: $
 *
 * @brief Red Pitaya Uart Module.
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 */

#ifndef __UART_H
#define __UART_H


#include "rp_hw.h"

int uart_Init();
int uart_InitDevice(char *_device);
int uart_SetSettings();
int uart_Release();

int uart_Timeout(uint8_t deca_sec);
uint8_t uart_GetTimeout();

int uart_SetSpeed(int _speed);
int uart_GetSpeed();

int uart_SetBits(rp_uart_bits_size_t _size);
rp_uart_bits_size_t uart_GetBits();

int uart_SetStopBits(rp_uart_stop_bits_t mode);
rp_uart_stop_bits_t uart_GetStopBits();

int uart_SetParityMode(rp_uart_parity_t mode);
rp_uart_parity_t uart_GetParityMode();


int uart_read(unsigned char *_buffer,int *size);
int uart_write(unsigned char *_buffer, int size);

#endif
