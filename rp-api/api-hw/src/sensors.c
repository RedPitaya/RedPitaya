#include <stdio.h>
#include "rp_hw.h"
#include "sensors.h"
#include "rp_log.h"


int sens_GetValueFromFile(const char* file, uint32_t *value){
    FILE *fp;
    fp = fopen (file, "r");
	if (fp==0) {
		ERROR_LOG("Can't open %s",file);
		return -1;
	}
    int rv = 0;
    int r = fscanf (fp, "%d", &rv) != 1;
    *value = rv;
    fclose(fp);
    return r;
}

int sens_GetFValueFromFile(const char* file, float *value){
    FILE *fp;
    fp = fopen (file, "r");
	if (fp==0) {
		ERROR_LOG("Can't open %s",file);
		return -1;
	}
    float r = fscanf (fp, "%f", value) != 1;
    fclose(fp);
    return r;
}

int GetTempValueRaw( uint32_t* raw , float* value) {
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
    float scale = 0;
    char s_raw[200];
    char s_scale[200];
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
