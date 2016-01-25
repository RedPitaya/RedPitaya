/**
 * $Id: $
 *
 * @brief Red Pitaya library Logic analyzer acquisition module interface
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */
#ifndef __LA_ACQ_H
#define __LA_ACQ_H

// For more information see: /doc/logic/README.md

/** trigger settings  */
struct {
    uint32_t cmp_msk; ///<  digital comparator mask
    uint32_t cmp_val; ///<  digital comparator value
    uint32_t edg_pos; ///<  digital edge positive mask
    uint32_t edg_neg; ///<  digital edge negative mask
} regset_la_trg_t;

/** decimation  */
typedef struct {
    uint32_t avg;  ///< (not used by logic analyzer!)
    uint32_t dec;  ///<  sample rate = 125Msps/(dec+1)
    uint32_t shr;  ///< (not used by logic analyzer!)
} rp_la_decimation_regset_t;

/** ADC filter (not used by logic analyzer!) */
typedef struct {
    uint32_t byp;  ///<
    int32_t  aa;   ///<
    int32_t  bb;   ///<
    int32_t  kk;   ///<
    int32_t  pp;   ///<
} rp_adc_eqfilter_regset_t;

/** control register masks  */
#define RP_LA_ACQ_CTL_RST_MASK (1<<0) ///< 1 - reset acq. state machine (TODO: check what exactly it does)
#define RP_LA_ACQ_CTL_STA_MASK (1<<1) ///< 1 - starts acq.; 0 - when acq. is stopped
#define RP_LA_ACQ_CTL_STO_MASK (1<<2) ///< 1 - stops acq. (also used to about the acq.)
#define RP_LA_ACQ_CTL_SWT_MASK (1<<3) ///< 1 - triggers acq. (sw trigger must be enabled)

/** configuration register  */
#define RP_LA_ACQ_CFG_CONT_MASK (1<<0) ///< if set acq. will work in continuous mode and will not stop after post trig.
#define RP_LA_ACQ_CFG_AUTO_MASK (1<<1) ///< if set acq. will be triggered when started (only post trig. acq.)

/** pre & post trigger mask  */
#define RP_LA_ACQ_TRIG_MAX ((1<<26)-1) ///< max. number of samples for pre & post trigger

/** TODO: trigger enable mask  */
#define RP_LA_TRI_EN_SWT_MASK (1<<0) ///< software trigger mask
#define RP_LA_TRI_EN_EXT_MASK (1<<1) ///< external trigger mask
#define RP_LA_TRI_EN_LGA_MASK (1<<2) ///< logic analyzer trigger mask
#define RP_LA_TRI_EN_LGA_MASK (1<<3) ///< oscilloscope trigger mask

/** logic analyzer acquire structure declaration */
typedef struct {
    uint32_t ctl; 		///< control register
    uint32_t cfg; 		///< configuration register
    uint32_t cfg_trg; 	///< trig. enable mask, triggers are ORed
    uint32_t cfg_pre; 	///< pre-trigger [number of samples] ; 0 - means that no sample will be taken before trigger
	uint32_t cfg_pst; 	///< post-trigger [number of samples] ; 0 - means that no sample will be taken after trigger
    regset_la_trg_t    trg;
    rp_scope_decimation_regset_t dec;
    rp_adc_eqfilter_regset_t     fil; // (TODO: this filter will be removed for LA)
    // TODO: data pointer are missing (start, trigger, stopped)
} acq_regset_t;

// Expected procedure:
// set trig.
// start acg.
// irq - > when it stops
// stop (abort acq.)

#endif // __LA_ACQ_H
