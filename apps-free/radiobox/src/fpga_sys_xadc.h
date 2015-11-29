/**
 * @brief Red Pitaya FPGA Interface for the system XADC module.
 *
 * @author Ulrich Habel (DF4IAH) <espero7757@gmx.net>
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef __FPGA_SYS_XADC_H
#define __FPGA_SYS_XADC_H

#include <stdint.h>

#include "main.h"


/** @defgroup fpga_sys_xadc_h FPGA system access
 * @{
 */

/** @brief system XADC starting address of FPGA registers. */
#define FPGA_SYS_XADC_BASE_ADDR       0x83C00000
#define FPGA_SYS_XADC_BASE_SIZE       0x10000

#if 0
/** @brief system GPIO XADC memory map size of FPGA registers. */
#define FPGA_SYS_GPIO_XADC_BASE_ADDR  0x81200000
#define FPGA_SYS_GPIO_XADC_BASE_SIZE  0x10000

/** @brief system GPIO LEDa memory map size of FPGA registers. */
#define FPGA_SYS_GPIO_LEDS_BASE_SIZE  0x10000
#define FPGA_SYS_GPIO_LEDS_BASE_ADDR  0x81210000
#endif


/** @brief FPGA register offset addresses of the system XADC module base address.
 *
 * All registers are 16 bit wide thus the address has to be shifted left by one position
 * to get the access register for the PS.
 */
#if 0
enum {
    FPGA_SYS_XADC_STAT_TEMP        = 0x00000,
    FPGA_SYS_XADC_STAT_VCCINT      = 0x00001,
    FPGA_SYS_XADC_STAT_VCCAUX      = 0x00002,
    FPGA_SYS_XADC_STAT_VPVN        = 0x00003,
    FPGA_SYS_XADC_STAT_VREFP       = 0x00004,
    FPGA_SYS_XADC_STAT_VREFN       = 0x00005,
    FPGA_SYS_XADC_STAT_VCCBRAM     = 0x00006,

    FPGA_SYS_XADC_NA_X07           = 0x00007,

    FPGA_SYS_XADC_STAT_SUPPLYAOFS  = 0x00008,
    FPGA_SYS_XADC_STAT_ADCAOFS     = 0x00009,
    FPGA_SYS_XADC_STAT_ADCAGAIN    = 0x0000A,

    FPGA_SYS_XADC_NA_X0B           = 0x0000B,
    FPGA_SYS_XADC_NA_X0C           = 0x0000C,

    FPGA_SYS_XADC_STAT_VCCPINT     = 0x0000D,
    FPGA_SYS_XADC_STAT_VCCPAUX     = 0x0000E,
    FPGA_SYS_XADC_STAT_VCCODDR     = 0x0000F,

    FPGA_SYS_XADC_STAT_VAUX_00     = 0x00010,
    FPGA_SYS_XADC_STAT_VAUX_01     = 0x00011,
    FPGA_SYS_XADC_STAT_VAUX_02     = 0x00012,
    FPGA_SYS_XADC_STAT_VAUX_03     = 0x00013,
    FPGA_SYS_XADC_STAT_VAUX_04     = 0x00014,
    FPGA_SYS_XADC_STAT_VAUX_05     = 0x00015,
    FPGA_SYS_XADC_STAT_VAUX_06     = 0x00016,
    FPGA_SYS_XADC_STAT_VAUX_07     = 0x00017,
    FPGA_SYS_XADC_STAT_VAUX_08     = 0x00018,
    FPGA_SYS_XADC_STAT_VAUX_09     = 0x00019,
    FPGA_SYS_XADC_STAT_VAUX_10     = 0x0001A,
    FPGA_SYS_XADC_STAT_VAUX_11     = 0x0001B,
    FPGA_SYS_XADC_STAT_VAUX_12     = 0x0001C,
    FPGA_SYS_XADC_STAT_VAUX_13     = 0x0001D,
    FPGA_SYS_XADC_STAT_VAUX_14     = 0x0001E,
    FPGA_SYS_XADC_STAT_VAUX_15     = 0x0001F,

    FPGA_SYS_XADC_STAT_MAXTEMP     = 0x00020,
    FPGA_SYS_XADC_STAT_MAXVCCINT   = 0x00021,
    FPGA_SYS_XADC_STAT_MAXVCCAUX   = 0x00022,
    FPGA_SYS_XADC_STAT_MAXVCCBRAM  = 0x00023,
    FPGA_SYS_XADC_STAT_MINTEMP     = 0x00024,
    FPGA_SYS_XADC_STAT_MINVCCINT   = 0x00025,
    FPGA_SYS_XADC_STAT_MINVCCAUX   = 0x00026,
    FPGA_SYS_XADC_STAT_MINVCCBRAM  = 0x00027,

    FPGA_SYS_XADC_STAT_VCCPINTMAX  = 0x00028,
    FPGA_SYS_XADC_STAT_VCCPAUXMAX  = 0x00029,
    FPGA_SYS_XADC_STAT_VCCODDRMAX  = 0x0002A,

    FPGA_SYS_XADC_NA_X2B           = 0x0002B,

    FPGA_SYS_XADC_STAT_VCCPINTMIN  = 0x0002C,
    FPGA_SYS_XADC_STAT_VCCPAUXMIN  = 0x0002D,
    FPGA_SYS_XADC_STAT_VCCODDRMIN  = 0x0002E,

    FPGA_SYS_XADC_NA_X2F           = 0x0002F,

    FPGA_SYS_XADC_STAT_SUPPLYBOFS  = 0x00030,
    FPGA_SYS_XADC_STAT_ADCBOFS     = 0x00031,
    FPGA_SYS_XADC_STAT_ADCBGAIN    = 0x00032,

    FPGA_SYS_XADC_NA_X33           = 0x00033,
    FPGA_SYS_XADC_NA_X34           = 0x00034,
    FPGA_SYS_XADC_NA_X35           = 0x00035,
    FPGA_SYS_XADC_NA_X36           = 0x00036,
    FPGA_SYS_XADC_NA_X37           = 0x00037,
    FPGA_SYS_XADC_NA_X38           = 0x00038,
    FPGA_SYS_XADC_NA_X39           = 0x00039,
    FPGA_SYS_XADC_NA_X3A           = 0x0003A,
    FPGA_SYS_XADC_NA_X3B           = 0x0003B,
    FPGA_SYS_XADC_NA_X3C           = 0x0003C,
    FPGA_SYS_XADC_NA_X3D           = 0x0003D,
    FPGA_SYS_XADC_NA_X3E           = 0x0003E,

    FPGA_SYS_XADC_STAT_FLAG        = 0x0003F,

    FPGA_SYS_XADC_CONF_1           = 0x00040,
    FPGA_SYS_XADC_CONF_2           = 0x00041,
    FPGA_SYS_XADC_CONF_3           = 0x00042,

    FPGA_SYS_XADC_NA_X43           = 0x00043,
    FPGA_SYS_XADC_NA_X44           = 0x00044,
    FPGA_SYS_XADC_NA_X45           = 0x00045,
    FPGA_SYS_XADC_NA_X46           = 0x00046,
    FPGA_SYS_XADC_NA_X47           = 0x00047,

    FPGA_SYS_XADC_SEQ0_CH_SEL      = 0x00048,
    FPGA_SYS_XADC_SEQ1_CH_SEL      = 0x00049,
    FPGA_SYS_XADC_SEQ2_CH_AVG      = 0x0004A,
    FPGA_SYS_XADC_SEQ3_CH_AVG      = 0x0004B,
    FPGA_SYS_XADC_SEQ4_CH_BU       = 0x0004C,
    FPGA_SYS_XADC_SEQ5_CH_BU       = 0x0004D,
    FPGA_SYS_XADC_SEQ6_CH_ST       = 0x0004E,
    FPGA_SYS_XADC_SEQ7_CH_ST       = 0x0004F,

    FPGA_SYS_XADC_ALM00_TMP_HI     = 0x00050,
    FPGA_SYS_XADC_ALM01_VCCINT_HI  = 0x00051,
    FPGA_SYS_XADC_ALM02_VCCAUX_HI  = 0x00052,
    FPGA_SYS_XADC_ALM03_OT_LIM     = 0x00053,
    FPGA_SYS_XADC_ALM04_TMP_LO     = 0x00054,
    FPGA_SYS_XADC_ALM05_VCCINT_LO  = 0x00055,
    FPGA_SYS_XADC_ALM06_VCCAUX_LO  = 0x00056,
    FPGA_SYS_XADC_ALM07_OT_RST     = 0x00057,
    FPGA_SYS_XADC_ALM08_VCCBRAM_HI = 0x00058,
    FPGA_SYS_XADC_ALM09_VCCPINT_HI = 0x00059,
    FPGA_SYS_XADC_ALM10_VCCPAUX_HI = 0x0005A,
    FPGA_SYS_XADC_ALM11_VCCODDR_HI = 0x0005B,
    FPGA_SYS_XADC_ALM12_VCCBRAM_LO = 0x0005C,
    FPGA_SYS_XADC_ALM13_VCCPINT_LO = 0x0005D,
    FPGA_SYS_XADC_ALM14_VCCPAUX_LO = 0x0005E,
    FPGA_SYS_XADC_ALM15_VCCODDR_LO = 0x0005F

} FPGA_SYS_XADC_REG_ENUMS;
#endif


/** @brief FPGA registry structure for the system XADC module.
 *
 * This structure is the direct image of the physical FPGA memory for the system XADC module.
 * It assures direct read / write FPGA access when it is mapped to the appropriate memory address
 * through the /dev/mem device.
 */
typedef struct fpga_sys_xadc_reg_mem_s {

    /** @brief  -/W SRR  (addr: 0x83C00000)
     *
     * Software reset register.
     * To reset the AXI4 XADC core write 0x0000000A to this register
     *
     */
    uint32_t srr;

    /** @brief  R/- SR  (addr: 0x83C00004)
     *
     * Status register.
     *
     */
    uint32_t sr;

    /** @brief  R/- AOSR  (addr: 0x83C00008)
     *
     * Alarm output status register.
     *
     */
    uint32_t aosr;

    /** @brief  -/W CONVSTR  (addr: 0x83C0000C)
     *
     * CONVST register.
     *
     */
    uint32_t convstr;

    /** @brief  -/W SYSMONRR  (addr: 0x83C00010)
     *
     * XADC reset register.
     *
     */
    uint32_t sysmonrr;


    /** @brief ignore */
    uint32_t _na_014[0x48 >> 2];


    /** @brief  R/W GIER  (addr: 0x83C0005C)
     *
     * Global interrupt enable register.
     *
     */
    uint32_t gier;

    /** @brief  R/TOW IPISR  (addr: 0x83C00060)
     *
     * IP interrupt status reset register.
     *
     */
    uint32_t ipisr;

    /** @brief ignore */
    uint32_t _na_064;

    /** @brief  R/W IPIER  (addr: 0x83C00068)
     *
     * IP interrupt enable reset register.
     *
     */
    uint32_t ipier;


    /** @brief ignore */
    uint32_t _na_06c[0x194 >> 2];


    /** @brief  R/- STAT_TEMP  (addr: 0x83C00200)
     *
     * Current temperature on the die.
     *
     */
    uint32_t stat_temp;

    /** @brief  R/- STAT_VCCINT  (addr: 0x83C00204)
     *
     * Current Vcc_INT voltage.
     *
     */
    uint32_t stat_vccint;

    /** @brief  R/- STAT_VCCAUX  (addr: 0x83C00208)
     *
     * Current Vcc_AUX voltage.
     *
     */
    uint32_t stat_vccaux;


    /** @brief  R/W STAT_VPVN  (addr: 0x83C0020C)
     *
     * Current VpVn voltage.
     * When written, resets the XADC hard macro.
     *
     */
    uint32_t stat_vpvn;

    /** @brief  R/- STAT_VREFP  (addr: 0x83C00210)
     *
     * Current Vref_P voltage.
     *
     */
    uint32_t stat_vrefp;

    /** @brief  R/- STAT_VREFN  (addr: 0x83C00214)
     *
     * Current Vref_N voltage.
     *
     */
    uint32_t stat_vrefn;

    /** @brief  R/- STAT_VCCBRAM  (addr: 0x83C00218)
     *
     * Current Vcc_BRAM voltage.
     *
     */
    uint32_t stat_vccbram;


    /** @brief  N/A - do not use  (addr: 0x83C0021C) */
    uint32_t _na_21c;


    /** @brief  R/- STAT_SUPPLYAOFS  (addr: 0x83C00220)
     *
     * Correction value for supply-A offset.
     * @see ug480_7Series_XADC.pdf, page 38
     *
     */
    uint32_t stat_supplyaofs;

    /** @brief  R/- STAT_ADCAOFS  (addr: 0x83C00224)
     *
     * Correction value for ADC-A offset.
     * @see ug480_7Series_XADC.pdf, page 38
     *
     */
    uint32_t stat_adcaofs;

    /** @brief  R/- STAT_ADCAGAIN  (addr: 0x83C00228)
     *
     * Correction value for ADC-A gain.
     * @see ug480_7Series_XADC.pdf, page 38
     *
     */
    uint32_t stat_adcagain;


    /** @brief  N/A - do not use  (addr: 0x83C0022C) */
    uint32_t _na_22c;

    /** @brief  N/A - do not use  (addr: 0x83C00230) */
    uint32_t _na_230;


    /** @brief  R/- STAT_VCCPINT  (addr: 0x83C00234)
     *
     * Current Vcc_PINT voltage of the PS supply.
     *
     */
    uint32_t stat_vccpint;

    /** @brief  R/- STAT_VCCPAUX  (addr: 0x83C00238)
     *
     * Current Vcc_PAUX voltage of the PS supply.
     *
     */
    uint32_t stat_vccpaux;

    /** @brief  R/- STAT_VCCODDR  (addr: 0x83C0023C)
     *
     * Current Vcco_DDR voltage.
     *
     */
    uint32_t stat_vccoddr;


    /** @brief  R/- STAT_VAUX_00  (addr: 0x83C00240)
     *
     * Current Vaux_P[0]/Vaux_N[0] voltage.
     *
     */
    uint32_t stat_vaux_00;

    /** @brief  R/- STAT_VAUX_01  (addr: 0x83C00244)
     *
     * Current Vaux_P[1]/Vaux_N[1] voltage.
     *
     */
    uint32_t stat_vaux_01;

    /** @brief  R/- STAT_VAUX_02  (addr: 0x83C00248)
     *
     * Current Vaux_P[2]/Vaux_N[2] voltage.
     *
     */
    uint32_t stat_vaux_02;

    /** @brief  R/- STAT_VAUX_03  (addr: 0x83C0024C)
     *
     * Current Vaux_P[3]/Vaux_N[3] voltage.
     *
     */
    uint32_t stat_vaux_03;

    /** @brief  R/- STAT_VAUX_04  (addr: 0x83C00250)
     *
     * Current Vaux_P[4]/Vaux_N[4] voltage.
     *
     */
    uint32_t stat_vaux_04;

    /** @brief  R/- STAT_VAUX_05  (addr: 0x83C00254)
     *
     * Current Vaux_P[5]/Vaux_N[5] voltage.
     *
     */
    uint32_t stat_vaux_05;

    /** @brief  R/- STAT_VAUX_06  (addr: 0x83C00258)
     *
     * Current Vaux_P[6]/Vaux_N[6] voltage.
     *
     */
    uint32_t stat_vaux_06;

    /** @brief  R/- STAT_VAUX_07  (addr: 0x83C0025C)
     *
     * Current Vaux_P[7]/Vaux_N[7] voltage.
     *
     */
    uint32_t stat_vaux_07;

    /** @brief  R/- STAT_VAUX_08  (addr: 0x83C00260)
     *
     * Current Vaux_P[8]/Vaux_N[8] voltage.
     *
     */
    uint32_t stat_vaux_08;

    /** @brief  R/- STAT_VAUX_09  (addr: 0x83C00264)
     *
     * Current Vaux_P[9]/Vaux_N[9] voltage.
     *
     */
    uint32_t stat_vaux_09;

    /** @brief  R/- STAT_VAUX_10  (addr: 0x83C00268)
     *
     * Current Vaux_P[10]/Vaux_N[10] voltage.
     *
     */
    uint32_t stat_vaux_10;

    /** @brief  R/- STAT_VAUX_11  (addr: 0x83C0026C)
     *
     * Current Vaux_P[11]/Vaux_N[11] voltage.
     *
     */
    uint32_t stat_vaux_11;

    /** @brief  R/- STAT_VAUX_12  (addr: 0x83C00270)
     *
     * Current Vaux_P[12]/Vaux_N[12] voltage.
     *
     */
    uint32_t stat_vaux_12;

    /** @brief  R/- STAT_VAUX_13  (addr: 0x83C00274)
     *
     * Current Vaux_P[13]/Vaux_N[13] voltage.
     *
     */
    uint32_t stat_vaux_13;

    /** @brief  R/- STAT_VAUX_14  (addr: 0x83C00278)
     *
     * Current Vaux_P[14]/Vaux_N[14] voltage.
     *
     */
    uint32_t stat_vaux_14;

    /** @brief  R/- STAT_VAUX_15  (addr: 0x83C0027C)
     *
     * Current Vaux_P[15]/Vaux_N[15] voltage.
     *
     */
    uint32_t stat_vaux_15;


    /** @brief  R/- STAT_MAXTEMP  (addr: 0x83C00280)
     *
     * Max temperature on the die.
     *
     */
    uint32_t stat_maxtemp;

    /** @brief  R/- STAT_MAXVCCINT  (addr: 0x83C00284)
     *
     * Max Vcc_INT voltage.
     *
     */
    uint32_t stat_maxvccint;

    /** @brief  R/- STAT_MAXVCCAUX  (addr: 0x83C00288)
     *
     * Max Vcc_AUX voltage.
     *
     */
    uint32_t stat_maxvccaux;

    /** @brief  R/- STAT_MAXVCCBRAM  (addr: 0x83C0028C)
     *
     * Max Vcc_BRAM voltage.
     *
     */
    uint32_t stat_maxvccbram;

    /** @brief  R/- STAT_MINTEMP  (addr: 0x83C00290)
     *
     * Min temperature on the die.
     *
     */
    uint32_t stat_mintemp;

    /** @brief  R/- STAT_MINVCCINT  (addr: 0x83C00294)
     *
     * Min Vcc_INT voltage.
     *
     */
    uint32_t stat_minvccint;

    /** @brief  R/- STAT_MINVCCAUX  (addr: 0x83C00298)
     *
     * Min Vcc_AUX voltage.
     *
     */
    uint32_t stat_minvccaux;

    /** @brief  R/- STAT_MINVCCBRAM  (addr: 0x83C0029C)
     *
     * Min Vcc_BRAM voltage.
     *
     */
    uint32_t stat_minvccbram;


    /** @brief  R/- STAT_MAXVCCPINT  (addr: 0x83C002A0)
     *
     * Max Vcc_PINT voltage of the PS supply.
     *
     */
    uint32_t stat_vccpintmax;

    /** @brief  R/- STAT_MAXVCCPAUX  (addr: 0x83C002A4)
     *
     * Max Vcc_PAUX voltage of the PS supply.
     *
     */
    uint32_t stat_vccpauxmax;

    /** @brief  R/- STAT_VCCODDRMAX  (addr: 0x83C002A8)
     *
     * Max Vcco_DDR voltage.
     *
     */
    uint32_t stat_vccoddrmax;


    /** @brief  N/A - do not use  (addr: 0x83C002AC) */
    uint32_t _na_2ac;


    /** @brief  R/- STAT_MINVCCPINT  (addr: 0x83C002B0)
     *
     * Min Vcc_PINT voltage of the PS supply.
     *
     */
    uint32_t stat_vccpintmin;

    /** @brief  R/- STAT_MINVCCPAUX  (addr: 0x83C002B4)
     *
     * Min Vcc_PAUX voltage of the PS supply.
     *
     */
    uint32_t stat_vccpauxmin;

    /** @brief  R/- STAT_VCCODDRMIN  (addr: 0x83C002B8)
     *
     * Min Vcco_DDR voltage.
     *
     */
    uint32_t stat_vccoddrmin;


    /** @brief  N/A - do not use  (addr: 0x83C002BC) */
    uint32_t _na_2bc;

    /** @brief  N/A - do not use  (addr: 0x83C002C0) */
    uint32_t _na_2c0;

    /** @brief  N/A - do not use  (addr: 0x83C002C4) */
    uint32_t _na_2c4;

    /** @brief  N/A - do not use  (addr: 0x83C002C8) */
    uint32_t _na_2c8;

    /** @brief  N/A - do not use  (addr: 0x83C002CC) */
    uint32_t _na_2cc;

    /** @brief  N/A - do not use  (addr: 0x83C002D0) */
    uint32_t _na_2d0;

    /** @brief  N/A - do not use  (addr: 0x83C002D4) */
    uint32_t _na_2d4;

    /** @brief  N/A - do not use  (addr: 0x83C002D8) */
    uint32_t _na_2d8;

    /** @brief  N/A - do not use  (addr: 0x83C002DC) */
    uint32_t _na_2dc;

    /** @brief  N/A - do not use  (addr: 0x83C002E0) */
    uint32_t _na_2e0;

    /** @brief  N/A - do not use  (addr: 0x83C002E4) */
    uint32_t _na_2e4;

    /** @brief  N/A - do not use  (addr: 0x83C002E8) */
    uint32_t _na_2e8;

    /** @brief  N/A - do not use  (addr: 0x83C002EC) */
    uint32_t _na_2ec;

    /** @brief  N/A - do not use  (addr: 0x83C002F0) */
    uint32_t _na_2f0;

    /** @brief  N/A - do not use  (addr: 0x83C002F4) */
    uint32_t _na_2f4;

    /** @brief  N/A - do not use  (addr: 0x83C002F8) */
    uint32_t _na_2f8;


    /** @brief  R/W STAT_FLAG  (addr: 0x83C002FC)
     *
     * bit h02..h00: ALM[2:0] - Alarm register.
     *
     * bit h03: OT - over temperature flag.
     *
     * bit h07..h04: ALM[6:3] - Alarm register.
     *
     * bit h08: n/a
     *
     * bit h09: REF - internal reference voltage is used.
     *
     * bit h0A: JTGR - JTAG access restricted to read only.
     *
     * bit h0B: JTGD - JTAG access disabled.
     *
     * bit h0C..h0F: n/a
     *
     */
    uint32_t stat_flag;


    /** @brief  R/W CONF_1  (addr: 0x83C00300)
     *
     * bit h04..h00: CH[4:0] - input channel selection for 'single channel mode' or 'external multiplexer mode'.
     *
     * bit h07..h05: '000'.
     *
     * bit h08: ACQ - six ADCCLK cycles to be added for 'continuous sampling mode'.
     *
     * bit h09: E/!C - 1: event driven sampling mode - 0: in contrast to continuous mode.
     *
     * bit h0A: B/!U - 1: bipolar input for single channel mode - 0: unipolar input for single channel mode.
     *
     * bit h0B: MUX - external multiplexer mode.
     *
     * bit h0D..h0C: AVG[1:0] - amount for averaging: 0=no averaging, 1=16 samples, 2=64 samples, 3=256 samples.
     *
     * bit h0E: n/a
     *
     * bit h0F: CAVG - disable averaging for calibration values.
     *
     */
    uint32_t conf_1;

    /** @brief  R/W CONF_2  (addr: 0x83C00304)
     *
     * bit h00: OT - 1: disable over temperature signal.
     *
     * bit h01: ALM[0] - 1: disable alarm for TEMP.
     * bit h02: ALM[1] - 1: disable alarm for Vcc_INT.
     * bit h03: ALM[2] - 1: disable alarm for Vcc_AUX.
     *
     * bit h04: CAL[0] - ADCs offset correction enable.
     * bit h05: CAL[1] - ADCs offset and gain correction enable.
     * bit h06: CAL[2] - Supply sensor offset correction enable.
     * bit h07: CAL[3] - Supply sensor offset and gain correction enable.
     *
     * bit h08: ALM[3] - 1: disable alarm for Vcc_BRAM.
     * bit h09: ALM[4] - 1: disable alarm for Vcc_PINT.
     * bit h0A: ALM[5] - 1: disable alarm for Vcc_PAUX.
     * bit h0B: ALM[6] - 1: disable alarm for Vcco_DDR.
     *
     * bit h0F..h0C: SEQ[3:0] - 0=Default mode, 1=single pass sequence, 2=continuous sequence mode, 3=single channel mode,
     *                          4..7=simultaneous sampling mode, 8-11=independent ADC mode, 12-15=default mode.
     *
     */
    uint32_t conf_2;


    /** @brief  R/W CONF_3  (addr: 0x83C00308)
     *
     * bit h03..h00: '0000'.
     *
     * bit h05..h04: PD[1:0] - Power down modus: 0=Default/all XADC blocks powered up, 2=ADC B powered down, 3=XADC powered down

     * bit h07..h06: '00'.

     * bit h0F..h08: CD[7:0] - Division ratio between DCLK and ADCCLK. Binary value but min saturated at 2.
     *
     */
    uint32_t conf_3;


    /** @brief  N/A - do not use  (addr: 0x83C0030C) */
    uint32_t _na_30c;

    /** @brief  N/A - do not use  (addr: 0x83C00310) */
    uint32_t _na_310;

    /** @brief  N/A - do not use  (addr: 0x83C00314) */
    uint32_t _na_314;

    /** @brief  N/A - do not use  (addr: 0x83C00318) */
    uint32_t _na_318;

    /** @brief  N/A - do not use  (addr: 0x83C0031C) */
    uint32_t _na_31c;


    /** @brief  R/W SEQ0_CH_SEL - ADC Channel Selection Registers (48h and 49h)  (addr: 0x83C00320)
     *
     * Setting a bit enables that channel in the sequence.
     *
     * bit h00: ADC_CH[8]  - XADC calibration.
     *
     * bit h01: ADC_CH[9]  - Invalid channel selection.
     *
     * bit h02: ADC_CH[10] - Invalid channel selection.
     *
     * bit h03: ADC_CH[11] - Invalid channel selection.
     *
     * bit h04: ADC_CH[12] - Invalid channel selection.
     *
     * bit h05: ADC_CH[13] - Vcc_PINT.
     *
     * bit h06: ADC_CH[14] - Vcc_PAUX.
     *
     * bit h07: ADC_CH[15] - Vcco_DDR.
     *
     * bit h08: ADC_CH[0]  - On-chip temperature.
     *
     * bit h09: ADC_CH[1]  - Vcc_INT.
     *
     * bit h0A: ADC_CH[2]  - Vcc_AUX.
     *
     * bit h0B: ADC_CH[3]  - Vp/Vn.
     *
     * bit h0C: ADC_CH[4]  - Vref_P.
     *
     * bit h0D: ADC_CH[5]  - Vref_N.
     *
     * bit h0E: ADC_CH[6]  - Vcc_BRAM.
     *
     * bit h0F: ADC_CH[7]  - Invalid channel selection.
     *
     */
    uint32_t seq0_ch_sel;

    /** @brief  R/W SEQ1_CH_SEL - ADC Channel Selection Registers (48h and 49h)  (addr: 0x83C00324)
     *
     * Setting a bit enables that channel in the sequence.
     *
     * bit h00: ADC_CH[16] - Vaux_P[0]/Vaux_N[0]   - when continuous mode: Vaux_P[0]/Vaux_N[0] and Vaux_P[8]/Vaux_N[8].
     *
     * bit h01: ADC_CH[17] - Vaux_P[1]/Vaux_N[1]   - when continuous mode: Vaux_P[1]/Vaux_N[1] and Vaux_P[9]/Vaux_N[9].
     *
     * bit h02: ADC_CH[18] - Vaux_P[2]/Vaux_N[2]   - when continuous mode: Vaux_P[2]/Vaux_N[2] and Vaux_P[10]/Vaux_N[10].
     *
     * bit h03: ADC_CH[19] - Vaux_P[3]/Vaux_N[3]   - when continuous mode: Vaux_P[3]/Vaux_N[3] and Vaux_P[11]/Vaux_N[11].
     *
     * bit h04: ADC_CH[20] - Vaux_P[4]/Vaux_N[4]   - when continuous mode: Vaux_P[4]/Vaux_N[4] and Vaux_P[12]/Vaux_N[12].
     *
     * bit h05: ADC_CH[21] - Vaux_P[5]/Vaux_N[5]   - when continuous mode: Vaux_P[5]/Vaux_N[5] and Vaux_P[13]/Vaux_N[13].
     *
     * bit h06: ADC_CH[22] - Vaux_P[6]/Vaux_N[6]   - when continuous mode: Vaux_P[6]/Vaux_N[6] and Vaux_P[14]/Vaux_N[14].
     *
     * bit h07: ADC_CH[23] - Vaux_P[7]/Vaux_N[7]   - when continuous mode: Vaux_P[7]/Vaux_N[7] and Vaux_P[15]/Vaux_N[15].
     *
     * bit h07: ADC_CH[24] - Vaux_P[8]/Vaux_N[8]   - when continuous mode: Undefined.
     *
     * bit h07: ADC_CH[25] - Vaux_P[9]/Vaux_N[9]   - when continuous mode: Undefined.
     *
     * bit h07: ADC_CH[26] - Vaux_P[10]/Vaux_N[10] - when continuous mode: Undefined.
     *
     * bit h07: ADC_CH[27] - Vaux_P[11]/Vaux_N[11] - when continuous mode: Undefined.
     *
     * bit h07: ADC_CH[28] - Vaux_P[12]/Vaux_N[12] - when continuous mode: Undefined.
     *
     * bit h07: ADC_CH[29] - Vaux_P[13]/Vaux_N[13] - when continuous mode: Undefined.
     *
     * bit h07: ADC_CH[30] - Vaux_P[14]/Vaux_N[14] - when continuous mode: Undefined.
     *
     * bit h07: ADC_CH[31] - Vaux_P[15]/Vaux_N[15] - when continuous mode: Undefined.
     *
     */
    uint32_t seq1_ch_sel;

    /** @brief  R/W SEQ2_CH_AVG - ADC Channel Averaging (4Ah and 4Bh)  (addr: 0x83C00328)
     *
     * Setting a bit enables the channel averaging for external channels.
     *
     * bit h00: ADC_CH[8]  - XADC calibration.
     *
     * bit h01: ADC_CH[9]  - Invalid channel selection.
     *
     * bit h02: ADC_CH[10] - Invalid channel selection.
     *
     * bit h03: ADC_CH[11] - Invalid channel selection.
     *
     * bit h04: ADC_CH[12] - Invalid channel selection.
     *
     * bit h05: ADC_CH[13] - Vcc_PINT.
     *
     * bit h06: ADC_CH[14] - Vcc_PAUX.
     *
     * bit h07: ADC_CH[15] - Vcco_DDR.
     *
     * bit h08: ADC_CH[0]  - On-chip temperature.
     *
     * bit h09: ADC_CH[1]  - Vcc_INT.
     *
     * bit h0A: ADC_CH[2]  - Vcc_AUX.
     *
     * bit h0B: ADC_CH[3]  - Vp/Vn.
     *
     * bit h0C: ADC_CH[4]  - Vref_P.
     *
     * bit h0D: ADC_CH[5]  - Vref_N.
     *
     * bit h0E: ADC_CH[6]  - Vcc_BRAM.
     *
     * bit h0F: ADC_CH[7]  - Invalid channel selection.
     *
     */
    uint32_t seq2_ch_avg;

    /** @brief  R/W SEQ3_CH_AVG - ADC Channel Averaging (4Ah and 4Bh)  (addr: 0x83C0032C)
     *
     * Setting a bit enables the channel averaging for external channels.
     *
     * bit h00: ADC_CH[16] - Vaux_P[0]/Vaux_N[0].
     *
     * bit h01: ADC_CH[17] - Vaux_P[1]/Vaux_N[1].
     *
     * bit h02: ADC_CH[18] - Vaux_P[2]/Vaux_N[2].
     *
     * bit h03: ADC_CH[19] - Vaux_P[3]/Vaux_N[3].
     *
     * bit h04: ADC_CH[20] - Vaux_P[4]/Vaux_N[4].
     *
     * bit h05: ADC_CH[21] - Vaux_P[5]/Vaux_N[5].
     *
     * bit h06: ADC_CH[22] - Vaux_P[6]/Vaux_N[6].
     *
     * bit h07: ADC_CH[23] - Vaux_P[7]/Vaux_N[7].
     *
     * bit h07: ADC_CH[24] - Vaux_P[8]/Vaux_N[8].
     *
     * bit h07: ADC_CH[25] - Vaux_P[9]/Vaux_N[9].
     *
     * bit h07: ADC_CH[26] - Vaux_P[10]/Vaux_N[10].
     *
     * bit h07: ADC_CH[27] - Vaux_P[11]/Vaux_N[11].
     *
     * bit h07: ADC_CH[28] - Vaux_P[12]/Vaux_N[12].
     *
     * bit h07: ADC_CH[29] - Vaux_P[13]/Vaux_N[13].
     *
     * bit h07: ADC_CH[30] - Vaux_P[14]/Vaux_N[14].
     *
     * bit h07: ADC_CH[31] - Vaux_P[15]/Vaux_N[15].
     *
     */
    uint32_t seq3_ch_avg;

    /** @brief  R/W SEQ4_CH_BU - ADC Channel Analog-Input Mode (4Ch and 4Dh)  (addr: 0x83C00330)
     *
     * Setting a bit selects the bipolar input mode for external channels.
     *
     * bit h00: ADC_CH[8]  - XADC calibration.
     *
     * bit h01: ADC_CH[9]  - Invalid channel selection.
     *
     * bit h02: ADC_CH[10] - Invalid channel selection.
     *
     * bit h03: ADC_CH[11] - Invalid channel selection.
     *
     * bit h04: ADC_CH[12] - Invalid channel selection.
     *
     * bit h05: ADC_CH[13] - Vcc_PINT.
     *
     * bit h06: ADC_CH[14] - Vcc_PAUX.
     *
     * bit h07: ADC_CH[15] - Vcco_DDR.
     *
     * bit h08: ADC_CH[0]  - On-chip temperature.
     *
     * bit h09: ADC_CH[1]  - Vcc_INT.
     *
     * bit h0A: ADC_CH[2]  - Vcc_AUX.
     *
     * bit h0B: ADC_CH[3]  - Vp/Vn.
     *
     * bit h0C: ADC_CH[4]  - Vref_P.
     *
     * bit h0D: ADC_CH[5]  - Vref_N.
     *
     * bit h0E: ADC_CH[6]  - Vcc_BRAM.
     *
     * bit h0F: ADC_CH[7]  - Invalid channel selection.
     *
     */
    uint32_t seq4_ch_bu;

    /** @brief  R/W SEQ5_CH_BU - ADC Channel Analog-Input Mode (4Ch and 4Dh)  (addr: 0x83C00334)
     *
     * Setting a bit selects the bipolar input mode for external channels.
     *
     * bit h00: ADC_CH[16] - Vaux_P[0]/Vaux_N[0].
     *
     * bit h01: ADC_CH[17] - Vaux_P[1]/Vaux_N[1].
     *
     * bit h02: ADC_CH[18] - Vaux_P[2]/Vaux_N[2].
     *
     * bit h03: ADC_CH[19] - Vaux_P[3]/Vaux_N[3].
     *
     * bit h04: ADC_CH[20] - Vaux_P[4]/Vaux_N[4].
     *
     * bit h05: ADC_CH[21] - Vaux_P[5]/Vaux_N[5].
     *
     * bit h06: ADC_CH[22] - Vaux_P[6]/Vaux_N[6].
     *
     * bit h07: ADC_CH[23] - Vaux_P[7]/Vaux_N[7].
     *
     * bit h07: ADC_CH[24] - Vaux_P[8]/Vaux_N[8].
     *
     * bit h07: ADC_CH[25] - Vaux_P[9]/Vaux_N[9].
     *
     * bit h07: ADC_CH[26] - Vaux_P[10]/Vaux_N[10].
     *
     * bit h07: ADC_CH[27] - Vaux_P[11]/Vaux_N[11].
     *
     * bit h07: ADC_CH[28] - Vaux_P[12]/Vaux_N[12].
     *
     * bit h07: ADC_CH[29] - Vaux_P[13]/Vaux_N[13].
     *
     * bit h07: ADC_CH[30] - Vaux_P[14]/Vaux_N[14].
     *
     * bit h07: ADC_CH[31] - Vaux_P[15]/Vaux_N[15].
     *
     */
    uint32_t seq5_ch_bu;

    /** @brief  R/W SEQ6_CH_ST - ADC Channel Settling Time (4Eh and 4Fh)  (addr: 0x83C00338)
     *
     * Setting a bit enables extended settling time for external channels.
     *
     * bit h00: ADC_CH[8]  - XADC calibration.
     *
     * bit h01: ADC_CH[9]  - Invalid channel selection.
     *
     * bit h02: ADC_CH[10] - Invalid channel selection.
     *
     * bit h03: ADC_CH[11] - Invalid channel selection.
     *
     * bit h04: ADC_CH[12] - Invalid channel selection.
     *
     * bit h05: ADC_CH[13] - Vcc_PINT.
     *
     * bit h06: ADC_CH[14] - Vcc_PAUX.
     *
     * bit h07: ADC_CH[15] - Vcco_DDR.
     *
     * bit h08: ADC_CH[0]  - On-chip temperature.
     *
     * bit h09: ADC_CH[1]  - Vcc_INT.
     *
     * bit h0A: ADC_CH[2]  - Vcc_AUX.
     *
     * bit h0B: ADC_CH[3]  - Vp/Vn.
     *
     * bit h0C: ADC_CH[4]  - Vref_P.
     *
     * bit h0D: ADC_CH[5]  - Vref_N.
     *
     * bit h0E: ADC_CH[6]  - Vcc_BRAM.
     *
     * bit h0F: ADC_CH[7]  - Invalid channel selection.
     *
     */
    uint32_t seq6_ch_st;

    /** @brief  R/W SEQ7_CH_ST - ADC Channel Settling Time (4Eh and 4Fh)  (addr: 0x83C0033C)
     *
     * Setting a bit enables extended settling time for external channels.
     *
     * bit h00: ADC_CH[16] - Vaux_P[0]/Vaux_N[0].
     *
     * bit h01: ADC_CH[17] - Vaux_P[1]/Vaux_N[1].
     *
     * bit h02: ADC_CH[18] - Vaux_P[2]/Vaux_N[2].
     *
     * bit h03: ADC_CH[19] - Vaux_P[3]/Vaux_N[3].
     *
     * bit h04: ADC_CH[20] - Vaux_P[4]/Vaux_N[4].
     *
     * bit h05: ADC_CH[21] - Vaux_P[5]/Vaux_N[5].
     *
     * bit h06: ADC_CH[22] - Vaux_P[6]/Vaux_N[6].
     *
     * bit h07: ADC_CH[23] - Vaux_P[7]/Vaux_N[7].
     *
     * bit h07: ADC_CH[24] - Vaux_P[8]/Vaux_N[8].
     *
     * bit h07: ADC_CH[25] - Vaux_P[9]/Vaux_N[9].
     *
     * bit h07: ADC_CH[26] - Vaux_P[10]/Vaux_N[10].
     *
     * bit h07: ADC_CH[27] - Vaux_P[11]/Vaux_N[11].
     *
     * bit h07: ADC_CH[28] - Vaux_P[12]/Vaux_N[12].
     *
     * bit h07: ADC_CH[29] - Vaux_P[13]/Vaux_N[13].
     *
     * bit h07: ADC_CH[30] - Vaux_P[14]/Vaux_N[14].
     *
     * bit h07: ADC_CH[31] - Vaux_P[15]/Vaux_N[15].
     *
     */
    uint32_t seq7_ch_st;

    /** @brief  R/W ALM00_TMP_HI  (addr: 0x83C00340)
     *
     * Setting of the alarm value.
     *
     */
    uint32_t alarm00_tmp_hi;

    /** @brief  R/W ALM01_VCCINT_HI  (addr: 0x83C00344)
     *
     * Setting of the alarm value.
     *
     */
    uint32_t alarm01_vccint_hi;

    /** @brief  R/W ALM02_VCCAUX_HI  (addr: 0x83C00348)
     *
     * Setting of the alarm value.
     *
     */
    uint32_t alarm02_vccaux_hi;

    /** @brief  R/W ALM03_OT_LIM  (addr: 0x83C0034C)
     *
     * Setting of the alarm value.
     *
     */
    uint32_t alarm03_ot_lim;

    /** @brief  R/W ALM04_TMP_LO  (addr: 0x83C00350)
     *
     * Setting of the alarm value.
     *
     */
    uint32_t alarm04_tmp_lo;

    /** @brief  R/W ALM05_VCCINT_LO  (addr: 0x83C00354)
     *
     * Setting of the alarm value.
     *
     */
    uint32_t alarm05_vccint_lo;

    /** @brief  R/W ALM06_VCCAUX_LO  (addr: 0x83C00358)
     *
     * Setting of the alarm value.
     *
     */
    uint32_t alarm06_vccaux_lo;

    /** @brief  R/W ALM07_OT_RST  (addr: 0x83C0035C)
     *
     * Setting of the alarm value.
     *
     */
    uint32_t alarm07_ot_rst;

    /** @brief  R/W ALM08_VCCBRAM_HI  (addr: 0x83C00360)
     *
     * Setting of the alarm value.
     *
     */
    uint32_t alarm08_vccbram_hi;

    /** @brief  R/W ALM09_VCCPINT_HI  (addr: 0x83C00364)
     *
     * Setting of the alarm value.
     *
     */
    uint32_t alarm09_vccpint_hi;

    /** @brief  R/W ALM10_VCCPAUX_HI  (addr: 0x83C00368)
     *
     * Setting of the alarm value.
     *
     */
    uint32_t alarm10_vccpaux_hi;

    /** @brief  R/W ALM11_VCCODDR_HI  (addr: 0x83C0036C)
     *
     * Setting of the alarm value.
     *
     */
    uint32_t alarm11_vccoddr_hi;

    /** @brief  R/W ALM12_VCCBRAM_LO  (addr: 0x83C00370)
     *
     * Setting of the alarm value.
     *
     */
    uint32_t alarm12_vccbram_lo;

    /** @brief  R/W ALM13_VCCPINT_LO  (addr: 0x83C00374)
     *
     * Setting of the alarm value.
     *
     */
    uint32_t alarm13_vccpint_lo;

    /** @brief  R/W ALM14_VCCPAUX_LO  (addr: 0x83C00378)
     *
     * Setting of the alarm value.
     *
     */
    uint32_t alarm14_vccpaux_lo;

    /** @brief  R/W ALM15_VCCODDR_LO  (addr: 0x83C0037C)
     *
     * Setting of the alarm value.
     *
     */
    uint32_t alarm15_vccoddr_lo;

} fpga_sys_xadc_reg_mem_t;


typedef struct fpga_sys_gpio_reg_mem_s {

    /** @brief  R/W GPIO_DATA
     */
    uint32_t gpio_data;

    /** @brief  R/W GPIO_TRI
     */
    uint32_t gpio_tri;

    /** @brief  R/W GPIO2_DATA
     */
    uint32_t gpio2_data;

    /** @brief  R/W GPIO2_TRI
     */
    uint32_t gpio2_tri;


    /** @brief  N/A - do not use */
    uint32_t _na_010[0x10C >> 2];


    /** @brief  R/W GIER
     */
    uint32_t gier;

    /** @brief  R/TOW IP_ISR
     */
    uint32_t ip_isr;


    /** @brief  N/A - do not use */
    uint32_t _na_124;


    /** @brief  R/W IP_IER
     */
    uint32_t ip_ier;

} fpga_sys_gpio_reg_mem_t;



/* function declarations, detailed descriptions is in apparent implementation file  */

/**
 * @brief Initialize interface to the system XADC module
 *
 * Set-up for FPGA access to the system XADC module.
 *
 * @retval  0 Success
 * @retval -1 Failure, error message is printed on standard error device
 *
 */
int fpga_sys_xadc_init(void);

/**
 * @brief Finalize and release allocated resources of the system XADC module
 *
 * @retval 0 Success, never fails
 */
int fpga_sys_xadc_exit(void);


/** @} */


#endif /* __FPGA_SYS_XADC_H */
