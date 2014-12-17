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
    {"BE_CH2_DC_offs"}
};


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

    fclose(fp);
    return 0;
}

int RpEepromCalDataWrite(eepromWpData_t * eepromData, bool factory)
{
    FILE *fp;
    size_t size;

    /* Fix ID and set reserved data */
    eepromData->dataStructureId = 1;
    eepromData->wpCheck += 1;
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

    for(i=0; i < eCalParEnd; i++) {
        printf("%s = %d\n", c_wpCalParDesc[i][0], a_eepromData.feCalPar[i]);
    }
}


