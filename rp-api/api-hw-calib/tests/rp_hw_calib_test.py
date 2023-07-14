#!/usr/bin/python3

import rp_hw_profiles
import rp_hw_calib

print("rp_hw_calib.rp_CalibInit()")
res = rp_hw_calib.rp_CalibInit()
print(res)

print("rp_hw_calib.rp_CalibInitSpecific(rp_hw_profiles.STEM_125_14_v1_0)")
res = rp_hw_calib.rp_CalibInitSpecific(rp_hw_profiles.STEM_125_14_v1_0)
print(res)

print("rp_hw_calib.rp_GetCalibrationSettings()")
res = rp_hw_calib.rp_GetCalibrationSettings()
print(res.fast_adc_count_1_1)

print("rp_hw_calib.rp_GetDefaultCalibrationSettings()")
res = rp_hw_calib.rp_GetDefaultCalibrationSettings()
print(res.fast_adc_count_1_1)
