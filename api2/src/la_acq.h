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

#include "common.h"

#define LA_ACQ_BASE_SIZE 0x00010000

/** configuration register  */
#define RP_LA_ACQ_CFG_CONT_MASK (1<<0) ///< if set acq. will work in continuous mode and will not stop after post trig.
#define RP_LA_ACQ_CFG_AUTO_MASK (1<<1) ///< if set acq. will be triggered when started (only post trig. acq.)

/** pre & post trigger mask  */
#define RP_LA_ACQ_CFG_TRIG_MAX ((1<<26)-1) ///< max. number of samples for pre & post trigger
#define RP_LA_ACQ_CFG_TRIG_MIN 0

/** Configuration registers */
typedef struct {
    uint32_t acq;     ///< acq. configuration register
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

/** Data buf. pointers  */
/*
typedef struct {
    uint32_t start;      ///< position in the buffer where acq. was started
    uint32_t trig;       ///< position in the buffer where trigger appeared
    uint32_t stopped;    ///< position in the buffer where acq. was stopped
} rp_data_ptrs_regset_t;
*/

/** logic analyzer acquire structure declaration */
typedef struct {
    uint32_t ctl;                   ///< control register
    uint32_t trig_mask;             ///< global trigger registers
    rp_la_cfg_regset_t cfg;         ///< configuration registers
    rp_la_trg_regset_t trg;         ///< trigger settings register
    rp_la_decimation_regset_t dec;  ///< decimation
    //rp_data_ptrs_regset_t dpt;    ///< data buf. pointers
} rp_la_acq_regset_t;


int rp_LaAcqOpen(const char *dev, rp_handle_uio_t *handle);
int rp_LaAcqClose(rp_handle_uio_t *handle);
int rp_LaAcqDefaultSettings(rp_handle_uio_t *handle);

int rp_LaAcqReset(rp_handle_uio_t *handle);
int rp_LaAcqRunAcq(rp_handle_uio_t *handle);
int rp_LaAcqStopAcq(rp_handle_uio_t *handle);
int rp_LaAcqTriggerAcq(rp_handle_uio_t *handle);
int rp_LaAcqAcqIsStopped(rp_handle_uio_t *handle, bool * status);
int rp_LaAcqGlobalTrigSet(rp_handle_uio_t *handle, uint32_t mask);

int rp_LaAcqSetConfig(rp_handle_uio_t *handle, rp_la_cfg_regset_t a_reg);
int rp_LaAcqGetConfig(rp_handle_uio_t *handle, rp_la_cfg_regset_t * a_reg);
int rp_LaAcqSetTrigSettings(rp_handle_uio_t *handle, rp_la_trg_regset_t a_reg);
int rp_LaAcqGetTrigSettings(rp_handle_uio_t *handle, rp_la_trg_regset_t * a_reg);
int rp_LaAcqSetDecimation(rp_handle_uio_t *handle, rp_la_decimation_regset_t a_reg);
int rp_LaAcqGetDecimation(rp_handle_uio_t *handle, rp_la_decimation_regset_t * a_reg);
//int rp_LaAcqGetDataPointers(rp_handle_uio_t *handle, rp_data_ptrs_regset_t * a_reg);
int rp_LaAcqFpgaRegDump(rp_handle_uio_t *handle);

#endif // __LA_ACQ_H
