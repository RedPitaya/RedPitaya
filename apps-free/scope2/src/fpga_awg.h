/**
 * $Id: fpga_awg.h 881 2013-12-16 05:37:34Z rp_jmenart $
 *
 * @brief Red Pitaya Arbitrary Waveform Generator (AWG) FPGA controller.
 *
 * @Author Jure Menart <juremenart@gmail.com>
 *         
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef _FPGA_AWG_H_
#define _FPGA_AWG_H_

#include <stdint.h>

/** @defgroup fpga_awg_h Arbitrary Waveform Generator
 * @{
 */

/** Base AWG FPGA address */
#define AWG_BASE_ADDR 0x40200000
/** Base AWG FPGA core size */
#define AWG_BASE_SIZE 0x30000
/** FPGA AWG output signal buffer length */
#define AWG_SIG_LEN   (16*1024)
/** FPGA AWG output signal 1 offset */
#define AWG_CHA_OFFSET    0x10000
/** FPGA AWG output signal 2 offset */
#define AWG_CHB_OFFSET    0x20000

/** @brief AWG FPGA registry structure.
 *
 * This structure is direct image of physical FPGA memory. When accessing it all
 * reads/writes are performed directly from/to FPGA AWG.
 */
typedef struct awg_reg_t {
    /** @brief Offset 0x00 - State machine configuration
     *
     * State machine configuration register (offset 0x00):
     *  bits [31:24] - Reserved
     *  bit  [   23] - Channel B output set to 0
     *  bit  [   22] - Channel B state machine reset
     *  bit  [   21] - Channel B set one time trigger
     *  bit  [   20] - Channel B state machine wrap pointer
     *  bits [19:16] - Channel B trigger selector 
     *  bits [15: 8] - Reserved
     *  bit  [    7] - Channel B output set to 0
     *  bit  [    6] - Channel B state machine reset
     *  bit  [    5] - Channel B set one time trigger
     *  bit  [    4] - Channel B state machine wrap pointer
     *  bits [ 3: 0] - Channel B trigger selector 
     *
     */
    uint32_t state_machine_conf;
    /** @brief Offset 0x04 - Channel A amplitude scale and offset 
     *
     * Channel A amplitude scale and offset register (offset 0x04) used to set the
     * amplitude and scale of output signal: out = (data * scale)/0x2000 + offset
     * bits [31:30] - Reserved
     * bits [29:16] - Amplitude offset
     * bits [15:14] - Reserved 
     * bits [13: 0] - Amplitude scale (0x2000 == multiply by 1, unsigned)
     */
    uint32_t cha_scale_off;
    /** @brief Offset 0x08 - Channel A counter wrap
     *
     * Channel A counter wrap (offset 0x08) - value at which FPGA AWG state 
     * machine will wrap the output signal buffer readout:
     * bits [31:30] - Reserved
     * bits [29: 0] - Output signal counter wrap
     */
    uint32_t cha_count_wrap;
    /** @brief Offset 0x0C - Channel A starting counter offset
     *
     * Channel A starting counter offset (offset 0x0C) - start offset when 
     * trigger arrives
     * bits [31:30] - Reserved
     * bits [29: 0] - Counter start offset
     */
    uint32_t cha_start_off;
    /** @brief Offset 0x10 - Channel A counter step
     *
     * Channel A counter step (offset 0x10) - value by which FPGA AWG state 
     * machine increment readout from output signal buffer:
     * bits [31:30] - Reserved
     * bits [29: 0] - Counter step
     */
    uint32_t cha_count_step;

    /** @brief Reserved */
    uint32_t reserved_regs[4];

    /** @brief Offset 0x24 - Channel B amplitude scale and offset 
     *
     * Channel B amplitude scale and offset register (offset 0x24) used to set the
     * amplitude and scale of output signal: out = (data * scale)/0x2000 + offset
     * bits [31:30] - Reserved
     * bits [29:16] - Amplitude offset
     * bits [15:14] - Reserved 
     * bits [13: 0] - Amplitude scale (0x2000 == multiply by 1, unsigned)
     */
    uint32_t chb_scale_off;
    /** @brief Offset 0x28 - Channel B counter wrap
     *
     * Channel B counter wrap (offset 0x28) - value at which FPGA AWG state 
     * machine will wrap the output signal buffer readout:
     * bits [31:30] - Reserved
     * bits [29: 0] - Output signal counter wrap
     */
    uint32_t chb_count_wrap;
    /** @brief Offset 0x2C - Channel B starting counter offset
     *
     * Channel B starting counter offset (offset 0x2C) - start offset when 
     * trigger arrives
     * bits [31:30] - Reserved
     * bits [29: 0] - Counter start offset
     */
    uint32_t chb_start_off;
    /** @brief Offset 0x30 - Channel B counter step
     *
     * Channel B counter step (offset 0x30) - value by which FPGA AWG state 
     * machine increment readout from output signal buffer:
     * bits [31:30] - Reserved
     * bits [29: 0] - Counter step
     */
    uint32_t chb_count_step;
} awg_reg_t;

/** @} */

/* Description of the following variables or function declarations is in 
 * fpga_awg.c 
 */
extern awg_reg_t    *g_awg_reg;
extern uint32_t     *g_awg_cha_mem;
extern uint32_t     *g_awg_chb_mem;
extern const double  c_awg_smpl_freq;
extern const int     c_awg_fpga_dac_bits;

int fpga_awg_init(void);
int fpga_awg_exit(void);

float fpga_awg_calc_dac_max_v(uint32_t be_gain_fs);

#endif // _FPGA_AWG_H_
