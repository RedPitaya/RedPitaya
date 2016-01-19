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

int rp_GenInit(char *dev, rp_handle_uio_t *handle) {
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

int rp_GenRelease(rp_handle_uio_t *handle) {
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
    gen_regset_t *regset = (gen_regset_t *) handle->regset;
    // reset generator state machine
    iowrite32(0x1, &regset->ctl_sys);
//    gen_setGenMode(channel, RP_GEN_MODE_CONTINUOUS);
    // clean up table (TODO, should I really)
    uint32_t length = 1 << RP_GEN_CWM;
    int16_t waveform [length];
    for (unsigned int i=0; i<length; i++) {
        waveform[i] = 0;
    }
    rp_GenSetWaveform        (handle, waveform, length);
    rp_GenSetAmp             (handle, 1);
    rp_GenSetOffset          (handle, 0);
    rp_GenSetFreq            (handle, 0);
    rp_GenSetPhase           (handle, 0);
    rp_GenSetTriggerSource   (handle, 0);
    rp_GenSetBurstRepetitions(handle, 0);
    rp_GenSetBurstCount      (handle, 0);
    rp_GenSetBurstDelay      (handle, 0);
    return RP_OK;
}

int rp_GenSetAmp(rp_handle_uio_t *handle, float amplitude) {
    gen_regset_t *regset = (gen_regset_t *) handle->regset;
    iowrite32((uint32_t) (amplitude * (1 << RP_GEN_DWM)), &regset->cfg_lmul);
    return RP_OK;
}

int rp_GenGetAmp(rp_handle_uio_t *handle, float *amplitude) {
    gen_regset_t *regset = (gen_regset_t *) handle->regset;
    *amplitude = ((float) ioread32(&regset->cfg_lmul)) / (1 << RP_GEN_DWM);
    return RP_OK;
}

int rp_GenSetOffset(rp_handle_uio_t *handle, float offset) {
    gen_regset_t *regset = (gen_regset_t *) handle->regset;
    iowrite32((uint32_t) (offset * (1 << RP_GEN_DWS)),  &regset->cfg_lsum);
    return RP_OK;
}

int rp_GenGetOffset(rp_handle_uio_t *handle, float *offset) {
    gen_regset_t *regset = (gen_regset_t *) handle->regset;
    *offset = ((float) ioread32(&regset->cfg_lsum)) / (1 << RP_GEN_DWS);
    return RP_OK;
}

int rp_GenSetFreq(rp_handle_uio_t *handle, double frequency) {
    gen_regset_t *regset = (gen_regset_t *) handle->regset;
    if (frequency < FREQUENCY_MIN || frequency > FREQUENCY_MAX) {
        return RP_EOOR;
    }
    uint32_t size = ioread32(&regset->cfg_lsum) + 1;
    iowrite32((uint32_t) (size * (frequency / RP_GEN_SR)), &regset->cfg_step);
    return RP_OK;
}

int rp_GenGetFreq(rp_handle_uio_t *handle, double *frequency) {
    gen_regset_t *regset = (gen_regset_t *) handle->regset;
    uint32_t size = ioread32(&regset->cfg_lsum) + 1;
    *frequency = (double) (ioread32(&regset->cfg_step) / size * RP_GEN_SR);
    return RP_OK;
}

int rp_GenSetPhase(rp_handle_uio_t *handle, double phase) {
    gen_regset_t *regset = (gen_regset_t *) handle->regset;
    uint32_t size = ioread32(&regset->cfg_lsum) + 1;
    iowrite32((uint32_t) ((double) size * fmod(phase,360)), &regset->cfg_offs);
    return RP_OK;
}

int rp_GenGetPhase(rp_handle_uio_t *handle, double *phase) {
    gen_regset_t *regset = (gen_regset_t *) handle->regset;
    uint32_t size = ioread32(&regset->cfg_lsum) + 1;
    *phase = ioread32(&regset->cfg_offs) / size * 360;
    return RP_OK;
}

int rp_GenSetWaveform(rp_handle_uio_t *handle, int16_t *waveform, uint32_t length) {
    gen_regset_t *regset = (gen_regset_t *) handle->regset;
    // Check if data is normalized, else return error
    for (int unsigned i=0; i<length; i++) {
        // TODO: use a mask macro
        if (waveform[i] & ~((1<<RP_GEN_DWO)-1))  return RP_ENN;
    }
    // store array into hardware buffer
    for (int unsigned i=0; i<length; i++) {
        iowrite32(waveform[i], &regset->table[i]);
    }
    iowrite32((length << RP_GEN_CWM) - 1, &regset->cfg_size);
    return RP_OK;
}

int rp_GenGetWaveform(rp_handle_uio_t *handle, int16_t *waveform, uint32_t *length) {
    gen_regset_t *regset = (gen_regset_t *) handle->regset;
    *length = (ioread32(&regset->cfg_size) + 1) >> RP_GEN_CWM;
    // store array into hardware buffer
    for (int unsigned i=0; i<*length; i++) {
        waveform[i] = ioread32(&regset->table[i]);
    }
    return RP_OK;
}

int rp_GenSetBurstCount(rp_handle_uio_t *handle, int num) {
    gen_regset_t *regset = (gen_regset_t *) handle->regset;
    iowrite32(num, &regset->cfg_bcyc);
    return RP_OK;
}

int rp_GenGetBurstCount(rp_handle_uio_t *handle, int *num) {
    gen_regset_t *regset = (gen_regset_t *) handle->regset;
    *num = ioread32(&regset->cfg_bcyc);
    return RP_OK;
}

int rp_GenSetBurstRepetitions(rp_handle_uio_t *handle, int repetitions) {
    gen_regset_t *regset = (gen_regset_t *) handle->regset;
    iowrite32(repetitions, &regset->cfg_bnum);
    return RP_OK;
}

int rp_GenGetBurstRepetitions(rp_handle_uio_t *handle, int *repetitions) {
    gen_regset_t *regset = (gen_regset_t *) handle->regset;
    *repetitions = ioread32(&regset->cfg_bnum);
    return RP_OK;
}

int rp_GeniSetBurstDelay(rp_handle_uio_t *handle, uint32_t delay) {
    gen_regset_t *regset = (gen_regset_t *) handle->regset;
    iowrite32(delay, &regset->cfg_bdly);
    return RP_OK;
}

int rp_GenGetBurstDelay(rp_handle_uio_t *handle, uint32_t *delay) {
    gen_regset_t *regset = (gen_regset_t *) handle->regset;
    *delay = ioread32(&regset->cfg_bdly);
    return RP_OK;
}

int rp_GenSetTriggerSource(rp_handle_uio_t *handle, rp_trig_src_t src) {
    gen_regset_t *regset = (gen_regset_t *) handle->regset;
    uint32_t tmp = ioread32(&regset->cfg_sys);
    uint32_t MASK = 0x00000004;
    tmp = (src & MASK) | (tmp & MASK);
    iowrite32(tmp, &regset->cfg_sys);
    return RP_OK;
}

int rp_GenGetTriggerSource(rp_handle_uio_t *handle, rp_trig_src_t *src) {
    gen_regset_t *regset = (gen_regset_t *) handle->regset;
    uint32_t MASK = 0x00000004;
    *src = ioread32(&regset->cfg_sys) & MASK;
    return RP_OK;
}

int rp_GenSetMode(rp_handle_uio_t *handle, rp_gen_mode_t mode) {
    return RP_OK;
}

int rp_GenGetMode(rp_handle_uio_t *handle, rp_gen_mode_t *mode) {
    return RP_OK;
}

int rp_GenTrigger(rp_handle_uio_t *handle) {
    return RP_OK;
}

