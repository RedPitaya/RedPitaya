/**
 * $Id: rp_eeprom.h 996 2014-02-04 09:36:58Z ales.bardorfer $
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

#ifndef __RP_EEPROM_H
#define __RP_EEPROM_H

#include <stdbool.h>

#if defined Z10 || defined Z20_125
#define PACK_ID 1
typedef enum {
    eCalPar_FE_CH1_FS_G_HI,
    eCalPar_FE_CH2_FS_G_HI,
    eCalPar_FE_CH1_FS_G_LO,
    eCalPar_FE_CH2_FS_G_LO,
    eCalPar_FE_CH1_DC_offs,
    eCalPar_FE_CH2_DC_offs,
    eCalPar_BE_CH1_FS,
    eCalPar_BE_CH2_FS,
    eCalPar_BE_CH1_DC_offs,
    eCalPar_BE_CH2_DC_offs,
    eCalParMagic,   // needed for compatibility with a very old version of calibration parameters
    eCalPar_FE_CH1_DC_offs_HI,
    eCalPar_FE_CH2_DC_offs_HI,
    eCalPar_F_LOW_AA_CH1,
    eCalPar_F_LOW_BB_CH1,
    eCalPar_F_LOW_PP_CH1,
    eCalPar_F_LOW_KK_CH1,
    eCalPar_F_LOW_AA_CH2,
    eCalPar_F_LOW_BB_CH2,
    eCalPar_F_LOW_PP_CH2,
    eCalPar_F_LOW_KK_CH2,
    eCalPar_F_HI_AA_CH1,
    eCalPar_F_HI_BB_CH1,
    eCalPar_F_HI_PP_CH1,
    eCalPar_F_HI_KK_CH1,
    eCalPar_F_HI_AA_CH2,
    eCalPar_F_HI_BB_CH2,
    eCalPar_F_HI_PP_CH2,
    eCalPar_F_HI_KK_CH2,
    eCalParEnd
} calPar_t;
#endif

#ifdef Z20
#define PACK_ID 1
typedef enum {
    eCalPar_FE_CH1_FS_G_HI,
    eCalPar_FE_CH2_FS_G_HI,
    eCalPar_FE_CH1_FS_G_LO,
    eCalPar_FE_CH2_FS_G_LO,
    eCalPar_FE_CH1_DC_offs,
    eCalPar_FE_CH2_DC_offs,
    eCalPar_BE_CH1_FS,
    eCalPar_BE_CH2_FS,
    eCalPar_BE_CH1_DC_offs,
    eCalPar_BE_CH2_DC_offs,
    eCalParMagic,   // needed for compatibility with a very old version of calibration parameters
    eCalPar_FE_CH1_DC_offs_HI,
    eCalPar_FE_CH2_DC_offs_HI,
    eCalParEnd
} calPar_t;
#endif

#ifdef Z20_250_12
#define PACK_ID 2
typedef enum {
    eCalPar_GEN_CH1_G_1,
    eCalPar_GEN_CH2_G_1,
    eCalPar_GEN_CH1_OFF_1,
    eCalPar_GEN_CH2_OFF_1,
    eCalPar_GEN_CH1_G_5,
    eCalPar_GEN_CH2_G_5,
    eCalPar_GEN_CH1_OFF_5,
    eCalPar_GEN_CH2_OFF_5,
    eCalPar_OSC_CH1_G_1_AC,
    eCalPar_OSC_CH2_G_1_AC,
    eCalPar_OSC_CH1_OFF_1_AC,
    eCalPar_OSC_CH2_OFF_1_AC,
    eCalPar_OSC_CH1_G_1_DC,
    eCalPar_OSC_CH2_G_1_DC,
    eCalPar_OSC_CH1_OFF_1_DC,
    eCalPar_OSC_CH2_OFF_1_DC,
    eCalPar_OSC_CH1_G_20_AC,
    eCalPar_OSC_CH2_G_20_AC,
    eCalPar_OSC_CH1_OFF_20_AC,
    eCalPar_OSC_CH2_OFF_20_AC,
    eCalPar_OSC_CH1_G_20_DC,
    eCalPar_OSC_CH2_G_20_DC,
    eCalPar_OSC_CH1_OFF_20_DC,
    eCalPar_OSC_CH2_OFF_20_DC,
    eCalParEnd
} calPar_t;
#endif





typedef struct {
    char dataStructureId;
    char wpCheck;
    char reserved[6];
    int  feCalPar[eCalParEnd];
} eepromWpData_t;

int RpEepromCalDataRead(eepromWpData_t * eepromData, bool factory);
int RpEepromCalDataWrite(eepromWpData_t * eepromData, bool factory);
int RpEepromCalDataVerify(eepromWpData_t * a_eepromData, bool factory);
void RpPrintEepromCalData(eepromWpData_t a_eepromData);

#endif /* __RP_EEPROM_H */
