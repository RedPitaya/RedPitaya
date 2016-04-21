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
    handle->struct_size=sizeof(asg_regset_t);
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
    //rp_GenSetFreqPhase(handle, 0, 0); // TODO: not used
    rp_GenSetMode(handle, RP_GEN_MODE_BURST);
    rp_GenSetBurstModeRepetitions(handle, 0);
    rp_GenSetBurstModeDataLen(handle, 1);
    rp_GenSetBurstModePeriodLen(handle, 1);
    rp_GenOutputDisable(handle,RP_GEN_OUT_ALL_MASK);
    return RP_OK;
}

/** Control registers setter & getter */
static int rp_GenSetControl(rp_handle_uio_t *handle, uint32_t ctl) {
    asg_regset_t *regset = (asg_regset_t *) handle->regset;
    iowrite32(ctl, &regset->ctl);
    return RP_OK;
}


static int rp_GenGetControl(rp_handle_uio_t *handle, uint32_t * ctl) {
    asg_regset_t *regset = (asg_regset_t *) handle->regset;
    *ctl = ioread32(&regset->ctl);
    return RP_OK;
}


/** Control */
int rp_GenReset(rp_handle_uio_t *handle) {

    return rp_GenSetControl(handle,RP_CTL_RST_MASK);
}

int rp_GenRun(rp_handle_uio_t *handle) {
    return rp_GenSetControl(handle,RP_CTL_STA_MASK);
}

int rp_GenStop(rp_handle_uio_t *handle) {
    return rp_GenSetControl(handle,RP_CTL_STO_MASK);
}

int rp_GenTrigger(rp_handle_uio_t *handle) {
    return rp_GenSetControl(handle,RP_CTL_SWT_MASK);
}

int rp_GenIsStopped(rp_handle_uio_t *handle, bool * status){
    uint32_t ctl;
    rp_GenGetControl(handle, &ctl);
    if(ctl&RP_CTL_STO_MASK){
        *status=true;
    }
    else{
        *status=false;
    }
    return RP_OK;
}

int rp_GenGlobalTrigSet(rp_handle_uio_t *handle, uint32_t mask)
{
    asg_regset_t *regset = (asg_regset_t *) handle->regset;
    iowrite32(mask, &regset->trig_mask);
    return RP_OK;
}

// TODO: will be used for analog generator
int rp_GenSetLinear(rp_handle_uio_t *handle, float amplitude, float offset) {
    asg_regset_t *regset = (asg_regset_t *) handle->regset;
    int32_t mul = (int32_t) (amplitude * (1 << RP_GEN_DWM));
    int32_t sum = (int32_t) (offset    * (1 << RP_GEN_DWS));
    iowrite32(mul, &regset->gen_spec.ag_spec.mul);
    iowrite32(sum, &regset->gen_spec.ag_spec.sum);
    return RP_OK;
}

int rp_GenGetLinear(rp_handle_uio_t *handle, float *amplitude, float *offset) {
    asg_regset_t *regset = (asg_regset_t *) handle->regset;
    int32_t mul = ioread32(&regset->gen_spec.ag_spec.mul);
    int32_t sum = ioread32(&regset->gen_spec.ag_spec.sum);
    *amplitude = ((float) mul) / (1 << RP_GEN_DWM);
    *offset    = ((float) sum) / (1 << RP_GEN_DWS);
    return RP_OK;
}

// 

int rp_GenSetStepOffset(rp_handle_uio_t *handle, uint32_t stp, uint32_t off) {
    asg_regset_t *regset = (asg_regset_t *) handle->regset;
    if (stp >= (1 << (RP_GEN_CWM+RP_GEN_CWF))) {
        return RP_EOOR;
    }
    if (off >= (1 << (RP_GEN_CWM+RP_GEN_CWF))) {
        return RP_EOOR;
    }
    iowrite32(stp-1, &regset->cfg_stp);
    iowrite32(off  , &regset->cfg_off);
    return RP_OK;
}

int rp_GenGetStepOffset(rp_handle_uio_t *handle, uint32_t *stp, uint32_t *off) {
    asg_regset_t *regset = (asg_regset_t *) handle->regset;
    *stp = ioread32(&regset->cfg_stp);
    *off = ioread32(&regset->cfg_off);
    return RP_OK;
}

int rp_GenSetFreqPhase(rp_handle_uio_t *handle, double frequency, double phase) {
    if(!inrangeDouble(frequency,FREQUENCY_MIN,FREQUENCY_MAX)){
         return RP_EOOR;
    }
    asg_regset_t *regset = (asg_regset_t *) handle->regset;
    uint32_t siz = ioread32(&regset->cfg_siz) + 1;
    uint32_t stp = (uint32_t) ((double) siz * (frequency / RP_GEN_SR)) - 1;
    uint32_t off = (uint32_t) ((double) siz * fmod(phase,360)/360    )    ;
    return rp_GenSetStepOffset(handle, stp, off);
}

int rp_GenGetFreqPhase(rp_handle_uio_t *handle, double *frequency, double *phase) {
    asg_regset_t *regset = (asg_regset_t *) handle->regset;
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
    asg_regset_t *regset = (asg_regset_t *) handle->regset;
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
    asg_regset_t *regset = (asg_regset_t *) handle->regset;
    *size = (ioread32(&regset->cfg_siz) + 1) >> RP_GEN_CWF;
    for (uint32_t i=0; i<*size; i++) {
        waveform[i] = ioread32(&regset->table[i]);
    }
    return RP_OK;
}

int rp_GenSetWaveformUpCountSeq(rp_handle_uio_t *handle, uint32_t size) {
    uint16_t ramp[RP_GEN_SIG_SAMPLES];
    for (uint32_t i=0; i<size; i++) {
        ramp[i] = i;
    }
    //ramp[size-1]=0; // to remain low level
    rp_GenSetWaveform(handle, ramp, size);
    return RP_OK;
}

//
static int rp_GenSetBst(rp_handle_uio_t *handle, uint32_t a_mask) {
    asg_regset_t *regset = (asg_regset_t *) handle->regset;
    uint32_t tmp;
    tmp = ioread32(&regset->cfg_bst);
    tmp |= a_mask;
    iowrite32(tmp, &regset->cfg_bst);
    return RP_OK;
}

static int rp_GenClearBst(rp_handle_uio_t *handle, uint32_t a_mask) {
    asg_regset_t *regset = (asg_regset_t *) handle->regset;
    uint32_t tmp;
    tmp = ioread32(&regset->cfg_bst);
    tmp &= ~a_mask;
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
    asg_regset_t *regset = (asg_regset_t *) handle->regset;
    if(val==RP_GEN_REP_INF){
        iowrite32(2, &regset->cfg_bnm);
        return rp_GenSetBst(handle, RP_GEN_CFG_BURST_INF_MASK);
    }
    else if(inrangeUint32(val,BURST_REPETITIONS_MIN,BURST_REPETITIONS_MAX)){
        iowrite32((val-1), &regset->cfg_bnm);
        return rp_GenClearBst(handle, RP_GEN_CFG_BURST_INF_MASK);
    }
    else{
        return RP_EOOR;
    }
    return RP_OK;
}

int rp_GenSetBurstModeDataLen(rp_handle_uio_t *handle, uint32_t length)
{
    asg_regset_t *regset = (asg_regset_t *) handle->regset;
    if(!inrangeUint32(length,1,RP_GEN_SIG_SAMPLES)){
         return RP_EOOR;
    }
    iowrite32((length-1), &regset->cfg_bdl);
    return RP_OK;
}

int rp_GenSetBurstModePeriodLen(rp_handle_uio_t *handle, uint32_t length)
{
    asg_regset_t *regset = (asg_regset_t *) handle->regset;
    if(!inrangeUint32(length,BURST_PERIOD_LEN_MIN,BURST_PERIOD_LEN_MAX)){
         return RP_EOOR;
    }
    iowrite32((length-1), &regset->cfg_bln);
    return RP_OK;
}

int rp_GenOutputEnable(rp_handle_uio_t *handle, uint32_t a_mask)
{
    asg_regset_t *regset = (asg_regset_t *) handle->regset;
    uint32_t tmp;
    tmp = ioread32(&regset->gen_spec.lg_spec.dig_out_en);
    tmp |= a_mask;
    iowrite32(tmp, &regset->gen_spec.lg_spec.dig_out_en);
    iowrite32(0  , &regset->gen_spec.lg_spec.dig_openc );
    return RP_OK;
}

int rp_GenOutputDisable(rp_handle_uio_t *handle, uint32_t a_mask)
{
    asg_regset_t *regset = (asg_regset_t *) handle->regset;
    uint32_t tmp;
    tmp = ioread32(&regset->gen_spec.lg_spec.dig_out_en);
    tmp &= ~a_mask;
    iowrite32(tmp, &regset->gen_spec.lg_spec.dig_out_en);
    iowrite32(0  , &regset->gen_spec.lg_spec.dig_openc );
    return RP_OK;
}

int rp_GenSetWaveformSampleRate(rp_handle_uio_t *handle, double * sample_rate)
{
    if(!inrangeDouble(*sample_rate,c_min_waveform_sample_rate_freq,c_max_waveform_sample_rate_freq)){
        return RP_EOOR;
    }
    uint32_t reg_val=(*sample_rate/(double)RP_GEN_SR)*(double)(1<<RP_GEN_CWF);
    *sample_rate=((double)reg_val/(double)(1<<RP_GEN_CWF))*c_max_waveform_sample_rate_freq;
    asg_regset_t *regset = (asg_regset_t *) handle->regset;
    iowrite32(reg_val, &regset->cfg_stp);
    return RP_OK;
}

int rp_GenFpgaRegDump(rp_handle_uio_t *handle, uint32_t data_len)
{
    int r;
    r=FpgaRegDump("Gen reg",0,(uint32_t*)handle->regset,16);

    {
        lg_spec_regset_t *regset = (lg_spec_regset_t *) &(((asg_regset_t*)handle->regset)->gen_spec.lg_spec);
        FpgaRegDump("Logic gen reg",0,(uint32_t*)regset,2);
    }


    if(!inrangeUint32(data_len,1,RP_GEN_SIG_SAMPLES)){
            return RP_EOOR;
    }
    else{
        asg_regset_t *regset = (asg_regset_t *) handle->regset;
        r=FpgaRegDump("Pattern/waveform",0,(uint32_t*)regset->table,data_len);
    }
    return r;
}

