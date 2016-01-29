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

int rp_GenOpen(char *dev, rp_handle_uio_t *handle) {
    handle->length = GENERATE_BASE_SIZE;
    int status = common_Open (dev, handle);
    if (status != RP_OK) {
        return status;
    }
    status = rp_GenReset(handle);
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

/** Control registers setter & getter */
static int rp_GenSetControl(rp_handle_uio_t *handle, rp_ctl_regset_t a_reg) {
    rp_ctl_regset_t *regset = (rp_ctl_regset_t *) &(((asg_regset_t*)handle->regset)->ctl);
    iowrite32(a_reg.ctl, &regset->ctl);
    return RP_OK;
}


static int rp_GenGetControl(rp_handle_uio_t *handle, rp_ctl_regset_t * a_reg) {
    rp_ctl_regset_t *regset = (rp_ctl_regset_t *) &(((asg_regset_t*)handle->regset)->ctl);
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
    return RP_OK;
}

int rp_GenGlobalTrigEnable(rp_handle_uio_t *handle, uint32_t a_mask)
{
    rp_global_trig_regset_t *regset = (rp_global_trig_regset_t *) &(((asg_regset_t*)handle->regset)->gtrg);
    uint32_t tmp;
    tmp=ioread32(&regset->msk);
    tmp|=a_mask;
    iowrite32(tmp, &regset->msk);
    return RP_OK;
}

int rp_GenGlobalTrigDisable(rp_handle_uio_t *handle, uint32_t a_mask)
{
    rp_global_trig_regset_t *regset = (rp_global_trig_regset_t *) &(((asg_regset_t*)handle->regset)->gtrg);
    uint32_t tmp;
    tmp=ioread32(&regset->msk);
    tmp&=~a_mask;
    iowrite32(tmp, &regset->msk);
    return RP_OK;
}



int rp_GenSetLinear(rp_handle_uio_t *handle, float amplitude, float offset) {
    linear_regset_t *regset = (linear_regset_t *) &(((gen_regset_t *) handle->regset)->lin);
    int32_t mul = (int32_t) (amplitude * (1 << RP_GEN_DWM));
    int32_t sum = (int32_t) (offset    * (1 << RP_GEN_DWS));
    iowrite32(mul, &regset->mul);
    iowrite32(sum, &regset->sum);
    return RP_OK;
}

int rp_GenGetLinear(rp_handle_uio_t *handle, float *amplitude, float *offset) {
    linear_regset_t *regset = (linear_regset_t *) &(((gen_regset_t *) handle->regset)->lin);
    int32_t mul = ioread32(&regset->mul);
    int32_t sum = ioread32(&regset->sum);
    *amplitude = ((float) mul) / (1 << RP_GEN_DWM);
    *offset    = ((float) sum) / (1 << RP_GEN_DWS);
    return RP_OK;
}

int rp_GenSetFreqPhase(rp_handle_uio_t *handle, double frequency, double phase) {
    asg_regset_t *regset = (asg_regset_t *) &(((gen_regset_t *) handle->regset)->asg);
    if (frequency < FREQUENCY_MIN || frequency > FREQUENCY_MAX) {
        return RP_EOOR;
    }
    uint32_t size = ioread32(&regset->cfg_siz) + 1;
    iowrite32((uint32_t) ((double) size * (frequency / RP_GEN_SR)), &regset->cfg_stp);
    iowrite32((uint32_t) ((double) size * fmod(phase,360)/360    ), &regset->cfg_off);
    return RP_OK;
}

int rp_GenGetFreqPhase(rp_handle_uio_t *handle, double *frequency, double *phase) {
    asg_regset_t *regset = (asg_regset_t *) &(((gen_regset_t *) handle->regset)->asg);
    uint32_t size = ioread32(&regset->cfg_siz) + 1;
    *frequency = (double) (ioread32(&regset->cfg_stp) / (double) size * RP_GEN_SR);
    *phase     = (double) (ioread32(&regset->cfg_off) / (double) size * 360      );
    return RP_OK;
}

int rp_GenSetWaveform(rp_handle_uio_t *handle, uint16_t *waveform, uint32_t size) {
    asg_regset_t *regset = (asg_regset_t *) &(((gen_regset_t *) handle->regset)->asg);
    if((size==0)||(size>RP_GEN_SIG_SAMPLES)){
        return -1;
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

int rp_GenSetMode(rp_handle_uio_t *handle, RP_GEN_MODE a_mode)
{
    switch(a_mode){
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
int rp_GenGetMode(rp_handle_uio_t *handle, RP_GEN_MODE * a_mode)
{
    asg_regset_t *regset = (asg_regset_t *) handle->regset;
    uint32_t tmp;
    tmp=ioread32(&regset->cfg_bst);
    if(tmp&RP_GEN_MODE_BURST){
        *a_mode=RP_GEN_MODE_BURST;
    }
    else{
        *a_mode=RP_GEN_MODE_CONTINUOUS;
    }
    return RP_OK;
}
*/

int rp_GenSetBurstModeRepetitions(rp_handle_uio_t *handle, uint32_t a_val)
{
    asg_regset_t *regset = (asg_regset_t *) &(((gen_regset_t *) handle->regset)->asg);
    if(a_val==RP_GEN_REP_INF){
        return rp_GenSetBst(handle, RP_GEN_CFG_BURST_INF_MASK);

    }
    else{
        iowrite32(a_val, &regset->cfg_bnm);
        return rp_GenClearBst(handle, RP_GEN_CFG_BURST_INF_MASK);
    }
    return RP_OK;
}

int rp_GenSetBurstModeBurstDataLen(rp_handle_uio_t *handle, uint32_t length)
{
    asg_regset_t *regset = (asg_regset_t *) &(((gen_regset_t *) handle->regset)->asg);
    if(length>RP_GEN_SIG_SAMPLES){
        return -1;
    }
    iowrite32(length, &regset->cfg_bdl);
    return RP_OK;
}

int rp_GenSetBurstModeIdle(rp_handle_uio_t *handle, uint32_t a_val)
{
    asg_regset_t *regset = (asg_regset_t *) &(((gen_regset_t *) handle->regset)->asg);
    iowrite32(a_val, &regset->cfg_bil);
    return RP_OK;
}

int rp_GenOutputEnable(rp_handle_uio_t *handle, uint32_t a_mask)
{
    rp_dig_out_t *regset = (rp_dig_out_t *) &(((asg_regset_t*)handle->regset)->dig);
    uint32_t tmp;
    tmp=ioread32(&regset->dig_out_en);
    tmp|=a_mask;
    iowrite32(tmp, &regset->dig_out_en);
    return RP_OK;
}

int rp_GenOutputDisable(rp_handle_uio_t *handle, uint32_t a_mask)
{
    rp_dig_out_t *regset = (rp_dig_out_t *) &(((asg_regset_t*)handle->regset)->dig);
    uint32_t tmp;
    tmp=ioread32(&regset->dig_out_en);
    tmp&=~a_mask;
    iowrite32(tmp, &regset->dig_out_en);
    return RP_OK;
}

int rp_GenSetSampleRate(rp_handle_uio_t *handle, double a_freq)
{
    uint32_t reg_val=(a_freq/RP_GEN_SR)+1.0;
    asg_regset_t *regset = (asg_regset_t *) &(((gen_regset_t *) handle->regset)->asg);
    iowrite32(reg_val, &regset->cfg_stp);
    return RP_OK;
}

int rp_GenFpgaRegDump(rp_handle_uio_t *handle)
{
    return FpgaRegDump(0,(uint32_t*)handle->regset,sizeof(uint32_t)*15);
}

