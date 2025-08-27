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
#include "rp.h"
#include "rp_hw_calib.h"

#define CALIB_MAGIC 0xAABBCCDD

/** Bit flags to represent options on the command-line. */
typedef enum {
    WANT_READ = 0x01,
    WANT_WRITE = 0x02,
    WANT_DEFAULTS = 0x04,
    WANT_VERBOSE = 0x08,
    WANT_Z_MODE = 0x10,
    WANT_HEX = 0x20,
    WANT_INIT = 0x40,
    WANT_PRINT = 0x80,
    WANT_NEW_FORMAT = 0x100,
    WANT_MODIFY = 0x200,
    WANT_TO_OLD = 0x400,
    WANT_FILTER_ZERO = 0x800

} WANT_FLAGS;

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
    eCalParMagic,  // needed for compatibility with a very old version of calibration parameters
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
    eCalParEnd_v1
} calPar_v1_t;

typedef enum {
    eCalPar_OSC_C1_HI,
    eCalPar_OSC_C2_HI,
    eCalPar_OSC_C3_HI,
    eCalPar_OSC_C4_HI,
    eCalPar_OSC_C1_LO,
    eCalPar_OSC_C2_LO,
    eCalPar_OSC_C3_LO,
    eCalPar_OSC_C4_LO,

    eCalPar_OSC_C1_HI_OFF,
    eCalPar_OSC_C2_HI_OFF,
    eCalPar_OSC_C3_HI_OFF,
    eCalPar_OSC_C4_HI_OFF,
    eCalPar_OSC_C1_LO_OFF,
    eCalPar_OSC_C2_LO_OFF,
    eCalPar_OSC_C3_LO_OFF,
    eCalPar_OSC_C4_LO_OFF,

    eCalPar_OSC_C1_HI_AA,
    eCalPar_OSC_C1_HI_BB,
    eCalPar_OSC_C1_HI_PP,
    eCalPar_OSC_C1_HI_KK,
    eCalPar_OSC_C1_LO_AA,
    eCalPar_OSC_C1_LO_BB,
    eCalPar_OSC_C1_LO_PP,
    eCalPar_OSC_C1_LO_KK,

    eCalPar_OSC_C2_HI_AA,
    eCalPar_OSC_C2_HI_BB,
    eCalPar_OSC_C2_HI_PP,
    eCalPar_OSC_C2_HI_KK,
    eCalPar_OSC_C2_LO_AA,
    eCalPar_OSC_C2_LO_BB,
    eCalPar_OSC_C2_LO_PP,
    eCalPar_OSC_C2_LO_KK,

    eCalPar_OSC_C3_HI_AA,
    eCalPar_OSC_C3_HI_BB,
    eCalPar_OSC_C3_HI_PP,
    eCalPar_OSC_C3_HI_KK,
    eCalPar_OSC_C3_LO_AA,
    eCalPar_OSC_C3_LO_BB,
    eCalPar_OSC_C3_LO_PP,
    eCalPar_OSC_C3_LO_KK,

    eCalPar_OSC_C4_HI_AA,
    eCalPar_OSC_C4_HI_BB,
    eCalPar_OSC_C4_HI_PP,
    eCalPar_OSC_C4_HI_KK,
    eCalPar_OSC_C4_LO_AA,
    eCalPar_OSC_C4_LO_BB,
    eCalPar_OSC_C4_LO_PP,
    eCalPar_OSC_C4_LO_KK,

    eCalParEnd_v2
} calPar_v2_t;

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
    eCalParEnd_v3
} calPar_v3_t;

void print_eeprom(rp_HPeModels_t model, rp_eepromWpData_t* data, int mode);
void print_eepromUni(rp_eepromUniData_t* data, int mode);
int getCalibSize(rp_HPeModels_t model);

#endif
