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



const char * c_wpCalParDesc_v1[eCalParEnd_v1][20]={
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

const char * c_wpCalParDesc_v2[eCalParEnd_v2][20]={
    {"OSC_CH1_HIGH"},
    {"OSC_CH2_HIGH"},
    {"OSC_CH3_HIGH"},
    {"OSC_CH4_HIGH"},
    {"OSC_CH1_LOW"},
    {"OSC_CH2_LOW"},
    {"OSC_CH3_LOW"},
    {"OSC_CH4_LOW"},
    {"OSC_CH1_HIGH_OFFSET"},
    {"OSC_CH1_HIGH_OFFSET"},
    {"OSC_CH1_HIGH_OFFSET"},
    {"OSC_CH1_HIGH_OFFSET"},
    {"OSC_CH1_LOW_OFFSET"},
    {"OSC_CH1_LOW_OFFSET"},
    {"OSC_CH1_LOW_OFFSET"},
    {"OSC_CH1_LOW_OFFSET"},

    {"OSC_CH1_HIGH_AA"},
    {"OSC_CH1_HIGH_BB"},
    {"OSC_CH1_HIGH_PP"},
    {"OSC_CH1_HIGH_KK"},
    {"OSC_CH1_LOW_AA"},
    {"OSC_CH1_LOW_BB"},
    {"OSC_CH1_LOW_PP"},
    {"OSC_CH1_LOW_KK"},

    {"OSC_CH2_HIGH_AA"},
    {"OSC_CH2_HIGH_BB"},
    {"OSC_CH2_HIGH_PP"},
    {"OSC_CH2_HIGH_KK"},
    {"OSC_CH2_LOW_AA"},
    {"OSC_CH2_LOW_BB"},
    {"OSC_CH2_LOW_PP"},
    {"OSC_CH2_LOW_KK"},

    {"OSC_CH3_HIGH_AA"},
    {"OSC_CH3_HIGH_BB"},
    {"OSC_CH3_HIGH_PP"},
    {"OSC_CH3_HIGH_KK"},
    {"OSC_CH3_LOW_AA"},
    {"OSC_CH3_LOW_BB"},
    {"OSC_CH3_LOW_PP"},
    {"OSC_CH3_LOW_KK"},

    {"OSC_CH4_HIGH_AA"},
    {"OSC_CH4_HIGH_BB"},
    {"OSC_CH4_HIGH_PP"},
    {"OSC_CH4_HIGH_KK"},
    {"OSC_CH4_LOW_AA"},
    {"OSC_CH4_LOW_BB"},
    {"OSC_CH4_LOW_PP"},
    {"OSC_CH4_LOW_KK"}

};

const char * c_wpCalParDesc_v3[eCalParEnd_v3][20]={
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

int getCalibSize(rp_HPeModels_t model){
    switch (model)
    {
        case STEM_125_10_v1_0:
        case STEM_125_14_v1_0:
        case STEM_125_14_v1_1:
        case STEM_125_14_LN_v1_1:
        case STEM_125_14_Z7020_v1_0:
        case STEM_125_14_Z7020_LN_v1_1:
            return eCalParEnd_v1;
        case STEM_122_16SDR_v1_0:
        case STEM_122_16SDR_v1_1:
            return eCalPar_F_LOW_AA_CH1;
        case STEM_125_14_Z7020_4IN_v1_0:
        case STEM_125_14_Z7020_4IN_v1_2:
        case STEM_125_14_Z7020_4IN_v1_3:
            return eCalParEnd_v2;

        case STEM_250_12_v1_0:
        case STEM_250_12_v1_1:
        case STEM_250_12_v1_2:
        case STEM_250_12_120:
            return eCalParEnd_v3;
        default:{
            fprintf(stderr,"[Error:getCalibSize] Unknown model: %d.\n",model);
            return -1;
        }
    }
}

void RpPrintEepromCalData(rp_HPeModels_t model,rp_eepromWpData_t *_eepromData,bool verb,bool hex)
{
    int size = 0;

    switch (model)
    {
        case STEM_125_10_v1_0:
        case STEM_125_14_v1_0:
        case STEM_125_14_v1_1:
        case STEM_125_14_LN_v1_1:
        case STEM_125_14_Z7020_v1_0:
        case STEM_125_14_Z7020_LN_v1_1:
        case STEM_122_16SDR_v1_0:
        case STEM_122_16SDR_v1_1:{
            size = eCalParEnd_v1;
            if (_eepromData->feCalPar[eCalParMagic] == (int32_t)CALIB_MAGIC){
                size = eCalPar_F_LOW_AA_CH1;
            }
            if (verb){
               printf(hex ? "dataStructureId = 0x%X\n" : "dataStructureId = %d\n",_eepromData->dataStructureId);
               printf(hex ? "wpCheck = 0x%X\n" : "wpCheck = %d\n",_eepromData->wpCheck);
               for(int i = 0; i < size; ++i) {
                    printf( hex ? "%s = 0x%X\n" : "%s = %d\n", c_wpCalParDesc_v1[i][0], _eepromData->feCalPar[i]);
                }
                return;
            }
            break;
        }

        case STEM_125_14_Z7020_4IN_v1_0:
        case STEM_125_14_Z7020_4IN_v1_2:
        case STEM_125_14_Z7020_4IN_v1_3:{
            size = eCalParEnd_v2;
            if (verb){
               printf(hex ? "dataStructureId = 0x%X\n" : "dataStructureId = %d\n",_eepromData->dataStructureId);
               printf(hex ? "wpCheck = 0x%X\n" : "wpCheck = %d\n",_eepromData->wpCheck);
               for(int i = 0; i < size; ++i) {
                    printf( hex ? "%s = 0x%X\n" : "%s = %d\n", c_wpCalParDesc_v2[i][0], _eepromData->feCalPar[i]);
                }
                return;
            }
            break;
        }
        case STEM_250_12_v1_0:
        case STEM_250_12_v1_1:
        case STEM_250_12_v1_2:
        case STEM_250_12_120:{
            size = eCalParEnd_v3;
            if (verb){
                printf(hex ? "dataStructureId = 0x%X\n" : "dataStructureId = %d\n",_eepromData->dataStructureId);
                printf(hex ? "wpCheck = 0x%X\n" : "wpCheck = %d\n",_eepromData->wpCheck);
                for(int i = 0; i < size; ++i) {
                    printf( hex ? "%s = 0x%X\n" : "%s = %d\n", c_wpCalParDesc_v3[i][0], _eepromData->feCalPar[i]);
                }
                return;
            }
            break;
        }

        default:{
            fprintf(stderr,"[Error:calib_LoadFromFactoryZone] Unknown model: %d.\n",model);
            return;
        }
    }

    for(int i = 0; i < size; ++i) {
        fprintf(stdout, hex ? "0x%X\t" : "%20d", _eepromData->feCalPar[i]);
    }
    fprintf(stdout, "\n");
}

void print_eeprom(rp_HPeModels_t model,rp_eepromWpData_t *data,int mode){
    /* Print */
    if (mode & WANT_VERBOSE ) {
        RpPrintEepromCalData(model, data,true,mode & WANT_HEX);
    } else {
        if (!(mode & WANT_Z_MODE)) {
            RpPrintEepromCalData(model, data,false,mode & WANT_HEX);
        }else{
            switch (model)
            {
                case STEM_125_10_v1_0:
                case STEM_125_14_v1_0:
                case STEM_125_14_v1_1:
                case STEM_125_14_LN_v1_1:
                case STEM_125_14_Z7020_v1_0:
                case STEM_125_14_Z7020_LN_v1_1:
                case STEM_122_16SDR_v1_0:
                case STEM_122_16SDR_v1_1:{
                   fprintf(stdout, "%20d %20d %20d %20d %20d %20d %20d %20d %20d %20d %20d %20d\n",
                            data->feCalPar[eCalPar_FE_CH1_DC_offs],
                            data->feCalPar[eCalPar_FE_CH2_DC_offs],
                            data->feCalPar[eCalPar_FE_CH1_FS_G_LO],
                            data->feCalPar[eCalPar_FE_CH2_FS_G_LO],
                            data->feCalPar[eCalPar_FE_CH1_DC_offs_HI],
                            data->feCalPar[eCalPar_FE_CH2_DC_offs_HI],
                            data->feCalPar[eCalPar_FE_CH1_FS_G_HI],
                            data->feCalPar[eCalPar_FE_CH2_FS_G_HI],
                            data->feCalPar[eCalPar_BE_CH1_DC_offs],
                            data->feCalPar[eCalPar_BE_CH2_DC_offs],
                            data->feCalPar[eCalPar_BE_CH1_FS],
                            data->feCalPar[eCalPar_BE_CH2_FS]);
                    break;
                }

                case STEM_125_14_Z7020_4IN_v1_0:
                case STEM_125_14_Z7020_4IN_v1_2:
                case STEM_125_14_Z7020_4IN_v1_3:
                case STEM_250_12_v1_0:
                case STEM_250_12_v1_1:
                case STEM_250_12_v1_2:
                case STEM_250_12_120:{
                    fprintf(stdout, "Unsupport mode\n");
                    break;
                }

                default:{
                    fprintf(stderr,"[Error:print_eeprom] Unknown model: %d.\n",model);
                    break;
                }
            }
        }
    }
}