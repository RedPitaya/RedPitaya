/**
 * $Id: $
 *
 * @brief Red Pitaya library API Hardware interface implementation
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
#include <stdint.h>

#include "uart.h"
#include "led_system.h"

int rp_UartInit(){
    return uart_Init();
}

int rp_UartRelease(){
    return uart_Release();
}

int rp_UartRead(unsigned char *buffer, int *size){
    return uart_read(buffer,size);
}

int rp_UartWrite(unsigned char *buffer, int size){
    return uart_write(buffer,size);
}

int rp_UartSpeed(int value){
    return uart_SetSpeed(value);
}

int rp_UartSetBits(rp_uart_bits_size_t _size){
    return uart_SetBits(_size);
}

int rp_UartSetStopBits(rp_uart_stop_bits_t _size){
    return uart_SetStopBits(_size);
}

int rp_UartSetParityMode(rp_uart_parity_t mode){
    return uart_SetParityMode(mode);
}

int rp_GetLEDMMCState(bool *_enable){
    return led_GetMMCState(_enable);
}

int rp_SetLEDMMCState(bool _enable){
    return led_SetMMCState(_enable);
}

int rp_GetLEDHeartBeatState(bool *_enable){
    return led_GetHeartBeatState(_enable);
}

int rp_SetLEDHeartBeatState(bool _enable){
    return led_SetHeartBeatState(_enable);
}

int rp_GetLEDEthState(bool *_state){
    return led_GetEthState(_state);
}

int rp_SetLEDEthState(bool _state){
    return led_SetEthState(_state);
}
