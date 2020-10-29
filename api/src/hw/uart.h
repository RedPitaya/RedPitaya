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


int uart_Init();
int uart_InitDevice(char *_device);
int uart_SetDefaultSettings();
int uart_Release();

int uart_read(bool _wait_data, unsigned char *_buffer,int *size);
int uart_write(unsigned char *_buffer, int size);

#endif
