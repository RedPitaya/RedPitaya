#!/usr/bin/python3


import rp_hw_can
import numpy as np


print("rp_hw_can.rp_CanSetFPGAEnable(True)")
res = rp_hw_can.rp_CanSetFPGAEnable(True)
print(res)

print("rp_hw_can.rp_CanStop(rp_hw_can.RP_CAN_0)")
res = rp_hw_can.rp_CanStop(rp_hw_can.RP_CAN_0)
print(res)

print("rp_hw_can.rp_CanStop(rp_hw_can.RP_CAN_1)")
res = rp_hw_can.rp_CanStop(rp_hw_can.RP_CAN_1)
print(res)

print("rp_hw_can.rp_CanSetBitrate(rp_hw_can.RP_CAN_0,200000)")
res = rp_hw_can.rp_CanSetBitrate(rp_hw_can.RP_CAN_0,200000)
print(res)

print("rp_hw_can.rp_CanSetBitrate(rp_hw_can.RP_CAN_1,200000)")
res = rp_hw_can.rp_CanSetBitrate(rp_hw_can.RP_CAN_1,200000)
print(res)

print("rp_hw_can.rp_CanSetControllerMode(rp_hw_can.RP_CAN_0,rp_hw_can.RP_CAN_MODE_LOOPBACK,False)")
res = rp_hw_can.rp_CanSetControllerMode(rp_hw_can.RP_CAN_0,rp_hw_can.RP_CAN_MODE_LOOPBACK,False)
print(res)

print("rp_hw_can.rp_CanSetControllerMode(rp_hw_can.RP_CAN_1,rp_hw_can.RP_CAN_MODE_LOOPBACK,False)")
res = rp_hw_can.rp_CanSetControllerMode(rp_hw_can.RP_CAN_1,rp_hw_can.RP_CAN_MODE_LOOPBACK,False)
print(res)

print("rp_hw_can.rp_CanStart(rp_hw_can.RP_CAN_0)")
res = rp_hw_can.rp_CanStart(rp_hw_can.RP_CAN_0)
print(res)

print("rp_hw_can.rp_CanStart(rp_hw_can.RP_CAN_1)")
res = rp_hw_can.rp_CanStart(rp_hw_can.RP_CAN_1)
print(res)

print("rp_hw_can.rp_CanOpen(rp_hw_can.RP_CAN_0)")
res = rp_hw_can.rp_CanOpen(rp_hw_can.RP_CAN_0)
print(res,rp_hw_can.rp_CanGetError(res))

print("rp_hw_can.rp_CanOpen(rp_hw_can.RP_CAN_1)")
res = rp_hw_can.rp_CanOpen(rp_hw_can.RP_CAN_1)
print(res,rp_hw_can.rp_CanGetError(res))

tx_buff = rp_hw_can.Buffer(8)
tx_buff[0] = 1
tx_buff[1] = 2
tx_buff[2] = 3
tx_buff[3] = 4
tx_buff[4] = 5

frame_read = rp_hw_can.rp_can_frame_t()
print("rp_hw_can.rp_CanSend(rp_hw_can.RP_CAN_0,123,tx_buff,3,False,False,0)")
res = rp_hw_can.rp_CanSend(rp_hw_can.RP_CAN_0,123,tx_buff,3,False,False,0)
print(res,rp_hw_can.rp_CanGetError(res))

print("rp_hw_can.rp_CanSend(rp_hw_can.RP_CAN_0,321,tx_buff,5,True,False,0)")
res = rp_hw_can.rp_CanSend(rp_hw_can.RP_CAN_0,321,tx_buff,5,True,False,0)
print(res,rp_hw_can.rp_CanGetError(res))

print("rp_hw_can.rp_CanRead(rp_hw_can.RP_CAN_1,5000,frame_read)")
res = rp_hw_can.rp_CanRead(rp_hw_can.RP_CAN_1,5000,frame_read)
print(res,rp_hw_can.rp_CanGetError(res))

print("can_id", frame_read.can_id)
print("can_id_raw", frame_read.can_id_raw)
print("is_extended_format", frame_read.is_extended_format)
print("is_error_frame", frame_read.is_error_frame)
print("is_remote_request", frame_read.is_remote_request)
d=frame_read.data
print("data", rp_hw_can.uintArr_getitem(d,0), rp_hw_can.uintArr_getitem(d,1), rp_hw_can.uintArr_getitem(d,2))

print("rp_hw_can.rp_CanRead(rp_hw_can.RP_CAN_1,5000,frame_read)")
res = rp_hw_can.rp_CanRead(rp_hw_can.RP_CAN_1,5000,frame_read)
print(res,rp_hw_can.rp_CanGetError(res))

print("can_id", frame_read.can_id)
print("can_id_raw", frame_read.can_id_raw)
print("is_extended_format", frame_read.is_extended_format)
print("is_error_frame", frame_read.is_error_frame)
print("is_remote_request", frame_read.is_remote_request)
d=frame_read.data
print("data", rp_hw_can.uintArr_getitem(d,0), rp_hw_can.uintArr_getitem(d,1), rp_hw_can.uintArr_getitem(d,2), rp_hw_can.uintArr_getitem(d,3), rp_hw_can.uintArr_getitem(d,4))

tx_data = "54321"
tx_data_np = np.array(list(tx_data), dtype=np.uint8)
print(tx_data_np)

print("rp_hw_can.rp_CanSendNP(rp_hw_can.RP_CAN_0,555,False,False,0,tx_data_np)")
res = rp_hw_can.rp_CanSendNP(rp_hw_can.RP_CAN_0,555,False,False,0,tx_data_np)
print(res,rp_hw_can.rp_CanGetError(res))

rx_data_np = np.zeros(8, dtype=np.uint8)

print("rp_hw_can.rp_CanReadNP(rp_hw_can.RP_CAN_1,1000,rx_data_np)")
res = rp_hw_can.rp_CanReadNP(rp_hw_can.RP_CAN_1,1000,rx_data_np)
print(res)
print(rx_data_np)

print("rp_hw_can.rp_CanClose(rp_hw_can.RP_CAN_0)")
res = rp_hw_can.rp_CanClose(rp_hw_can.RP_CAN_0)
print(res,rp_hw_can.rp_CanGetError(res))

print("rp_hw_can.rp_CanClose(rp_hw_can.RP_CAN_1)")
res = rp_hw_can.rp_CanClose(rp_hw_can.RP_CAN_1)
print(res,rp_hw_can.rp_CanGetError(res))
