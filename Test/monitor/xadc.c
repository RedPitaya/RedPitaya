#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <stdint.h>



#define ADC_FULL_RANGE_CNT 0xfff
#define ADC_POS_RANGE_CNT  0x7ff

#define SLOW_DAC_NUM 4
#define SLOW_DAC_RANGE_CNT 0x9c
#define ioread32(p) (*(volatile uint32_t *)(p))
#define iowrite32(v,p) (*(volatile uint32_t *)(p) = (v))

// Base Analog Mixed Signals address
static const int ANALOG_MIXED_SIGNALS_BASE_ADDR = 0x00400000;
static const int ANALOG_MIXED_SIGNALS_BASE_SIZE = 0x30;
static const uint32_t ANALOG_OUT_MASK            = 0xFF;
static const uint32_t ANALOG_OUT_BITS            = 16;
static const uint32_t ANALOG_IN_MASK             = 0xFFF;

static const float    ANALOG_IN_MAX_VAL          = 7.0;
static const float    ANALOG_IN_MIN_VAL          = 0.0;
static const uint32_t ANALOG_IN_MAX_VAL_INTEGER  = 0xFFF;
static const float    ANALOG_OUT_MAX_VAL         = 1.8;
static const float    ANALOG_OUT_MIN_VAL         = 0.0;
static const uint32_t ANALOG_OUT_MAX_VAL_INTEGER = 156;

typedef struct analog_mixed_signals_control_s {
    uint32_t aif [4];
    uint32_t reserved[4];
    uint32_t dac [4];
} analog_mixed_signals_control_t;

static volatile analog_mixed_signals_control_t *ams = NULL;
static int fd = 0;

int AOpinSetValueRaw(int unsigned pin, uint32_t value);

int cmn_Init()
{
    if (!fd) {
        if((fd = open("/dev/uio/api", O_RDWR | O_SYNC)) == -1) {
            return -1;
        }
    }
    return 1;
}

int cmn_Release()
{
    if (fd) {
        if(close(fd) < 0) {
            return -1;
        }
    }

    return 1;
}

int cmn_Map(size_t size, size_t offset, void** mapped)
{
    if(fd == -1) {
        return -1;
    }

    offset = (offset >> 20) * sysconf(_SC_PAGESIZE);

    *mapped = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, offset);

    if(mapped == (void *) -1) {
        return -1;
    }

    return 0;
}

int cmn_Unmap(size_t size, void** mapped)
{
    if(fd == -1) {
        return -1;
    }

    if((mapped == (void *) -1) || (mapped == NULL)) {
        return -1;
    }

    if((*mapped == (void *) -1) || (*mapped == NULL)) {
        return -1;
    }

    if(munmap(*mapped, size) < 0){
        return -1;
    }
    *mapped = NULL;
    return 0;
}

int ams_Init() {
    cmn_Map(ANALOG_MIXED_SIGNALS_BASE_SIZE, ANALOG_MIXED_SIGNALS_BASE_ADDR, (void**)&ams);
    return 0;
}

int ams_Release() {
    cmn_Unmap(ANALOG_MIXED_SIGNALS_BASE_SIZE, (void**)&ams);
    return 0;
}


int AOpinReset() {
    for (int unsigned pin=0; pin<4; pin++) {
        AOpinSetValueRaw(pin, 0);
    }
    return 0;
}

int AOpinSetValueRaw(int unsigned pin, uint32_t value) {
    if (pin >= 4) {
        return -1;
    }
    if (value > ANALOG_OUT_MAX_VAL_INTEGER) {
        return -1;
    }
    iowrite32((value & ANALOG_OUT_MASK) << ANALOG_OUT_BITS, &ams->dac[pin]);
    return 0;
}

int AOpinSetValue(int unsigned pin, float value) {
    uint32_t value_raw = (uint32_t) (((value - ANALOG_OUT_MIN_VAL) / (ANALOG_OUT_MAX_VAL - ANALOG_OUT_MIN_VAL)) * ANALOG_OUT_MAX_VAL_INTEGER);
    return AOpinSetValueRaw(pin, value_raw);
}

int AOpinGetValueRaw(int unsigned pin, uint32_t* value) {
    if (pin >= 4) {
        return -1;
    }
    *value = (ioread32(&ams->dac[pin]) >> ANALOG_OUT_BITS) & ANALOG_OUT_MASK;
    return 0;
}

int AOpinGetValue(int unsigned pin, float* value) {
    uint32_t value_raw;
    int result = AOpinGetValueRaw(pin, &value_raw);
    *value = (((float)value_raw / ANALOG_OUT_MAX_VAL_INTEGER) * (ANALOG_OUT_MAX_VAL - ANALOG_OUT_MIN_VAL)) + ANALOG_OUT_MIN_VAL;
    return result;
}


void set_DAC(float *values,int count){
    cmn_Init();
    ams_Init();
    for (int i = 0 ;  i < count; ++i){
        AOpinSetValue(i,values[i]);
    }
    ams_Release();
    cmn_Release();
}

int AIpinGetValueRaw(int unsigned pin, uint32_t* value) {
    FILE *fp;
    switch (pin) {
        case 0:  fp = fopen ("/sys/devices/soc0/amba_pl/83c00000.xadc_wiz/iio:device1/in_voltage11_vaux8_raw", "r");  break;
        case 1:  fp = fopen ("/sys/devices/soc0/amba_pl/83c00000.xadc_wiz/iio:device1/in_voltage9_vaux0_raw" , "r");  break;
        case 2:  fp = fopen ("/sys/devices/soc0/amba_pl/83c00000.xadc_wiz/iio:device1/in_voltage10_vaux1_raw", "r");  break;
        case 3:  fp = fopen ("/sys/devices/soc0/amba_pl/83c00000.xadc_wiz/iio:device1/in_voltage12_vaux9_raw", "r");  break;
        case 4:  fp = fopen ("/sys/devices/soc0/amba_pl/83c00000.xadc_wiz/iio:device1/in_voltage8_vpvn_raw", "r");  break;
        default:
            return -1;
    }
    int r = !fscanf (fp, "%d", value);
    fclose(fp);
    return r;
}

int GetValueFromFile(char *file, uint32_t *value){
    FILE *fp;
    fp = fopen (file, "r");
    int r = !fscanf (fp, "%d", value);
    fclose(fp);
    return r;
}

int GetFValueFromFile(char *file, float *value){
    FILE *fp;
    fp = fopen (file, "r");
    float r = !fscanf (fp, "%f", value);
    fclose(fp);
    return r;
}

int GetTempValueRaw( uint32_t* raw , float* value) {
    uint32_t offset = 0;    
    float scale = 0;
    GetValueFromFile("/sys/devices/soc0/amba_pl/83c00000.xadc_wiz/iio:device1/in_temp0_offset",&offset);
    GetValueFromFile("/sys/devices/soc0/amba_pl/83c00000.xadc_wiz/iio:device1/in_temp0_raw",raw);
    GetFValueFromFile("/sys/devices/soc0/amba_pl/83c00000.xadc_wiz/iio:device1/in_temp0_scale",&scale);
    *value = ((float)(offset + *raw)) * scale / 1000.0;
    return 0;
}

int GetValueRaw(char * file, uint32_t* raw , float* value) {
    float scale = 0;
    char s_raw[200];
    char s_scale[200];
    sprintf(s_raw,"/sys/devices/soc0/amba_pl/83c00000.xadc_wiz/iio:device1/%s_raw",file);
    sprintf(s_scale,"/sys/devices/soc0/amba_pl/83c00000.xadc_wiz/iio:device1/%s_scale",file);
    GetValueFromFile(s_raw , raw);
    GetFValueFromFile(s_scale , &scale);
    *value = ((float)*raw) * scale / 1000.0;
    return 0;
}


typedef enum {
	eAmsTemp=0,
	eAmsAI0,
	eAmsAI1,
	eAmsAI2,
	eAmsAI3,
	eAmsAI4,
	eAmsVCCPINT,
	eAmsVCCPAUX,
	eAmsVCCBRAM,
	eAmsVCCINT,
	eAmsVCCAUX,
	eAmsVCCDDR,
	eAmsAO0,
	eAmsAO1,
	eAmsAO2,
	eAmsAO3,
	eSendNum
} ams_t;

const uint8_t amsDesc[eSendNum][20]={
	"Temp(0C-85C)",
	"AI0(0-3.5V)",
	"AI1(0-3.5V)",
	"AI2(0-3.5V)",
	"AI3(0-3.5V)",
	"AI4(5V0)",
	"VCCPINT(1V0)",
	"VCCPAUX(1V8)",
	"VCCBRAM(1V0)",
	"VCCINT(1V0)",
	"VCCAUX(1V8)",
	"VCCDDR(1V5)",
	"AO0(0-1.8V)",
	"AO1(0-1.8V)",
	"AO2(0-1.8V)",
	"AO3(0-1.8V)",
};


typedef struct {
	uint32_t aif[5];
	uint32_t reserved[3];
	uint32_t dac[SLOW_DAC_NUM];
	uint32_t temp;
    float    temp_val;
	uint32_t vccPint;
	uint32_t vccPaux;
	uint32_t vccBram;
	uint32_t vccInt;
	uint32_t vccAux;
	uint32_t vccDddr;
    float vccPintV;
	float vccPauxV;
	float vccBramV;
	float vccIntV;
	float vccAuxV;
	float vccDddrV;
} amsS_t;




static float AmsConversion(ams_t a_ch, unsigned int a_raw,amsS_t * reg)
{
	float uAdc;
	float val=0;
	switch(a_ch){
		case eAmsAI0:
		case eAmsAI1:
		case eAmsAI2:
		case eAmsAI3:{
			if(a_raw>0x7ff){
				a_raw=0;
			}
			uAdc=(float)a_raw/0x7ff*0.5;
			val=uAdc*(30.0+4.99)/4.99;
		}
		break;
		case eAmsAI4:{
			uAdc=(float)a_raw/ADC_FULL_RANGE_CNT*1.0;
			val=uAdc*(56.0+4.99)/4.99;
		}
		break;
		case eAmsTemp:{
			// val=((float)a_raw*503.975) / ADC_FULL_RANGE_CNT - 273.15;
            val = reg->temp_val;
		}
		break;
		case eAmsVCCPINT:{
			val= reg->vccPintV;
		}
		break;
		case eAmsVCCPAUX:{
			val= reg->vccPauxV;
		}
		break;
		case eAmsVCCBRAM:{
			val= reg->vccBramV;
		}
		break;
		case eAmsVCCINT:{
			val= reg->vccIntV;
		}
		break;
		case eAmsVCCAUX:{
			val= reg->vccAuxV;
		}
		break;
		case eAmsVCCDDR:{
			val= reg->vccDddrV;
		}
		break;
		case eAmsAO0:
		case eAmsAO1:
		case eAmsAO2:
		case eAmsAO3:
			val=((float)(a_raw)/SLOW_DAC_RANGE_CNT)*1.8;
		break;
		case eSendNum:
			break;
	}
	return val;
}

static void AmsList(amsS_t * a_amsReg)
{
	uint32_t i,raw;
	float val;
	printf("#ID\tDesc\t\tRaw\tVal\n");
	for(i=0;i<eSendNum;i++){
		switch(i){
			case eAmsTemp:
			    raw=a_amsReg->temp;
			break;
			case eAmsAI0:
				raw=a_amsReg->aif[0];
			break;
			case eAmsAI1:
				raw=a_amsReg->aif[1];
			break;
			case eAmsAI2:
				raw=a_amsReg->aif[2];
			break;
			case eAmsAI3:
				raw=a_amsReg->aif[3];
				break;
			case eAmsAI4:
				raw=a_amsReg->aif[4];
				break;
			case eAmsVCCPINT:
				raw=a_amsReg->vccPint;
				break;
			case eAmsVCCPAUX:
				raw=a_amsReg->vccPaux;
				break;
			case eAmsVCCBRAM:
				raw=a_amsReg->vccBram;
				break;
			case eAmsVCCINT:
				raw=a_amsReg->vccInt;
				break;
			case eAmsVCCAUX:
				raw=a_amsReg->vccAux;
				break;
			case eAmsVCCDDR:
				raw=a_amsReg->vccDddr;
				break;
			case eAmsAO0:
				raw=a_amsReg->dac[0];
				break;
			case eAmsAO1:
				raw=a_amsReg->dac[1];
				break;
			case eAmsAO2:
				raw=a_amsReg->dac[2];
				break;
			case eAmsAO3:
				raw=a_amsReg->dac[3];
				break;
			case eSendNum:
				break;
		}
		val=AmsConversion(i, raw,a_amsReg);
		printf("%d\t%s\t0x%08x\t%.3f\n",i,&amsDesc[i][0],raw,val);
	}
}


void showAMS(){
    amsS_t reg;
    cmn_Init();
    ams_Init();
    AOpinGetValueRaw(0,&reg.dac[0]);
    AOpinGetValueRaw(1,&reg.dac[1]);
    AOpinGetValueRaw(2,&reg.dac[2]);
    AOpinGetValueRaw(3,&reg.dac[3]);

    AIpinGetValueRaw(0,&reg.aif[0]);
    AIpinGetValueRaw(1,&reg.aif[1]);
    AIpinGetValueRaw(2,&reg.aif[2]);
    AIpinGetValueRaw(3,&reg.aif[3]);
    AIpinGetValueRaw(4,&reg.aif[4]);
     
    GetTempValueRaw(&reg.temp,&reg.temp_val);
    GetValueRaw("in_voltage0_vccint",&reg.vccInt,&reg.vccIntV);
    GetValueRaw("in_voltage4_vccpaux",&reg.vccPaux,&reg.vccPauxV);
    GetValueRaw("in_voltage3_vccpint",&reg.vccPint,&reg.vccPintV);
    GetValueRaw("in_voltage2_vccbram",&reg.vccBram,&reg.vccBramV);
    GetValueRaw("in_voltage1_vccaux",&reg.vccAux,&reg.vccAuxV);
    GetValueRaw("in_voltage5_vccoddr",&reg.vccDddr,&reg.vccDddrV);
    ams_Release();
    cmn_Release();
    AmsList(&reg);
}