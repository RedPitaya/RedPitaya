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

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "common.h"
#include "la_acq.h"
#include "generate.h"
#include "la_acq.h"
#include "rpdma.h"

#define LA_ACQ_BUF_SIZE 0x4000  // TODO: just for test..

int rp_LaAcqOpen(const char *dev, rp_handle_uio_t *handle) {
    int status;

    handle->length = LA_ACQ_BASE_SIZE;
    handle->struct_size=sizeof(rp_la_acq_regset_t);
    status = common_Open (dev, handle);
    if (status != RP_OK) {
           return status;
    }

    status = rp_LaAcqReset(handle);
    if (status != RP_OK) {
        return status;
    }

    status=rp_LaAcqDefaultSettings(handle);
    if (status != RP_OK) {
        return status;
    }
/*
    status = rp_DmaOpen("/dev/rpdma", handle);
    if (status != RP_OK) {
        return status;
    }
*/
    return RP_OK;
}

int rp_LaAcqClose(rp_handle_uio_t *handle) {
    int status = common_Close (handle);
    if (status != RP_OK) {
        return status;
    }
/*
    status = rp_DmaClose(handle);
    if (status != RP_OK) {
        return status;
    }
*/
    return RP_OK;
}

int rp_LaAcqDefaultSettings(rp_handle_uio_t *handle) {
    rp_LaAcqGlobalTrigSet(handle,RP_TRG_ALL_MASK);

    rp_LaAcqSetConfig(handle, RP_LA_ACQ_CFG_AUTO_MASK);

    rp_la_cfg_regset_t cfg;
    cfg.pre=0;
    cfg.pst=LA_ACQ_BUF_SIZE;
    rp_LaAcqSetCntConfig(handle,cfg);

    rp_la_trg_regset_t trg;
    trg.cmp_msk=0;
    trg.cmp_val=0;
    trg.edg_pos=0;
    trg.edg_neg=0;
    rp_LaAcqSetTrigSettings(handle,trg);

    rp_la_decimation_regset_t dec;
    dec.dec=0;
    rp_LaAcqSetDecimation(handle,dec);

    return RP_OK;
}

/** Control registers setter & getter */
static int rp_LaAcqSetControl(rp_handle_uio_t *handle, uint32_t ctl) {
    rp_la_acq_regset_t *regset = (rp_la_acq_regset_t *) handle->regset;
    iowrite32(ctl, &regset->ctl);
    return RP_OK;
}


static int rp_LaAcqGetControl(rp_handle_uio_t *handle, uint32_t * ctl) {
    rp_la_acq_regset_t *regset = (rp_la_acq_regset_t *) handle->regset;
    *ctl = ioread32(&regset->ctl);
    return RP_OK;
}


/** Acq. control */
int rp_LaAcqReset(rp_handle_uio_t *handle) {
    return rp_LaAcqSetControl(handle,RP_CTL_RST_MASK);
}

int rp_LaAcqRunAcq(rp_handle_uio_t *handle) {
    //rp_DmaCtrl(handle, RP_DMA_SINGLE);
    return rp_LaAcqSetControl(handle,RP_CTL_STA_MASK);
}

int rp_LaAcqStopAcq(rp_handle_uio_t *handle) {
    //rp_DmaCtrl(handle, RP_DMA_STOP_RX);
    return rp_LaAcqSetControl(handle,RP_CTL_STO_MASK);
}

int rp_LaAcqTriggerAcq(rp_handle_uio_t *handle) {
    return rp_LaAcqSetControl(handle,RP_CTL_SWT_MASK);
}

int rp_LaAcqAcqIsStopped(rp_handle_uio_t *handle, bool * status){
    uint32_t ctl;
    rp_LaAcqGetControl(handle, &ctl);
    if(ctl&RP_CTL_STO_MASK){
        *status=true;
    }
    else{
        *status=false;
    }
    return RP_OK;
}

int rp_LaAcqGlobalTrigSet(rp_handle_uio_t *handle, uint32_t mask)
{
    rp_la_acq_regset_t *regset = (rp_la_acq_regset_t *) handle->regset;
    iowrite32(mask, &regset->trig_mask);
    return RP_OK;
}

int rp_LaAcqBlockingRead(rp_handle_uio_t *handle) {
    //return rp_DmaRead(handle);
    return RP_OK;
}

int rp_LaAcqSetConfig(rp_handle_uio_t *handle, uint32_t mask)
{
    rp_la_acq_regset_t *regset = (rp_la_acq_regset_t *) handle->regset;
    iowrite32(mask, &regset->cfg__aut_con);
    return RP_OK;
}

/** Configuration registers setter & getter */
int rp_LaAcqSetCntConfig(rp_handle_uio_t *handle, rp_la_cfg_regset_t a_reg) {
    rp_la_cfg_regset_t *regset = (rp_la_cfg_regset_t *) &(((rp_la_acq_regset_t*)handle->regset)->cfg);
    if(!(inrangeUint32(a_reg.pre,RP_LA_ACQ_CFG_TRIG_MIN,RP_LA_ACQ_CFG_TRIG_MAX)&&
         inrangeUint32(a_reg.pst,RP_LA_ACQ_CFG_TRIG_MIN,RP_LA_ACQ_CFG_TRIG_MAX))){
         return RP_EOOR;
    }
    iowrite32(a_reg.pre, &regset->pre);
    iowrite32(a_reg.pst, &regset->pst);
    return RP_OK;
}

int rp_LaAcqGetCntConfig(rp_handle_uio_t *handle, rp_la_cfg_regset_t * a_reg) {
    rp_la_cfg_regset_t *regset = (rp_la_cfg_regset_t *) &(((rp_la_acq_regset_t*)handle->regset)->cfg);
    a_reg->pre = ioread32(&regset->pre);
    a_reg->pst = ioread32(&regset->pst);
    return RP_OK;
}

int rp_LaAcqGetCntStatus(rp_handle_uio_t *handle, uint32_t * trig_addr, uint32_t * pst_length) {
    rp_la_cfg_regset_t *regset = (rp_la_cfg_regset_t *) &(((rp_la_acq_regset_t*)handle->regset)->sts);
    rp_la_cfg_regset_t reg;
    reg.pre = ioread32(&regset->pre);
    reg.pst = ioread32(&regset->pst);
    *trig_addr=(reg.pre % LA_ACQ_BUF_SIZE);
    *pst_length=reg.pst;
    return RP_OK;
}

/** Trigger settings setter & getter */
int rp_LaAcqSetTrigSettings(rp_handle_uio_t *handle, rp_la_trg_regset_t a_reg) {
    rp_la_trg_regset_t *regset = (rp_la_trg_regset_t *) &(((rp_la_acq_regset_t*)handle->regset)->trg);
    iowrite32(a_reg.cmp_msk, &regset->cmp_msk);
    iowrite32(a_reg.cmp_val, &regset->cmp_val);
    iowrite32(a_reg.edg_pos, &regset->edg_pos);
    iowrite32(a_reg.edg_neg, &regset->edg_neg);
    return RP_OK;
}

int rp_LaAcqGetTrigSettings(rp_handle_uio_t *handle, rp_la_trg_regset_t * a_reg) {
    rp_la_trg_regset_t *regset = (rp_la_trg_regset_t *) &(((rp_la_acq_regset_t*)handle->regset)->trg);
    a_reg->cmp_msk = ioread32(&regset->cmp_msk);
    a_reg->cmp_val = ioread32(&regset->cmp_val);
    a_reg->edg_pos = ioread32(&regset->edg_pos);
    a_reg->edg_neg = ioread32(&regset->edg_neg);
    return RP_OK;
}

/** Decimation settings setter & getter */
int rp_LaAcqSetDecimation(rp_handle_uio_t *handle, rp_la_decimation_regset_t a_reg) {
    rp_la_decimation_regset_t *regset = (rp_la_decimation_regset_t *) &(((rp_la_acq_regset_t*)handle->regset)->dec);
    iowrite32(a_reg.dec, &regset->dec);
    return RP_OK;
}

int rp_LaAcqGetDecimation(rp_handle_uio_t *handle, rp_la_decimation_regset_t * a_reg) {
    rp_la_decimation_regset_t *regset = (rp_la_decimation_regset_t *) &(((rp_la_acq_regset_t*)handle->regset)->dec);
    a_reg->dec = ioread32(&regset->dec);
    return RP_OK;
}

/** Data buffer pointers */
/*
int rp_LaAcqGetDataPointers(rp_handle_uio_t *handle, rp_data_ptrs_regset_t * a_reg) {
    rp_data_ptrs_regset_t *regset = (rp_data_ptrs_regset_t *) &(((rp_la_acq_regset_t*)handle->regset)->dpt);
    a_reg->start = ioread32(&regset->start);
    a_reg->trig = ioread32(&regset->trig);
    a_reg->stopped = ioread32(&regset->stopped);
    return RP_OK;
}
*/

int rp_LaAcqFpgaRegDump(rp_handle_uio_t *handle)
{
    int r;
    r=FpgaRegDump(0,(uint32_t*)handle->regset,6);
    rp_la_trg_regset_t *regset = (rp_la_trg_regset_t *) &(((rp_la_acq_regset_t*)handle->regset)->trg);
    FpgaRegDump(0,(uint32_t*)&regset->cmp_msk,5);
    return r;
}
