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
#include "spi.h"
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


int rp_SPI_Init(){
    return spi_Init();
}

int rp_SPI_InitDevice(char *_device){
    return spi_InitDevice(_device);
}

int rp_SPI_SetDefaultSettings(){
    return spi_SetDefaultSettings();
}

int rp_SPI_GetSetings(){
    return spi_GetSettings();
}

int rp_SPI_SetSettings(){
    return spi_SetSettings();
}

int rp_SPI_Release(){
    return spi_Release();
}

int rp_SPI_GetMode(rp_spi_mode_t *mode){
    return spi_GetMode(mode);
}

int rp_SPI_SetMode(rp_spi_mode_t mode){
    return spi_SetMode(mode);
}

int rp_SPI_GetState(rp_spi_state_t *state){
    return spi_GetState(state);
}

int rp_SPI_SetState(rp_spi_state_t state){
    return spi_SetState(state);
}

int rp_SPI_GetOrderBit(rp_spi_order_bit_t *order){
    return spi_GetOrderBit(order);
}

int rp_SPI_SetOrderBit(rp_spi_order_bit_t order){
    return spi_SetOrderBit(order);
}

int rp_SPI_GetSpeed(int *speed){
    return spi_GetSpeed(speed);
}

int rp_SPI_SetSpeed(int speed){
    return spi_SetSpeed(speed);
}

int rp_SPI_GetWordLen(int *len){
    return spi_GetWordLen(len);
}

int rp_SPI_SetWordLen(int len){
    return spi_SetWordLen(len);
}

int rp_SPI_ReadWrite(void *tx_buffer, void *rx_buffer, size_t length){
    return spi_ReadWrite(tx_buffer,rx_buffer,length);
}