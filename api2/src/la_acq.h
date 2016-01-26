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

// For more information see: /doc/logic/README.md & la_top.sv

#ifndef __LA_ACQ_H
#define __LA_ACQ_H

#include <stdint.h>
#include <stdbool.h>

#include "rp2.h"

/** control register masks  */
#define RP_LA_ACQ_CTL_RST_MASK (1<<0) ///< 1 - reset acq. state machine (TODO: check what exactly it does)
#define RP_LA_ACQ_CTL_STA_MASK (1<<1) ///< 1 - starts acq.; 0 - when acq. is stopped
#define RP_LA_ACQ_CTL_STO_MASK (1<<2) ///< 1 - stops acq. (also used to about the acq.)
#define RP_LA_ACQ_CTL_SWT_MASK (1<<3) ///< 1 - triggers acq. (sw trigger must be enabled)


/** configuration register  */
#define RP_LA_ACQ_CFG_CONT_MASK (1<<0) ///< if set acq. will work in continuous mode and will not stop after post trig.
#define RP_LA_ACQ_CFG_AUTO_MASK (1<<1) ///< if set acq. will be triggered when started (only post trig. acq.)

/** pre & post trigger mask  */
#define RP_LA_ACQ_CFG_TRIG_MAX ((1<<26)-1) ///< max. number of samples for pre & post trigger

/** TODO: trigger enable mask  */
#define RP_LA_TRI_EN_SWT_MASK (1<<0) ///< software trigger mask
#define RP_LA_TRI_EN_EXT_MASK (1<<1) ///< external trigger mask
#define RP_LA_TRI_EN_LGA_MASK (1<<2) ///< logic analyzer trigger mask
#define RP_LA_TRI_EN_OSC_MASK (1<<3) ///< oscilloscope trigger mask


/** Control registers */
typedef struct {
    uint32_t ctl;
} rp_la_ctl_regset_t;


/** Configuration registers */
typedef struct {
    uint32_t acq;     ///< acq. configuration register
    uint32_t trg;     ///< trig. enable mask, triggers are ORed
    uint32_t pre;     ///< pre-trigger [number of samples] ; 0 - means that no sample will be taken before trigger
    uint32_t pst;     ///< post-trigger [number of samples] ; 0 - means that no sample will be taken after trigger
} rp_la_cfg_regset_t;

/** Trigger settings  */
typedef struct {
    uint32_t cmp_msk; ///<  digital comparator mask
    uint32_t cmp_val; ///<  digital comparator value
    uint32_t edg_pos; ///<  digital edge positive mask
    uint32_t edg_neg; ///<  digital edge negative mask
} rp_la_trg_regset_t;

/** Decimation  */
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

/** Data buf. pointers  */
typedef struct {
    uint32_t start;      ///< position in the buffer where acq. was started
    uint32_t trig;       ///< position in the buffer where trigger appeared
    uint32_t stopped;    ///< position in the buffer where acq. was stopped
} rp_data_ptrs_regset_t;

/** logic analyzer acquire structure declaration */
typedef struct {
    rp_la_ctl_regset_t ctl;         ///< control register
    rp_la_cfg_regset_t cfg;         ///< configuration registers
    rp_la_trg_regset_t trg;         ///< trigger settings register
    rp_la_decimation_regset_t dec;  ///< decimation
    rp_adc_eqfilter_regset_t fil;   ///< (TODO: this filter will be removed for LA)
    rp_data_ptrs_regset_t dpt;      ///< data buf. pointers
} rp_la_acq_regset_t;


int rp_LaGenOpen(const char *a_dev, rp_handle_uio_t *handle);
int rp_LaGenClose(rp_handle_uio_t *handle);
int rp_LaGenReset(rp_handle_uio_t *handle);
int rp_LaGenRunAcq(rp_handle_uio_t *handle);
int rp_LaGenStopAcq(rp_handle_uio_t *handle);
int rp_LaGenTriggerAcq(rp_handle_uio_t *handle);
int rp_LaGenAcqIsStopped(rp_handle_uio_t *handle, bool * status);
int rp_LaGenSetConfig(rp_handle_uio_t *handle, rp_la_cfg_regset_t a_reg);
int rp_LaGenGetConfig(rp_handle_uio_t *handle, rp_la_cfg_regset_t * a_reg);
int rp_LaGenSetTrigSettings(rp_handle_uio_t *handle, rp_la_trg_regset_t a_reg);
int rp_LaGenGetTrigSettings(rp_handle_uio_t *handle, rp_la_trg_regset_t * a_reg);
int rp_LaGenSetDecimation(rp_handle_uio_t *handle, rp_la_decimation_regset_t a_reg);
int rp_LaGenGetDecimation(rp_handle_uio_t *handle, rp_la_decimation_regset_t * a_reg);
int rp_LaGenGetDataPointers(rp_handle_uio_t *handle, rp_data_ptrs_regset_t * a_reg);

#endif // __LA_ACQ_H
