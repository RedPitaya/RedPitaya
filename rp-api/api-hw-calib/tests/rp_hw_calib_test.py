#!/usr/bin/python3


import rp_hw_profiles
import rp_hw_calib


print("rp_hw_calib.rp_CalibInitSpecific(rp_hw_profiles.STEM_125_14_v1_0)")
res = rp_hw_calib.rp_CalibInitSpecific(rp_hw_profiles.STEM_125_14_v1_0)
print(res)

print("rp_hw_calib.rp_CalibInit()")
res = rp_hw_calib.rp_CalibInit()
print(res)

print("rp_hw_calib.rp_GetCalibrationSettings()")
current_settings = rp_hw_calib.rp_GetCalibrationSettings()
print(current_settings.fast_adc_count_1_1)

print("rp_hw_calib.rp_GetDefaultCalibrationSettings()")
res = rp_hw_calib.rp_GetDefaultCalibrationSettings()
print(res.fast_adc_count_1_1)

print("rp_hw_calib.rp_CalibrationReset()")
res = rp_hw_calib.rp_CalibrationReset(False,False)
print(res)

print("rp_hw_calib.rp_CalibrationFactoryReset()")
res = rp_hw_calib.rp_CalibrationFactoryReset(False)
print(res)

print("rp_hw_calib.rp_GetCalibrationSettings()")
rp_calib = rp_hw_calib.rp_GetCalibrationSettings()

calib_1 = rp_hw_calib.cCalibArr_getitem(rp_calib.fast_adc_1_1,0)
calib_2 = rp_hw_calib.cCalibArr_getitem(rp_calib.fast_adc_1_1,0)

print(calib_1.gainCalc)
print(calib_2.gainCalc)

calib_1.gainCalc = 1.1
calib_2.gainCalc = 1.1

rp_hw_calib.cCalibArr_setitem(rp_calib.fast_adc_1_1,0,calib_1)
rp_hw_calib.cCalibArr_setitem(rp_calib.fast_adc_1_1,1,calib_2)


print("rp_hw_calib.rp_CalibrationWriteParams(res)")
res = rp_hw_calib.rp_CalibrationWriteParams(rp_calib,False)
print(res)

print("rp_hw_calib.rp_CalibrationSetParams(res)")
res = rp_hw_calib.rp_CalibrationSetParams(rp_calib)
print(res)

print("rp_hw_calib.rp_CalibPrint(rp_calib)")
res = rp_hw_calib.rp_CalibPrint(rp_calib)
print(res)

print("rp_hw_calib.rp_CalibGetEEPROM(false)")
raw_data = rp_hw_calib.rp_CalibGetEEPROM(False)
print(raw_data)

data_arr = rp_hw_calib.uint8Arr_frompointer(raw_data[1])

for n in range(raw_data[2]):
    print(hex(data_arr[n]), end=",")
print("")

t = rp_hw_calib.new_p_rp_calib_params_t()

print("rp_hw_calib.rp_CalibConvertEEPROM(raw_data[1],raw_data[2],t)")
res = rp_hw_calib.rp_CalibConvertEEPROM(raw_data[1],raw_data[2],t)
print(res)

print("rp_hw_calib.rp_CalibPrint(t)")
res = rp_hw_calib.rp_CalibPrint(t)
print(res)

rp_hw_calib.delete_puint8(raw_data[1])

# IDs defined in calib_universal.h
print("rp_hw_calib.rp_GetNameOfUniversalId(1)")
res = rp_hw_calib.rp_GetNameOfUniversalId(1)
print(res)

t = rp_hw_calib.new_p_channel_filter_t()
print("rp_hw_calib.rp_CalibGetFastADCFilter(0,t)")
res = rp_hw_calib.rp_CalibGetFastADCFilter(0,t)
print(res,t)
print(t.aa,t.bb,t.pp,t.kk)

print("rp_hw_calib.rp_CalibGetFastADCFilter_1_20(0,t)")
res = rp_hw_calib.rp_CalibGetFastADCFilter_1_20(0,t)
print(res,t)
print(t.aa,t.bb,t.pp,t.kk)

print("rp_hw_calib.rp_CalibGetFastADCCalibValue(0,0)")
res = rp_hw_calib.rp_CalibGetFastADCCalibValue(0,0)
print(res)

t = rp_hw_calib.new_p_uint_gain_calib_t()
print("rp_hw_calib.rp_CalibGetFastADCCalibValueI(0,0,t)")
res = rp_hw_calib.rp_CalibGetFastADCCalibValueI(0,0,t)
print(res,t)
print(t.gain,t.base,t.precision,t.offset)

print("rp_hw_calib.rp_CalibGetFastADCCalibValue_1_20(0,0)")
res = rp_hw_calib.rp_CalibGetFastADCCalibValue_1_20(0,0)
print(res)

t = rp_hw_calib.new_p_uint_gain_calib_t()
print("rp_hw_calib.rp_CalibGetFastADCCalibValue_1_20I(0,0,t)")
res = rp_hw_calib.rp_CalibGetFastADCCalibValue_1_20I(0,0,t)
print(res,t)
print(t.gain,t.base,t.precision,t.offset)

print("rp_hw_calib.rp_CalibGetFastDACCalibValue(0,0)")
res = rp_hw_calib.rp_CalibGetFastDACCalibValue(0,0)
print(res)

t = rp_hw_calib.new_p_uint_gain_calib_t()
print("rp_hw_calib.rp_CalibGetFastDACCalibValueI(0,0,t)")
res = rp_hw_calib.rp_CalibGetFastDACCalibValueI(0,0,t)
print(res,t)
print(t.gain,t.base,t.precision,t.offset)

print("rp_hw_calib.rp_CalibrationWriteParams(current_settings,False)")
res = rp_hw_calib.rp_CalibrationWriteParams(current_settings,False)
print(res)