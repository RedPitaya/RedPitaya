/**
 * $Id: $
 *
 * @brief Red Pitaya Hardware Profiles
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <dirent.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <stdbool.h>
#include <sys/queue.h>
#include <mtd/mtd-user.h>
#include "common.h"
#include "stem_125_10_v1.0.h"
#include "stem_125_14_v1.0.h"
#include "stem_125_14_v1.1.h"

#define LINE_LENGTH 0x400


profiles_t *g_profile = NULL;

// "STEM_125-14_v1.0" && "$HW_REV" != "STEM_125-14_v1.1" && "$HW_REV" != "STEM_125-14_LN_v1.1" && "$HW_REV" != "STEM_125-14_Z7020_v1.0" && "$HW_REV" != "STEM_125-14_Z7020_LN_v1.1"
// "STEM_125-14_v1.0" && "$HW_REV" != "STEM_125-14_Z7020_v1.0" && "$HW_REV" != "STEM_125-10_v1.0"
// "STEM_125-14_LN_CE1_v1.1"
// "STEM_125-14_LN_CE2_v1.1"
// "STEM_122-16SDR_v1.0"
// "STEM_125-14_Z7020_4IN_v1.0"
// "STEM_125-14_Z7020_4IN_v1.2"
// "STEM_125-14_Z7020_4IN_v1.3"

// "STEM_250-12_v1.2"  "STEM_250-12_v1.1"



void hp_checkModel(char *model,char *eth_mac){
	if (!model) return;
	if (strcmp(model,"STEM_125-10_v1.0") == 0){
		g_profile = getProfile_STEM_125_10_v1_0();
		strcpy(g_profile->boardModelEEPROM,model);
		if (eth_mac)
			strcpy(g_profile->boardETH_MAC,eth_mac);
	}
	if (strcmp(model,"STEM_125-14_v1.0") == 0){
		g_profile = getProfile_STEM_125_14_v1_0();
		strcpy(g_profile->boardModelEEPROM,model);
		if (eth_mac)
			strcpy(g_profile->boardETH_MAC,eth_mac);
	}

	if (strcmp(model,"STEM_125-14_v1.1") == 0){
		g_profile = getProfile_STEM_125_14_v1_1();
		strcpy(g_profile->boardModelEEPROM,model);
		if (eth_mac)
			strcpy(g_profile->boardETH_MAC,eth_mac);
	}
}

int hp_cmn_Init(){
	char *buf;
	char *name, *value;
    char *model = NULL;
    char *eth_mac = NULL;

	FILE *fp = fopen("/sys/bus/i2c/devices/0-0050/eeprom", "r");
	if (!fp)
		return RP_HP_ERE;

	if(fseek(fp, 0x1800	, SEEK_SET) < 0) {
        fclose(fp);
        return RP_HP_ERE;
    }

	buf = (char *)malloc(LINE_LENGTH);
	if (!buf) {
		fclose(fp);
		return RP_HP_EAL;
	}


	int size = fread(buf, sizeof(char), LINE_LENGTH, fp);
	int position = 0;
	while(position <  size){
		int slen = strlen(&buf[position]);
		if (!slen) break;
		name = &buf[position];
		value = strchr(name, '=');
	 	if (!value)
	 		break;
		*value++ = '\0';
		if (!strlen(value))
			value = NULL;

		if (!strcmp(name,"hw_rev") && value != NULL){
            if (strlen(value)+1 < 255){
                model = (char*)malloc(strlen(value)+1);
                if (model)
                    strcpy(model,value);
            }
        }

        if (!strcmp(name,"ethaddr") && value != NULL){
            if (strlen(value)+1 < 20){
                eth_mac = (char*)malloc(strlen(value)+1);
                if (eth_mac)
                    strcpy(eth_mac,value);
            }
        }
		position += slen + 1;

	}

	fclose(fp);
	free(buf);
    if (!model){
        if (eth_mac) free(eth_mac);
        return RP_HP_ERM;
    }
	hp_checkModel(model,eth_mac);
    if (model) free(model);
    if (eth_mac) free(eth_mac);
    return RP_HP_OK;
}

profiles_t* hp_cmn_GetLoadedProfile(){
    return g_profile;
}

int hp_cmn_Print(profiles_t *p){
	if (!p) {
		return RP_HP_EU;
	}
	fprintf(stdout,"***********************************************************************\n");
	fprintf(stdout,"Board\n");
	fprintf(stdout,"\t* Board model (rp_HPeModels_t) %d\n",p->boardModel);
	fprintf(stdout,"\t* Board name: %s\n",p->boardName);
	fprintf(stdout,"\t* Board model from eeprom: %s\n",p->boardModelEEPROM);
	fprintf(stdout,"\t* Board MAC address from eeprom: %s\n",p->boardETH_MAC);
	fprintf(stdout,"\t* Zynq model (rp_HPeZynqModels_t) %d\n",p->zynqCPUModel);
	fprintf(stdout,"\t* Oscillator Rate: %u\n",p->oscillator_rate);

	fprintf(stdout,"FAST ADC\n");
	fprintf(stdout,"\t* Rate: %u\n",p->fast_adc_rate);
	fprintf(stdout,"\t* Count: %u\n",p->fast_adc_count_channels);

	for(int i = 0 ; i < p->fast_adc_count_channels; i++){
		fprintf(stdout,"\t\t- Is signed: %u\n",p->fast_adc[i].is_signed);
		fprintf(stdout,"\t\t- Bits: %u\n",p->fast_adc[i].bits);
		fprintf(stdout,"\t\t- Full scale: %f\n",p->fast_adc[i].fullScale);
	}


	fprintf(stdout,"FAST DAC\n");
	fprintf(stdout,"\t* Rate: %u\n",p->fast_dac_rate);
	fprintf(stdout,"\t* Count: %u\n",p->fast_dac_count_channels);

	for(int i = 0 ; i < p->fast_dac_count_channels; i++){
		fprintf(stdout,"\t\t- Is signed: %u\n",p->fast_dac[i].is_signed);
		fprintf(stdout,"\t\t- Bits: %u\n",p->fast_dac[i].bits);
		fprintf(stdout,"\t\t- Full scale: %f\n",p->fast_dac[i].fullScale);
	}

	fprintf(stdout,"FAST ADC HV mode (1:20): %d\n",p->is_LV_HV_mode);

	for(int i = 0 ; i < p->fast_adc_count_channels; i++){
		fprintf(stdout,"\t\t- Is signed: %u\n",p->fast_adc_1_20[i].is_signed);
		fprintf(stdout,"\t\t- Bits: %u\n",p->fast_adc_1_20[i].bits);
		fprintf(stdout,"\t\t- Full scale: %f\n",p->fast_adc_1_20[i].fullScale);
	}

	fprintf(stdout,"FAST ADC AD/DC mode: %d\n",p->is_LV_HV_mode);

	fprintf(stdout,"SLOW ADC\n");
	fprintf(stdout,"\t* Count: %u\n",p->slow_adc_count_channels);

	for(int i = 0 ; i < p->slow_adc_count_channels; i++){
		fprintf(stdout,"\t\t- Is signed: %u\n",p->slow_adc[i].is_signed);
		fprintf(stdout,"\t\t- Bits: %u\n",p->slow_adc[i].bits);
		fprintf(stdout,"\t\t- Full scale: %f\n",p->slow_adc[i].fullScale);
	}

	fprintf(stdout,"SLOW DAC\n");
	fprintf(stdout,"\t* Count: %u\n",p->slow_dac_count_channels);

	for(int i = 0 ; i < p->slow_dac_count_channels; i++){
		fprintf(stdout,"\t\t- Is signed: %u\n",p->slow_dac[i].is_signed);
		fprintf(stdout,"\t\t- Bits: %u\n",p->slow_dac[i].bits);
		fprintf(stdout,"\t\t- Full scale: %f\n",p->slow_dac[i].fullScale);
	}

	fprintf(stdout,"FAST DAC x5 gain: %d\n",p->is_DAC_gain_x5);
	fprintf(stdout,"***********************************************************************\n");
	return RP_HP_OK;
}