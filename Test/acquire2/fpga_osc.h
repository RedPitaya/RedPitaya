/**
 * $Id: fpga_osc.h 881 2013-12-16 05:37:34Z rp_jmenart $
 * 
 * @brief Red Pitaya Oscilloscope FPGA controller.
 *
 * @Author Jure Menart <juremenart@gmail.com>
 * 
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#ifndef __FPGA_OSC_H
#define __FPGA_OSC_H

#include <stdint.h>

#ifdef Z20_250_12
#define ADC_SAMPLE_RATE 250e6
#define ADC_BITS 14
#define ADC_MASK 0x3fff
#define ADC_MAX_V 1.0
#endif

#ifdef Z20
#define ADC_SAMPLE_RATE 122.880e6
#define ADC_BITS 16
#define ADC_MASK 0xffff
#define ADC_MAX_V 1.0
#endif

#if defined Z10 || defined Z20_125
#define ADC_SAMPLE_RATE 125e6
#define ADC_BITS 14
#define ADC_MASK 0x3fff
#define ADC_MAX_V 1.0
#endif

/** @defgroup fpga_osc_h fpga_osc_h
 * @{
 */

/** Base OSC FPGA address */
#define OSC_FPGA_BASE_ADDR 0x40100000
/** Base OSC FPGA core size */
#define OSC_FPGA_BASE_SIZE 0x30000
/** OSC FPGA input signal buffer length */
#define OSC_FPGA_SIG_LEN   (16*1024)
/** OSC FPGA ARM bit in configuration register */
#define OSC_FPGA_CONF_ARM_BIT  1
/** OSC FPGA reset bit in configuration register */
#define OSC_FPGA_CONF_RST_BIT  2

/** OSC FPGA trigger source register mask */
#define OSC_FPGA_TRIG_SRC_MASK  0x00000007
/** OSC FPGA trigger source register mask */
#define OSC_FPGA_BUFF_FILL_MASK 0x00000010
/** OSC FPGA Channel A threshold register mask */
#define OSC_FPGA_CHA_THR_MASK   0x00003fff
/** OSC FPGA Channel B threshold register mask */
#define OSC_FPGA_CHB_THR_MASK   0x00003fff
/** OSC FPGA trigger delay register register mask */
#define OSC_FPGA_TRIG_DLY_MASK  0xffffffff
/** OSC FPGA data decimation mask */
#define OSC_FPGA_DATA_DEC_MASK  0x0001ffff

/** OSC FPGA Channel A input signal buffer offset */
#define OSC_FPGA_CHA_OFFSET    0x10000
/** OSC FPGA Channel B input signal buffer offset */
#define OSC_FPGA_CHB_OFFSET    0x20000

/** @brief OSC FPGA registry structure.
 *
 * This structure is direct image of physical FPGA memory. When accessing it all
 * reads/writes are performed directly from/to FPGA OSC core.
 */
typedef struct osc_fpga_reg_mem_s {
    /** @brief Offset 0x00 - configuration register
     *
     * Configuration register (offset 0x00):
     * bit     [0] - arm_trigger
     * bit     [1] - rst_wr_state_machine
     * bit     [2] - Trigger has arrived stays on (1) until next arm or reset (R)
     * bit     [3] - Trigger remains armed after ACQ delay passes (W)
     * bit     [4] - ACQ delay has passed / (all data was written to buffer) (R)
     * bits [31:5] - reserved 
     */
    uint32_t conf;

    /** @brief Offset 0x04 - trigger source register
     *
     * Trigger source register (offset 0x04):
     * bits [ 2 : 0] - trigger source:
     *     1 - trig immediately
     *     2 - ChA positive edge
     *     3 - ChA negative edge
     *     4 - ChB positive edge 
     *     5 - ChB negative edge
     *     6 - External trigger 0
     *     7 - External trigger 1 
     * bits [31 : 3] -reserved
     */
    uint32_t trig_source;

    /** @brief Offset 0x08 - Channel A threshold register
     *
     * Channel A threshold register (offset 0x08):
     * bits [13: 0] - ChA threshold
     * bits [31:14] - reserved
     */
    uint32_t cha_thr;

    /** @brief Offset 0x0C - Channel B threshold register
     *
     * Channel B threshold register (offset 0x0C):
     * bits [13: 0] - ChB threshold
     * bits [31:14] - reserved
     */
    uint32_t chb_thr;

    /** @brief Offset 0x10 - After trigger delay register
     *
     * After trigger delay register (offset 0x10)
     * bits [31: 0] - trigger delay 
     * 32 bit number - how many decimated samples should be stored into a buffer.
     * (max 16k samples)
     */
    uint32_t trigger_delay;

    /** @brief Offset 0x14 - Data decimation register
     *
     * Data decimation register (offset 0x14):
     * bits [16: 0] - decimation factor, legal values:
     *   1, 8, 64, 1024, 8192 65536
     *   If other values are written data is undefined 
     * bits [31:17] - reserved
     */
    uint32_t data_dec;

    /** @brief Offset 0x18 - Current write pointer register
     *
     * Current write pointer register (offset 0x18), read only:
     * bits [13: 0] - current write pointer
     * bits [31:14] - reserved
     */
    uint32_t wr_ptr_cur;
    /** @brief Offset 0x1C - Trigger write pointer register
     *
     * Trigger write pointer register (offset 0x1C), read only:
     * bits [13: 0] - trigger pointer (pointer where trigger was detected)
     * bits [31:14] - reserved
     */
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
    
    /* ChA & ChB data - 14 LSB bits valid starts from 0x10000 and
     * 0x20000 and are each 16k samples long */
} osc_fpga_reg_mem_t;

/** @} */

// TODO: Move to a shared folder and share with scope & spectrum.
/** Equalization & shaping filter coefficients */
typedef struct {
    uint32_t aa;
    uint32_t bb;
    uint32_t pp;
    uint32_t kk;
} ecu_shape_filter_t;

int osc_fpga_init(void);
int osc_fpga_exit(void);

void get_equ_shape_filter(ecu_shape_filter_t *filt, uint32_t equal,
                          uint32_t shaping, uint32_t gain);
int osc_fpga_update_params(int trig_imm, int trig_source, int trig_edge, 
                           float trig_delay, float trig_level, int time_range,
                           int equal, int shaping, int gain1, int gain2);
int osc_fpga_reset(void);
int osc_fpga_arm_trigger(void);
int osc_fpga_set_trigger(uint32_t trig_source);
int osc_fpga_set_trigger_delay(uint32_t trig_delay);

/* Returns 0 if no trigger, 1 if trigger */
int osc_fpga_triggered(void);

/* Returns pointer to the ChA and ChB signals (of length OSC_FPGA_SIG_LEN) */
int osc_fpga_get_sig_ptr(int **cha_signal, int **chb_signal);

/* Returns signal pointers from the FPGA */
int osc_fpga_get_wr_ptr(int *wr_ptr_curr, int *wr_ptr_trig);

/* Returnes signal content */
/* various constants */
extern const float c_osc_fpga_smpl_freq;
extern const float c_osc_fpga_smpl_period;

/* helper conversion functions */
/* Convert correct value for FPGA trigger source from trig_immediately, 
 * trig_source and trig_edge from application params.
 */
int osc_fpga_cnv_trig_source(int trig_imm, int trig_source, int trig_edge);
/* Converts time_range parameter (0-5) to decimation factor */
int osc_fpga_cnv_time_range_to_dec(int time_range);
/* Converts time in [s] to ADC samples (depends on decimation) */
int osc_fpga_cnv_time_to_smpls(float time, int dec_factor);
/* Converts voltage in [V] to ADC counts */
int osc_fpga_cnv_v_to_cnt(float voltage);
/* Converts ADC ounts to [V] */
float osc_fpga_cnv_cnt_to_v(int cnts); // Need for worker.c

/* Converts ADC ounts to [V] */
float osc_fpga_cnv_cnt_to_v2(int cnts);

/* Calibrate ADC count */
#ifdef Z20_250_12
int   osc_calibrate_value(int cnts,int channel,int attenuator,int mode); 
#else
//int   osc_calibrate_value(int cnts,int ); 
int   osc_calibrate_value(int cnts,int channel,int mode); 
#endif

/* Debug - dump to stderr current parameter settings (leave out data) */
void osc_fpga_dump_regs(void);

/* debugging - will be removed */
extern osc_fpga_reg_mem_t *g_osc_fpga_reg_mem;
extern                int  g_osc_fpga_mem_fd;
int __osc_fpga_cleanup_mem(void);

#endif /* __FPGA_OSC_H*/
