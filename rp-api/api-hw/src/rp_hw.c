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
#include "i2c.h"

int rp_UartInit(){
    return uart_Init();
}

int rp_UartRelease(){
    return uart_Release();
}

int rp_UartSetSettings(){
    return uart_SetSettings();
}

int rp_UartRead(unsigned char *buffer, int *size){
    return uart_read(buffer,size);
}

int rp_UartWrite(unsigned char *buffer, int size){
    return uart_write(buffer,size);
}

int rp_UartSetSpeed(int value){
    return uart_SetSpeed(value);
}

int rp_UartGetSpeed(int *value){
    *value = uart_GetSpeed(value);
    return RP_HW_OK;
}

int rp_UartSetBits(rp_uart_bits_size_t _size){
    return uart_SetBits(_size);
}

int rp_UartGetBits(rp_uart_bits_size_t *value){
    *value = uart_GetBits();
    return RP_HW_OK;
}

int rp_UartSetStopBits(rp_uart_stop_bits_t _size){
    return uart_SetStopBits(_size);
}

int rp_UartGetStopBits(rp_uart_stop_bits_t *value){
    *value = uart_GetStopBits();
    return RP_HW_OK;
}

int rp_UartSetParityMode(rp_uart_parity_t mode){
    return uart_SetParityMode(mode);
}

int rp_UartGetParityMode(rp_uart_parity_t *value){
    *value = uart_GetParityMode();
    return RP_HW_OK;
}

int rp_UartSetTimeout(uint8_t deca_sec){
    return uart_Timeout(deca_sec);
}

int rp_UartGetTimeout(uint8_t *value){
    *value = uart_GetTimeout();
    return RP_HW_OK;
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

int rp_SPI_ReadWrite(void *tx_buffer, void *rx_buffer, unsigned int length){
    return spi_ReadWrite(tx_buffer,rx_buffer,length);
}

int rp_I2C_InitDevice(char *_device,uint8_t addr){
    return i2c_InitDevice(_device,addr);
}

int rp_I2C_setForceMode(bool force){
    return i2c_setForceMode(force);
}

int rp_I2C_getForceMode(bool *value){
    *value = i2c_getForceMode();
    return RP_HW_OK;
}

int rp_I2C_getDevAddress(int *address){
    *address = i2c_getDevAddress();
    return RP_HW_OK;
}

int rp_I2C_Read(uint8_t reg,uint8_t *value){
    return i2c_Read(reg,value);
}

int rp_I2C_ReadWord(uint8_t reg,uint16_t *value){
    return i2c_ReadWord(reg,value);
}

int rp_I2C_ReadCommand(uint8_t *value){
    return i2c_ReadCommand(value);
}

int rp_I2C_ReadBuffer(uint8_t reg, uint8_t *buffer, int *len){
    return i2c_ReadBuffer(reg,buffer,len);
}

int rp_I2C_Write(uint8_t reg,uint8_t value){
    return i2c_Write(reg,value);
}

int rp_I2C_WriteWord(uint8_t reg,uint16_t value){
    return i2c_WriteWord(reg,value);
}

int rp_I2C_WriteCommand(uint8_t value){
    return i2c_WriteCommand(value);
}

int rp_I2C_WriteBuffer(uint8_t reg, uint8_t *buffer, int len){
    return rp_I2C_WriteBuffer(reg,buffer,len);
}