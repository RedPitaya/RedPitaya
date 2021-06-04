/**
 * $Id: $
 *
 * @brief Red Pitaya Uart Module.
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef __UART_H
#define __UART_H


#include "rp_cross.h"

int uart_Init();
int uart_InitDevice(char *_device);
int uart_SetDefaultSettings();
int uart_Release();

int uart_SetSpeed(int _speed);
int uart_GetSpeedType(int _speed);

int uart_SetBits(rp_uart_bits_size_t _size);
int uart_SetStopBits(rp_uart_stop_bits_t mode);
int uart_SetParityMode(rp_uart_parity_t mode);


int uart_read(unsigned char *_buffer,int *size);
int uart_write(unsigned char *_buffer, int size);

#endif
