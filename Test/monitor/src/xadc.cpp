#include <stdio.h>
#include "rp.h"
#include "rp_hw.h"


void set_DAC(float *values,int count){
    rp_InitReset(false);
    for (int i = 0 ;  i < count; ++i){
        rp_AOpinSetValue(i,values[i]);
    }
    rp_Release();
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

static void AmsList()
{
	uint32_t i,raw;
	float val;
	printf("#ID\tDesc\t\tRaw\tVal\n");
	for(i=0;i<eSendNum;i++){
		switch(i){
			case eAmsTemp:
			    val = rp_GetCPUTemperature(&raw);
			break;
			case eAmsAI0:
				rp_ApinGetValue(RP_AIN0,&val,&raw);
			break;
			case eAmsAI1:
				rp_ApinGetValue(RP_AIN1,&val,&raw);
			break;
			case eAmsAI2:
				rp_ApinGetValue(RP_AIN2,&val,&raw);
			break;
			case eAmsAI3:
				rp_ApinGetValue(RP_AIN3,&val,&raw);
				break;
			case eAmsAI4:
				rp_GetPowerI4(&raw,&val);
				break;
			case eAmsVCCPINT:
				rp_GetPowerVCCPINT(&raw,&val);
				break;
			case eAmsVCCPAUX:
				rp_GetPowerVCCPAUX(&raw,&val);
				break;
			case eAmsVCCBRAM:
				rp_GetPowerVCCBRAM(&raw,&val);
				break;
			case eAmsVCCINT:
				rp_GetPowerVCCINT(&raw,&val);
				break;
			case eAmsVCCAUX:
				rp_GetPowerVCCAUX(&raw,&val);
				break;
			case eAmsVCCDDR:
				rp_GetPowerVCCDDR(&raw,&val);
				break;
			case eAmsAO0:
				rp_ApinGetValue(RP_AOUT0,&val,&raw);
				break;
			case eAmsAO1:
				rp_ApinGetValue(RP_AOUT1,&val,&raw);
				break;
			case eAmsAO2:
				rp_ApinGetValue(RP_AOUT2,&val,&raw);
				break;
			case eAmsAO3:
				rp_ApinGetValue(RP_AOUT3,&val,&raw);
				break;
			case eSendNum:
				break;
		}
		printf("%d\t%s\t0x%08x\t%.3f\n",i,&amsDesc[i][0],raw,val);
	}
}


void showAMS(){
	rp_InitReset(false);
	AmsList();
	rp_Release();
}