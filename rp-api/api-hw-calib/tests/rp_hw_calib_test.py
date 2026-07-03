#!/usr/bin/python3

import rp_hw_profiles
import rp_hw_calib
import numpy as np

print("=" * 60)
print("Red Pitaya Hardware Calibration Test")
print("=" * 60)

# Initialize calibration
print("\n1. rp_hw_calib.rp_CalibInitSpecific(rp_hw_profiles.STEM_125_14_v1_0)")
res = rp_hw_calib.rp_CalibInitSpecific(rp_hw_profiles.STEM_125_14_v1_0)
print(f"Result: {res}")

print("\n2. rp_hw_calib.rp_CalibInit()")
res = rp_hw_calib.rp_CalibInit()
print(f"Result: {res}")

# Get calibration version
print("\n3. rp_hw_calib.rp_GetCalibrationVersion()")
res = rp_hw_calib.rp_GetCalibrationVersion()
print(f"Result: {res}")

# Get calibration settings
print("\n4. rp_hw_calib.rp_GetCalibrationSettings()")
current_settings = rp_hw_calib.rp_GetCalibrationSettings()
print(f"ADC count 1:1 = {current_settings.fast_adc_count_1_1}")

print("\n5. rp_hw_calib.rp_GetDefaultCalibrationSettings()")
res = rp_hw_calib.rp_GetDefaultCalibrationSettings()
print(f"ADC count 1:1 = {res.fast_adc_count_1_1}")

print("\n6. rp_hw_calib.rp_GetDefaultUniCalibrationSettings()")
res = rp_hw_calib.rp_GetDefaultUniCalibrationSettings()
print(f"ADC count 1:1 = {res.fast_adc_count_1_1}")

# Reset calibration
print("\n7. rp_hw_calib.rp_CalibrationReset(False, False, rp_hw_calib.RP_HW_CFM_DEFAULT, 5)")
res = rp_hw_calib.rp_CalibrationReset(False, False, rp_hw_calib.RP_HW_CFM_DEFAULT, 5)
print(f"Result: {res}")

print("\n8. rp_hw_calib.rp_CalibrationFactoryReset(False)")
res = rp_hw_calib.rp_CalibrationFactoryReset(False)
print(f"Result: {res}")

# Modify calibration settings
print("\n9. Modifying calibration settings")
rp_calib = rp_hw_calib.rp_GetCalibrationSettings()

calib_1 = rp_hw_calib.cCalibArr_getitem(rp_calib.fast_adc_1_1, 0)
calib_2 = rp_hw_calib.cCalibArr_getitem(rp_calib.fast_adc_1_1, 1)

print(f"Original calib_1.gainCalc = {calib_1.gainCalc}")
print(f"Original calib_2.gainCalc = {calib_2.gainCalc}")

calib_1.gainCalc = 1.1
calib_2.gainCalc = 1.1

rp_hw_calib.cCalibArr_setitem(rp_calib.fast_adc_1_1, 0, calib_1)
rp_hw_calib.cCalibArr_setitem(rp_calib.fast_adc_1_1, 1, calib_2)

# Write calibration
print("\n10. rp_hw_calib.rp_CalibrationWriteParams(rp_calib, False)")
res = rp_hw_calib.rp_CalibrationWriteParams(rp_calib, False)
print(f"Result: {res}")

print("\n11. rp_hw_calib.rp_CalibrationWriteParamsEx(rp_calib, False)")
res = rp_hw_calib.rp_CalibrationWriteParamsEx(rp_calib, False)
print(f"Result: {res}")

print("\n12. rp_hw_calib.rp_CalibrationSetParams(rp_calib)")
res = rp_hw_calib.rp_CalibrationSetParams(rp_calib)
print(f"Result: {res}")

# Print calibration
print("\n13. rp_hw_calib.rp_CalibPrint(rp_calib)")
res = rp_hw_calib.rp_CalibPrint(rp_calib)
print(f"Result: {res}")

# Get EEPROM data with numpy support
print("\n14. Getting EEPROM data")
print("Using numpy method:")
status, data_array = rp_hw_calib.rp_CalibGetEEPROMNP(False)
if status == rp_hw_calib.RP_HW_CALIB_OK and data_array is not None:
    print(f"Status: {status}")
    print(f"Data size: {len(data_array)} bytes")
    print(f"First 10 bytes: {[hex(x) for x in data_array[:10].tolist()]}")
else:
    print(f"Failed to get EEPROM data, status: {status}")

# Original method with SWIG pointer (backup)
print("\nUsing original method:")
raw_data = rp_hw_calib.rp_CalibGetEEPROM(False)
print(f"Result tuple length: {len(raw_data)}")
print(f"Status: {raw_data[0]}, Size: {raw_data[2]}")

if raw_data[0] == rp_hw_calib.RP_HW_CALIB_OK:
    # Method 1: Using uint8Arr_frompointer
    print("Method 1: uint8Arr_frompointer")
    try:
        data_arr = rp_hw_calib.uint8Arr.frompointer(raw_data[1])
        print("First 10 bytes:", end=" ")
        for n in range(min(raw_data[2], 10)):
            print(hex(data_arr[n]), end=" ")
        print()
    except AttributeError as e:
        print(f"uint8Arr_frompointer not available: {e}")
        print("Using alternative method with ctypes")
        import ctypes
        ptr_value = int(raw_data[1])
        if ptr_value != 0 and raw_data[2] > 0:
            buffer = (ctypes.c_uint8 * raw_data[2]).from_address(ptr_value)
            data_arr = np.frombuffer(buffer, dtype=np.uint8)
            print("First 10 bytes:", end=" ")
            for n in range(min(raw_data[2], 10)):
                print(hex(data_arr[n]), end=" ")
            print()

    # Don't forget to free memory
    rp_hw_calib.delete_puint8(raw_data[1])

# Convert EEPROM data to calibration
print("\n15. Converting EEPROM to calibration")
status, data_array = rp_hw_calib.rp_CalibGetEEPROMNP(False)
if status == rp_hw_calib.RP_HW_CALIB_OK:
    res, t = rp_hw_calib.rp_CalibConvertEEPROMNP(data_array)
    print(f"Conversion result: {res}")

    if res == rp_hw_calib.RP_HW_CALIB_OK:
        print("Printing converted calibration:")
        rp_hw_calib.rp_CalibPrint(t)

    # Convert to old format
    print("\n16. Converting to old format")
    res = rp_hw_calib.rp_CalibConvertToOld(t)
    print(f"Convert to old result: {res}")

    print("Printing after old format conversion:")
    rp_hw_calib.rp_CalibPrint(t)

# Get name of universal ID
print("\n17. Getting universal ID name")
res = rp_hw_calib.rp_GetNameOfUniversalId(1)
print(f"Universal ID 1 name: {res}")

# Get ADC filter values
print("\n18. Getting ADC filter values")
t = rp_hw_calib.new_p_channel_filter_t()
res = rp_hw_calib.rp_CalibGetFastADCFilter(0, t)
print(f"Result: {res}")
print(f"Filter AA={t.aa}, BB={t.bb}, PP={t.pp}, KK={t.kk}")

res = rp_hw_calib.rp_CalibGetFastADCFilter_1_20(0, t)
print(f"1:20 Filter AA={t.aa}, BB={t.bb}, PP={t.pp}, KK={t.kk}")

# Get ADC calibration values
print("\n19. Getting ADC calibration values")
res = rp_hw_calib.rp_CalibGetFastADCCalibValue(0, 0)
print(f"ADC float calib: {res}")

t_uint = rp_hw_calib.new_p_uint_gain_calib_t()
res = rp_hw_calib.rp_CalibGetFastADCCalibValueI(0, 0, t_uint)
print(f"ADC int calib: gain={t_uint.gain}, base={t_uint.base}, precision={t_uint.precision}, offset={t_uint.offset}")

# Get 1:20 ADC calibration
print("\n20. Getting 1:20 ADC calibration")
res = rp_hw_calib.rp_CalibGetFastADCCalibValue_1_20(0, 0)
print(f"ADC 1:20 float calib: {res}")

t_uint = rp_hw_calib.new_p_uint_gain_calib_t()
res = rp_hw_calib.rp_CalibGetFastADCCalibValue_1_20I(0, 0, t_uint)
print(f"ADC 1:20 int calib: gain={t_uint.gain}, base={t_uint.base}, precision={t_uint.precision}, offset={t_uint.offset}")

# Get DAC calibration
print("\n21. Getting DAC calibration")
res = rp_hw_calib.rp_CalibGetFastDACCalibValue(0, 0)
print(f"DAC float calib: {res}")

t_uint = rp_hw_calib.new_p_uint_gain_calib_t()
res = rp_hw_calib.rp_CalibGetFastDACCalibValueI(0, 0, t_uint)
print(f"DAC int calib: gain={t_uint.gain}, base={t_uint.base}, precision={t_uint.precision}, offset={t_uint.offset}")

# Write current settings back
print("\n22. Writing calibration settings back")
res = rp_hw_calib.rp_CalibrationWriteParams(current_settings, False)
print(f"Write result: {res}")

res = rp_hw_calib.rp_CalibrationWriteParams(current_settings, False)
print(f"Write again result: {res}")

# Get EEPROM sizes
print("\n23. Getting EEPROM sizes")
res = rp_hw_calib.rp_CalibGetMaxSizeEepromData()
print(f"Max EEPROM data size: {res}")

res = rp_hw_calib.rp_CalibGetMaxSizeEepromDataUnify()
print(f"Max unified EEPROM data size: {res}")

# Convenience function - get and convert in one call
print("\n24. Convenience function (get and convert)")
status, calib = rp_hw_calib.rp_CalibGetAndConvertNP(False)
print(f"Status: {status}")
if calib is not None:
    print(f"Calibration obtained successfully")
    print(f"ADC count 1:1 = {calib.fast_adc_count_1_1}")
    rp_hw_calib.rp_CalibPrint(calib)

print("\n" + "=" * 60)
print("Test completed successfully!")
print("=" * 60)