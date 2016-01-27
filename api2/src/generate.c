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

// for Init
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <math.h>
#include "common.h"
#include "generate.h"

int rp_OpenUnit3(){
int x=5;
x=4;
   return x;
}


int rp_GenOpen(char *dev, rp_handle_uio_t *handle) {
    // make a copy of the device path
    handle->dev = (char*) malloc((strlen(dev)+1) * sizeof(char));
    strncpy(handle->dev, dev, strlen(dev)+1);
    // try opening the device
    handle->fd = open(handle->dev, O_RDWR);
    if (!handle->fd) {
        return -1;
    } else {
        // get regset pointer
        handle->regset = mmap(NULL, GENERATE_BASE_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, handle->fd, 0x0);
        if (handle->regset == NULL) {
            return -1;
        }
    }
    return RP_OK;
}

int rp_GenClose(rp_handle_uio_t *handle) {
    // release regset
    munmap((void *) handle->regset, GENERATE_BASE_SIZE);
    // close device
    close (handle->fd);
    // free device path
    free(handle->dev);
    // free name
    // TODO
    return RP_OK;
}

int rp_GenReset(rp_handle_uio_t *handle) {
    asg_regset_t *regset = (asg_regset_t *) &(((gen_regset_t *) handle->regset)->asg);
    // reset generator state machine
    iowrite32(0x1, &regset->ctl_sys);
//    gen_setGenMode(channel, RP_GEN_MODE_CONTINUOUS);
    // clean up table (TODO, should I really)
    uint32_t length = 1 << RP_GEN_CWM;
    uint16_t waveform [length];
    for (unsigned int i=0; i<length; i++) {
        waveform[i] = 0;
    }
    rp_GenSetWaveform    (handle, waveform, length);
    rp_GenSetFreqPhase   (handle, 1000.0, 0.0);
    rp_DigGenGlobalTrigDisable (handle, RP_TRG_ALL);
    rp_GenSetBurst       (handle, 0, 0, 0);
    rp_GenSetLinear      (handle, 1.0, 0.0);
    return RP_OK;
}

// 

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

int rp_GenSetWaveform(rp_handle_uio_t *handle, uint16_t *waveform, uint32_t length) {
    asg_regset_t *regset = (asg_regset_t *) &(((gen_regset_t *) handle->regset)->asg);
    if(length>RP_GEN_SIG_SAMPLES){
        return -1;
    }
    for (uint32_t i=0; i<length; i++) {
        iowrite32(waveform[i], &regset->table[i]);
    }
    return RP_OK;
}

int rp_GenGetWaveform(rp_handle_uio_t *handle, uint16_t *waveform, uint32_t length) {
    asg_regset_t *regset = (asg_regset_t *) &(((gen_regset_t *) handle->regset)->asg);
    if(length>RP_GEN_SIG_SAMPLES){
        return -1;
    }
    for (uint32_t i=0; i<length; i++) {
        waveform[i] = ioread32(&regset->table[i]);
    }
    return RP_OK;
}

int rp_GenSetWaveformUpCountSeq(rp_handle_uio_t *handle) {
    uint16_t ramp[256];
    for (uint32_t i=0; i<256; i++) {
        ramp[i] = i;
    }
    return rp_GenSetWaveform(handle, ramp, 256);
}

int rp_GenSetBurst(rp_handle_uio_t *handle, uint32_t len_data, uint32_t len_idle, uint32_t repetitions) {
    asg_regset_t *regset = (asg_regset_t *) &(((gen_regset_t *) handle->regset)->asg);
    iowrite32(len_data   , &regset->cfg_bdl);
    iowrite32(len_idle   , &regset->cfg_bil);
    iowrite32(repetitions, &regset->cfg_bnm);
    return RP_OK;
}

int rp_GenGetBurst(rp_handle_uio_t *handle, uint32_t *len_data, uint32_t *len_idle, uint32_t *repetitions) {
    asg_regset_t *regset = (asg_regset_t *) &(((gen_regset_t *) handle->regset)->asg);
    *len_data    = ioread32(&regset->cfg_bdl);
    *len_idle    = ioread32(&regset->cfg_bil);
    *repetitions = ioread32(&regset->cfg_bnm);
    return RP_OK;
}

int rp_GenSetMode(rp_handle_uio_t *handle, uint32_t mode) {
    return RP_OK;
}

int rp_GenGetMode(rp_handle_uio_t *handle, uint32_t *mode) {
    return RP_OK;
}

int rp_GenTrigger(rp_handle_uio_t *handle) {
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

int rp_DigGenGlobalTrigEnable(rp_handle_uio_t *handle, uint32_t a_mask)
{
    rp_global_trig_regset_t *regset = (rp_global_trig_regset_t *) &(((asg_regset_t*)handle->regset)->gtrg);
    uint32_t tmp;
    tmp=ioread32(&regset->msk);
    tmp|=a_mask;
    iowrite32(tmp, &regset->msk);
    return RP_OK;
}

int rp_DigGenGlobalTrigDisable(rp_handle_uio_t *handle, uint32_t a_mask)
{
    rp_global_trig_regset_t *regset = (rp_global_trig_regset_t *) &(((asg_regset_t*)handle->regset)->gtrg);
    uint32_t tmp;
    tmp=ioread32(&regset->msk);
    tmp&=~a_mask;
    iowrite32(tmp, &regset->msk);
    return RP_OK;
}

