
// Wrapper for compatibility with С code

#pragma once

#ifdef  __cplusplus
extern "C" {
#endif

#define RP_I2C_OK   0  // Normal
#define RP_I2C_EOOR 1  // Out of range
#define RP_I2C_EFRB 2  // Error read form i2c 
#define RP_I2C_EFWB 3  // Error write to  i2c

int rp_setExtTriggerLevel(float  voltage);
int rp_getExtTriggerLevel(float *voltage);

#ifdef  __cplusplus
}
#endif