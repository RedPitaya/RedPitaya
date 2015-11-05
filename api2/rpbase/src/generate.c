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


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "redpitaya/rp.h"
#include "common.h"
#include "generate.h"

static volatile generate_regset_t (*regset)[RP_MNG];

int generate_Init() {
    cmn_Map((GENERATE_BASE_SIZE * RP_MNG), GENERATE_BASE_ADDR, (void **) &regset);
    return RP_OK;
}

int generate_Release() {
    cmn_Unmap(GENERATE_BASE_SIZE * RP_MNG, (void **) &regset);
    return RP_OK;
}

int rp_GenReset() {
    for (int unsigned channel=0; channel<RP_MNG; channel++) {
        // reset generator state machine
        iowrite32(0x1, &regset[channel]->ctl_sys);
//    gen_setGenMode(channel, RP_GEN_MODE_CONTINUOUS);
        // clean up table (TODO, should I really)
        uint32_t length = 1 << RP_GEN_CWM;
        int16_t waveform [length];
        for (unsigned int i=0; i<length; i++) {
            waveform[i] = 0;
        }
        rp_GenSetWaveform        (channel, waveform, length);
        rp_GenSetAmp             (channel, 1);
        rp_GenSetOffset          (channel, 0);
        rp_GenSetFreq            (channel, 0);
        rp_GenSetPhase           (channel, 0);
        rp_GenSetTriggerSource   (channel, 0);
        rp_GenSetBurstRepetitions(channel, 0);
        rp_GenSetBurstCount      (channel, 0);
        rp_GenSetBurstDelay      (channel, 0);
    }
    return RP_OK;
}

int rp_GenSetAmp(int unsigned channel, float amplitude) {
    if (channel > RP_MNG)  return RP_EPN;
    // TODO add range check
    iowrite32((uint32_t) (amplitude * (1 << RP_GEN_DWM)), &regset[channel]->cfg_lmul);
    return RP_OK;
}

int rp_GenGetAmp(int unsigned channel, float *amplitude) {
    if (channel > RP_MNG)  return RP_EPN;
    *amplitude = ((float) ioread32(&regset[channel]->cfg_lmul)) / (1 << RP_GEN_DWM);
    return RP_OK;
}

int rp_GenSetOffset(int unsigned channel, float offset) {
    if (channel > RP_MNG)  return RP_EPN;
    // TODO add range check
    iowrite32((uint32_t) (offset * (1 << RP_GEN_DWS)),  &regset[channel]->cfg_lsum);
    return RP_OK;
}

int rp_GenGetOffset(int unsigned channel, float *offset) {
    if (channel > RP_MNG)  return RP_EPN;
    *offset = ((float) ioread32(&regset[channel]->cfg_lsum)) / (1 << RP_GEN_DWS);
    return RP_OK;
}

int rp_GenSetFreq(int unsigned channel, double frequency) {
    if (channel > RP_MNG)  return RP_EPN;
    // TODO add range check
    if (frequency < FREQUENCY_MIN || frequency > FREQUENCY_MAX) {
        return RP_EOOR;
    }
    uint32_t size = ioread32(&regset[channel]->cfg_lsum) + 1;
    iowrite32((uint32_t) (size * (frequency / RP_GEN_SR)), &regset[channel]->cfg_step);
    return RP_OK;
}

int rp_GenGetFreq(int unsigned channel, double *frequency) {
    if (channel > RP_MNG)  return RP_EPN;
    uint32_t size = ioread32(&regset[channel]->cfg_lsum) + 1;
    *frequency = (double) (ioread32(&regset[channel]->cfg_step) / size * RP_GEN_SR);
    return RP_OK;
}

int rp_GenSetPhase(int unsigned channel, double phase) {
    if (channel > RP_MNG)  return RP_EPN;
    uint32_t size = ioread32(&regset[channel]->cfg_lsum) + 1;
    iowrite32((uint32_t) ((double) size * fmod(phase,360)), &regset[channel]->cfg_offs);
    return RP_OK;
}

int rp_GenGetPhase(int unsigned channel, double *phase) {
    if (channel > RP_MNG)  return RP_EPN;
    uint32_t size = ioread32(&regset[channel]->cfg_lsum) + 1;
    *phase = ioread32(&regset[channel]->cfg_offs) / size * 360;
    return RP_OK;
}

int rp_GenSetWaveform(int unsigned channel, int16_t *waveform, uint32_t length) {
    if (channel > RP_MNG)  return RP_EPN;
    // Check if data is normalized, else return error
    for (int unsigned i=0; i<length; i++) {
        // TODO: use a mask macro
        if (waveform[i] & ~((1<<RP_GEN_DWO)-1))  return RP_ENN;
    }
    // store array into hardware buffer
    for (int unsigned i=0; i<length; i++) {
        iowrite32(waveform[i], &regset[channel]->table[i]);
    }
    iowrite32((length << RP_GEN_CWM) - 1, &regset[channel]->cfg_size);
    return RP_OK;
}

int rp_GenGetWaveform(int unsigned channel, int16_t *waveform, uint32_t *length) {
    if (channel > RP_MNG)  return RP_EPN;
    *length = (ioread32(&regset[channel]->cfg_size) + 1) >> RP_GEN_CWM;
    // store array into hardware buffer
    for (int unsigned i=0; i<*length; i++) {
        waveform[i] = ioread32(&regset[channel]->table[i]);
    }
    return RP_OK;
}

int rp_GenSetBurstCount(int unsigned channel, int num) {
    if (channel > RP_MNG)  return RP_EPN;
    // TODO add range check
    iowrite32(num, &regset[channel]->cfg_bcyc);
    return RP_OK;
}

int rp_GenGetBurstCount(int unsigned channel, int *num) {
    if (channel > RP_MNG)  return RP_EPN;
    *num = ioread32(&regset[channel]->cfg_bcyc);
    return RP_OK;
}

int rp_GenSetBurstRepetitions(int unsigned channel, int repetitions) {
    if (channel > RP_MNG)  return RP_EPN;
    // TODO add range check
    iowrite32(repetitions, &regset[channel]->cfg_bnum);
    return RP_OK;
}

int rp_GenGetBurstRepetitions(int unsigned channel, int *repetitions) {
    if (channel > RP_MNG)  return RP_EPN;
    *repetitions = ioread32(&regset[channel]->cfg_bnum);
    return RP_OK;
}

int rp_GeniSetBurstDelay(int unsigned channel, uint32_t delay) {
    if (channel > RP_MNG)  return RP_EPN;
    // TODO add range check
    iowrite32(delay, &regset[channel]->cfg_bdly);
    return RP_OK;
}

int rp_GenGetBurstDelay(int unsigned channel, uint32_t *delay) {
    if (channel > RP_MNG)  return RP_EPN;
    *delay = ioread32(&regset[channel]->cfg_bdly);
    return RP_OK;
}

int rp_GenSetTriggerSource(int unsigned channel, rp_trig_src_t src) {
    if (channel > RP_MNG)  return RP_EPN;
    // TODO add range check
    uint32_t tmp = ioread32(&regset[channel]->cfg_sys);
    uint32_t MASK = 0x00000004;
    tmp = (src & MASK) | (tmp & MASK);
    iowrite32(tmp, &regset[channel]->cfg_sys);
    return RP_OK;
}

int rp_GenGetTriggerSource(int unsigned channel, rp_trig_src_t *src) {
    if (channel > RP_MNG)  return RP_EPN;
    uint32_t MASK = 0x00000004;
    *src = ioread32(&regset[channel]->cfg_sys) & MASK;
    return RP_OK;
}

int rp_GenSetMode(int unsigned channel, rp_gen_mode_t mode) {
    return RP_OK;
}

int rp_GenGetMode(int unsigned channel, rp_gen_mode_t *mode) {
    return RP_OK;
}

int rp_GenTrigger(int unsigned channel) {
    return RP_OK;
}

