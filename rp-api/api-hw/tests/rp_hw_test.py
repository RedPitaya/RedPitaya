#!/usr/bin/python3


import rp_hw


print("For test need connect with wire RX<->TX and MOSI<->MISO")

print("rp_hw.rp_UartInit()")
res = rp_hw.rp_UartInit()
print(res)

print("rp_hw.rp_UartSetSettings()")
res = rp_hw.rp_UartSetSettings()
print(res)

print("rp_hw.rp_UartSetTimeout(10)")
res = rp_hw.rp_UartSetTimeout(10)
print(res)

print("rp_hw.rp_UartGetTimeout()")
res = rp_hw.rp_UartGetTimeout()
print(res)

print("rp_hw.rp_UartSetSpeed(9600)")
res = rp_hw.rp_UartSetSpeed(9600)
print(res)

print("rp_hw.rp_UartGetSpeed()")
res = rp_hw.rp_UartGetSpeed()
print(res)

print("rp_hw.rp_UartSetBits(rp_hw.RP_UART_CS7)")
res = rp_hw.rp_UartSetBits(rp_hw.RP_UART_CS7)
print(res)

print("rp_hw.rp_UartGetBits()")
res = rp_hw.rp_UartGetBits()
print(res)

print("rp_hw.rp_UartSetStopBits(rp_hw.RP_UART_STOP2)")
res = rp_hw.rp_UartSetStopBits(rp_hw.RP_UART_STOP2)
print(res)

print("rp_hw.rp_UartGetStopBits()")
res = rp_hw.rp_UartGetStopBits()
print(res)

print("rp_hw.rp_UartSetParityMode(rp_hw.RP_UART_MARK)")
res = rp_hw.rp_UartSetParityMode(rp_hw.RP_UART_MARK)
print(res)

print("rp_hw.rp_UartGetParityMode()")
res = rp_hw.rp_UartGetParityMode()
print(res)

# Disable timeout
print("rp_hw.rp_UartSetTimeout(0)")
res = rp_hw.rp_UartSetTimeout(0)
print(res)


buff = rp_hw.Buffer(10)
buff_size = 10

buff2 = rp_hw.Buffer(10)
buff_size2 = 10

buff[0] = 1
buff[1] = 2
buff[2] = 3


print("rp_hw.rp_UartWrite(buff,buff_size)")
res = rp_hw.rp_UartWrite(buff,buff_size)
print(res)

print("rp_hw.rp_UartRead(buff,buff_size)")
res = rp_hw.rp_UartRead(buff2,buff_size2)
print("Result:",res[0],"Size:",res[1])
print(buff2[0],buff2[1],buff2[2])

print("rp_hw.rp_UartRelease()")
res = rp_hw.rp_UartRelease()
print(res)



print("rp_hw.rp_SetLEDMMCState(False)")
res = rp_hw.rp_SetLEDMMCState(False)
print(res)

print("rp_hw.rp_GetLEDMMCState()")
res = rp_hw.rp_GetLEDMMCState()
print(res)

print("rp_hw.rp_SetLEDMMCState(True)")
res = rp_hw.rp_SetLEDMMCState(True)
print(res)


print("rp_hw.rp_SetLEDHeartBeatState(False)")
res = rp_hw.rp_SetLEDHeartBeatState(False)
print(res)

print("rp_hw.rp_GetLEDHeartBeatState()")
res = rp_hw.rp_GetLEDHeartBeatState()
print(res)

print("rp_hw.rp_SetLEDHeartBeatState(True)")
res = rp_hw.rp_SetLEDHeartBeatState(True)
print(res)


print("rp_hw.rp_SetLEDEthState(False)")
res = rp_hw.rp_SetLEDEthState(False)
print(res)

print("rp_hw.rp_GetLEDEthState()")
res = rp_hw.rp_GetLEDEthState()
print(res)

print("rp_hw.rp_SetLEDEthState(True)")
res = rp_hw.rp_SetLEDEthState(True)
print(res)


print("rp_hw.rp_SPI_Init()")
res = rp_hw.rp_SPI_Init()
print(res)


print("rp_hw.rp_SPI_Release()")
res = rp_hw.rp_SPI_Release()
print(res)


print("rp_hw.rp_SPI_InitDevice()")
res = rp_hw.rp_SPI_InitDevice("/dev/spidev1.0")
print(res)

print("rp_hw.rp_SPI_SetDefaultSettings()")
res = rp_hw.rp_SPI_SetDefaultSettings()
print(res)

print("rp_hw.rp_SPI_GetSettings()")
res = rp_hw.rp_SPI_GetSettings()
print(res)

print("rp_hw.rp_SPI_SetMode(rp_hw.RP_SPI_MODE_HISL)")
res = rp_hw.rp_SPI_SetMode(rp_hw.RP_SPI_MODE_HISL)
print(res)

print("rp_hw.rp_SPI_GetMode()")
res = rp_hw.rp_SPI_GetMode()
print(res)

print("rp_hw.rp_SPI_SetState(rp_hw.RP_SPI_STATE_NOT)")
res = rp_hw.rp_SPI_SetState(rp_hw.RP_SPI_STATE_NOT)
print(res)

print("rp_hw.rp_SPI_GetState()")
res = rp_hw.rp_SPI_GetState()
print(res)

print("rp_hw.rp_SPI_SetOrderBit(rp_hw.RP_SPI_ORDER_BIT_MSB)")
res = rp_hw.rp_SPI_SetOrderBit(rp_hw.RP_SPI_ORDER_BIT_MSB)
print(res)

print("rp_hw.rp_SPI_GetOrderBit()")
res = rp_hw.rp_SPI_GetOrderBit()
print(res)

print("rp_hw.rp_SPI_SetCSMode(rp_hw.RP_SPI_CS_HIGH)")
res = rp_hw.rp_SPI_SetCSMode(rp_hw.RP_SPI_CS_HIGH)
print(res)

print("rp_hw.rp_SPI_GetCSMode()")
res = rp_hw.rp_SPI_GetCSMode()
print(res)


print("rp_hw.rp_SPI_SetSpeed(100000)")
res = rp_hw.rp_SPI_SetSpeed(100000)
print(res)

print("rp_hw.rp_SPI_GetSpeed()")
res = rp_hw.rp_SPI_GetSpeed()
print(res)

print("rp_hw.rp_SPI_SetWordLen(8)")
res = rp_hw.rp_SPI_SetWordLen(8)
print(res)

print("rp_hw.rp_SPI_GetWordLen()")
res = rp_hw.rp_SPI_GetWordLen()
print(res)


print("rp_hw.rp_SPI_SetSettings()")
res = rp_hw.rp_SPI_SetSettings()
print(res)


print("rp_hw.rp_SPI_CreateMessage(1)")
res = rp_hw.rp_SPI_CreateMessage(1)
print(res)

print("rp_hw.rp_SPI_GetMessageLen()")
res = rp_hw.rp_SPI_GetMessageLen()
print(res)

print("rp_hw.rp_SPI_GetCSChangeState(0)")
res = rp_hw.rp_SPI_GetCSChangeState(0)
print(res)

tx_buff = rp_hw.Buffer(10)
tx_buff[0] = 1
tx_buff[1] = 2
tx_buff[2] = 3

print("rp_hw.rp_SPI_SetBufferForMessage(0,tx_buff,True ,10, True)")
res = rp_hw.rp_SPI_SetBufferForMessage(0,tx_buff,True ,10, True)
print(res)

print("rp_hw.rp_SPI_ReadWrite()")
res = rp_hw.rp_SPI_ReadWrite()
print(res)

print("rp_hw.rp_SPI_GetRxBuffer(0)")
res = rp_hw.rp_SPI_GetRxBuffer(0)
print(res)
tmp = rp_hw.Buffer_frompointer(res[1])
print("RxBuffer",tmp[0],tmp[1],tmp[2])

print("rp_hw.rp_SPI_GetTxBuffer(0)")
res = rp_hw.rp_SPI_GetTxBuffer(0)
print(res)
tmp = rp_hw.Buffer_frompointer(res[1])
print("TxBuffer",tmp[0],tmp[1],tmp[2])

print("rp_hw.rp_SPI_DestoryMessage()")
res = rp_hw.rp_SPI_DestoryMessage()
print(res)


print("rp_hw.rp_SPI_Release()")
res = rp_hw.rp_SPI_Release()
print(res)


print("rp_hw.rp_I2C_InitDevice('/dev/i2c-0',80)")
res = rp_hw.rp_I2C_InitDevice("/dev/i2c-0",80)
print(res)

print("rp_hw.rp_I2C_setForceMode(True)")
res = rp_hw.rp_I2C_setForceMode(True)
print(res)

print("rp_hw.rp_I2C_getForceMode()")
res = rp_hw.rp_I2C_getForceMode()
print(res)

print("rp_hw.rp_I2C_getDevAddress()")
res = rp_hw.rp_I2C_getDevAddress()
print(res)

print("rp_hw.rp_I2C_SMBUS_Read(0)")
res = rp_hw.rp_I2C_SMBUS_Read(0)
print(res)

print("rp_hw.rp_I2C_SMBUS_ReadWord(0)")
res = rp_hw.rp_I2C_SMBUS_ReadWord(0)
print(res)

print("rp_hw.rp_I2C_SMBUS_ReadCommand()")
res = rp_hw.rp_I2C_SMBUS_ReadCommand()
print(res)

rx_buff = rp_hw.Buffer(10)
rx_len = 10
print("rp_hw.rp_I2C_SMBUS_ReadBuffer(0, rx_buff,rx_len)")
res = rp_hw.rp_I2C_SMBUS_ReadBuffer(0,rx_buff,rx_len)
print(res)

print("rp_hw.rp_I2C_SMBUS_Write(0,0)")
res = rp_hw.rp_I2C_SMBUS_Write(0,0)
print(res)

print("rp_hw.rp_I2C_SMBUS_WriteWord(0,0)")
res = rp_hw.rp_I2C_SMBUS_WriteWord(0,0)
print(res)

print("rp_hw.rp_I2C_SMBUS_WriteCommand(0)")
res = rp_hw.rp_I2C_SMBUS_WriteCommand(0)
print(res)

print("rp_hw.rp_I2C_SMBUS_WriteBuffer(0, rx_buff,rx_len)")
res = rp_hw.rp_I2C_SMBUS_WriteBuffer(0,rx_buff,rx_len)
print(res)

print("rp_hw.rp_I2C_IOCTL_ReadBuffer(rx_buff,rx_len)")
res = rp_hw.rp_I2C_IOCTL_ReadBuffer(rx_buff,rx_len)
print(res)

print("rp_hw.rp_I2C_IOCTL_WriteBuffer(rx_buff,rx_len)")
res = rp_hw.rp_I2C_IOCTL_WriteBuffer(rx_buff,rx_len)
print(res)


print("rp_hw.rp_GetCPUTemperature()")
res = rp_hw.rp_GetCPUTemperature()
print(res)

print("rp_hw.rp_GetPowerI4()")
res = rp_hw.rp_GetPowerI4()
print(res)

print("rp_hw.rp_GetPowerVCCPINT()")
res = rp_hw.rp_GetPowerVCCPINT()
print(res)

print("rp_hw.rp_GetPowerVCCPAUX()")
res = rp_hw.rp_GetPowerVCCPAUX()
print(res)

print("rp_hw.rp_GetPowerVCCBRAM()")
res = rp_hw.rp_GetPowerVCCBRAM()
print(res)

print("rp_hw.rp_GetPowerVCCINT()")
res = rp_hw.rp_GetPowerVCCINT()
print(res)

print("rp_hw.rp_GetPowerVCCAUX()")
res = rp_hw.rp_GetPowerVCCAUX()
print(res)

print("rp_hw.rp_GetPowerVCCDDR()")
res = rp_hw.rp_GetPowerVCCDDR()
print(res)