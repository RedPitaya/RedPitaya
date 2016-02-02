/**
 * $Id: $
 *
 * @brief Red Pitaya library Generate module interface
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
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <math.h>
#include "common.h"
#include "generate.h"

const double c_max_waveform_sample_rate_freq=125e6;
const double c_min_waveform_sample_rate_freq=0;

int rp_GenOpen(const char  *dev, rp_handle_uio_t *handle) {
    int status;

    handle->length = GENERATE_BASE_SIZE;
    handle->struct_size=sizeof(gen_regset_t);
    status = common_Open (dev, handle);
    if (status != RP_OK) {
           return status;
    }

    status = rp_GenReset(handle);
    if (status != RP_OK) {
        return status;
    }

    status=rp_GenDefaultSettings(handle);
    if (status != RP_OK) {
        return status;
    }
    return RP_OK;
}

int rp_GenClose(rp_handle_uio_t *handle) {

    int status = common_Close (handle);
    if (status != RP_OK) {
        return status;
    }
    return RP_OK;
}

int rp_GenDefaultSettings(rp_handle_uio_t *handle) {
    rp_GenGlobalTrigSet(handle,RP_TRG_ALL_MASK);
    rp_GenSetFreqPhase(handle, 0, 0); // TODO: not used
    rp_GenSetMode(handle, RP_GEN_CFG_BURST_MASK);
    rp_GenSetBurstModeRepetitions(handle, 0);
    rp_GenSetBurstModeDataLen(handle, 1);
    rp_GenSetBurstModePeriodLen(handle, 1);
    rp_GenOutputDisable(handle,RP_GEN_OUT_ALL_MASK);
    return RP_OK;
}

/** Control registers setter & getter */
static int rp_GenSetControl(rp_handle_uio_t *handle, rp_ctl_regset_t a_reg) {
    asg_regset_t *regset = (asg_regset_t *) &(((gen_regset_t *) handle->regset)->asg);
    iowrite32(a_reg.ctl, &regset->ctl);
    return RP_OK;
}


static int rp_GenGetControl(rp_handle_uio_t *handle, rp_ctl_regset_t * a_reg) {
    asg_regset_t *regset = (asg_regset_t *) &(((gen_regset_t *) handle->regset)->asg);
    a_reg->ctl = ioread32(&regset->ctl);
    return RP_OK;
}


/** Control */
int rp_GenReset(rp_handle_uio_t *handle) {
    rp_ctl_regset_t reg;
    reg.ctl=RP_CTL_RST_MASK;
    return rp_GenSetControl(handle,reg);
}

int rp_GenRun(rp_handle_uio_t *handle) {
    rp_ctl_regset_t reg;
    reg.ctl=RP_CTL_STA_MASK;
    return rp_GenSetControl(handle,reg);
}

int rp_GenStop(rp_handle_uio_t *handle) {
    rp_ctl_regset_t reg;
    reg.ctl=RP_CTL_STO_MASK;
    return rp_GenSetControl(handle,reg);
}

int rp_GenTrigger(rp_handle_uio_t *handle) {
    rp_ctl_regset_t reg;
    reg.ctl=RP_CTL_SWT_MASK;
    return rp_GenSetControl(handle,reg);
}

int rp_GenIsStopped(rp_handle_uio_t *handle, bool * status){
    rp_ctl_regset_t reg;
    rp_GenGetControl(handle, &reg);
    if(reg.ctl&RP_CTL_STA_MASK){
        *status=false;
    }
    else{
        *status=true;
    }
    return RP_OK;
}

int rp_GenDefaultSettings(rp_handle_uio_t *handle) {
    //asg_regset_t *regset = (asg_regset_t *) &(((gen_regset_t *) handle->regset)->asg);
    // reset generator state machine
    //iowrite32(0x1, &regset->ctl_sys);
//    gen_setGenMode(channel, RP_GEN_MODE_CONTINUOUS);
    // clean up table (TODO, should I really)
   /*
    uint32_t size = 1 << RP_GEN_CWM;
    uint16_t waveform [size];
    for (unsigned int i=0; i<size; i++) {
        waveform[i] = 0;
    }
    rp_GenSetWaveform    (handle, waveform, size);
    rp_GenSetFreqPhase   (handle, 1000.0, 0.0);
    rp_DigGenGlobalTrigDisable (handle, RP_TRG_ALL);
    rp_GenSetBurst       (handle, 0, 0, 0);
    rp_GenSetLinear      (handle, 1.0, 0.0);
    */

    rp_GenGlobalTrigDisable(handle,RP_TRG_ALL_MASK);
    rp_GenOutputDisable(handle,RP_GEN_OUT_EN_ALL_MASK);
    return RP_OK;
}

int rp_GenGlobalTrigSet(rp_handle_uio_t *handle, uint32_t a_mask)
{
    asg_regset_t *regset = (asg_regset_t *) &(((gen_regset_t *) handle->regset)->asg);
    iowrite32(a_mask, &regset->trig_mask);
    return RP_OK;
}

// TODO: will be used for analog generator
int rp_GenSetLinear(rp_handle_uio_t *handle, float amplitude, float offset) {
    ag_regset_t *regset = (ag_regset_t *) &(((gen_regset_t *) handle->regset)->gen_spec.ag_spec);
    int32_t mul = (int32_t) (amplitude * (1 << RP_GEN_DWM));
    int32_t sum = (int32_t) (offset    * (1 << RP_GEN_DWS));
    iowrite32(mul, &regset->mul);
    iowrite32(sum, &regset->sum);
    return RP_OK;
}

int rp_GenGetLinear(rp_handle_uio_t *handle, float *amplitude, float *offset) {
    ag_regset_t *regset = (ag_regset_t *) &(((gen_regset_t *) handle->regset)->gen_spec.ag_spec);
    int32_t mul = ioread32(&regset->mul);
    int32_t sum = ioread32(&regset->sum);
    *amplitude = ((float) mul) / (1 << RP_GEN_DWM);
    *offset    = ((float) sum) / (1 << RP_GEN_DWS);
    return RP_OK;
}

// 

int rp_GenSetStepOffset(rp_handle_uio_t *handle, uint32_t stp, uint32_t off) {
    asg_regset_t *regset = (asg_regset_t *) &(((gen_regset_t *) handle->regset)->asg);
    if (stp >= (1 << (RP_GEN_CWM+RP_GEN_CWF))) {
        return RP_EOOR;
    }
    if (off >= (1 << (RP_GEN_CWM+RP_GEN_CWF))) {
        return RP_EOOR;
    }
    iowrite32(stp, &regset->cfg_stp);
    iowrite32(off, &regset->cfg_off);
    return RP_OK;
}

int rp_GenGetStepOffset(rp_handle_uio_t *handle, uint32_t *stp, uint32_t *off) {
    asg_regset_t *regset = (asg_regset_t *) &(((gen_regset_t *) handle->regset)->asg);
    *stp = ioread32(&regset->cfg_stp);
    *off = ioread32(&regset->cfg_off);
    return RP_OK;
}

int rp_GenSetFreqPhase(rp_handle_uio_t *handle, double frequency, double phase) {
    asg_regset_t *regset = (asg_regset_t *) &(((gen_regset_t *) handle->regset)->asg);
    if(!inrangeDouble(frequency,FREQUENCY_MIN,FREQUENCY_MAX)){
         return RP_EOOR;
    }
    uint32_t siz = ioread32(&regset->cfg_siz) + 1;
    uint32_t stp = (uint32_t) ((double) siz * (frequency / RP_GEN_SR)) - 1;
    uint32_t off = (uint32_t) ((double) siz * fmod(phase,360)/360    )    ;
    return rp_GenSetStepOffset(handle, stp, off);
}

int rp_GenGetFreqPhase(rp_handle_uio_t *handle, double *frequency, double *phase) {
    asg_regset_t *regset = (asg_regset_t *) &(((gen_regset_t *) handle->regset)->asg);
    uint32_t siz = ioread32(&regset->cfg_siz) + 1;
    uint32_t stp, off;
    int status = rp_GenGetStepOffset(handle, &stp, &off);
    if (status != RP_OK) {
        return status;
    }
    *frequency = (double) (stp + 1) / (double) siz * RP_GEN_SR;
    *phase     = (double) (off    ) / (double) siz * 360      ;
    return RP_OK;
}

int rp_GenSetWaveform(rp_handle_uio_t *handle, uint16_t *waveform, uint32_t size) {
    asg_regset_t *regset = (asg_regset_t *) &(((gen_regset_t *) handle->regset)->asg);
    if(!inrangeUint32(size,1,RP_GEN_SIG_SAMPLES)){
         return RP_EOOR;
    }
    for (uint32_t i=0; i<size; i++) {
        iowrite32(waveform[i], &regset->table[i]);
    }
    iowrite32((size << RP_GEN_CWF) - 1, &regset->cfg_siz);
    return RP_OK;
}

int rp_GenGetWaveform(rp_handle_uio_t *handle, uint16_t *waveform, uint32_t *size) {
    asg_regset_t *regset = (asg_regset_t *) &(((gen_regset_t *) handle->regset)->asg);
    *size = (ioread32(&regset->cfg_siz) + 1) >> RP_GEN_CWF;
    for (uint32_t i=0; i<*size; i++) {
        waveform[i] = ioread32(&regset->table[i]);
    }
    return RP_OK;
}

int rp_GenSetWaveformUpCountSeq(rp_handle_uio_t *handle, uint32_t size) {
    uint16_t ramp[size];
    for (uint32_t i=0; i<size; i++) {
        ramp[i] = i;
    }
    rp_GenSetWaveform(handle, ramp, size);
    return RP_OK;
}

//

static int rp_GenSetBst(rp_handle_uio_t *handle, uint32_t a_mask) {
    asg_regset_t *regset = (asg_regset_t *) handle->regset;
    uint32_t tmp;
    tmp=ioread32(&regset->cfg_bst);
    tmp|=a_mask;
    iowrite32(tmp, &regset->cfg_bst);
    return RP_OK;
}

static int rp_GenClearBst(rp_handle_uio_t *handle, uint32_t a_mask) {
    asg_regset_t *regset = (asg_regset_t *) handle->regset;
    uint32_t tmp;
    tmp=ioread32(&regset->cfg_bst);
    tmp&=~a_mask;
    iowrite32(tmp, &regset->cfg_bst);
    return RP_OK;
}

int rp_GenSetMode(rp_handle_uio_t *handle, RP_GEN_MODE mode)
{
    switch(mode){
        case RP_GEN_MODE_CONTINUOUS:
            return rp_GenClearBst(handle, RP_GEN_CFG_BURST_MASK);
        break;
        case RP_GEN_MODE_BURST:
            return rp_GenSetBst(handle, RP_GEN_CFG_BURST_MASK);
        break;
        default:
            return -1;
    }
    return RP_OK;
}
/*
int rp_GenGetMode(rp_handle_uio_t *handle, RP_GEN_MODE * mode)
{
    asg_regset_t *regset = (asg_regset_t *) handle->regset;
    uint32_t tmp;
    tmp=ioread32(&regset->cfg_bst);
    if(tmp&RP_GEN_MODE_BURST){
        *mode=RP_GEN_MODE_BURST;
    }
    else{
        *mode=RP_GEN_MODE_CONTINUOUS;
    }
    return RP_OK;
}
*/

int rp_GenSetBurstModeRepetitions(rp_handle_uio_t *handle, uint32_t val)
{
    asg_regset_t *regset = (asg_regset_t *) &(((gen_regset_t *) handle->regset)->asg);
    if(!inrangeUint32(val,RP_GEN_REP_INF,BURST_REPETITIONS_MAX)){
         return RP_EOOR;
    }
    if(val==RP_GEN_REP_INF){
        iowrite32(0, &regset->cfg_bnm);
        return rp_GenSetBst(handle, RP_GEN_CFG_BURST_INF_MASK);
    }
    else{
        iowrite32(val, &regset->cfg_bnm);
        return rp_GenClearBst(handle, RP_GEN_CFG_BURST_INF_MASK);
    }
    return RP_OK;
}

int rp_GenSetBurstModeDataLen(rp_handle_uio_t *handle, uint32_t length)
{

    asg_regset_t *regset = (asg_regset_t *) &(((gen_regset_t *) handle->regset)->asg);
    if(!inrangeUint32(length,1,RP_GEN_SIG_SAMPLES)){
         return RP_EOOR;
    }
    iowrite32((length-1), &regset->cfg_bdl);
    return RP_OK;
}

int rp_GenSetBurstModePeriodLen(rp_handle_uio_t *handle, uint32_t length)
{
    asg_regset_t *regset = (asg_regset_t *) &(((gen_regset_t *) handle->regset)->asg);
    if(!inrangeUint32(length,1,BURST_PERIOD_LEN_MAX)){
         return RP_EOOR;
    }
    iowrite32((length-1), &regset->cfg_bil);
    return RP_OK;
}

int rp_GenOutputEnable(rp_handle_uio_t *handle, uint32_t a_mask)
{
    lg_spec_regset_t *regset = (lg_spec_regset_t *) &(((gen_regset_t*)handle->regset)->gen_spec.lg_spec);
    uint32_t tmp;
    tmp=ioread32(&regset->dig_out_en);
    tmp|=a_mask;
    iowrite32(tmp, &regset->dig_out_en);
    return RP_OK;
}

int rp_GenOutputDisable(rp_handle_uio_t *handle, uint32_t a_mask)
{
    lg_spec_regset_t *regset = (lg_spec_regset_t *) &(((gen_regset_t*)handle->regset)->gen_spec.lg_spec);
    uint32_t tmp;
    tmp=ioread32(&regset->dig_out_en);
    tmp&=~a_mask;
    iowrite32(tmp, &regset->dig_out_en);
    return RP_OK;
}

int rp_GenSetWaveformSampleRate(rp_handle_uio_t *handle, double * sample_rate)
{
    if(!inrangeDouble(*sample_rate,c_min_waveform_sample_rate_freq,c_max_waveform_sample_rate_freq)){
        return RP_EOOR;
    }
    uint32_t reg_val=(*sample_rate/(double)RP_GEN_SR)*(double)(1<<RP_GEN_CWF);
    *sample_rate=((double)reg_val/(double)(1<<RP_GEN_CWF))*c_max_waveform_sample_rate_freq;
    asg_regset_t *regset = (asg_regset_t *) &(((gen_regset_t*)handle->regset)->asg);
    iowrite32(reg_val, &regset->cfg_stp);
    return RP_OK;
}

int rp_GenFpgaRegDump(rp_handle_uio_t *handle, uint32_t data_len)
{
    int r;
    r=FpgaRegDump(0,(uint32_t*)handle->regset,13);
    if((data_len)||(data_len<=RP_GEN_SIG_SAMPLES)){
        if(!inrangeUint32(data_len,1,RP_GEN_SIG_SAMPLES)){
            return RP_EOOR;
        }
        asg_regset_t *regset = (asg_regset_t *)&(((gen_regset_t*)handle->regset)->asg);
        r=FpgaRegDump(0,(uint32_t*)regset->table,data_len);
    }
    return r;
}

