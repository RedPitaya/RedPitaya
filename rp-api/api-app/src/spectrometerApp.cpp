/**
 * $Id: worker.c 881 2013-12-16 05:37:34Z rp_jmenart $
 *
 * @brief Red Pitaya Spectrum Analyzer worker.
 *
 * @Author Jure Menart <juremenart@gmail.com>
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/time.h>
#include <atomic>
#include <mutex>

#include "spectrometerApp.h"
#include "common.h"
#include "version.h"
#include "math/rp_math.h"


typedef enum rp_spectr_worker_state_e {
    IDLE_STATE = 0, /* do nothing */
    QUIT_STATE, /* shutdown worker */
    RESET_STATE, /* reset current measurement */
    AUTO_STATE, /* auto mode acquisition */
    NONEXISTS_STATE /* must be last */
} rp_spectr_worker_state_t;

/* Worker results (not signal but calculated peaks and jpeg index */
typedef struct rp_spectr_worker_res_s {
    float peak_pw_ch[MAX_ADC_CHANNELS];
    float peak_pw_freq_ch[MAX_ADC_CHANNELS];
} rp_spectr_worker_res_t;

/* Output signals */
// 0 - Xaxis; 1 - Ch 1; 2 - Ch 2; 3 - Ch 3; 4 - Ch 4
#define SPECTR_OUT_SIG_NUM   (MAX_ADC_CHANNELS + 1)

int rp_spectr_get_signals_channel(float **signals, size_t size);
int rp_spectr_get_params(rp_spectr_worker_res_t *result);

/* Internal helper functions */
int  rp_create_signals(float ***a_signals);
void rp_cleanup_signals(float ***a_signals);
int rp_spectr_worker_init();
int rp_spectr_worker_clean(void);
int rp_spectr_worker_exit(void);
int rp_spectr_worker_change_state(rp_spectr_worker_state_t new_state);
/* Returns:
 *  0 - new signals (dirty signal) are copied to the output
 *  1 - no new signals available (dirty signal was not set - we need to wait)
 */
int rp_spectr_get_signals(float ***signals, rp_spectr_worker_res_t *result);

/* Fills the output signal structure from temp one after calculation is done
 * and marks it dirty
 */
int rp_spectr_set_signals(float *source_freq,float **source, rp_spectr_worker_res_t result);


pthread_t *rp_spectr_thread_handler = NULL;
void *rp_spectr_worker_thread(void *args);
void clearAll();

/* Signals directly pointing at the FPGA mem space */
//int                  *rp_fpga_cha_signal, *rp_fpga_chb_signal;

double              **rp_ch_in = NULL;
double              **rp_ch_fft = NULL;
float               **rp_spectr_signals = NULL;
float               **rp_tmp_signals = NULL;

/* Parameters & signals communicating with 'external world' */
std::mutex                        rp_spectr_ctrl_mutex;
volatile rp_spectr_worker_state_t rp_spectr_ctrl;


int                    g_decimation = 1;
std::mutex             rp_spectr_sig_mutex;
std::mutex             rp_spectr_window_mutex;
std::mutex             rp_spectr_buf_size_mutex;
rp_spectr_worker_res_t rp_spectr_result;
int                    rp_spectr_signals_dirty = 0;

rp_dsp_api::CDSP      *g_dsp;
rp_dsp_api::data_t *g_data;

static float freq_min, freq_max, current_freq_range;

template<typename T>
auto createArray(uint32_t count,uint32_t signalLen) -> T** {
    try{
        auto arr = new T*[count];
        for(uint32_t i = 0; i < count; i++){
            arr[i] = new T[signalLen];
        }
        return arr;
    }catch (const std::bad_alloc& e) {
        ERROR_LOG("Can not allocate memory");
        return nullptr;
    }
}

template<typename T>
auto deleteArray(uint32_t count,T **arr) -> void {
    if (!arr) return;
    for(uint32_t i = 0; i < count; i++){
        delete[] arr[i];
    }
    delete[] arr;
}

int rp_spectr_worker_init()
{
    std::lock_guard<std::mutex> lock(rp_spectr_sig_mutex);
    int ret_val;
    rp_spectr_ctrl               = IDLE_STATE;

    clearAll();
    auto adc_channels = getADCChannels();
    auto rate = getADCRate();
    g_dsp = new rp_dsp_api::CDSP(adc_channels,ADC_BUFFER_SIZE,rate);
    g_data = g_dsp->createData();
    rp_spectr_signals = createArray<float>(SPECTR_OUT_SIG_NUM,g_dsp->getOutSignalMaxLength());

    if(!g_data) {
        clearAll();
        return -1;
    }

    if(g_dsp->window_init(rp_dsp_api::HANNING)  < 0) {
        clearAll();
        return -1;
    }

    if(g_dsp->fftInit() < 0) {
        clearAll();
        return -1;
    }

    rp_spectr_thread_handler = (pthread_t *)malloc(sizeof(pthread_t));
    if(rp_spectr_thread_handler == NULL) {
        clearAll();
        return -1;
    }

    ret_val =
        pthread_create(rp_spectr_thread_handler, NULL,
                       rp_spectr_worker_thread, NULL);
    if(ret_val != 0) {
        clearAll();
        ERROR_LOG("pthread_create() failed: %s\n",strerror(errno));
        return -1;
    }
    return 0;
}

void clearAll(){
    deleteArray<float>(SPECTR_OUT_SIG_NUM,rp_spectr_signals);
    if (g_dsp){
        g_dsp->deleteData(g_data);
        g_data = nullptr;
    }
    delete g_dsp;
    g_dsp = nullptr;

}

int rp_spectr_worker_clean(void)
{
    std::lock_guard<std::mutex> lock(rp_spectr_sig_mutex);
    clearAll();
    return 0;
}

int rp_spectr_worker_exit(void)
{
    int ret_val = 0;

    rp_spectr_worker_change_state(QUIT_STATE);
    if(rp_spectr_thread_handler) {
        ret_val = pthread_join(*rp_spectr_thread_handler, NULL);
        free(rp_spectr_thread_handler);
        rp_spectr_thread_handler = NULL;
    }
    if(ret_val != 0) {
        ERROR_LOG("pthread_join() failed: %s\n",
                strerror(errno));
    }
    rp_spectr_worker_clean();
    return 0;
}

int rp_spectr_worker_change_state(rp_spectr_worker_state_t new_state)
{
    std::lock_guard lock(rp_spectr_ctrl_mutex);
    if(new_state >= NONEXISTS_STATE)
        return -1;
    rp_spectr_ctrl = new_state;
    return 0;
}

int rp_spectr_get_signals_channel(float** signals, size_t size)
{
    std::lock_guard lock(rp_spectr_sig_mutex);
    if(rp_spectr_signals_dirty == 0) {
        return -1;
    }

	for (auto i = 0u; i < SPECTR_OUT_SIG_NUM; ++i)
		memcpy_neon(signals[i], rp_spectr_signals[i], sizeof(float)*size);

    rp_spectr_signals_dirty = 0;
    return 0;
}

int rp_spectr_get_params(rp_spectr_worker_res_t *result)
{
    std::lock_guard lock(rp_spectr_sig_mutex);
    auto adc_channels = getADCChannels();
    for(auto ch = 0u; ch < adc_channels;ch++){
        result->peak_pw_ch[ch] = rp_spectr_result.peak_pw_ch[ch];
        result->peak_pw_freq_ch[ch] = rp_spectr_result.peak_pw_freq_ch[ch];
    }
    return 0;
}

int rp_spectr_set_signals(float *source_freq,float **source, rp_spectr_worker_res_t result)
{
    std::lock_guard lock(rp_spectr_sig_mutex);
    if (!g_dsp) return -1;

    memcpy_neon(rp_spectr_signals[0],source_freq, sizeof(float) * g_dsp->getOutSignalLength());

    auto adc_channels = getADCChannels();

    for (auto i = 0u; i < adc_channels; ++i){
        memcpy_neon(rp_spectr_signals[i+1], source[i], sizeof(float) * g_dsp->getOutSignalLength());
    }

    rp_spectr_signals_dirty = 1;

    for(auto ch = 0u; ch < adc_channels;ch++){
        rp_spectr_result.peak_pw_ch[ch] = result.peak_pw_ch[ch];
        rp_spectr_result.peak_pw_freq_ch[ch] = result.peak_pw_freq_ch[ch];
    }
    return 0;
}

void *rp_spectr_worker_thread(void *args)
{
    rp_spectr_worker_state_t old_state;
    int                      params_dirty = 1;
    rp_spectr_worker_res_t   tmp_result;
    int                      current_decimation = 1;
    uint32_t                 buffer_size = 0;
    auto adc_rate = getADCRate();

    while(1) {
        /* update states - we save also old state to see if we need to reset
         * FPGA
         */


        old_state = rp_spectr_ctrl;

        /* request to stop worker thread, we will shut down */
        if(rp_spectr_ctrl == QUIT_STATE) {
            return 0;
        }
        if(rp_spectr_ctrl == IDLE_STATE) {
            usleep(1000);
            continue;
        }
        // pthread_mutex_unlock(&rp_spectr_ctrl_mutex);
        if(rp_spectr_ctrl == RESET_STATE) {
            rp_AcqResetFpga();
            rp_spectr_worker_change_state(AUTO_STATE);
            continue;
        }
        /* Start the writting machine */
        current_decimation = g_decimation;
        buffer_size = g_dsp->getSignalLength();
        rp_AcqSetDecimationFactor(current_decimation);
        rp_AcqSetTriggerDelay(buffer_size - ADC_BUFFER_SIZE / 2.0);
        rp_AcqStart();
        rp_AcqSetTriggerSrc(RP_TRIG_SRC_NOW);

        // /* start working */

        rp_acq_trig_state_t stateTrig = RP_TRIG_STATE_WAITING;
        /* polling until data is ready */
        while(1) {
            /* change in state, abort polling */
            if((rp_spectr_ctrl != old_state)) {
                break;
            }

            if(rp_spectr_ctrl == QUIT_STATE) {
                return 0;
            }

            if (rp_AcqGetTriggerState(&stateTrig)){
                return 0;
            }

            if(stateTrig == RP_TRIG_STATE_TRIGGERED){
                break;
            }
        }
        bool fillState = false;
        while(!fillState){
            rp_AcqGetBufferFillState(&fillState);
            if((rp_spectr_ctrl != old_state)) {
                break;
            }

            if(rp_spectr_ctrl == QUIT_STATE) {
                return 0;
            }
        }
        rp_AcqStop();

        if((rp_spectr_ctrl != old_state) || params_dirty) {
            if (rp_spectr_ctrl == RESET_STATE) {
                rp_AcqResetFpga();
                rp_spectr_worker_change_state(AUTO_STATE);
            }
            params_dirty = 0;
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
            for(auto z = 0 ; z < adc_channels; z++){
                buff_out.ch_d[z] = g_data->m_in[z];
                buff_out.ch_f[z] = NULL;
                buff_out.ch_i[z] = NULL;
            }

            rp_AcqGetDataWithCorrection(trig_pos,&buffer_size,0,&buff_out);

            /* retrieve data and process it*/

            g_dsp->prepareFreqVector(g_data,adc_rate,g_decimation);


            rp_spectr_window_mutex.lock();
            g_dsp->windowFilter(g_data);

            rp_spectr_window_mutex.unlock();

            g_dsp->fft(g_data);
            //float** start_y_data  = reinterpret_cast<float**>(&rp_tmp_signals[1]);
            g_dsp->decimate(g_data,g_dsp->getOutSignalLength(),g_dsp->getOutSignalLength());
            g_dsp->cnvToMetric(g_data,g_decimation);
            /* Copy the result to the output part */
            for(auto i = 0u; i < g_data->m_channels; i++){
                tmp_result.peak_pw_ch[i] = g_data->m_peak_power[i];
                tmp_result.peak_pw_freq_ch[i] = g_data->m_peak_freq[i];
            }

            rp_spectr_set_signals(g_data->m_freq_vector, g_data->m_converted, tmp_result);
        }
        usleep(10000);
    }

    return 0;
}

int rp_create_signals(float ***a_signals)
{
    int i;
    float **s;
    s = (float **)malloc(SPECTR_OUT_SIG_NUM * sizeof(float *));
    if(s == NULL) {
        return -1;
    }
    for(i = 0; i < SPECTR_OUT_SIG_NUM; i++)
        s[i] = NULL;

    for(i = 0; i < SPECTR_OUT_SIG_NUM; i++) {
        s[i] = (float *)malloc(g_dsp->getOutSignalMaxLength()  * sizeof(float));
        if(s[i] == NULL) {
            rp_cleanup_signals(a_signals);
            return -1;
        }
        memset(&s[i][0], 0, g_dsp->getOutSignalMaxLength() * sizeof(float));
    }
    *a_signals = s;

    return 0;
}

void rp_cleanup_signals(float ***a_signals)
{
    int i;
    float **s = *a_signals;

    if(s) {
        for(i = 0; i < SPECTR_OUT_SIG_NUM; i++) {
            if(s[i]) {
                free(s[i]);
                s[i] = NULL;
            }
        }
        free(s);
        *a_signals = NULL;
    }
}

int spec_run()
{
    if(rp_spectr_worker_init() < 0) {
	    ERROR_LOG("rp_spectr_worker_init failed");
        return -1;
    }
    auto adc_rate = getADCRate();

    rp_spectr_worker_change_state(AUTO_STATE);
	spec_setFreqRange(0, adc_rate / 2.0);
    return 0;
}

int spec_running() // true/false
{
	return rp_spectr_ctrl == AUTO_STATE;
}

int spec_stop()
{
    rp_spectr_worker_exit();
    return 0;
}

int spec_setWindow(rp_dsp_api::window_mode_t  mode){
    std::lock_guard lock(rp_spectr_window_mutex);
    if (!g_dsp) return -1;
    g_dsp->window_init(mode);
    rp_spectr_worker_change_state(RESET_STATE);
    return 0;
}

int spec_getWindow(rp_dsp_api::window_mode_t *mode){
    if (!g_dsp) return -1;
    *mode = g_dsp->getCurrentWindowMode();
    return RP_OK;
}

int spec_setProbe(rp_channel_t channel, uint32_t probe){
    if (!g_dsp) return -1;
    g_dsp->setProbe(channel,probe);
    return RP_OK;
}

int spec_getProbe(rp_channel_t channel, uint32_t* probe){
    if (!g_dsp) return -1;
    g_dsp->getProbe(channel,probe);
    return RP_OK;
}

int spec_setRemoveDC(bool state){
    if (!g_dsp) return -1;
    g_dsp->setRemoveDC(state);
    return 0;
}

int spec_getRemoveDC(){
    if (!g_dsp) return false;
    return g_dsp->getRemoveDC();
}

int spec_reset()
{
	rp_spectr_worker_change_state(AUTO_STATE);
	return 0;
}

int spec_getViewData(float **signals, size_t size)
{
    return rp_spectr_get_signals_channel(signals, size);
}

int spec_getViewSize(size_t *size){
    if (!g_dsp) return -1;
    *size = g_dsp->getOutSignalLength();
    return 0;
}


int spec_getPeakPower(rp_channel_t channel, float* power)
{
	rp_spectr_worker_res_t res;
	int ret = rp_spectr_get_params(&res);
	if (!ret)
	{
        if ((int)channel >= getADCChannels()) return -1;
		*power = res.peak_pw_ch[(int)channel];
	}

	return ret;
}

int spec_getPeakFreq(rp_channel_t channel, float* freq)
{
	rp_spectr_worker_res_t res;
	int ret = rp_spectr_get_params(&res);
	if (!ret)
	{
        if ((int)channel >= getADCChannels()) return -1;
		*freq = res.peak_pw_freq_ch[(int)channel];
	}

	return ret;
}

int spec_setFreqRange(float _freq_min, float freq)
{
    auto adc_rate = getADCRate();
    int decimation =     adc_rate /  (freq * 2);
    if (decimation < 16){
        if (decimation >= 8)
            decimation = 8;
        else
            if (decimation >= 4)
                decimation = 4;
            else
                if (decimation >= 2)
                    decimation = 2;
                else
                    decimation = 1;
    }
    if (decimation > 65536){
        decimation = 65536;
    }

	current_freq_range = adc_rate  / (decimation * 2);
	freq_max = freq;
	freq_min = _freq_min;
    g_decimation = decimation;
    rp_spectr_worker_change_state(RESET_STATE);
	return RP_OK;
}


int spec_getFpgaFreq(float* freq)
{
	*freq = current_freq_range;

	return 0;
}

int spec_getFreqMax(float* freq) {
	*freq = freq_max;
	return 0;
}

int spec_getFreqMin(float* freq) {
	*freq = freq_min;
	return 0;
}

int spec_setADCBufferSize(size_t size){
    std::lock_guard lock(rp_spectr_buf_size_mutex);
    if (!g_dsp) return -1;

    if (g_dsp->setSignalLength(size) != 0){
        ERROR_LOG("Wrong size %d",size);
    }

    if(g_dsp->window_init(g_dsp->getCurrentWindowMode()) < 0) {
        rp_spectr_worker_clean();
        return -1;
    }

    if(g_dsp->fftInit() < 0) {
        rp_spectr_worker_clean();
        return -1;
    }
    rp_spectr_worker_change_state(RESET_STATE);
    return 0;
}

int spec_getADCBufferSize(){
    if (!g_dsp) return ADC_BUFFER_SIZE;
    return g_dsp->getOutSignalLength();
}

int spec_getGetADCFreq(){
    auto adc_rate = getADCRate();
    return adc_rate / g_decimation;
}

int spec_getVoltMode(rp_dsp_api::mode_t *mode){
    if (!g_dsp) return -1;
    *mode = g_dsp->getMode();
    return 0;
}

int spec_setVoltMode(rp_dsp_api::mode_t mode){
    std::lock_guard lock(rp_spectr_buf_size_mutex);
    g_dsp->setMode(mode);
    rp_spectr_worker_change_state(RESET_STATE);
    return RP_OK;
}

int spec_setImpedance(double value){
    if (value > 0){
        g_dsp->setImpedance(value);
        return RP_OK;
    }
    return RP_EOOR;
}

int spec_getImpedance(double *value){
    *value = g_dsp->getImpedance();
    return RP_OK;
}