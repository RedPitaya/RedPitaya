/**
 * $Id: rp_eeprom.c 996 2014-02-04 09:36:58Z ales.bardorfer $
 *
 * @brief Red Pitaya calibration EEPROM library routines.
 *
 * @Author Crt Valentincic <crt.valentincic@redpitaya.com>
 *         Ales Bardorfer <ales.bardorfer@redpitaya.com>
 *         
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "rp_eeprom.h"

#define EEPROM_DEVICE "/sys/bus/i2c/devices/0-0050/eeprom"

const int c_wpCalParAddrOffset =  0x0000;
const int c_wpFactoryAddrOffset = 0x1c00;

#ifdef Z20_250_12
const char * c_wpCalParDesc[eCalParEnd][20]={
    {"GEN_CH1_G_1"},
    {"GEN_CH2_G_1"},
    {"GEN_CH1_OFF_1"},
    {"GEN_CH2_OFF_1"},
    {"GEN_CH1_G_5"},
    {"GEN_CH2_G_5"},
    {"GEN_CH1_OFF_5"},
    {"GEN_CH2_OFF_5"},
    {"OSC_CH1_G_1_AC"},
    {"OSC_CH2_G_1_AC"},
    {"OSC_CH1_OFF_1_AC"},
    {"OSC_CH2_OFF_1_AC"},
    {"OSC_CH1_G_1_DC"},
    {"OSC_CH2_G_1_DC"},
    {"OSC_CH1_OFF_1_DC"},
    {"OSC_CH2_OFF_1_DC"},
    {"OSC_CH1_G_20_AC"},
    {"OSC_CH2_G_20_AC"},
    {"OSC_CH1_OFF_20_AC"},
    {"OSC_CH2_OFF_20_AC"},
    {"OSC_CH1_G_20_DC"},
    {"OSC_CH2_G_20_DC"},
    {"OSC_CH1_OFF_20_DC"},
    {"OSC_CH2_OFF_20_DC"}
};
#endif

#if defined Z10 || defined Z20_125
#define CALIB_MAGIC 0xAABBCCDD
#define CALIB_MAGIC_FILTER 0xDDCCBBAA
// Default values
#define GAIN_LO_FILT_AA 0x7D93
#define GAIN_LO_FILT_BB 0x437C7
#define GAIN_LO_FILT_PP 0x2666
#define GAIN_LO_FILT_KK 0xd9999a
#define GAIN_HI_FILT_AA 0x4C5F
#define GAIN_HI_FILT_BB 0x2F38B
#define GAIN_HI_FILT_PP 0x2666
#define GAIN_HI_FILT_KK 0xd9999a

const char * c_wpCalParDesc[eCalParEnd][20]={
    {"FE_CH1_FS_G_HI"},
    {"FE_CH2_FS_G_HI"},
    {"FE_CH1_FS_G_LO"},
    {"FE_CH2_FS_G_LO"},
    {"FE_CH1_DC_offs"},
    {"FE_CH2_DC_offs"},
    {"BE_CH1_FS"},
    {"BE_CH2_FS"},
    {"BE_CH1_DC_offs"},
    {"BE_CH2_DC_offs"},
    {"Magic"},
    {"FE_CH1_DC_offs_HI"},
    {"FE_CH2_DC_offs_HI"},
    {"LOW_FILTER_AA_CH1"},
    {"LOW_FILTER_BB_CH1"},
    {"LOW_FILTER_PP_CH1"},
    {"LOW_FILTER_KK_CH1"},
    {"LOW_FILTER_AA_CH2"},
    {"LOW_FILTER_BB_CH2"},
    {"LOW_FILTER_PP_CH2"},
    {"LOW_FILTER_KK_CH2"},
    {"HI_FILTER_AA_CH1"},
    {"HI_FILTER_BB_CH1"},
    {"HI_FILTER_PP_CH1"},
    {"HI_FILTER_KK_CH1"},
    {"HI_FILTER_AA_CH2"},
    {"HI_FILTER_BB_CH2"},
    {"HI_FILTER_PP_CH2"},
    {"HI_FILTER_KK_CH2"}
};
#endif

#ifdef Z20
#define CALIB_MAGIC 0xAABBCCDD
const char * c_wpCalParDesc[eCalParEnd][20]={
    {"FE_CH1_FS_G_HI"},
    {"FE_CH2_FS_G_HI"},
    {"FE_CH1_FS_G_LO"},
    {"FE_CH2_FS_G_LO"},
    {"FE_CH1_DC_offs"},
    {"FE_CH2_DC_offs"},
    {"BE_CH1_FS"},
    {"BE_CH2_FS"},
    {"BE_CH1_DC_offs"},
    {"BE_CH2_DC_offs"},
    {"Magic"},
    {"FE_CH1_DC_offs_HI"},
    {"FE_CH2_DC_offs_HI"},
};
#endif


int RpEepromCalDataRead(eepromWpData_t * eepromData, bool factory)
{

    FILE *fp;
    size_t size;

    /* Open device */
    fp = fopen(EEPROM_DEVICE, "rw+");
    if(fp == NULL) {
        fprintf(stderr, "Cannot open eeprom device!\n");
        return -1;
    }

    /* Read eeprom content */
    int offset = factory ? c_wpFactoryAddrOffset : c_wpCalParAddrOffset;
    fseek(fp, offset, SEEK_SET);

    size = fread(eepromData, sizeof(char), sizeof(eepromWpData_t), fp);
    if(size != sizeof(eepromWpData_t)) {
        fprintf(stderr, "Eeprom read failed\n");
        fclose(fp);
        return -1;
    }
#if defined Z10 || defined Z20_125
if (eepromData->feCalPar[eCalParMagic] != CALIB_MAGIC_FILTER){
    eepromData->feCalPar[eCalPar_F_LOW_AA_CH1] = GAIN_LO_FILT_AA;
    eepromData->feCalPar[eCalPar_F_LOW_BB_CH1] = GAIN_LO_FILT_BB;
    eepromData->feCalPar[eCalPar_F_LOW_PP_CH1] = GAIN_LO_FILT_PP;
    eepromData->feCalPar[eCalPar_F_LOW_KK_CH1] = GAIN_LO_FILT_KK;
    eepromData->feCalPar[eCalPar_F_LOW_AA_CH2] = GAIN_LO_FILT_AA;
    eepromData->feCalPar[eCalPar_F_LOW_BB_CH2] = GAIN_LO_FILT_BB;
    eepromData->feCalPar[eCalPar_F_LOW_PP_CH2] = GAIN_LO_FILT_PP;
    eepromData->feCalPar[eCalPar_F_LOW_KK_CH2] = GAIN_LO_FILT_KK;

    eepromData->feCalPar[eCalPar_F_HI_AA_CH1] = GAIN_HI_FILT_AA;
    eepromData->feCalPar[eCalPar_F_HI_BB_CH1] = GAIN_HI_FILT_BB;
    eepromData->feCalPar[eCalPar_F_HI_PP_CH1] = GAIN_HI_FILT_PP;
    eepromData->feCalPar[eCalPar_F_HI_KK_CH1] = GAIN_HI_FILT_KK;
    eepromData->feCalPar[eCalPar_F_HI_AA_CH2] = GAIN_HI_FILT_AA;
    eepromData->feCalPar[eCalPar_F_HI_BB_CH2] = GAIN_HI_FILT_BB;
    eepromData->feCalPar[eCalPar_F_HI_PP_CH2] = GAIN_HI_FILT_PP;
    eepromData->feCalPar[eCalPar_F_HI_KK_CH2] = GAIN_HI_FILT_KK;
}
#endif
    fclose(fp);
    return 0;
}

int RpEepromCalDataWrite(eepromWpData_t * eepromData, bool factory)
{
    FILE *fp;
    size_t size;

    /* Fix ID and set reserved data */
    eepromData->dataStructureId = PACK_ID;
    eepromData->wpCheck += 1;
#ifdef Z20
    eepromData->feCalPar[eCalParMagic]=CALIB_MAGIC;
#endif
    memset((char*)&eepromData->reserved[0], 0, 6);

    /* Open device */
    fp = fopen(EEPROM_DEVICE, "rw+");
    if(fp == NULL){
    	fprintf(stderr, "Cannot open eeprom device!\n");
        fclose(fp);
        return -1;
    }

    /* Write to eeprom */
    int offset = factory ? c_wpFactoryAddrOffset : c_wpCalParAddrOffset;
    fseek(fp, offset, SEEK_SET);

    size = fwrite(eepromData, sizeof(char), sizeof(eepromWpData_t), fp);
    if(size != sizeof(eepromWpData_t)) {
        fprintf(stderr, "Eeprom write failed\n");
        fclose(fp);
        return -1;
    }

    fclose(fp);

    if(RpEepromCalDataVerify(eepromData, factory)) {
        fprintf(stderr, "Eeprom verify failed\n");
        return -1;
    }

    return 0;
}

int RpEepromCalDataVerify(eepromWpData_t * a_eepromData, bool factory)
{
    eepromWpData_t data;
    if(RpEepromCalDataRead(&data, factory)) {
        return -1;
    }

    if(memcmp((char*)&data, a_eepromData, sizeof(eepromWpData_t))) {
        return -1;
    }
    return 0;
}

void RpPrintEepromCalData(eepromWpData_t a_eepromData)
{
    int i;
    int size = eCalParEnd;
#if defined Z10 || defined Z20_125
    if (a_eepromData.feCalPar[eCalParMagic] == CALIB_MAGIC){
        size = eCalPar_F_LOW_AA_CH1;
    }
#endif
    for(i=0; i < size; i++) {
        printf("%s = %d\n", c_wpCalParDesc[i][0], a_eepromData.feCalPar[i]);
    }
}


