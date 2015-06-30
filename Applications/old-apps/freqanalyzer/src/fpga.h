/**
 * @brief Red Pitaya Frequency Response Analyzer FPGA Interface.
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

/* Housekeeping base address 0x40000000 */
#define HK_FPGA_BASE_ADDR 0x40000000
#define HK_FPGA_HW_REV_MASK 0x0000000f

typedef enum {
	eHwRevC=0,
	eHwRevD
} hw_rev_t;

/* FPGA housekeeping structure */
typedef struct hk_fpga_reg_mem_s {
    /* configuration:
     * bit   [3:0] - hw revision
     * bits [31:4] - reserved
     */
    uint32_t rev;
    /* DNA low word */
    uint32_t dna_lo;
    /* DNA high word */
    uint32_t dna_hi;
} hk_fpga_reg_mem_t;

/* Base address 0x40100000 */
#define SPECTR_FPGA_BASE_ADDR 0x40100000
#define SPECTR_FPGA_BASE_SIZE 0x00130000

//0x30000 MEMORY SPACE OF SPECTRUM ANALYSER EXTENDED IN ORDER TO INCLUDE SIGNAL GEN. (TODO: PROVIDE A SEPARATE MODULE FOR SIGNAL GENERATION)

#define SPECTR_FPGA_SIG_LEN   (16*1024)

#define SPECTR_FPGA_CONF_ARM_BIT  1
#define SPECTR_FPGA_CONF_RST_BIT  2

#define SPECTR_FPGA_TRIG_SRC_MASK 0x00000007
#define SPECTR_FPGA_CHA_THR_MASK  0x00003fff
#define SPECTR_FPGA_CHB_THR_MASK  0x00003fff
#define SPECTR_FPGA_TRIG_DLY_MASK 0xffffffff
#define SPECTR_FPGA_DATA_DEC_MASK 0x0001ffff

#define SPECTR_FPGA_CHA_OFFSET    0x10000
#define SPECTR_FPGA_CHB_OFFSET    0x20000

#define SPECTR_FPGA_SG_OFFSET     0x00100000

typedef struct spectr_fpga_reg_mem_s {
    /* configuration:
     * bit     [0] - arm_trigger
     * bit     [1] - rst_wr_state_machine
     * bits [31:2] - reserved 
     */
    uint32_t conf;

    /* trigger source:
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

    /* ChA threshold:
     * bits [13: 0] - ChA threshold
     * bits [31:14] - reserved
     */
    uint32_t cha_thr;

    /* ChB threshold:
     * bits [13: 0] - ChB threshold
     * bits [31:14] - reserved
     */
    uint32_t chb_thr;

    /* After trigger delay:
     * bits [31: 0] - trigger delay 
     * 32 bit number - how many decimated samples should be stored into a buffer.
     * (max 16k samples)
     */
    uint32_t trigger_delay;

    /* Data decimation
     * bits [16: 0] - decimation factor, legal values:
     *   1, 8, 64, 1024, 8192 65536
     *   If other values are written data is undefined 
     * bits [31:17] - reserved
     */
    uint32_t data_dec;

    /* Write pointers - both of the format:
     * bits [13: 0] - pointer
     * bits [31:14] - reserved
     * Current pointer - where machine stopped writting after trigger
     * Trigger pointer - where trigger was detected 
     */
    uint32_t wr_ptr_cur;
    uint32_t wr_ptr_trigger;
    
    /* ChA & ChB hysteresis - both of the format:
     * bits [13: 0] - hysteresis threshold
     * bits [31:14] - reserved
     */
    uint32_t cha_hystersis;
    uint32_t chb_hystersis;

    /*
     * bits [0] - enable signal average at decimation
     * bits [31:1] - reserved
     */
    uint32_t other;
    
    uint32_t reserved;
    

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
    

    /* ChA & ChB data - 14 LSB bits valid starts from 0x10000 and
     * 0x20000 and are each 16k samples long */
} spectr_fpga_reg_mem_t;

int spectr_fpga_init(void);
int spectr_fpga_exit(void);

int set_gen(uint32_t loc, uint32_t val);

int spectr_fpga_update_params(int trig_imm, int trig_source, int trig_edge, 
                              float trig_delay, float trig_level, int time_range,
                              int enable_avg_at_dec);
int spectr_fpga_reset(void);
int spectr_fpga_arm_trigger(void);
int spectr_fpga_set_trigger(uint32_t trig_source);
int spectr_fpga_set_trigger_delay(uint32_t trig_delay);

/* Returns 0 if no trigger, 1 if trigger */
int spectr_fpga_triggered(void);

/* Returns pointer to the ChA and ChB signals (of length SPECTR_FPGA_SIG_LEN) */
int spectr_fpga_get_sig_ptr(int **cha_signal, int **chb_signal);

/* Copies the last acquisition (trig wr. ptr -> curr. wr. ptr) */
int spectr_fpga_get_signal(double **cha_signal, double **chb_signal);

/* Returns signal pointers from the FPGA */
int spectr_fpga_get_wr_ptr(int *wr_ptr_curr, int *wr_ptr_trig);

/* Returnes signal content */
/* various constants */
extern const float c_spectr_fpga_smpl_freq;
extern const float c_spectr_fpga_smpl_period;

/* helper conversion functions */
/* Convert correct value for FPGA trigger source from trig_immediately, 
 * trig_source and trig_edge from application params.
 */
int spectr_fpga_cnv_trig_source(int trig_imm, int trig_source, int trig_edge);
/* Converts freq_range parameter (0-5) to decimation factor */
int spectr_fpga_cnv_freq_range_to_dec(int freq_range);
/* Converts freq_range parameter (0-5) to unit enumerator */
int spectr_fpga_cnv_freq_range_to_unit(int freq_range);

/* Converts time in [s] to ADC samples (depends on decimation) */
int spectr_fpga_cnv_time_to_smpls(float time, int dec_factor);
/* Converts voltage in [V] to ADC counts */
int spectr_fpga_cnv_v_to_cnt(float voltage);
/* Converts ADC ounts to [V] */
float spectr_fpga_cnv_cnt_to_v(int cnts);

int __spectr_fpga_cleanup_mem(void);

#endif /* __FPGA_H*/
