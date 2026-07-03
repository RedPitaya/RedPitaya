#include <stdio.h>
#include <errno.h>
#include "rp_hw.h"
#include "sensors.h"
#include "rp_log.h"


int sens_GetValueFromFile(const char* file, uint32_t *value){

    if (!file || !value) {
        ERROR_LOG("NULL pointer");
        return -1;
    }

    FILE *fp = fopen (file, "r");
	if (!fp) {
        ERROR_LOG("Can't open %s: %s", file, strerror(errno));
        return -1;
    }
    int ret = fscanf(fp, "%u", value);
    fclose(fp);

    if (ret != 1) {
        ERROR_LOG("Failed to read value from %s", file);
        return -1;
    }

    return 0;
}

int sens_GetFValueFromFile(const char* file, float *value){
    if (!file || !value) {
        ERROR_LOG("NULL pointer");
        return -1;
    }

    FILE *fp = fopen(file, "r");
    if (!fp) {
        ERROR_LOG("Can't open %s: %s", file, strerror(errno));
        return -1;
    }

    int ret = fscanf(fp, "%f", value);
    fclose(fp);

    if (ret != 1) {
        ERROR_LOG("Failed to read float from %s", file);
        return -1;
    }

    return 0;
}

int GetTempValueRaw( uint32_t* raw , float* value) {
    if (!raw || !value) {
        ERROR_LOG("NULL pointer");
        return -1;
    }
    uint32_t offset = 0;
    float scale = 0;
    if (sens_GetValueFromFile("/sys/devices/soc0/axi/83c00000.xadc_wiz/iio:device1/in_temp0_offset",&offset))
        return -1;
    if (sens_GetValueFromFile("/sys/devices/soc0/axi/83c00000.xadc_wiz/iio:device1/in_temp0_raw",raw))
        return -1;
    if (sens_GetFValueFromFile("/sys/devices/soc0/axi/83c00000.xadc_wiz/iio:device1/in_temp0_scale",&scale))
        return -1;
    *value = ((float)(offset + *raw)) * scale / 1000.0;
    return 0;
}

int GetValueRaw(const char* file, uint32_t* raw , float* value) {
    if (!file || !raw || !value) {
        ERROR_LOG("NULL pointer");
        return -1;
    }
    float scale = 0;
    char s_raw[255];
    char s_scale[255];
    sprintf(s_raw,"/sys/devices/soc0/axi/83c00000.xadc_wiz/iio:device1/%s_raw",file);
    sprintf(s_scale,"/sys/devices/soc0/axi/83c00000.xadc_wiz/iio:device1/%s_scale",file);
    if (sens_GetValueFromFile(s_raw , raw))
        return -1;
    if (sens_GetFValueFromFile(s_scale , &scale))
        return -1;
    *value = ((float)*raw) * scale / 1000.0;
    return 0;
}

float sens_GetCPUTemp(uint32_t *raw){
    if (!raw) {
        ERROR_LOG("NULL pointer");
        return -1;
    }
    float temp_val = 0;
    if (GetTempValueRaw(raw,&temp_val))
        return -1;
    return temp_val;
}

int AI4pinGetValueRaw(uint32_t* value) {
    FILE *fp = fopen ("/sys/devices/soc0/axi/83c00000.xadc_wiz/iio:device1/in_voltage12_raw", "r");
    if (!fp){
        ERROR_LOG("Can't open %s","/sys/devices/soc0/axi/83c00000.xadc_wiz/iio:device1/in_voltage12_raw");
        return -1;
    }
    int r = fscanf (fp, "%u", value)  != 1;
    fclose(fp);
    return r;
}

int sens_GetPowerI4(uint32_t *raw,float* value){
    uint32_t value_raw;
    int result = AI4pinGetValueRaw(&value_raw);
    float uAdc=(float)value_raw/0xfff;
    *raw = value_raw;
	*value = uAdc*(56.0+4.99)/4.99;
    return result;
}

int sens_GetPowerVCCPINT(uint32_t *raw,float* value){
    return GetValueRaw("in_voltage3_vccpint",raw,value);
}

int sens_GetPowerVCCPAUX(uint32_t *raw,float* value){
    return GetValueRaw("in_voltage4_vccpaux",raw,value);
}

int sens_GetPowerVCCBRAM(uint32_t *raw,float* value){
    return GetValueRaw("in_voltage2_vccbram",raw,value);
}

int sens_GetPowerVCCINT(uint32_t *raw,float* value){
    return GetValueRaw("in_voltage0_vccint",raw,value);
}

int sens_GetPowerVCCAUX(uint32_t *raw,float* value){
    return GetValueRaw("in_voltage1_vccaux",raw,value);
}

int sens_GetPowerVCCDDR(uint32_t *raw,float* value){
    return GetValueRaw("in_voltage5_vccoddr",raw,value);
}
