#!/usr/bin/python3

import numpy as np
import rp_hw


print("For test need connect with wire RX<->TX and MOSI<->MISO")

print("rp_hw.rp_HwGetError(rp_hw.RP_HW_EAL)")
res = rp_hw.rp_HwGetError(rp_hw.RP_HW_EAL)
print(res)

print("rp_hw.rp_UartInit()")
res = rp_hw.rp_UartInit()
print(res,rp_hw.rp_HwGetError(res))

print("rp_hw.rp_UartSetSettings()")
res = rp_hw.rp_UartSetSettings()
print(res,rp_hw.rp_HwGetError(res))

print("rp_hw.rp_UartSetTimeout(10)")
res = rp_hw.rp_UartSetTimeout(10)
print(res,rp_hw.rp_HwGetError(res))

print("rp_hw.rp_UartGetTimeout()")
res = rp_hw.rp_UartGetTimeout()
print(res,rp_hw.rp_HwGetError(res[0]))

print("rp_hw.rp_UartSetSpeed(9600)")
res = rp_hw.rp_UartSetSpeed(9600)
print(res,rp_hw.rp_HwGetError(res))

print("rp_hw.rp_UartGetSpeed()")
res = rp_hw.rp_UartGetSpeed()
print(res,rp_hw.rp_HwGetError(res[0]))

print("rp_hw.rp_UartSetBits(rp_hw.RP_UART_CS7)")
res = rp_hw.rp_UartSetBits(rp_hw.RP_UART_CS7)
print(res,rp_hw.rp_HwGetError(res))

print("rp_hw.rp_UartGetBits()")
res = rp_hw.rp_UartGetBits()
print(res,rp_hw.rp_HwGetError(res[0]))

print("rp_hw.rp_UartSetStopBits(rp_hw.RP_UART_STOP2)")
res = rp_hw.rp_UartSetStopBits(rp_hw.RP_UART_STOP2)
print(res,rp_hw.rp_HwGetError(res))

print("rp_hw.rp_UartGetStopBits()")
res = rp_hw.rp_UartGetStopBits()
print(res,rp_hw.rp_HwGetError(res[0]))

print("rp_hw.rp_UartSetParityMode(rp_hw.RP_UART_MARK)")
res = rp_hw.rp_UartSetParityMode(rp_hw.RP_UART_MARK)
print(res,rp_hw.rp_HwGetError(res))

print("rp_hw.rp_UartGetParityMode()")
res = rp_hw.rp_UartGetParityMode()
print(res,rp_hw.rp_HwGetError(res[0]))

# Disable timeout
print("rp_hw.rp_UartSetTimeout(0)")
res = rp_hw.rp_UartSetTimeout(0)
print(res,rp_hw.rp_HwGetError(res))


data = np.array([0x01, 0x02, 0x03], dtype=np.uint8)

print("rp_hw.rp_UartWrite(data)")
res = rp_hw.rp_UartWrite(data)
print(res,rp_hw.rp_HwGetError(res))

buff_size = 10
status, data = rp_hw.rp_UartRead(buff_size)

print(f"Result: {status}")
if status == 0:
    print(f"Read data: {data}")
    if len(data) >= 2:
        print(f"Elements: {data[0]}, {data[1]}, {data[2]}")

print("rp_hw.rp_UartRelease()")
res = rp_hw.rp_UartRelease()
print(res,rp_hw.rp_HwGetError(res))

print("rp_hw.rp_SetLEDMMCState(False)")
res = rp_hw.rp_SetLEDMMCState(False)
print(res,rp_hw.rp_HwGetError(res))

print("rp_hw.rp_GetLEDMMCState()")
res = rp_hw.rp_GetLEDMMCState()
print(res,rp_hw.rp_HwGetError(res[0]))

print("rp_hw.rp_SetLEDMMCState(True)")
res = rp_hw.rp_SetLEDMMCState(True)
print(res,rp_hw.rp_HwGetError(res))


print("rp_hw.rp_SetLEDHeartBeatState(False)")
res = rp_hw.rp_SetLEDHeartBeatState(False)
print(res,rp_hw.rp_HwGetError(res))

print("rp_hw.rp_GetLEDHeartBeatState()")
res = rp_hw.rp_GetLEDHeartBeatState()
print(res,rp_hw.rp_HwGetError(res[0]))

print("rp_hw.rp_SetLEDHeartBeatState(True)")
res = rp_hw.rp_SetLEDHeartBeatState(True)
print(res,rp_hw.rp_HwGetError(res))


print("rp_hw.rp_SetLEDEthState(False)")
res = rp_hw.rp_SetLEDEthState(False)
print(res,rp_hw.rp_HwGetError(res))

print("rp_hw.rp_GetLEDEthState()")
res = rp_hw.rp_GetLEDEthState()
print(res,rp_hw.rp_HwGetError(res[0]))

print("rp_hw.rp_SetLEDEthState(True)")
res = rp_hw.rp_SetLEDEthState(True)
print(res,rp_hw.rp_HwGetError(res))


print("rp_hw.rp_SPI_Init()")
res = rp_hw.rp_SPI_Init()
print(res,rp_hw.rp_HwGetError(res))


print("rp_hw.rp_SPI_Release()")
res = rp_hw.rp_SPI_Release()
print(res,rp_hw.rp_HwGetError(res))


print("rp_hw.rp_SPI_InitDevice()")
res = rp_hw.rp_SPI_InitDevice("/dev/spidev2.0")
print(res,rp_hw.rp_HwGetError(res))

print("rp_hw.rp_SPI_SetDefaultSettings()")
res = rp_hw.rp_SPI_SetDefaultSettings()
print(res,rp_hw.rp_HwGetError(res))

print("rp_hw.rp_SPI_GetSettings()")
res = rp_hw.rp_SPI_GetSettings()
print(res,rp_hw.rp_HwGetError(res))

print("rp_hw.rp_SPI_SetMode(rp_hw.RP_SPI_MODE_HISL)")
res = rp_hw.rp_SPI_SetMode(rp_hw.RP_SPI_MODE_HISL)
print(res,rp_hw.rp_HwGetError(res))

print("rp_hw.rp_SPI_GetMode()")
res = rp_hw.rp_SPI_GetMode()
print(res,rp_hw.rp_HwGetError(res[0]))

print("rp_hw.rp_SPI_SetState(rp_hw.RP_SPI_STATE_NOT)")
res = rp_hw.rp_SPI_SetState(rp_hw.RP_SPI_STATE_NOT)
print(res,rp_hw.rp_HwGetError(res))

print("rp_hw.rp_SPI_GetState()")
res = rp_hw.rp_SPI_GetState()
print(res,rp_hw.rp_HwGetError(res[0]))

print("rp_hw.rp_SPI_SetOrderBit(rp_hw.RP_SPI_ORDER_BIT_MSB)")
res = rp_hw.rp_SPI_SetOrderBit(rp_hw.RP_SPI_ORDER_BIT_MSB)
print(res,rp_hw.rp_HwGetError(res))

print("rp_hw.rp_SPI_GetOrderBit()")
res = rp_hw.rp_SPI_GetOrderBit()
print(res,rp_hw.rp_HwGetError(res[0]))

print("rp_hw.rp_SPI_SetCSMode(rp_hw.RP_SPI_CS_HIGH)")
res = rp_hw.rp_SPI_SetCSMode(rp_hw.RP_SPI_CS_HIGH)
print(res,rp_hw.rp_HwGetError(res))

print("rp_hw.rp_SPI_GetCSMode()")
res = rp_hw.rp_SPI_GetCSMode()
print(res,rp_hw.rp_HwGetError(res[0]))


print("rp_hw.rp_SPI_SetSpeed(100000)")
res = rp_hw.rp_SPI_SetSpeed(100000)
print(res,rp_hw.rp_HwGetError(res))

print("rp_hw.rp_SPI_GetSpeed()")
res = rp_hw.rp_SPI_GetSpeed()
print(res,rp_hw.rp_HwGetError(res[0]))

print("rp_hw.rp_SPI_SetWordLen(8)")
res = rp_hw.rp_SPI_SetWordLen(8)
print(res,rp_hw.rp_HwGetError(res))

print("rp_hw.rp_SPI_GetWordLen()")
res = rp_hw.rp_SPI_GetWordLen()
print(res,rp_hw.rp_HwGetError(res[0]))


print("rp_hw.rp_SPI_SetSettings()")
res = rp_hw.rp_SPI_SetSettings()
print(res,rp_hw.rp_HwGetError(res))


print("rp_hw.rp_SPI_CreateMessage(1)")
res = rp_hw.rp_SPI_CreateMessage(1)
print(res,rp_hw.rp_HwGetError(res))

print("rp_hw.rp_SPI_GetMessageLen()")
res = rp_hw.rp_SPI_GetMessageLen()
print(res,rp_hw.rp_HwGetError(res[0]))

print("rp_hw.rp_SPI_GetCSChangeState(0)")
res = rp_hw.rp_SPI_GetCSChangeState(0)
print(res,rp_hw.rp_HwGetError(res[0]))

tx_data = np.array([0xAA, 0xBB, 0xCC], dtype=np.uint8)

print("rp_hw.rp_SPI_SetBufferForMessage(0, tx_data, True, True)")
res = rp_hw.rp_SPI_SetBufferForMessage(0, tx_data, True, True)
print(res,rp_hw.rp_HwGetError(res))

print("rp_hw.rp_SPI_ReadWrite()")
res = rp_hw.rp_SPI_ReadWrite()
print(res,rp_hw.rp_HwGetError(res))

print("rp_hw.rp_SPI_GetRxBuffer(0)")
res = rp_hw.rp_SPI_GetRxBuffer(0)
print(res[0],res[1])

print("rp_hw.rp_SPI_GetTxBuffer(0)")
res = rp_hw.rp_SPI_GetTxBuffer(0)
print(res[0],res[1])

print("rp_hw.rp_SPI_DestroyMessage()")
res = rp_hw.rp_SPI_DestroyMessage()
print(res,rp_hw.rp_HwGetError(res))


print("rp_hw.rp_SPI_Release()")
res = rp_hw.rp_SPI_Release()
print(res,rp_hw.rp_HwGetError(res))


print("rp_hw.rp_I2C_InitDevice('/dev/i2c-0',80)")
res = rp_hw.rp_I2C_InitDevice("/dev/i2c-0",80)
print(res,rp_hw.rp_HwGetError(res))

print("rp_hw.rp_I2C_setForceMode(True)")
res = rp_hw.rp_I2C_setForceMode(True)
print(res,rp_hw.rp_HwGetError(res))

print("rp_hw.rp_I2C_getForceMode()")
res = rp_hw.rp_I2C_getForceMode()
print(res,rp_hw.rp_HwGetError(res[0]))

print("rp_hw.rp_I2C_getDevAddress()")
res = rp_hw.rp_I2C_getDevAddress()
print(res,rp_hw.rp_HwGetError(res[0]))

print("rp_hw.rp_I2C_SMBUS_Read(0)")
res = rp_hw.rp_I2C_SMBUS_Read(0)
print(res,rp_hw.rp_HwGetError(res[0]))

print("rp_hw.rp_I2C_SMBUS_ReadWord(0)")
res = rp_hw.rp_I2C_SMBUS_ReadWord(0)
print(res,rp_hw.rp_HwGetError(res[0]))

print("rp_hw.rp_I2C_SMBUS_ReadCommand()")
res = rp_hw.rp_I2C_SMBUS_ReadCommand()
print(res,rp_hw.rp_HwGetError(res[0]))

max_read_size = 10
print("rp_hw.rp_I2C_SMBUS_ReadBuffer(0, max_read_size)")
status, data = rp_hw.rp_I2C_SMBUS_ReadBuffer(0, max_read_size)

if status == 0:
    print(f"Successfully read {len(data)} bytes")
    print("Data (hex):", [hex(x) for x in data])

    if len(data) > 0:
        first_byte = data[0]
else:
    print(f"I2C Read Error: {status}")

print("rp_hw.rp_I2C_SMBUS_Write(0,0)")
res = rp_hw.rp_I2C_SMBUS_Write(0,0)
print(res,rp_hw.rp_HwGetError(res))

print("rp_hw.rp_I2C_SMBUS_WriteWord(0,0)")
res = rp_hw.rp_I2C_SMBUS_WriteWord(0,0)
print(res,rp_hw.rp_HwGetError(res))

print("rp_hw.rp_I2C_SMBUS_WriteCommand(0)")
res = rp_hw.rp_I2C_SMBUS_WriteCommand(0)
print(res,rp_hw.rp_HwGetError(res))

print("rp_hw.rp_I2C_SMBUS_WriteBuffer(0, data)")
res = rp_hw.rp_I2C_SMBUS_WriteBuffer(0, data)
print(res,rp_hw.rp_HwGetError(res))

print("rp_hw.rp_I2C_IOCTL_ReadBuffer(rx_buff,rx_len)")
res = rp_hw.rp_I2C_IOCTL_ReadBuffer(data)
print(res,rp_hw.rp_HwGetError(res))

print("rp_hw.rp_I2C_IOCTL_WriteBuffer(rx_buff,rx_len)")
res = rp_hw.rp_I2C_IOCTL_WriteBuffer(data)
print(res,rp_hw.rp_HwGetError(res))


print("rp_hw.rp_GetCPUTemperature()")
res = rp_hw.rp_GetCPUTemperature()
print(res)

print("rp_hw.rp_GetPowerI4()")
res = rp_hw.rp_GetPowerI4()
print(res,rp_hw.rp_HwGetError(res[0]))

print("rp_hw.rp_GetPowerVCCPINT()")
res = rp_hw.rp_GetPowerVCCPINT()
print(res,rp_hw.rp_HwGetError(res[0]))

print("rp_hw.rp_GetPowerVCCPAUX()")
res = rp_hw.rp_GetPowerVCCPAUX()
print(res,rp_hw.rp_HwGetError(res[0]))

print("rp_hw.rp_GetPowerVCCBRAM()")
res = rp_hw.rp_GetPowerVCCBRAM()
print(res,rp_hw.rp_HwGetError(res[0]))

print("rp_hw.rp_GetPowerVCCINT()")
res = rp_hw.rp_GetPowerVCCINT()
print(res,rp_hw.rp_HwGetError(res[0]))

print("rp_hw.rp_GetPowerVCCAUX()")
res = rp_hw.rp_GetPowerVCCAUX()
print(res,rp_hw.rp_HwGetError(res[0]))

print("rp_hw.rp_GetPowerVCCDDR()")
res = rp_hw.rp_GetPowerVCCDDR()
print(res,rp_hw.rp_HwGetError(res[0]))