#!/usr/bin/python3

import time
import rp_api_250

print("rp_api_250.rp_spi_disable_verbous()")
res = rp_api_250.rp_spi_disable_verbous()
print(res)

print("rp_api_250.rp_spi_enable_verbous()")
res = rp_api_250.rp_spi_enable_verbous()
print(res)

print("rp_api_250.rp_spi_load_via_fpga(\"/opt/redpitaya/lib/configs/AD9613BCPZ-250.xml\")")
res = rp_api_250.rp_spi_load_via_fpga("/opt/redpitaya/lib/configs/AD9613BCPZ-250.xml")
print(res)

print("rp_api_250.rp_write_to_spi_fpga(\"/dev/mem\",0x40000000,0x50,0xFF,0)")
res = rp_api_250.rp_write_to_spi_fpga("/dev/mem",0x40000000,0x50,0xFF,0)
print(res)

print("rp_api_250.rp_read_from_spi_fpga(\"/dev/mem\",0x40000000,0x50,0x14)")
res = rp_api_250.rp_read_from_spi_fpga("/dev/mem",0x40000000,0x50,0x14)
print(res)

print("rp_api_250.rp_write_to_spi_fpga(\"/dev/mem\",0x40000000,0x50,0xFF,1)")
res = rp_api_250.rp_write_to_spi_fpga("/dev/mem",0x40000000,0x50,0xFF,1)
print(res)


print("rp_api_250.rp_i2c_disable_verbous()")
res = rp_api_250.rp_i2c_disable_verbous()
print(res)

print("rp_api_250.rp_i2c_enable_verbous()")
res = rp_api_250.rp_i2c_enable_verbous()
print(res)

print("rp_api_250.rp_i2c_print(\"/opt/redpitaya/lib/configs/SI571.xml\",True)")
res = rp_api_250.rp_i2c_print("/opt/redpitaya/lib/configs/SI571.xml",True)
print(res)

print("rp_api_250.rp_i2c_load(\"/opt/redpitaya/lib/configs/SI571.xml\",True)")
res = rp_api_250.rp_i2c_load("/opt/redpitaya/lib/configs/SI571.xml",True)
print(res)

print("rp_api_250.rp_i2c_compare(\"/opt/redpitaya/lib/configs/SI571.xml\",True)")
res = rp_api_250.rp_i2c_compare("/opt/redpitaya/lib/configs/SI571.xml",True)
print(res)

print("rp_api_250.rp_i2c_print(\"/opt/redpitaya/lib/configs/SI571.xml\",True)")
res = rp_api_250.rp_i2c_print("/opt/redpitaya/lib/configs/SI571.xml",True)
print(res)

print("rp_api_250.mcp47x6(rp_api_250.MCP4716,\"/dev/i2c-0\")")
obj = rp_api_250.mcp47x6(rp_api_250.MCP4716,"/dev/i2c-0")
print(obj)

print("obj.getMaxLevel()")
res = obj.getMaxLevel()
print(res)

print("obj.readConfig()")
res = obj.readConfig()
print(res)

print("obj.getGain()")
res = obj.getGain()
print(res)

print("obj.getGainEeprom()")
res = obj.getGainEeprom()
print(res)

print("obj.getPowerDown()")
res = obj.getPowerDown()
print(res)

print("obj.getPowerDownEeprom()")
res = obj.getPowerDownEeprom()
print(res)

print("obj.getVReferenc()")
res = obj.getVReferenc()
print(res)

print("obj.getVReferencEeprom()")
res = obj.getVReferencEeprom()
print(res)

print("obj.getOutputLevel()")
res = obj.getOutputLevel()
print(res)

print("obj.getOutputLevelEeprom()")
res = obj.getOutputLevelEeprom()
print(res)

print("obj.setGain(rp_api_250.MCP47X6_GAIN_2X)")
res = obj.setGain(rp_api_250.MCP47X6_GAIN_2X)
print(res)

print("obj.setPowerDown(rp_api_250.MCP47X6_AWAKE)")
res = obj.setPowerDown(rp_api_250.MCP47X6_AWAKE)
print(res)

print("obj.setVReference(rp_api_250.MCP47X6_VREF_VDD)")
res = obj.setVReference(rp_api_250.MCP47X6_VREF_VDD)
print(res)

print("obj.setOutputLevel(0)")
res = obj.setOutputLevel(0)
print(res)

print("obj.writeConfig()")
res = obj.writeConfig()
print(res)


print("rp_api_250.rp_initController()")
res = rp_api_250.rp_initController()
print(res)

print("rp_api_250.rp_check()")
res = rp_api_250.rp_check()
print(res)

time.sleep(1)

print("rp_api_250.rp_setAC_DC(rp_api_250.RP_MAX7311_IN1,rp_api_250.RP_DC_MODE)")
res = rp_api_250.rp_setAC_DC(rp_api_250.RP_MAX7311_IN1,rp_api_250.RP_DC_MODE)
print(res)

time.sleep(1)

print("rp_api_250.setPIN_GROUP(rp_api_250.PIN_K1,rp_api_250.RP_AC_MODE)")
res = rp_api_250.setPIN_GROUP(rp_api_250.PIN_K1,rp_api_250.RP_AC_MODE)
print(res)

time.sleep(1)

print("rp_api_250.rp_setAC_DC(rp_api_250.RP_MAX7311_IN2,rp_api_250.RP_DC_MODE)")
res = rp_api_250.rp_setAC_DC(rp_api_250.RP_MAX7311_IN2,rp_api_250.RP_DC_MODE)
print(res)


time.sleep(1)

print("rp_api_250.rp_setAttenuator(rp_api_250.RP_MAX7311_IN1,rp_api_250.RP_ATTENUATOR_1_20)")
res = rp_api_250.rp_setAttenuator(rp_api_250.RP_MAX7311_IN1,rp_api_250.RP_ATTENUATOR_1_20)
print(res)

time.sleep(1)

print("rp_api_250.rp_setAttenuator(rp_api_250.RP_MAX7311_IN2,rp_api_250.RP_ATTENUATOR_1_20)")
res = rp_api_250.rp_setAttenuator(rp_api_250.RP_MAX7311_IN2,rp_api_250.RP_ATTENUATOR_1_20)
print(res)

time.sleep(1)

print("rp_api_250.rp_setGainOut(rp_api_250.RP_MAX7311_OUT1,rp_api_250.RP_GAIN_10V)")
res = rp_api_250.rp_setGainOut(rp_api_250.RP_MAX7311_OUT1,rp_api_250.RP_GAIN_10V)
print(res)

time.sleep(1)

print("rp_api_250.rp_setGainOut(rp_api_250.RP_MAX7311_OUT2,rp_api_250.RP_GAIN_10V)")
res = rp_api_250.rp_setGainOut(rp_api_250.RP_MAX7311_OUT2,rp_api_250.RP_GAIN_10V)
print(res)

print("rp_api_250.rp_initController()")
res = rp_api_250.rp_initController()
print(res)