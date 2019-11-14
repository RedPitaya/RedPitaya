#include <math.h>
#include "rp-i2c-mcp47x6-c.h"
#include "rp-i2c-mcp47x6.h"

#define EXT_TRIGGER_MAX_VOL 2.5

int rp_setExtTriggerLevel(float voltage){

    RP_MCP47X6::mcp47x6 chip(RP_MCP47X6::mcp47x6_model::MCP4716,"/dev/i2c-0");
    if (!chip.readConfig()){
        return RP_I2C_EFRB;
    }
    voltage = voltage / 2.0;  // Convert UI (+/-5V) to (chip) +/-2.5
    if (fabsf(voltage) > EXT_TRIGGER_MAX_VOL){
        return RP_I2C_EOOR;
    }
    
    chip.setGain(MCP47X6_GAIN_2X);        // VREF is 1.25V, need gain x2 for output max 2.5v 
    chip.setPowerDown(MCP47X6_AWAKE);     // POWER ON
    chip.setVReference(MCP47X6_VREF_VDD); // SET INPUT DC FROM VREF
    float max_cnt = (float)chip.getMaxLevel();
    short cnt =  (short)((fabsf(voltage) / EXT_TRIGGER_MAX_VOL) * max_cnt); //Volt to cnt
    chip.setOutputLevel(cnt); 
    if (!chip.writeConfig()){
        return RP_I2C_EFWB;
    }
    return RP_I2C_OK;
}


int rp_getExtTriggerLevel(float *voltage){
     RP_MCP47X6::mcp47x6 chip(RP_MCP47X6::mcp47x6_model::MCP4716,"/dev/i2c-0");
    if (!chip.readConfig()){
        return RP_I2C_EFRB;
    }
    if (MCP47X6_AWAKE == chip.getPowerDown()){
        float gain = chip.getGain() == MCP47X6_GAIN_2X ? 2.0 : 1.0;           
        float max_cnt = (float)chip.getMaxLevel();
        float cnt     = (float)chip.getOutputLevel(); 
        cnt =  ((cnt / max_cnt) * EXT_TRIGGER_MAX_VOL) / gain; //Volt to cnt
        *voltage = cnt * 2.0; // Convert (chip) +/-2.5 to UI (+/-5V) 
    }else{
        *voltage = 0;
    }
    return RP_I2C_OK;
}