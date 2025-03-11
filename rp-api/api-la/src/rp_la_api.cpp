#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <mutex>

#include "common/common.h"
#include "common/dma.h"

#include "rp.h"
#include "rp_la_acq.h"
#include "rp_la_api.h"

namespace rp_la {

rp_handle_uio_t g_la_acq_handle;
rp_acq_data_t g_acq_data;

std::mutex g_open_mtx;
std::mutex g_trigger_mtx;
std::mutex g_data_mtx;

/**
 * Open device
 */
int rp_OpenUnit() {
    std::lock_guard lock(g_open_mtx);
    return rp_LaAcqOpen("/dev/uio/la", &g_la_acq_handle);
}

/**
 * Close device
 */
int rp_CloseUnit() {
    std::lock_guard lock(g_open_mtx);
    return rp_LaAcqClose(&g_la_acq_handle);
}

int rp_SetTriggerDigitalPortProperties(RP_DIGITAL_CHANNEL_DIRECTIONS* directions, uint16_t nDirections) {
    std::lock_guard lock(g_trigger_mtx);

    rp_LaAcqGlobalTrigSet(&g_la_acq_handle, RP_TRG_ALL_MASK);

    rp_la_trg_regset_t trg;
    memset(&trg, 0, sizeof(rp_la_trg_regset_t));

    // none of triggers is enabled
    if (directions == NULL) {
        ERROR_LOG("Directions is NULL")
        return RP_OK;
    }

    uint32_t n;
    uint32_t edge_cnt = 0;
    for (n = 0; n < nDirections; n++) {
        uint32_t mask = (1 << directions[n].channel);
        if (edge_cnt > 1) {
            // more than one pin is set to rising/falling edge
            return RP_EIPV;
        }
        switch (directions[n].direction) {
            case RP_DIGITAL_DONT_CARE:
                break;
            case RP_DIGITAL_DIRECTION_LOW:
                trg.cmp_msk |= mask;
                break;
            case RP_DIGITAL_DIRECTION_HIGH:
                trg.cmp_msk |= mask;
                trg.cmp_val |= mask;
                break;
            case RP_DIGITAL_DIRECTION_RISING:
                trg.edg_pos |= mask;
                edge_cnt++;
                break;
            case RP_DIGITAL_DIRECTION_FALLING:
                trg.edg_neg |= mask;
                edge_cnt++;
                break;
            case RP_DIGITAL_DIRECTION_RISING_OR_FALLING:
                trg.edg_pos |= mask;
                trg.edg_neg |= mask;
                edge_cnt++;
                break;
            default:
                return RP_EIPV;
                break;
        }
    }
    if (edge_cnt == 1) {
        // edge - enable pattern & software trigger
        rp_LaAcqSetTrigSettings(&g_la_acq_handle, trg);
        rp_LaAcqGlobalTrigSet(&g_la_acq_handle, RP_TRG_LOA_PAT_MASK | RP_TRG_LOA_SWE_MASK);
    } else {
        // else leave only software trigger on
        rp_LaAcqSetTrigSettings(&g_la_acq_handle, trg);
        rp_LaAcqGlobalTrigSet(&g_la_acq_handle, RP_TRG_LOA_SWE_MASK);
    }

    return RP_OK;
}

int rp_EnableDigitalPortDataRLE(bool enable) {
    return enable ? rp_LaAcqEnableRLE(&g_la_acq_handle) : rp_LaAcqDisableRLE(&g_la_acq_handle);
}

int rp_IsRLEEnable(bool* enable) {
    return rp_LaAcqIsRLE(&g_la_acq_handle, enable);
}

int rp_SoftwareTrigger() {
    std::lock_guard lock(g_trigger_mtx);
    std::lock_guard lock2(g_data_mtx);
    if (g_acq_data.inProcess) {
        TRACE_SHORT("Call softwere trigger")
        rp_LaAcqTriggerAcq(&g_la_acq_handle);
    }
    return RP_OK;
}

int rp_SetPolarity(uint32_t reg) {
    return rp_LaAcqSetPolarity(&g_la_acq_handle, reg);
}

int rp_GetTimebase(uint32_t decimation, double* timeIntervalNanoseconds) {
    *timeIntervalNanoseconds = decimation * c_max_dig_sampling_rate_time_interval_ns;
    return RP_OK;
}

int rp_SetDataBuffer(int16_t* buffer, size_t size) {
    std::lock_guard lock(g_data_mtx);
    g_acq_data.buf = buffer;
    g_acq_data.buf_size = size;
    return RP_OK;
}

int rp_Run(uint32_t noOfPreTriggerSamples, uint32_t noOfPostTriggerSamples, uint32_t decimation, double* timeIndisposedMs) {

    // TODO
    // double timeIntervalNanoseconds;
    // uint32_t maxSamples = 0;
    // rp_LaAcqGetFullBufferSize(&g_la_acq_handle,&maxSamples);
    // rp_GetTimebase(decimation,&timeIntervalNanoseconds);

    // if(!(inrangeUint32 (noOfPreTriggerSamples + noOfPostTriggerSamples, 10, maxSamples))){
    //     return RP_EIPV;
    // }

    // *timeIndisposedMs = (noOfPreTriggerSamples + noOfPostTriggerSamples) * timeIntervalNanoseconds / 10e6;
    *timeIndisposedMs = 0;

    rp_la_decimation_regset_t dec;
    dec.dec = decimation;
    rp_LaAcqSetDecimation(&g_la_acq_handle, dec);

    rp_la_cfg_regset_t cfg;
    cfg.pre = noOfPreTriggerSamples;
    cfg.pst = noOfPostTriggerSamples;
    TRACE_SHORT("Set pre trigger %d post trigger %d", cfg.pre, cfg.pst)
    if (rp_LaAcqSetCntConfig(&g_la_acq_handle, cfg) != RP_OK) {
        return RP_EIPV;
    }

    TRACE_SHORT("rp_LaAcqRunAcq");
    std::lock_guard lock(g_data_mtx);
    // start acq.
    if (rp_LaAcqRunAcq(&g_la_acq_handle) != RP_OK) {
        rp_LaAcqStopAcq(&g_la_acq_handle);
        return RP_EOP;
    }
    g_acq_data.reset();
    g_acq_data.inProcess = true;
    g_acq_data.pre_samples = noOfPreTriggerSamples;
    g_acq_data.post_samples = noOfPostTriggerSamples;
    return RP_OK;
}

int rp_WaitDataRLE(int timeout) {
    // block till acq. is complete
    TRACE_SHORT("Blocking read");
    bool isTimeout = false;
    int ret = rp_LaAcqRead(&g_la_acq_handle, timeout, &isTimeout);

    rp_LaAcqStopAcq(&g_la_acq_handle);
    // make sure acq. is stopped
    bool isStoped;
    rp_LaAcqAcqIsStopped(&g_la_acq_handle, &isStoped);
    if (!isStoped) {
        ERROR_LOG("Read not stopped!!");
        return RP_EOP;
    }

    std::lock_guard lock(g_data_mtx);
    g_acq_data.inProcess = false;

    if (ret == -1) {
        ERROR_LOG("Read error")
        g_acq_data.isCapture = false;
        return RP_EOP;
    }

    if (isTimeout) {
        g_acq_data.isCapture = false;
        g_acq_data.isTimeout = true;
        return RP_OK;
    }

    // TODO Need FIX
    // in the RLE mode we only check which was the last sample where acq. stopped
    uint32_t last_index = 0;
    uint32_t current = 0;
    bool buf_ovfl;
    rp_LaAcqGetRLEStatus(&g_la_acq_handle, &current, &last_index, &buf_ovfl);

    g_acq_data.trig_blockIndexRLE = 0;
    g_acq_data.last_blockIndexRLE = last_index;
    g_acq_data.isCapture = true;
    return RP_OK;
}

int rp_WaitData(int timeout) {
    // block till acq. is complete
    TRACE_SHORT("Blocking read");
    bool isTimeout = false;
    int ret = rp_LaAcqRead(&g_la_acq_handle, timeout, &isTimeout);

    rp_LaAcqStopAcq(&g_la_acq_handle);
    // make sure acq. is stopped
    bool isStoped;
    rp_LaAcqAcqIsStopped(&g_la_acq_handle, &isStoped);
    if (!isStoped) {
        ERROR_LOG("Read not stopped!!");
        return RP_EOP;
    }

    std::lock_guard lock(g_data_mtx);
    g_acq_data.inProcess = false;

    if (ret == -1) {
        ERROR_LOG("Read error")
        g_acq_data.isCapture = false;
        return RP_EOP;
    }

    if (isTimeout) {
        g_acq_data.isCapture = false;
        g_acq_data.isTimeout = true;
        return RP_OK;
    }

    uint32_t trig_sample = 0;
    uint32_t last_sample = 0;

    // TODO Need TEST and FIX
    // get trigger position
    uint32_t pst_length;
    bool buf_ovfl;
    if (rp_LaAcqGetCntStatus(&g_la_acq_handle, &trig_sample, &pst_length, &buf_ovfl) != RP_OK) {
        ERROR_LOG("Error get status")
        g_acq_data.isCapture = false;
        return RP_EOP;
    }
    TRACE_SHORT("Trig_sample %d, pst_length %d, buf_ovfl %d", trig_sample, pst_length, buf_ovfl);
    if (pst_length != g_acq_data.post_samples) {
        ERROR_LOG("Acquired number of post samples must match to required")
        g_acq_data.isCapture = false;
        return RP_EOP;
    }

    g_acq_data.trig_indexNonRLE = trig_sample;
    g_acq_data.last_indexNonRLE = last_sample;
    g_acq_data.isCapture = true;
    return RP_OK;
}

int rp_GetTrigBlockPositionRLE(uint32_t* tigger_pos) {
    std::lock_guard lock(g_data_mtx);
    *tigger_pos = g_acq_data.trig_blockIndexRLE;
    return RP_OK;
}

int rp_GetTrigPosition(uint32_t* tigger_pos) {
    std::lock_guard lock(g_data_mtx);
    *tigger_pos = g_acq_data.trig_indexNonRLE;
    return RP_OK;
}

int rp_GetIsTimeout(bool* isTimeout) {
    std::lock_guard lock(g_data_mtx);
    *isTimeout = g_acq_data.isTimeout;
    return RP_OK;
}

int rp_GetValuesRLE(uint32_t* noOfBlocks, uint64_t* noOfSamples) {
    std::lock_guard lock(g_data_mtx);
    TRACE_SHORT("Dma size %d dev %s", g_la_acq_handle.dma_size, g_la_acq_handle.dma_dev.c_str())
    int16_t* map = (int16_t*)mmap(NULL, g_la_acq_handle.dma_size, PROT_READ | PROT_WRITE, MAP_SHARED, g_la_acq_handle.dma_fd, 0);
    if (map == NULL) {
        ERROR_LOG("Failed to mmap");
        rp_CloseUnit();
        return RP_EMMD;
    }

    if (g_acq_data.buf == NULL || g_acq_data.buf_size == 0) {
        FATAL("Buffer for writing is not set")
        return RP_EOP;
    }

    uint32_t buff_size = 0;
    rp_LaAcqBufLenInSamples(&g_la_acq_handle, &buff_size);

    uint32_t first_sample = 0;
    uint32_t blockCount = 0;
    g_acq_data.trig_blockIndexRLE = 0;

    uint32_t fist_sample_len_adj;

    uint32_t len = 0;
    uint32_t i = 0;
    uint32_t index = g_acq_data.last_blockIndexRLE;
    uint32_t total = g_acq_data.pre_samples + g_acq_data.post_samples;

    TRACE_SHORT("Samples total %d pre %d post %d index %d", total, g_acq_data.pre_samples, g_acq_data.post_samples, index)

    bool trig_sample_found = false;

    // Find trigger block position

    while (1) {
        i++;
        len += ((uint8_t)(map[index] >> 8)) + 1;

        // trigger position
        if (!trig_sample_found) {
            if (len >= (g_acq_data.post_samples - 1)) {
                g_acq_data.trig_blockIndexRLE = i;
                trig_sample_found = true;
            }
        }

        // first sample position
        if (len >= total) {
            first_sample = index;
            blockCount = i;
            // adjust first sample length so that it fits to the
            // length of requested data
            fist_sample_len_adj = len - total;
            break;
        }

        index = index == 0 ? buff_size - 1 : index - 1;
    }

    // Copy data from circular buffer
    index = first_sample;
    for (i = 0; i < blockCount; i++) {
        if (index >= buff_size) {
            index = 0;
        }
        g_acq_data.buf[i] = ((map[index] & 0xff) << 8) | ((map[index] >> 8) & 0xff);
        // adjust length of first sample
        if (i == 0) {
            g_acq_data.buf[i] -= (int16_t)(fist_sample_len_adj);
        }
        *noOfSamples += (g_acq_data.buf[i] & 0xff) + 1;
        index++;
    }

    g_acq_data.trig_blockIndexRLE = blockCount - g_acq_data.trig_blockIndexRLE;
    TRACE_SHORT("Post trig_blockIndexRLE %d last_blockIndexRLE %d blockCount %d", g_acq_data.trig_blockIndexRLE, g_acq_data.last_blockIndexRLE, blockCount)
    *noOfBlocks = blockCount;

    if (munmap(map, g_la_acq_handle.dma_size) == -1) {
        ERROR_LOG("Failed to munmap");
        return RP_EUMD;
    }

    return RP_OK;
};

int rp_GetValues(uint32_t* noOfSamples) {
    std::lock_guard lock(g_data_mtx);
    TRACE_SHORT("Dma size %d dev %s", g_la_acq_handle.dma_size, g_la_acq_handle.dma_dev.c_str())
    int16_t* map = (int16_t*)mmap(NULL, g_la_acq_handle.dma_size, PROT_READ | PROT_WRITE, MAP_SHARED, g_la_acq_handle.dma_fd, 0);
    if (map == NULL) {
        ERROR_LOG("Failed to mmap");
        rp_CloseUnit();
        return RP_EMMD;
    }

    if (g_acq_data.buf == NULL || g_acq_data.buf_size == 0) {
        FATAL("Buffer for writing is not set")
        return RP_EOP;
    }

    uint32_t sample_size = 0;
    rp_LaAcqBufLenInSamples(&g_la_acq_handle, &sample_size);

    int32_t first_sample = g_acq_data.trig_indexNonRLE - g_acq_data.pre_samples;
    int32_t last_sample = g_acq_data.trig_indexNonRLE + g_acq_data.post_samples;
    int32_t buf_len = sample_size;

    TRACE_SHORT("req: pre=%d pos=%d", g_acq_data.pre_samples, g_acq_data.post_samples);
    TRACE_SHORT("sta: first=%d trig=%d last=%d", first_sample, g_acq_data.trig_indexNonRLE, last_sample);

    int32_t wlen;
    if (first_sample < 0) {
        wlen = abs(first_sample);
        first_sample = buf_len + first_sample;
        for (int i = 0; i < wlen; i++) {
            g_acq_data.buf[i] = map[first_sample + i];
        }
        for (int i = 0; i < last_sample; i++) {
            g_acq_data.buf[wlen + i] = map[i];
        }
    } else if (last_sample >= buf_len) {
        wlen = buf_len - first_sample;
        for (int i = 0; i < wlen; i++) {
            g_acq_data.buf[i] = map[first_sample + i];
        }
        last_sample -= buf_len;
        for (int i = 0; i < last_sample; i++) {
            g_acq_data.buf[wlen + i] = map[i];
        }
    } else {
        for (uint32_t i = 0; i < (*noOfSamples); i++) {
            g_acq_data.buf[i] = map[first_sample + i];
        }
    }

    if (munmap(map, g_la_acq_handle.dma_size) == -1) {
        ERROR_LOG("Failed to munmap");
        return RP_EUMD;
    }

    return RP_OK;
};

int rp_Stop(void) {
    return rp_SoftwareTrigger();
}

int rp_GetFullBufferSize(uint32_t* size) {
    return rp_LaAcqGetFullBufferSize(&g_la_acq_handle, size);
}

}  // namespace rp_la