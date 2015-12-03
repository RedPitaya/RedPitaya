/**
 * @brief Red Pitaya FPGA Interface for the House-keeping sub-module.
 *
 * @author Ulrich Habel (DF4IAH) <espero7757@gmx.net>
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef __FPGA_HK_H
#define __FPGA_HK_H

#include <stdint.h>


/** @defgroup fpga_hk_h FPGA House-keeping sub-module access
 * @{
 */

/** @brief House-keeping starting address of FPGA registers. */
#define FPGA_HK_BASE_ADDR		0x40000000

/** @brief House-keeping memory map size of FPGA registers. */
#define FPGA_HK_BASE_SIZE		0x10000

/** @brief FPGA register offset addresses of the House-keeping sub-system base address.
 */
enum {
	HK_REV						= 0x00000,
	HK_DNA_LO					= 0x00004,
	HK_DNA_HI					= 0x00008,
	HK_DIGITAL_LOOP				= 0x0000C,
	HK_EXP_DIR_P				= 0x00010,
	HK_EXP_DIR_N				= 0x00014,
	HK_EXP_OUT_P				= 0x00018,
	HK_EXP_OUT_N				= 0x0001C,
	HK_EXP_IN_P					= 0x00020,
	HK_EXP_IN_N					= 0x00024,
	HK_LEDS						= 0x00030
} HK_REG_ENUMS;

/** @brief FPGA registry structure for the House-keeping sub-module.
 *
 * This structure is the direct image of the physical FPGA memory for the HouseKeeping sub-module.
 * It assures direct read / write FPGA access when it is mapped to the appropriate memory address
 * through the /dev/mem device.
 */
typedef struct fpga_hk_reg_mem_s {

	/** @brief  R/O HK_REV - ID of the FPGA (addr: 0x40000000
	 *
     * bit h03..h00: Hardware revision
     *
     * bit h31..h04: n/a (reserved)
     *
     */
    uint32_t rev;

    /** @brief  R/O HK_DNA_LO - DNA of the FPGA - the LSB part : 0x40000004
     *
     * bit h1F..h00: LSB of FPGA DNA data.
     *
     */
    uint32_t dna_lo;

    /** @brief  R/O HK_DNA_HI - DNA of the FPGA - the MSB part : 0x40000008
     *
     * bit h1F..h00: MSB of FPGA DNA data.
     *
     */
    uint32_t dna_hi;

    /** @brief  R/W HK_DIGITAL_LOOP - digital loop control : 0x4000000c
     *
     * bit h00: '1' output data for DAC is looped back as ADC input stream.
     *
     */
    uint32_t digital_loop;


    /** @brief  R/W HK_EXP_DIR_P - expansion port output, positive lines, direction mask : 0x40000010
     *
     * bit h00: '1' HK_EXP_DIR_0_P is set for output.
     *
     * bit h01: '1' HK_EXP_DIR_1_P is set for output.
     *
     * bit h02: '1' HK_EXP_DIR_2_P is set for output.
     *
     * bit h03: '1' HK_EXP_DIR_3_P is set for output.
     *
     * bit h04: '1' HK_EXP_DIR_4_P is set for output.
     *
     * bit h05: '1' HK_EXP_DIR_5_P is set for output.
     *
     * bit h06: '1' HK_EXP_DIR_6_P is set for output.
     *
     * bit h07: '1' HK_EXP_DIR_7_P is set for output.
     *
     * bit h08: '1' HK_EXP_DIR_8_P is set for output.
     *
     * bit h09: '1' HK_EXP_DIR_9_P is set for output.
     *
     * bit h0A: '1' HK_EXP_DIR_10_P is set for output.
     *
     * bit h0B: '1' HK_EXP_DIR_11_P is set for output.
     *
     * bit h0C: '1' HK_EXP_DIR_12_P is set for output.
     *
     * bit h0D: '1' HK_EXP_DIR_13_P is set for output.
     *
     * bit h0E: '1' HK_EXP_DIR_14_P is set for output.
     *
     * bit h0F: '1' HK_EXP_DIR_15_P is set for output.
     *
     * bit h1F..h10: n/a
     *
     */
    uint32_t exp_dir_p;

    /** @brief  R/W HK_EXP_DIR_N - expansion port output, negative lines, direction mask : 0x40000014
     *
     * bit h00: '1' HK_EXP_DIR_0_N is set for output.
     *
     * bit h01: '1' HK_EXP_DIR_1_N is set for output.
     *
     * bit h02: '1' HK_EXP_DIR_2_N is set for output.
     *
     * bit h03: '1' HK_EXP_DIR_3_N is set for output.
     *
     * bit h04: '1' HK_EXP_DIR_4_N is set for output.
     *
     * bit h05: '1' HK_EXP_DIR_5_N is set for output.
     *
     * bit h06: '1' HK_EXP_DIR_6_N is set for output.
     *
     * bit h07: '1' HK_EXP_DIR_7_N is set for output.
     *
     * bit h08: '1' HK_EXP_DIR_8_N is set for output.
     *
     * bit h09: '1' HK_EXP_DIR_9_N is set for output.
     *
     * bit h0A: '1' HK_EXP_DIR_10_N is set for output.
     *
     * bit h0B: '1' HK_EXP_DIR_11_N is set for output.
     *
     * bit h0C: '1' HK_EXP_DIR_12_N is set for output.
     *
     * bit h0D: '1' HK_EXP_DIR_13_N is set for output.
     *
     * bit h0E: '1' HK_EXP_DIR_14_N is set for output.
     *
     * bit h0F: '1' HK_EXP_DIR_15_N is set for output.
     *
     * bit h1F..h10: n/a
     *
     */
    uint32_t exp_dir_n;

    /** @brief  R/W HK_EXP_OUT_P - expansion port output, positive lines, data register : 0x40000018
     *
     * bit h00: '1' HK_EXP_OUT_0_P is set to high level.
     *
     * bit h01: '1' HK_EXP_OUT_1_P is set to high level.
     *
     * bit h02: '1' HK_EXP_OUT_2_P is set to high level.
     *
     * bit h03: '1' HK_EXP_OUT_3_P is set to high level.
     *
     * bit h04: '1' HK_EXP_OUT_4_P is set to high level.
     *
     * bit h05: '1' HK_EXP_OUT_5_P is set to high level.
     *
     * bit h06: '1' HK_EXP_OUT_6_P is set to high level.
     *
     * bit h07: '1' HK_EXP_OUT_7_P is set to high level.
     *
     * bit h08: '1' HK_EXP_OUT_8_P is set to high level.
     *
     * bit h09: '1' HK_EXP_OUT_9_P is set to high level.
     *
     * bit h0A: '1' HK_EXP_OUT_10_P is set to high level.
     *
     * bit h0B: '1' HK_EXP_OUT_11_P is set to high level.
     *
     * bit h0C: '1' HK_EXP_OUT_12_P is set to high level.
     *
     * bit h0D: '1' HK_EXP_OUT_13_P is set to high level.
     *
     * bit h0E: '1' HK_EXP_OUT_14_P is set to high level.
     *
     * bit h0F: '1' HK_EXP_OUT_15_P is set to high level.
     *
     * bit h1F..h10: n/a
     *
     */
    uint32_t exp_out_p;

    /** @brief  R/W HK_EXP_OUT_N - expansion port output, negative lines, data register : 0x4000001c
     *
     * bit h00: '1' HK_EXP_OUT_0_N is set to high level.
     *
     * bit h01: '1' HK_EXP_OUT_1_N is set to high level.
     *
     * bit h02: '1' HK_EXP_OUT_2_N is set to high level.
     *
     * bit h03: '1' HK_EXP_OUT_3_N is set to high level.
     *
     * bit h04: '1' HK_EXP_OUT_4_N is set to high level.
     *
     * bit h05: '1' HK_EXP_OUT_5_N is set to high level.
     *
     * bit h06: '1' HK_EXP_OUT_6_N is set to high level.
     *
     * bit h07: '1' HK_EXP_OUT_7_N is set to high level.
     *
     * bit h08: '1' HK_EXP_OUT_8_N is set to high level.
     *
     * bit h09: '1' HK_EXP_OUT_9_N is set to high level.
     *
     * bit h0A: '1' HK_EXP_OUT_10_N is set to high level.
     *
     * bit h0B: '1' HK_EXP_OUT_11_N is set to high level.
     *
     * bit h0C: '1' HK_EXP_OUT_12_N is set to high level.
     *
     * bit h0D: '1' HK_EXP_OUT_13_N is set to high level.
     *
     * bit h0E: '1' HK_EXP_OUT_14_N is set to high level.
     *
     * bit h0F: '1' HK_EXP_OUT_15_N is set to high level.
     *
     * bit h1F..h10: n/a
     *
     */
    uint32_t exp_out_n;

    /** @brief  R/O HK_EXP_IN_P - expansion port input, positive lines, data register : 0x40000020
     *
     * bit h00: '1' HK_EXP_IN_0_P is at high level.
     *
     * bit h01: '1' HK_EXP_IN_1_P is at high level.
     *
     * bit h02: '1' HK_EXP_IN_2_P is at high level.
     *
     * bit h03: '1' HK_EXP_IN_3_P is at high level.
     *
     * bit h04: '1' HK_EXP_IN_4_P is at high level.
     *
     * bit h05: '1' HK_EXP_IN_5_P is at high level.
     *
     * bit h06: '1' HK_EXP_IN_6_P is at high level.
     *
     * bit h07: '1' HK_EXP_IN_7_P is at high level.
     *
     * bit h08: '1' HK_EXP_IN_8_P is at high level.
     *
     * bit h09: '1' HK_EXP_IN_9_P is at high level.
     *
     * bit h0A: '1' HK_EXP_IN_10_P is at high level.
     *
     * bit h0B: '1' HK_EXP_IN_11_P is at high level.
     *
     * bit h0C: '1' HK_EXP_IN_12_P is at high level.
     *
     * bit h0D: '1' HK_EXP_IN_13_P is at high level.
     *
     * bit h0E: '1' HK_EXP_IN_14_P is at high level.
     *
     * bit h0F: '1' HK_EXP_IN_15_P is at high level.
     *
     * bit h1F..h10: n/a
     *
     */
    uint32_t exp_in_p;

    /** @brief  R/O HK_EXP_IN_N - expansion port input, negative lines, data register : 0x40000024
     *
     * bit h00: '1' HK_EXP_IN_0_N is at high level.
     *
     * bit h01: '1' HK_EXP_IN_1_N is at high level.
     *
     * bit h02: '1' HK_EXP_IN_2_N is at high level.
     *
     * bit h03: '1' HK_EXP_IN_3_N is at high level.
     *
     * bit h04: '1' HK_EXP_IN_4_N is at high level.
     *
     * bit h05: '1' HK_EXP_IN_5_N is at high level.
     *
     * bit h06: '1' HK_EXP_IN_6_N is at high level.
     *
     * bit h07: '1' HK_EXP_IN_7_N is at high level.
     *
     * bit h08: '1' HK_EXP_IN_8_N is at high level.
     *
     * bit h09: '1' HK_EXP_IN_9_N is at high level.
     *
     * bit h0A: '1' HK_EXP_IN_10_N is at high level.
     *
     * bit h0B: '1' HK_EXP_IN_11_N is at high level.
     *
     * bit h0C: '1' HK_EXP_IN_12_N is at high level.
     *
     * bit h0D: '1' HK_EXP_IN_13_N is at high level.
     *
     * bit h0E: '1' HK_EXP_IN_14_N is at high level.
     *
     * bit h0F: '1' HK_EXP_IN_15_N is at high level.
     *
     * bit h1F..h10: n/a
     *
     */
    uint32_t exp_in_n;

    /** @brief Placeholder */
    uint32_t _reserved01;

    /** @brief Placeholder */
    uint32_t _reserved02;


    /** @brief  R/W HK_LEDS - LEDs data register : 0x40000030
     *
     * bit h00: '1' HK_LEDS 0 is on.
     *
     * bit h01: '1' HK_LEDS 1 is on.
     *
     * bit h02: '1' HK_LEDS 2 is on.
     *
     * bit h03: '1' HK_LEDS 3 is on.
     *
     * bit h04: '1' HK_LEDS 4 is on.
     *
     * bit h05: '1' HK_LEDS 5 is on.
     *
     * bit h06: '1' HK_LEDS 6 is on.
     *
     * bit h07: '1' HK_LEDS 7 is on.
     *
     * bit h1F..h08: n/a
     *
     */
    uint32_t leds;

} fpga_hk_reg_mem_t;



/* function declarations, detailed descriptions is in apparent implementation file  */


// HouseKeepking FPGA accessors

/**
 * @brief Initialize interface to House-keeping FPGA sub-module
 *
 * Set-up for FPGA access to the House-keeping sub-module.
 *
 * @retval  0 Success
 * @retval -1 Failure, error message is printed on standard error device
 *
 */
int fpga_hk_init(void);

/**
 * @brief Finalize and release allocated resources of the House-keeping sub-module
 *
 * @retval 0 Success, never fails
 */
int fpga_hk_exit(void);

/**
 * @brief Activates the LEDs on the board.
 *
 * @param[in] doToggle   true: mask defines which LED states to be toggled, leds  is voided. false: mask defines the LED bits to be set, leds  each led is bright when its masked bit is set.
 * @param[in] mask   defines which LED states to be changed.
 * @param[in] leds   defines which masked LEDs should be bright. When toggling this parameter is voided

 * @retval 0 Success
 * @retval -1 FPGA HouseKeeping sub-module not initialized
 */
int fpga_hk_setLeds(unsigned char doToggle, unsigned char mask, unsigned char leds);

/** @} */


#endif /* __FPGA_HK_H */
