/**
 * $Id: worker.c 881 2013-12-16 05:37:34Z rp_jmenart $
 *
 * @brief Red Pitaya Spectrum Analyzer worker.
 *
 * @Author Jure Menart <juremenart@gmail.com>
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <dirent.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <atomic>
#include <memory>
#include <mutex>
#include <thread>

#include "common.h"
#include "common/profiler.h"
#include "math/rp_math.h"
#include "spectrometerApp.h"
#include "version.h"

typedef enum rp_spectr_worker_state_e {
    IDLE_STATE = 0, /* do nothing */
    QUIT_STATE,     /* shutdown worker */
    RESET_STATE,    /* reset current measurement */
    AUTO_STATE,     /* auto mode acquisition */
    NONEXISTS_STATE /* must be last */
} rp_spectr_worker_state_t;

/* Output signals */
// 0 - Xaxis; 1 - Ch 1; 2 - Ch 2; 3 - Ch 3; 4 - Ch 4
#define SPECTR_OUT_SIG_NUM (MAX_ADC_CHANNELS + 1)
#define NUM_SIGNAL_PERIODS 16

int rp_spectr_worker_init();
int rp_spectr_worker_clean(void);
int rp_spectr_worker_exit(void);
int rp_spectr_worker_change_state(rp_spectr_worker_state_t new_state);

std::thread* rp_spectr_thread_handler = NULL;
void rp_spectr_worker_thread();
void clearAll();

/* Parameters & signals communicating with 'external world' */
std::mutex rp_spectr_ctrl_mutex;
volatile rp_spectr_worker_state_t rp_spectr_ctrl;

int g_decimation = 1;
std::mutex rp_spectr_sig_mutex;
std::mutex rp_spectr_window_mutex;
std::mutex rp_spectr_buf_size_mutex;
int rp_spectr_signals_dirty = 0;

rp_dsp_api::CDSP* g_dsp;
rp_dsp_api::data_t g_data;

static float current_freq_range;

int rp_spectr_worker_init() {
    std::lock_guard lock(rp_spectr_sig_mutex);
    rp_spectr_ctrl = IDLE_STATE;

    clearAll();
    auto adc_channels = getADCChannels();
    auto rate = getADCRate();
    g_dsp = new rp_dsp_api::CDSP(adc_channels, ADC_BUFFER_SIZE, rate, true);

    if (!g_dsp->getStoredData()) {
        clearAll();
        return -1;
    }

    if (g_dsp->window_init(rp_dsp_api::HANNING) < 0) {
        clearAll();
        return -1;
    }

    if (g_dsp->fftInit() < 0) {
        clearAll();
        return -1;
    }

    try {
        rp_spectr_thread_handler = new std::thread(rp_spectr_worker_thread);
    } catch (const std::exception& e) {
        clearAll();
        ERROR_LOG("Thread creation failed: %s\n", e.what());
        return -1;
    }
    return 0;
}

void clearAll() {
    delete g_dsp;
    g_dsp = nullptr;
}

int rp_spectr_worker_clean(void) {
    std::lock_guard lock(rp_spectr_sig_mutex);
    clearAll();
    return 0;
}

int rp_spectr_worker_exit(void) {
    int ret_val = 0;

    if (rp_spectr_thread_handler) {
        if (rp_spectr_thread_handler->joinable()) {
            try {
                rp_spectr_thread_handler->join();
            } catch (const std::system_error& e) {
                ERROR_LOG("std::thread join failed: %s\n", e.what());
                ret_val = -1;
            }
        }
        delete rp_spectr_thread_handler;
    }
    rp_spectr_worker_clean();
    return ret_val;
}

int rp_spectr_worker_change_state(rp_spectr_worker_state_t new_state) {
    std::lock_guard lock(rp_spectr_ctrl_mutex);
    if (new_state >= NONEXISTS_STATE)
        return -1;
    rp_spectr_ctrl = new_state;
    return 0;
}

void rp_spectr_worker_thread() {
    rp_spectr_worker_state_t old_state;
    int current_decimation = 1;
    uint32_t buffer_size = 0;
    auto adc_rate = getADCRate();

    while (1) {
        /* update states - we save also old state to see if we need to reset
         * FPGA
         */

        old_state = rp_spectr_ctrl;

        /* request to stop worker thread, we will shut down */
        if (rp_spectr_ctrl == QUIT_STATE) {
            return;
        }
        if (rp_spectr_ctrl == IDLE_STATE) {
            usleep(1000);
            continue;
        }
        // pthread_mutex_unlock(&rp_spectr_ctrl_mutex);
        if (rp_spectr_ctrl == RESET_STATE) {
            rp_AcqResetFpga();
            rp_spectr_worker_change_state(AUTO_STATE);
            continue;
        }
        /* Start the writting machine */
        current_decimation = g_decimation;
        buffer_size = g_dsp->getSignalLength();
        rp_AcqSetDecimationFactor(current_decimation);
        rp_AcqSetTriggerDelayDirect(buffer_size);
        rp_AcqStart();
        rp_AcqSetTriggerSrc(RP_TRIG_SRC_NOW);

        // /* start working */

        rp_acq_trig_state_t stateTrig = RP_TRIG_STATE_WAITING;
        /* polling until data is ready */
        while (1) {
            /* change in state, abort polling */
            if ((rp_spectr_ctrl != old_state)) {
                break;
            }

            if (rp_spectr_ctrl == QUIT_STATE) {
                return;
            }

            if (rp_AcqGetTriggerState(&stateTrig)) {
                return;
            }

            if (stateTrig == RP_TRIG_STATE_TRIGGERED) {
                break;
            }
        }
        bool fillState = false;
        while (!fillState) {
            rp_AcqGetBufferFillState(&fillState);
            if ((rp_spectr_ctrl != old_state)) {
                break;
            }

            if (rp_spectr_ctrl == QUIT_STATE) {
                return;
            }
        }
        rp_AcqStop();

        if ((rp_spectr_ctrl != old_state)) {
            if (rp_spectr_ctrl == RESET_STATE) {
                rp_AcqResetFpga();
                rp_spectr_worker_change_state(AUTO_STATE);
            }
            continue;
        }

        {
            std::lock_guard lock(rp_spectr_buf_size_mutex);
            uint32_t trig_pos;
            rp_AcqGetWritePointerAtTrig(&trig_pos);

            static auto adc_channels = getADCChannels();
            buffers_t buff_out;
            buff_out.size = buffer_size;
            buff_out.use_calib_for_volts = true;
            auto data = g_dsp->getStoredData();
            bool state = false;
            for (auto z = 0; z < adc_channels; z++) {
                g_dsp->getChannel(z, &state);
                buff_out.ch_d[z] = NULL;
                buff_out.ch_f[z] = state ? data->m_in[z].data() : NULL;
                buff_out.ch_i[z] = NULL;
            }

            rp_AcqGetDataWithCorrection(trig_pos, &buffer_size, 0, &buff_out);

            // profiler::resetAll();
            // profiler::setTimePoint("1");

            rp_spectr_window_mutex.lock();
            g_dsp->prepareFreqVector(data, adc_rate, g_decimation);
            // profiler::printuS("1", "prepareFreqVector");
            g_dsp->windowFilter(data);
            // profiler::printuS("1", "windowFilter");
            rp_spectr_window_mutex.unlock();
            g_dsp->fft(data);
            // profiler::printuS("1", "fft");
            g_dsp->decimate(data, g_dsp->getOutSignalLength(), g_dsp->getOutSignalLength());
            // profiler::printuS("1", "decimate");
            g_dsp->cnvToMetric(data, g_decimation);
            // profiler::printuS("1", "DSP");
        }
        usleep(100);
    }
}

int spec_run() {
    if (rp_spectr_worker_init() < 0) {
        ERROR_LOG("rp_spectr_worker_init failed");
        return -1;
    }
    auto adc_rate = getADCRate();

    rp_spectr_worker_change_state(AUTO_STATE);
    spec_setFreqRange(adc_rate / 2.0);
    return 0;
}

int spec_running()  // true/false
{
    return rp_spectr_ctrl == AUTO_STATE;
}

int spec_lockData() {
    rp_spectr_buf_size_mutex.lock();
    return 0;
}

int spec_unlockData() {
    rp_spectr_buf_size_mutex.unlock();
    return 0;
}

int spec_stop() {
    rp_spectr_worker_exit();
    return 0;
}

int spec_setWindow(rp_dsp_api::window_mode_t mode) {
    std::lock_guard lock(rp_spectr_window_mutex);
    if (!g_dsp)
        return -1;
    g_dsp->window_init(mode);
    rp_spectr_worker_change_state(RESET_STATE);
    return 0;
}

int spec_getWindow(rp_dsp_api::window_mode_t* mode) {
    if (!g_dsp)
        return -1;
    *mode = g_dsp->getCurrentWindowMode();
    return RP_OK;
}

int spec_setProbe(rp_channel_t channel, uint32_t probe) {
    if (!g_dsp)
        return -1;
    g_dsp->setProbe(channel, probe);
    return RP_OK;
}

int spec_getProbe(rp_channel_t channel, uint32_t* probe) {
    if (!g_dsp)
        return -1;
    g_dsp->getProbe(channel, probe);
    return RP_OK;
}

int spec_setRemoveDC(bool state) {
    if (!g_dsp)
        return -1;
    g_dsp->setRemoveDC(state);
    return 0;
}

int spec_getRemoveDC() {
    if (!g_dsp)
        return false;
    return g_dsp->getRemoveDC();
}

int spec_reset() {
    rp_spectr_worker_change_state(AUTO_STATE);
    return 0;
}

int spec_SetEnable(rp_channel_t channel, bool state) {
    if (!g_dsp)
        return -1;
    g_dsp->setChannel(channel, state);
    return 0;
}

int spec_GetEnable(rp_channel_t channel, bool* state) {
    if (!g_dsp)
        return -1;
    g_dsp->getChannel(channel, state);
    return 0;
}

int spec_getViewData(const rp_dsp_api::rp_dsp_result_t** data) {
    *data = nullptr;
    if (!g_dsp)
        return -1;

    auto d = g_dsp->getStoredData();
    *data = &(d->m_converted);
    return 0;
}

int spec_getViewSize(size_t* size) {
    if (!g_dsp)
        return -1;
    *size = g_dsp->getOutSignalLength();
    return 0;
}

int spec_setFreqRange(float max_freq) {
    auto adc_rate = getADCRate();
    int decimation = adc_rate / (max_freq * 2 * NUM_SIGNAL_PERIODS);
    if (decimation < 16) {
        if (decimation >= 8)
            decimation = 8;
        else if (decimation >= 4)
            decimation = 4;
        else if (decimation >= 2)
            decimation = 2;
        else
            decimation = 1;
    }
    if (decimation > 65536) {
        decimation = 65536;
    }

    current_freq_range = adc_rate / (decimation * 2);
    g_decimation = decimation;
    rp_spectr_worker_change_state(RESET_STATE);
    return RP_OK;
}

int spec_getFpgaFreq(float* freq) {
    *freq = current_freq_range;

    return 0;
}

int spec_setADCBufferSize(size_t size) {
    std::lock_guard lock(rp_spectr_buf_size_mutex);
    if (!g_dsp)
        return -1;

    if (g_dsp->setSignalLength(size) != 0) {
        ERROR_LOG("Wrong size %d", size);
    }

    if (g_dsp->window_init(g_dsp->getCurrentWindowMode()) < 0) {
        rp_spectr_worker_clean();
        return -1;
    }

    if (g_dsp->fftInit() < 0) {
        rp_spectr_worker_clean();
        return -1;
    }
    rp_spectr_worker_change_state(RESET_STATE);
    return 0;
}

int spec_getADCBufferSize() {
    if (!g_dsp)
        return ADC_BUFFER_SIZE;
    return g_dsp->getOutSignalLength();
}

int spec_getGetADCFreq() {
    auto adc_rate = getADCRate();
    return adc_rate / g_decimation;
}

int spec_setImpedance(double value) {
    if (value > 0) {
        g_dsp->setImpedance(value);
        return RP_OK;
    }
    return RP_EOOR;
}

int spec_getImpedance(double* value) {
    *value = g_dsp->getImpedance();
    return RP_OK;
}