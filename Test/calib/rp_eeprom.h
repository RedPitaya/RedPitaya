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
    eCalParEnd
} calPar_t;

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
