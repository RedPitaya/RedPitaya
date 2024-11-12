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
#include <ctype.h>
#include <stdlib.h>
#include <stdint.h>
#include <dirent.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/queue.h>
#include <mtd/mtd-user.h>
#include "common.h"
#include "stem_125_10_v1.0.h"
#include "stem_125_14_v1.0.h"
#include "stem_125_14_v1.1.h"
#include "stem_122_16SDR_v1.0.h"
#include "stem_122_16SDR_v1.1.h"
#include "stem_125_14_LN_v1.1.h"
#include "stem_125_14_Z7020_v1.0.h"
#include "stem_125_14_Z7020_LN_v1.1.h"
#include "stem_125_14_Z7020_4IN_v1.0.h"
#include "stem_125_14_Z7020_4IN_v1.2.h"
#include "stem_125_14_Z7020_4IN_v1.3.h"
#include "stem_250_12_v1.0.h"
#include "stem_250_12_v1.1.h"
#include "stem_250_12_v1.2.h"
#include "stem_250_12_v1.2a.h"
#include "stem_250_12_v1.2b.h"
#include "stem_250_12_120.h"
#include "stem_125_14_LN_BO_v1.1.h"
#include "stem_125_14_LN_CE1_v1.1.h"
#include "stem_125_14_LN_CE2_v1.1.h"
#include "stem_special.h"
#include "stem_125_14_v2.0.h"
#include "stem_125_14_Pro_v2.0.h"
#include "stem_125_14_Pro_v2.0.h"
#include "stem_125_14_Z7020_Pro_v2.0.h"
#include "stem_125_14_Z7020_Ind_v2.0.h"

#define LINE_LENGTH 0x400


profiles_t *g_profile = NULL;

// "STEM_125-10_v1.0"
// "STEM_14_B_v1.0"
// "STEM_125-14_v1.0"
// "STEM_125-14_v1.1"
// "STEM_125-14_LN_v1.1"
// "STEM_125-14_Z7020_v1.0"
// "STEM_125-14_Z7020_LN_v1.1"
// "STEM_122-16SDR_v1.0"
// "STEM_122-16SDR_v1.1"
// "STEM_125-14_Z7020_4IN_v1.0"
// "STEM_125-14_Z7020_4IN_v1.2"
// "STEM_125-14_Z7020_4IN_v1.3"
// "STEM_250-12_v1.1"
// "STEM_250-12_v1.2"
// "STEM_250-12_v1.2a"
// "STEM_250-12_v1.2b"
// "STEM_250-12_v1.0"
// "STEM_250-12_120"
// "STEM_125-14_LN_BO_v1.1"
// "STEM_125-14_LN_CE1_v1.1"
// "STEM_125-14_LN_CE2_v1.1"

// Gen2

// "STEM_125-14_v2.0"
// "STEM_125-14_Pro_v2.0"
// "STEM_125-14_Z7020_Pro_v2.0"
// "STEM_125-14_Ind_v2.0"

void convertToLowerCase(char *buff){
	int size = strlen(buff);
	while(size > 0){
		size--;
		buff[size] = tolower(buff[size]);
	}
}


void hp_checkModel(char *model,char *eth_mac){
	char modelOrig[255];
	strcpy(modelOrig,model);
	convertToLowerCase(model);
	if (!model) return;

	if (strcmp(model,"stem_125-10_v1.0") == 0){
		g_profile = getProfile_STEM_125_10_v1_0();
		strcpy(g_profile->boardModelEEPROM,modelOrig);
		if (eth_mac)
			strcpy(g_profile->boardETH_MAC,eth_mac);
		return;
	}

	if (strcmp(model,"stem_125-14_v1.0") == 0){
		g_profile = getProfile_STEM_125_14_v1_0();
		strcpy(g_profile->boardModelEEPROM,modelOrig);
		if (eth_mac)
			strcpy(g_profile->boardETH_MAC,eth_mac);
		return;
	}

	if (strcmp(model,"stem_14_b_v1.0") == 0){
		g_profile = getProfile_STEM_125_14_v1_0();
		strcpy(g_profile->boardModelEEPROM,modelOrig);
		if (eth_mac)
			strcpy(g_profile->boardETH_MAC,eth_mac);
		return;
	}

	if (strcmp(model,"stem_125-14_v1.1") == 0){
		g_profile = getProfile_STEM_125_14_v1_1();
		strcpy(g_profile->boardModelEEPROM,modelOrig);
		if (eth_mac)
			strcpy(g_profile->boardETH_MAC,eth_mac);
		return;
	}

	if (strcmp(model,"stem_122-16sdr_v1.0") == 0){
		g_profile = getProfile_STEM_122_16SDR_v1_0();
		strcpy(g_profile->boardModelEEPROM,modelOrig);
		if (eth_mac)
			strcpy(g_profile->boardETH_MAC,eth_mac);
		return;
	}

	if (strcmp(model,"stem_122-16sdr_v1.1") == 0){
		g_profile = getProfile_STEM_122_16SDR_v1_1();
		strcpy(g_profile->boardModelEEPROM,modelOrig);
		if (eth_mac)
			strcpy(g_profile->boardETH_MAC,eth_mac);
		return;
	}

	if (strcmp(model,"stem_125-14_ln_v1.1") == 0){
		g_profile = getProfile_STEM_125_14_LN_v1_1();
		strcpy(g_profile->boardModelEEPROM,modelOrig);
		if (eth_mac)
			strcpy(g_profile->boardETH_MAC,eth_mac);
		return;
	}

	if (strcmp(model,"stem_125-14_z7020_v1.0") == 0){
		g_profile = getProfile_STEM_125_14_Z7020_v1_0();
		strcpy(g_profile->boardModelEEPROM,modelOrig);
		if (eth_mac)
			strcpy(g_profile->boardETH_MAC,eth_mac);
		return;
	}

	if (strcmp(model,"stem_125-14_z7020_ln_v1.1") == 0){
		g_profile = getProfile_STEM_125_14_Z7020_LN_v1_1();
		strcpy(g_profile->boardModelEEPROM,modelOrig);
		if (eth_mac)
			strcpy(g_profile->boardETH_MAC,eth_mac);
		return;
	}

	if (strcmp(model,"stem_125-14_z7020_4in_v1.0") == 0){
		g_profile = getProfile_STEM_125_14_Z7020_4IN_v1_0();
		strcpy(g_profile->boardModelEEPROM,modelOrig);
		if (eth_mac)
			strcpy(g_profile->boardETH_MAC,eth_mac);
		return;
	}

	if (strcmp(model,"stem_125-14_z7020_4in_v1.2") == 0){
		g_profile = getProfile_STEM_125_14_Z7020_4IN_v1_2();
		strcpy(g_profile->boardModelEEPROM,modelOrig);
		if (eth_mac)
			strcpy(g_profile->boardETH_MAC,eth_mac);
		return;
	}

	if (strcmp(model,"stem_125-14_z7020_4in_v1.3") == 0){
		g_profile = getProfile_STEM_125_14_Z7020_4IN_v1_3();
		strcpy(g_profile->boardModelEEPROM,modelOrig);
		if (eth_mac)
			strcpy(g_profile->boardETH_MAC,eth_mac);
		return;
	}

	if (strcmp(model,"stem_250-12_v1.0") == 0){
		g_profile = getProfile_STEM_250_12_v1_0();
		strcpy(g_profile->boardModelEEPROM,modelOrig);
		if (eth_mac)
			strcpy(g_profile->boardETH_MAC,eth_mac);
		return;
	}

	if (strcmp(model,"stem_250-12_v1.1") == 0){
		g_profile = getProfile_STEM_250_12_v1_1();
		strcpy(g_profile->boardModelEEPROM,modelOrig);
		if (eth_mac)
			strcpy(g_profile->boardETH_MAC,eth_mac);
		return;
	}

	if (strcmp(model,"stem_250-12_v1.2") == 0){
		g_profile = getProfile_STEM_250_12_v1_2();
		strcpy(g_profile->boardModelEEPROM,modelOrig);
		if (eth_mac)
			strcpy(g_profile->boardETH_MAC,eth_mac);
		return;
	}

	if (strcmp(model,"stem_250-12_v1.2a") == 0){
		g_profile = getProfile_STEM_250_12_v1_2a();
		strcpy(g_profile->boardModelEEPROM,modelOrig);
		if (eth_mac)
			strcpy(g_profile->boardETH_MAC,eth_mac);
		return;
	}

    if (strcmp(model,"stem_250-12_v1.2b") == 0){
		g_profile = getProfile_STEM_250_12_v1_2b();
		strcpy(g_profile->boardModelEEPROM,modelOrig);
		if (eth_mac)
			strcpy(g_profile->boardETH_MAC,eth_mac);
		return;
	}

	if (strcmp(model,"stem_250-12_120") == 0){
		g_profile = getProfile_STEM_250_12_120();
		strcpy(g_profile->boardModelEEPROM,modelOrig);
		if (eth_mac)
			strcpy(g_profile->boardETH_MAC,eth_mac);
		return;
	}

	if (strcmp(model,"stem_125-14_ln_bo_v1.1") == 0){
		g_profile = getProfile_STEM_125_14_LN_BO_v1_1();
		strcpy(g_profile->boardModelEEPROM,modelOrig);
		if (eth_mac)
			strcpy(g_profile->boardETH_MAC,eth_mac);
		return;
	}

	if (strcmp(model,"stem_125-14_ln_ce1_v1.1") == 0){
		g_profile = getProfile_STEM_125_14_LN_CE1_v1_1();
		strcpy(g_profile->boardModelEEPROM,modelOrig);
		if (eth_mac)
			strcpy(g_profile->boardETH_MAC,eth_mac);
		return;
	}

	if (strcmp(model,"stem_125-14_ln_ce2_v1.1") == 0){
		g_profile = getProfile_STEM_125_14_LN_CE2_v1_1();
		strcpy(g_profile->boardModelEEPROM,modelOrig);
		if (eth_mac)
			strcpy(g_profile->boardETH_MAC,eth_mac);
		return;
	}

	if (strcmp(model,"stem_125-14_v2.0") == 0){ // STEM_125-14_v2.0
		g_profile = getProfile_STEM_125_14_v2_0();
		strcpy(g_profile->boardModelEEPROM,modelOrig);
		if (eth_mac)
			strcpy(g_profile->boardETH_MAC,eth_mac);
		return;
	}

	if (strcmp(model,"stem_125-14_pro_v2.0") == 0){ // STEM_125-14_Pro_v2.0
		g_profile = getProfile_STEM_125_14_Pro_v2_0();
		strcpy(g_profile->boardModelEEPROM,modelOrig);
		if (eth_mac)
			strcpy(g_profile->boardETH_MAC,eth_mac);
		return;
	}

	if (strcmp(model,"stem_125-14_z7020_pro_v2.0") == 0){ // STEM_125-14_Z7020_Pro_v2.0
		g_profile = getProfile_STEM_125_14_Z7020_Pro_v2_0();
		strcpy(g_profile->boardModelEEPROM,modelOrig);
		if (eth_mac)
			strcpy(g_profile->boardETH_MAC,eth_mac);
		return;
	}

	if (strcmp(model,"stem_125-14_ind_v2.0") == 0){ // STEM_125-14_Ind_v2.0
		g_profile = getProfile_STEM_125_14_Z7020_Ind_v2_0();
		strcpy(g_profile->boardModelEEPROM,modelOrig);
		if (eth_mac)
			strcpy(g_profile->boardETH_MAC,eth_mac);
		return;
	}


	fprintf(stderr,"[Fatal error] Unknown model \"%s\"",model);
}

int hp_cmn_Init(){
	char *buf;
	char *name, *value;
    char *model = NULL;
    char *eth_mac = NULL;

	FILE *fp = fopen("/sys/bus/i2c/devices/0-0050/eeprom", "r");
	if (!fp){
		fprintf(stderr,"[hp_cmn_Init] Error open eeprom: %d\n",errno);
		return RP_HP_ERE;
	}

	if(fseek(fp, 0x1804	, SEEK_SET) < 0) {
        fclose(fp);
		fprintf(stderr,"[hp_cmn_Init] Error open eeprom\n");
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
	 	if (!value){
			position += slen + 1;
	 		continue;
		}
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

const char* getGainName(rp_HPADCGainMode_t mode){
	switch (mode)
	{
	case RP_HP_ADC_GAIN_NORMAL:
		return "NORMAL (LV)";
	case RP_HP_ADC_GAIN_HIGH:
		return "HIGH";
	default:
		return "ERROR GAIN MODE";
		break;
	}
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
	fprintf(stdout,"\t* RAM size: %d MB\n",p->ramMB);
	fprintf(stdout,"\t* Oscillator Rate: %u\n",p->oscillator_rate);
	fprintf(stdout,"\t* ADC chip Full Scale: %f\n",p->fast_adc_full_scale);
	fprintf(stdout,"\t* DAC chip Full Scale: %f\n",p->fast_dac_full_scale);

	fprintf(stdout,"FAST ADC\n");
	fprintf(stdout,"\t* Rate: %u\n",p->fast_adc_rate);
	fprintf(stdout,"\t* Filter present: %u\n",p->is_fast_adc_filter_present);
	fprintf(stdout,"\t* Spectrum resolution: %u\n",p->fast_adc_spectrum_resolution);
	fprintf(stdout,"\t* Count: %u\n",p->fast_adc_count_channels);
	fprintf(stdout,"\t* Is signed: %u\n",p->fast_adc_is_sign);
	fprintf(stdout,"\t* Bits: %u\n",p->fast_adc_bits);
	fprintf(stdout,"\t* HV mode (1:20): %d\n",p->is_LV_HV_mode);
	fprintf(stdout,"\t* AD/DC mode: %d\n",p->is_AC_DC_mode);

	for(int i = 0 ; i < p->fast_adc_count_channels; i++){
		for(int g = 0 ; g <= RP_HP_ADC_GAIN_HIGH; g++){
			fprintf(stdout,"\t\t- Channel: %d Gain %s = %f\n",i + 1, getGainName(g),p->fast_adc_gain[g][i]);
		}
		fprintf(stdout,"\n");
	}

	fprintf(stdout,"FAST DAC\n");
	fprintf(stdout,"\t* Is present: %u\n",p->is_dac_present);
	fprintf(stdout,"\t* Rate: %u\n",p->fast_dac_rate);
	fprintf(stdout,"\t* Count: %u\n",p->fast_dac_count_channels);
	fprintf(stdout,"\t* Is signed: %u\n",p->fast_dac_is_sign);
	fprintf(stdout,"\t* Bits: %u\n",p->fast_dac_bits);

	for(int i = 0 ; i < p->fast_dac_count_channels; i++){
		fprintf(stdout,"\t\t- Channel: %d Gain %f\n",i + 1, p->fast_dac_gain[i]);
	}

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
	fprintf(stdout,"FAST DAC 50 Ohm mode: %d\n",p->is_DAC_50_Ohm_mode);
	fprintf(stdout,"FAST DAC overheating protection: %d\n",p->is_fast_dac_temp_protection);
	fprintf(stdout,"FAST ADC/DAC calibration: %d\n",p->is_fast_calibration);
	fprintf(stdout,"FAST ADC attenuator controller: %d\n",p->is_attenuator_controller_present);
	fprintf(stdout,"FAST ADC External trigger level available: %d\n",p->is_ext_trigger_level_available);
	fprintf(stdout,"FAST ADC External trigger level full scale: %d\n",p->external_trigger_full_scale);
	fprintf(stdout,"FAST ADC External trigger level is signed: %d\n",p->is_ext_trigger_signed);
	fprintf(stdout,"FAST ADC DMA mode support (v0.94): %d\n",p->is_dma_mode_v0_94);

	fprintf(stdout,"FAST ADC Split trigger mode (v0.94): %d\n",p->is_split_osc_triggers);

	fprintf(stdout,"\nDaisy chain clock sync support: %u\n", p->is_daisy_chain_clock_sync);

	fprintf(stdout,"GPIO DIO_N count: %u\n", p->gpio_N_count);
	fprintf(stdout,"GPIO DIO_P count: %u\n", p->gpio_P_count);

	fprintf(stdout,"\nE3 Is present: %u\n", p->is_E3_present);
	fprintf(stdout,"E3 High speed GPIO support: %d\n", p->is_E3_high_speed_gpio);
	fprintf(stdout,"E3 High speed GPIO rate: %u\n", p->E3_high_speed_gpio_rate);
	fprintf(stdout,"E3 QSPI for eMMC support: %d\n", p->is_E3_high_speed_gpio);

	fprintf(stdout,"***********************************************************************\n");
	return RP_HP_OK;
}