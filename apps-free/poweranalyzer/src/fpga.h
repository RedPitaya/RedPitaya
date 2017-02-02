/**
 * $Id: fpga.h 881 2013-12-16 05:37:34Z rp_jmenart $
 *
 * @brief Red Pitaya Power analyzer FPGA Interface.
 *
 * @Author Jure Menart <juremenart@gmail.com>
 *         
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef __FPGA_H
#define __FPGA_H

#include <stdint.h>


/** @defgroup fpga_h Acquisition
 * @{
 */

/** Starting address of FPGA registers handling Oscilloscope module. */
#define PWR_FPGA_BASE_ADDR 	0x40100000
/** The size of FPGA registers handling Oscilloscope module. */
#define PWR_FPGA_BASE_SIZE 0x30000
/** Size of data buffer into which input signal is captured , must be 2^n!. */
#define PWR_FPGA_SIG_LEN   (16*1024)

/** Bit index in FPGA configuration register for arming the trigger. */
#define PWR_FPGA_CONF_ARM_BIT  1
/** Bit index in FPGA configuration register for reseting write state machine. */
#define PWR_FPGA_CONF_RST_BIT  2

/** Bit mask in the trigger_source register for depicting the trigger source type. */
#define PWR_FPGA_TRIG_SRC_MASK 0x00000007
/** Bit mask in the cha_thr register for depicting trigger threshold on channel A. */
#define PWR_FPGA_CHA_THR_MASK  0x00003fff
/** Bit mask in the cha_thr register for depicting trigger threshold on channel B. */
#define PWR_FPGA_CHB_THR_MASK  0x00003fff
/** Bit mask in the trigger_delay register for depicting trigger delay. */
#define PWR_FPGA_TRIG_DLY_MASK 0xffffffff

/** Offset to the memory buffer where signal on channel A is captured. */
#define PWR_FPGA_CHA_OFFSET    0x10000
/** Offset to the memory buffer where signal on channel B is captured. */
#define PWR_FPGA_CHB_OFFSET    0x20000

/** Hysteresis register default setting */

#define PWR_HYSTERESIS 0x3F

/** @brief FPGA registry structure for Oscillpwrope core module.
 *
 * This structure is direct image of physical FPGA memory. It assures
 * direct read/write FPGA access when it is mapped to the appropriate memory address
 * through /dev/mem device.
 */
typedef struct pwr_fpga_reg_mem_s {
    /** @brief  Configuration:
     * bit     [0] - arm_trigger
     * bit     [1] - rst_wr_state_machine
     * bits [31:2] - reserved 
     */
    uint32_t conf;

    /** @brief  Trigger source:
     * bits [ 2 : 0] - trigger source:
     *   1 - trig immediately
     *   2 - ChA positive edge
     *   3 - ChA negative edge
     *   4 - ChB positive edge 
     *   5 - ChB negative edge
     *   6 - External trigger 0
     *   7 - External trigger 1 
     * bits [31 : 3] -reserved
     */
    uint32_t trig_source;

    /** @brief  ChA threshold:
     * bits [13: 0] - ChA threshold
     * bits [31:14] - reserved
     */
    uint32_t cha_thr;

    /** @brief  ChB threshold:
     * bits [13: 0] - ChB threshold
     * bits [31:14] - reserved
     */
    uint32_t chb_thr;

    /** @brief  After trigger delay:
     * bits [31: 0] - trigger delay 
     * 32 bit number - how many decimated samples should be stored into a buffer.
     * (max 16k samples)
     */
    uint32_t trigger_delay;

    /** @brief  Data decimation
     * bits [16: 0] - decimation factor, legal values:
     *   1, 8, 64, 1024, 8192 65536
     *   If other values are written data is undefined 
     * bits [31:17] - reserved
     */
    uint32_t data_dec;

    /** @brief  Write pointers - both of the format:
     * bits [13: 0] - pointer
     * bits [31:14] - reserved
     * Current pointer - where machine stopped writing after trigger
     * Trigger pointer - where trigger was detected 
     */
    uint32_t wr_ptr_cur;
    uint32_t wr_ptr_trigger;

    /** @brief  ChA & ChB hysteresis - both of the format:
     * bits [13: 0] - hysteresis threshold
     * bits [31:14] - reserved
     */
    uint32_t cha_hystersis;
    uint32_t chb_hystersis;

    /** @brief
     * bits [0] - enable signal average at decimation
     * bits [31:1] - reserved
     */
    uint32_t other;
    
    uint32_t reseved;
    
    /** @brief ChA Equalization filter
     * bits [17:0] - AA coefficient (pole)
     * bits [31:18] - reserved
     */
    uint32_t cha_filt_aa;    
    
    /** @brief ChA Equalization filter
     * bits [24:0] - BB coefficient (zero)
     * bits [31:25] - reserved
     */
    uint32_t cha_filt_bb;    
    
    /** @brief ChA Equalization filter
     * bits [24:0] - KK coefficient (gain)
     * bits [31:25] - reserved
     */
    uint32_t cha_filt_kk;  
    
    /** @brief ChA Equalization filter
     * bits [24:0] - PP coefficient (pole)
     * bits [31:25] - reserved
     */
    uint32_t cha_filt_pp;     
    
    
    

    /** @brief ChB Equalization filter
     * bits [17:0] - AA coefficient (pole)
     * bits [31:18] - reserved
     */
    uint32_t chb_filt_aa;    
    
    /** @brief ChB Equalization filter
     * bits [24:0] - BB coefficient (zero)
     * bits [31:25] - reserved
     */
    uint32_t chb_filt_bb;    
    
    /** @brief ChB Equalization filter
     * bits [24:0] - KK coefficient (gain)
     * bits [31:25] - reserved
     */
    uint32_t chb_filt_kk;  
    
    /** @brief ChB Equalization filter
     * bits [24:0] - PP coefficient (pole)
     * bits [31:25] - reserved
     */
    uint32_t chb_filt_pp;            
    
    
    /** @brief  ChA & ChB data - 14 LSB bits valid starts from 0x10000 and
     * 0x20000 and are each 16k samples long */
} pwr_fpga_reg_mem_t;

/** @} */


/* constants */
extern const float c_pwr_fpga_smpl_freq;
extern const float c_pwr_fpga_smpl_period;
extern const int   c_pwr_fpga_adc_bits;


/* function declarations, detailed descriptions is in apparent implementation file  */
int   pwr_fpga_init(void);
int   pwr_fpga_exit(void);
int   pwr_fpga_update_params(int trig_imm, int trig_source, int trig_edge,
                             float trig_delay, float trig_level, int time_range,
                             float ch1_adc_max_v, float ch2_adc_max_v,
                             int ch1_calib_dc_off, float ch1_user_dc_off,
                             int ch2_calib_dc_off, float ch2_user_dc_off,
                             float diff_probe_att, float curr_probe_fact,
			                 int ch1_gain, int ch2_gain,
                             int enable_avg_at_dec);
int   pwr_fpga_reset(void);
int   pwr_fpga_arm_trigger(void);
int   pwr_fpga_set_trigger(uint32_t trig_source);
int   pwr_fpga_set_trigger_delay(uint32_t trig_delay);
int   pwr_fpga_triggered(void);
int   pwr_fpga_get_sig_ptr(int **cha_signal, int **chb_signal);

/* Copies the last acquisition (trig wr. ptr -> curr. wr. ptr) */
int   pwr_fpga_get_wr_ptr(int *wr_ptr_curr, int *wr_ptr_trig);

int   pwr_fpga_cnv_trig_source(int trig_imm, int trig_source, int trig_edge);
int   pwr_fpga_cnv_time_range_to_dec(int time_range);
int   pwr_fpga_cnv_time_to_smpls(float time, int dec_factor);
int   pwr_fpga_cnv_v_to_cnt(float u_voltage, float max_adc_v,
                            int calib_dc_off, float user_dc_off,
                            float diff_probe_att);
int   pwr_fpga_cnv_a_to_cnt(float current, float max_adc_v,
                            int calib_dc_off, float user_dc_off,
                            float k);
float pwr_fpga_cnv_cnt_to_v(int cnts, float max_adc_v,
                            int calib_dc_off, float user_dc_off,
                            float diff_probe_att);
float pwr_fpga_cnv_cnt_to_a(int cnts, float max_adc_v,
                            int calib_dc_off, float user_dc_off,
                            float k);                            
float pwr_fpga_calc_adc_max_v(uint32_t fe_gain_fs);

#endif /* __FPGA_H */
