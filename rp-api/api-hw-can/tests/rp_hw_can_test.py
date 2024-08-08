#!/usr/bin/python3


import rp_hw_can


print("rp_hw_can.rp_CanGetVersion()")
res = rp_hw_can.rp_CanGetVersion()
print(res)

print("rp_hw_can.rp_CanGetError(0)")
res = rp_hw_can.rp_CanGetError(0)
print(res)

print("rp_hw_can.rp_CanGetState(rp_hw_can.RP_CAN_0)")
res = rp_hw_can.rp_CanGetState(rp_hw_can.RP_CAN_0)
res2 = rp_hw_can.rp_CanGetStateName(res[1])
print(res,res2)


print("For testing CAN you need load v0.94 FPGA")

print("rp_hw_can.rp_CanGetVersion()")
res = rp_hw_can.rp_CanGetVersion()
print(res)

print("rp_hw_can.rp_CanGetError(0)")
res = rp_hw_can.rp_CanGetError(0)
print(res)

print("rp_hw_can.rp_CanGetState(rp_hw_can.RP_CAN_0)")
res = rp_hw_can.rp_CanGetState(rp_hw_can.RP_CAN_0)
res2 = rp_hw_can.rp_CanGetStateName(res[1])
print(res,res2)


print("rp_hw_can.rp_CanSetFPGAEnable(True)")
res = rp_hw_can.rp_CanSetFPGAEnable(True)
print(res)

print("rp_hw_can.rp_CanGetFPGAEnable()")
res = rp_hw_can.rp_CanGetFPGAEnable()
print(res)

print("rp_hw_can.rp_CanGetClockFreq(rp_hw_can.RP_CAN_0)")
res = rp_hw_can.rp_CanGetClockFreq(rp_hw_can.RP_CAN_0)
print(res)

print("rp_hw_can.rp_CanSetBitrate(rp_hw_can.RP_CAN_0,200000)")
res = rp_hw_can.rp_CanSetBitrate(rp_hw_can.RP_CAN_0,200000)
print(res)

print("rp_hw_can.rp_CanStart(rp_hw_can.RP_CAN_0)")
res = rp_hw_can.rp_CanStart(rp_hw_can.RP_CAN_0)
print(res)

print("rp_hw_can.rp_CanGetState(rp_hw_can.RP_CAN_0)")
res = rp_hw_can.rp_CanGetState(rp_hw_can.RP_CAN_0)
res2 = rp_hw_can.rp_CanGetStateName(res[1])
print(res,res2)

print("rp_hw_can.rp_CanStop(rp_hw_can.RP_CAN_0)")
res = rp_hw_can.rp_CanStop(rp_hw_can.RP_CAN_0)
print(res)

print("rp_hw_can.rp_CanGetState(rp_hw_can.RP_CAN_0)")
res = rp_hw_can.rp_CanGetState(rp_hw_can.RP_CAN_0)
res2 = rp_hw_can.rp_CanGetStateName(res[1])
print(res,res2)

print("rp_hw_can.rp_CanSetBitrate(rp_hw_can.RP_CAN_0,200000)")
res = rp_hw_can.rp_CanSetBitrate(rp_hw_can.RP_CAN_0,200000)
print(res)

print("rp_hw_can.rp_CanRestart(rp_hw_can.RP_CAN_0)")
res = rp_hw_can.rp_CanRestart(rp_hw_can.RP_CAN_0)
print(res)

print("rp_hw_can.rp_CanGetState(rp_hw_can.RP_CAN_0)")
res = rp_hw_can.rp_CanGetState(rp_hw_can.RP_CAN_0)
res2 = rp_hw_can.rp_CanGetStateName(res[1])
print(res,res2)

print("rp_hw_can.rp_CanSetBitrateAndSamplePoint(rp_hw_can.RP_CAN_0,100000,0.6)")
res = rp_hw_can.rp_CanSetBitrateAndSamplePoint(rp_hw_can.RP_CAN_0,100000,0.6)
print(res)

print("rp_hw_can.rp_CanGetBitrateAndSamplePoint(rp_hw_can.RP_CAN_0)")
res = rp_hw_can.rp_CanGetBitrateAndSamplePoint(rp_hw_can.RP_CAN_0)
print(res)

print("rp_hw_can.rp_CanStart(rp_hw_can.RP_CAN_0)")
res = rp_hw_can.rp_CanStart(rp_hw_can.RP_CAN_0)
print(res)

print("rp_hw_can.rp_CanSetBitrateAndSamplePoint(rp_hw_can.RP_CAN_0,100000,0.6)")
res = rp_hw_can.rp_CanSetBitrateAndSamplePoint(rp_hw_can.RP_CAN_0,100000,0.6)
print(res)

bt = rp_hw_can.rp_can_bittiming_t()
print("rp_hw_can.rp_CanGetBitTiming(rp_hw_can.RP_CAN_0,bt)")
res = rp_hw_can.rp_CanGetBitTiming(rp_hw_can.RP_CAN_0,bt)
print(res)

print("tq",bt.tq)
print("prop_seg",bt.prop_seg)
print("phase_seg1",bt.phase_seg1)
print("phase_seg2",bt.phase_seg2)
print("sjw",bt.sjw)
print("brp",bt.brp)

btl = rp_hw_can.new_p_rp_can_bittiming_limits_t()
print("rp_hw_can.rp_CanGetBitTimingLimits(rp_hw_can.RP_CAN_0,btl)")
res = rp_hw_can.rp_CanGetBitTimingLimits(rp_hw_can.RP_CAN_0,btl)
print(res)

print("tseg1_min",btl.tseg1_min)
print("tseg1_max",btl.tseg1_max)
print("tseg2_min",btl.tseg2_min)
print("tseg2_max",btl.tseg2_max)
print("sjw_max",btl.sjw_max)
print("brp_min",btl.brp_min)
print("brp_max",btl.brp_max)
print("brp_inc",btl.brp_inc)

print("rp_hw_can.rp_CanStop(rp_hw_can.RP_CAN_0)")
res = rp_hw_can.rp_CanStop(rp_hw_can.RP_CAN_0)
print(res)

bt2 = rp_hw_can.rp_can_bittiming_t()
print("rp_hw_can.rp_CanGetBitTiming(rp_hw_can.RP_CAN_0,bt2)")
res = rp_hw_can.rp_CanGetBitTiming(rp_hw_can.RP_CAN_0,bt2)
print(res)

bt2.tq = 2001
bt2.prop_seg = 2
bt2.phase_seg1 = 2
bt2.phase_seg2 = 3
bt2.sjw = 2
bt2.rp = 201

print("tq",bt2.tq)
print("prop_seg",bt2.prop_seg)
print("phase_seg1",bt2.phase_seg1)
print("phase_seg2",bt2.phase_seg2)
print("sjw",bt2.sjw)
print("brp",bt2.brp)

print("rp_hw_can.rp_CanSetBitTiming(rp_hw_can.RP_CAN_0,bt2)")
res = rp_hw_can.rp_CanSetBitTiming(rp_hw_can.RP_CAN_0,bt2)
print(res)

bt3 = rp_hw_can.rp_can_bittiming_t()
print("rp_hw_can.rp_CanGetBitTiming(rp_hw_can.RP_CAN_0,bt3)")
res = rp_hw_can.rp_CanGetBitTiming(rp_hw_can.RP_CAN_0,bt3)
print(res)

print("tq",bt3.tq)
print("prop_seg",bt3.prop_seg)
print("phase_seg1",bt3.phase_seg1)
print("phase_seg2",bt3.phase_seg2)
print("sjw",bt3.sjw)
print("brp",bt3.brp)

print("rp_hw_can.rp_CanSetBitTiming(rp_hw_can.RP_CAN_0,bt)")
res = rp_hw_can.rp_CanSetBitTiming(rp_hw_can.RP_CAN_0,bt)
print(res)

print("rp_hw_can.rp_CanSetRestartTime(rp_hw_can.RP_CAN_0,10)")
res = rp_hw_can.rp_CanSetRestartTime(rp_hw_can.RP_CAN_0,10)
print(res)

print("rp_hw_can.rp_CanGetRestartTime(rp_hw_can.RP_CAN_0)")
res = rp_hw_can.rp_CanGetRestartTime(rp_hw_can.RP_CAN_0)
print(res)

print("rp_hw_can.rp_CanSetControllerMode(rp_hw_can.RP_CAN_0,rp_hw_can.RP_CAN_MODE_LOOPBACK,True)")
res = rp_hw_can.rp_CanSetControllerMode(rp_hw_can.RP_CAN_0,rp_hw_can.RP_CAN_MODE_LOOPBACK,True)
print(res,rp_hw_can.rp_CanGetError(res))

print("rp_hw_can.rp_CanGetControllerMode(rp_hw_can.RP_CAN_0,rp_hw_can.RP_CAN_MODE_LOOPBACK)")
res = rp_hw_can.rp_CanGetControllerMode(rp_hw_can.RP_CAN_0,rp_hw_can.RP_CAN_MODE_LOOPBACK)
res2 = rp_hw_can.rp_CanGetError(res[0])
print(res,res2)

print("rp_hw_can.rp_CanSetControllerMode(rp_hw_can.RP_CAN_0,rp_hw_can.RP_CAN_MODE_LOOPBACK,False)")
res = rp_hw_can.rp_CanSetControllerMode(rp_hw_can.RP_CAN_0,rp_hw_can.RP_CAN_MODE_LOOPBACK,False)
print(res,rp_hw_can.rp_CanGetError(res))

print("rp_hw_can.rp_CanGetControllerMode(rp_hw_can.RP_CAN_0,rp_hw_can.RP_CAN_MODE_LOOPBACK)")
res = rp_hw_can.rp_CanGetControllerMode(rp_hw_can.RP_CAN_0,rp_hw_can.RP_CAN_MODE_LOOPBACK)
res2 = rp_hw_can.rp_CanGetError(res[0])
print(res,res2)

print("rp_hw_can.rp_CanSetControllerMode(rp_hw_can.RP_CAN_0,rp_hw_can.RP_CAN_MODE_LISTENONLY,True)")
res = rp_hw_can.rp_CanSetControllerMode(rp_hw_can.RP_CAN_0,rp_hw_can.RP_CAN_MODE_LISTENONLY,True)
res2 = rp_hw_can.rp_CanGetError(res)
print(res,res2)

print("rp_hw_can.rp_CanGetControllerMode(rp_hw_can.RP_CAN_0,rp_hw_can.RP_CAN_MODE_LISTENONLY)")
res = rp_hw_can.rp_CanGetControllerMode(rp_hw_can.RP_CAN_0,rp_hw_can.RP_CAN_MODE_LISTENONLY)
res2 = rp_hw_can.rp_CanGetError(res[0])
print(res,res2)

print("rp_hw_can.rp_CanSetControllerMode(rp_hw_can.RP_CAN_0,rp_hw_can.RP_CAN_MODE_LISTENONLY,False)")
res = rp_hw_can.rp_CanSetControllerMode(rp_hw_can.RP_CAN_0,rp_hw_can.RP_CAN_MODE_LISTENONLY,False)
res2 = rp_hw_can.rp_CanGetError(res)
print(res,res2)

print("rp_hw_can.rp_CanSetControllerMode(rp_hw_can.RP_CAN_0,rp_hw_can.RP_CAN_MODE_3_SAMPLES,True)")
res = rp_hw_can.rp_CanSetControllerMode(rp_hw_can.RP_CAN_0,rp_hw_can.RP_CAN_MODE_3_SAMPLES,True)
print(res,rp_hw_can.rp_CanGetError(res))

print("rp_hw_can.rp_CanGetControllerMode(rp_hw_can.RP_CAN_0,rp_hw_can.RP_CAN_MODE_3_SAMPLES)")
res = rp_hw_can.rp_CanGetControllerMode(rp_hw_can.RP_CAN_0,rp_hw_can.RP_CAN_MODE_3_SAMPLES)
res2 = rp_hw_can.rp_CanGetError(res[0])
print(res,res2)

print("rp_hw_can.rp_CanSetControllerMode(rp_hw_can.RP_CAN_0,rp_hw_can.RP_CAN_MODE_3_SAMPLES,False)")
res = rp_hw_can.rp_CanSetControllerMode(rp_hw_can.RP_CAN_0,rp_hw_can.RP_CAN_MODE_3_SAMPLES,False)
print(res,rp_hw_can.rp_CanGetError(res))

print("rp_hw_can.rp_CanSetControllerMode(rp_hw_can.RP_CAN_0,rp_hw_can.RP_CAN_MODE_ONE_SHOT,True)")
res = rp_hw_can.rp_CanSetControllerMode(rp_hw_can.RP_CAN_0,rp_hw_can.RP_CAN_MODE_ONE_SHOT,True)
print(res,rp_hw_can.rp_CanGetError(res))

print("rp_hw_can.rp_CanGetControllerMode(rp_hw_can.RP_CAN_0,rp_hw_can.RP_CAN_MODE_ONE_SHOT)")
res = rp_hw_can.rp_CanGetControllerMode(rp_hw_can.RP_CAN_0,rp_hw_can.RP_CAN_MODE_ONE_SHOT)
res2 = rp_hw_can.rp_CanGetError(res[0])
print(res,res2)

print("rp_hw_can.rp_CanSetControllerMode(rp_hw_can.RP_CAN_0,rp_hw_can.RP_CAN_MODE_ONE_SHOT,False)")
res = rp_hw_can.rp_CanSetControllerMode(rp_hw_can.RP_CAN_0,rp_hw_can.RP_CAN_MODE_ONE_SHOT,False)
print(res,rp_hw_can.rp_CanGetError(res))

print("rp_hw_can.rp_CanSetControllerMode(rp_hw_can.RP_CAN_0,rp_hw_can.RP_CAN_MODE_BERR_REPORTING,True)")
res = rp_hw_can.rp_CanSetControllerMode(rp_hw_can.RP_CAN_0,rp_hw_can.RP_CAN_MODE_BERR_REPORTING,True)
print(res,rp_hw_can.rp_CanGetError(res))

print("rp_hw_can.rp_CanGetControllerMode(rp_hw_can.RP_CAN_0,rp_hw_can.RP_CAN_MODE_BERR_REPORTING)")
res = rp_hw_can.rp_CanGetControllerMode(rp_hw_can.RP_CAN_0,rp_hw_can.RP_CAN_MODE_BERR_REPORTING)
res2 = rp_hw_can.rp_CanGetError(res[0])
print(res,res2)

print("rp_hw_can.rp_CanSetControllerMode(rp_hw_can.RP_CAN_0,rp_hw_can.RP_CAN_MODE_BERR_REPORTING,False)")
res = rp_hw_can.rp_CanSetControllerMode(rp_hw_can.RP_CAN_0,rp_hw_can.RP_CAN_MODE_BERR_REPORTING,False)
print(res,rp_hw_can.rp_CanGetError(res))

print("rp_hw_can.rp_CanStart(rp_hw_can.RP_CAN_0)")
res = rp_hw_can.rp_CanStart(rp_hw_can.RP_CAN_0)
print(res)

print("rp_hw_can.rp_CanStart(rp_hw_can.RP_CAN_1)")
res = rp_hw_can.rp_CanStart(rp_hw_can.RP_CAN_1)
print(res)

print("rp_hw_can.rp_CanGetBusErrorCounters(rp_hw_can.RP_CAN_0)")
res = rp_hw_can.rp_CanGetBusErrorCounters(rp_hw_can.RP_CAN_0)
print(res)

print("rp_hw_can.rp_CanGetState(rp_hw_can.RP_CAN_0)")
res = rp_hw_can.rp_CanGetState(rp_hw_can.RP_CAN_0)
res2 = rp_hw_can.rp_CanGetStateName(res[1])
print(res,res2)

print("rp_hw_can.rp_CanOpen(rp_hw_can.RP_CAN_0)")
res = rp_hw_can.rp_CanOpen(rp_hw_can.RP_CAN_0)
print(res,rp_hw_can.rp_CanGetError(res))

print("rp_hw_can.rp_CanShowErrorFrames(rp_hw_can.RP_CAN_0,True)")
res = rp_hw_can.rp_CanShowErrorFrames(rp_hw_can.RP_CAN_0,True)
print(res,rp_hw_can.rp_CanGetError(res))

print("rp_hw_can.rp_CanAddFilter(rp_hw_can.RP_CAN_0,0xFF,0xFF)")
res = rp_hw_can.rp_CanAddFilter(rp_hw_can.RP_CAN_0,0xFF,0xFF)
print(res,rp_hw_can.rp_CanGetError(res))

print("rp_hw_can.rp_CanAddFilter(rp_hw_can.RP_CAN_0,0xAF,0x0F)")
res = rp_hw_can.rp_CanAddFilter(rp_hw_can.RP_CAN_0,0xAF,0x0F)
print(res,rp_hw_can.rp_CanGetError(res))

print("rp_hw_can.rp_CanSetFilter(rp_hw_can.RP_CAN_0,True)")
res = rp_hw_can.rp_CanSetFilter(rp_hw_can.RP_CAN_0,True)
print(res,rp_hw_can.rp_CanGetError(res))

print("rp_hw_can.rp_CanSetFilter(rp_hw_can.RP_CAN_0,False)")
res = rp_hw_can.rp_CanSetFilter(rp_hw_can.RP_CAN_0,False)
print(res,rp_hw_can.rp_CanGetError(res))

print("rp_hw_can.rp_CanRemoveFilter(rp_hw_can.RP_CAN_0,0xAF,0x0F)")
res = rp_hw_can.rp_CanRemoveFilter(rp_hw_can.RP_CAN_0,0xAF,0x0F)
print(res,rp_hw_can.rp_CanGetError(res))

print("rp_hw_can.rp_CanSetFilter(rp_hw_can.RP_CAN_0,False)")
res = rp_hw_can.rp_CanSetFilter(rp_hw_can.RP_CAN_0,False)
print(res,rp_hw_can.rp_CanGetError(res))

print("rp_hw_can.rp_CanClearFilter(rp_hw_can.RP_CAN_0)")
res = rp_hw_can.rp_CanClearFilter(rp_hw_can.RP_CAN_0)
print(res,rp_hw_can.rp_CanGetError(res))

print("rp_hw_can.rp_CanSetFilter(rp_hw_can.RP_CAN_0,False)")
res = rp_hw_can.rp_CanSetFilter(rp_hw_can.RP_CAN_0,False)
print(res,rp_hw_can.rp_CanGetError(res))


print("rp_hw_can.rp_CanClose(rp_hw_can.RP_CAN_0)")
res = rp_hw_can.rp_CanClose(rp_hw_can.RP_CAN_0)
print(res,rp_hw_can.rp_CanGetError(res))

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

frame_read = rp_hw_can.rp_can_frame_t()
# Hang of not income data
#print("rp_hw_can.rp_CanRead(rp_hw_can.RP_CAN_0,0,frame_read)")
#res = rp_hw_can.rp_CanRead(rp_hw_can.RP_CAN_0,0,frame_read)
#print(res,rp_hw_can.rp_CanGetError(res))

# print("rp_hw_can.rp_CanRead(rp_hw_can.RP_CAN_0,1000,frame_read)")
# res = rp_hw_can.rp_CanRead(rp_hw_can.RP_CAN_0,1000,frame_read)
# print(res,rp_hw_can.rp_CanGetError(res))

print("rp_hw_can.rp_CanGetControllerMode(rp_hw_can.RP_CAN_0,rp_hw_can.RP_CAN_MODE_LOOPBACK)")
res = rp_hw_can.rp_CanGetControllerMode(rp_hw_can.RP_CAN_0,rp_hw_can.RP_CAN_MODE_LOOPBACK)
res2 = rp_hw_can.rp_CanGetError(res[0])
print(res,res2)

print("rp_hw_can.rp_CanGetControllerMode(rp_hw_can.RP_CAN_1,rp_hw_can.RP_CAN_MODE_LOOPBACK)")
res = rp_hw_can.rp_CanGetControllerMode(rp_hw_can.RP_CAN_1,rp_hw_can.RP_CAN_MODE_LOOPBACK)
res2 = rp_hw_can.rp_CanGetError(res[0])
print(res,res2)

tx_buff = rp_hw_can.Buffer(8)
tx_buff[0] = 1
tx_buff[1] = 2
tx_buff[2] = 3

frame_read = rp_hw_can.rp_can_frame_t()
print("rp_hw_can.rp_CanSend(rp_hw_can.RP_CAN_0,123,tx_buff,3,True,False,0)")
res = rp_hw_can.rp_CanSend(rp_hw_can.RP_CAN_0,123,tx_buff,3,True,False,0)
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
print("data", rp_hw_can.uintArr_getitem(d,0), rp_hw_can.uintArr_getitem(d,1), rp_hw_can.uintArr_getitem(d,2), rp_hw_can.uintArr_getitem(d,3))

print("rp_hw_can.rp_CanRead(rp_hw_can.RP_CAN_1,5000,frame_read)")
res = rp_hw_can.rp_CanRead(rp_hw_can.RP_CAN_1,5000,frame_read)
print(res,rp_hw_can.rp_CanGetError(res))

print("can_id", frame_read.can_id)
print("can_id_raw", frame_read.can_id_raw)
print("is_extended_format", frame_read.is_extended_format)
print("is_error_frame", frame_read.is_error_frame)
print("is_remote_request", frame_read.is_remote_request)
d=frame_read.data
print("data", rp_hw_can.uintArr_getitem(d,0), rp_hw_can.uintArr_getitem(d,1), rp_hw_can.uintArr_getitem(d,2), rp_hw_can.uintArr_getitem(d,3))



print("rp_hw_can.rp_CanClose(rp_hw_can.RP_CAN_0)")
res = rp_hw_can.rp_CanClose(rp_hw_can.RP_CAN_0)
print(res,rp_hw_can.rp_CanGetError(res))

print("rp_hw_can.rp_CanClose(rp_hw_can.RP_CAN_0)")
res = rp_hw_can.rp_CanClose(rp_hw_can.RP_CAN_0)
print(res,rp_hw_can.rp_CanGetError(res))


print("rp_hw_can.rp_CanClose(rp_hw_can.RP_CAN_1)")
res = rp_hw_can.rp_CanClose(rp_hw_can.RP_CAN_1)
print(res,rp_hw_can.rp_CanGetError(res))
