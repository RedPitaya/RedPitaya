/**
 * $Id: $
 *
 * @brief Red Pitaya library Bode analyzer module interface
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */
#include <iostream>
#include <fstream>
#include <complex.h>
#include <limits.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <math.h>
#include <pthread.h>
#include "bodeApp.h"
#include <chrono>

#include "common.h"
#include "rp_hw-calib.h"
#include "rp_hw-profiles.h"
#include "rp_dsp.h"

using namespace std::chrono;

static auto adc_rate = rpApp_BaGetADCSpeed();
static auto adc_channels = rpApp_BaGetADCChannels();

typedef double data_t;

#define EXEC_CHECK_MUTEX(x, mutex){ \
 		int retval = (x); \
 		if(retval != RP_OK) { \
            pthread_mutex_unlock((&mutex)); \
 			return retval; \
 		} \
}


static std::vector<float> calib_data;
static pthread_mutex_t mutex;

rp_dsp_api::CDSP    g_dsp_logic(adc_channels,ADC_BUFFER_SIZE,adc_rate);


uint8_t rpApp_BaGetADCChannels(){
    uint8_t c = 0;
    if (rp_HPGetFastADCChannelsCount(&c) != RP_HP_OK){
        fprintf(stderr,"[Error] Can't get fast ADC channels count\n");
    }
    return c;
}

uint8_t rpApp_BaGetDACChannels(){
    uint8_t c = 0;
    if (rp_HPGetFastADCChannelsCount(&c) != RP_HP_OK){
        fprintf(stderr,"[Error] Can't get fast DAC channels count\n");
    }
    return c;
}

uint32_t rpApp_BaGetADCSpeed(){
    uint32_t c = 0;
    if (rp_HPGetBaseFastADCSpeedHz(&c) != RP_HP_OK){
        fprintf(stderr,"[Error] Can't get fast ADC speed\n");
    }
    return c;
}

data_t MEAN(const std::vector<float> &data, int size){
    data_t result = 0;
    for(int i = 0; i < size; i++){
        result += data[i];
    }
    result =  result / (double)size;
    return result;
}

data_t RMS(const std::vector<float> &data, int size){
    data_t result = 0;
    for(int i = 0; i < size; i++){
		int x = data[i] * 100000;
		double d = (double)x / 100000.0;
        result += d * d;
    }
    result =  sqrt(result / (double)size);
    return result;
}

data_t P_P(const std::vector<float> &data, int size){
    data_t result = 0;
    data_t min = data[0];
    data_t max = data[0];
    for(int i = 0; i < size; i++){
        min = min > data[i] ? data[i] : min;
        max = max < data[i] ? data[i] : max;
    }
    result =  max - min;
    return result;
}

float ba_trapezoidalApprox(double *data, float T, int size)
{
        double result = 0;

        for(int i = 0; i < size - 1; i++){
                result += data[i] + data[i+1];
        }
        result = ((T / 2.0) * result);
        return result;
}


float l_inter(float a, float b, float f){
    return a + f * (b - a);
}

data_t InterpolateLagrangePolynomial (data_t x, data_t* x_values, data_t* y_values, int size)
{
	data_t lagrange_pol = 0;
	data_t basics_pol;

	for (int i = 0; i < size; i++)
	{
		basics_pol = 1;
		for (int j = 0; j < size; j++)
		{
			if (j == i) continue;
			basics_pol *= (x - x_values[j])/(x_values[i] - x_values[j]);
		}
		lagrange_pol += basics_pol*y_values[i];
	}
	return lagrange_pol;
}

data_t calcPoint(data_t *a, data_t *b,int lenghtArray,int index){
	data_t x = 0;
	int j = 0;
	int i = index - lenghtArray > 0 ? index - lenghtArray : 0;
	for (; i < lenghtArray && i <= index; i++) {
		j = index - i;
		x +=a[i]*b[lenghtArray - j - 1];
	}
	return x;
}

data_t getMaxArg(data_t *a, data_t *b,int start,int stop, int step,int lenghtArray){
	int len = lenghtArray / step;
	if (stop == 0)  stop = len;

	std::vector<data_t> packBufA;
	packBufA.reserve(len);
	std::vector<data_t> packBufB;
	packBufB.reserve(len);
	std::vector<data_t> corralate;
	corralate.reserve(len);
	std::vector<int> corralateIndex;
	corralateIndex.reserve(len);

	// data_t packBufA[len];
	// data_t packBufB[len];
	// data_t corralate[len];
	// int    corralateIndex[len];

	for(auto k = 0; k < len ; k++){
		packBufA[k] = a[k*step];
		packBufB[k] = b[k*step];
		corralateIndex[k] = -1;
	}

	for(auto k = start; k <stop ; k++){
		corralate[k] = calcPoint(packBufA.data(),packBufB.data(),len,k);
		corralateIndex[k] = k;
	}
	int maxK = 0;
	int maxCor = 0;
	bool init = true;
	for(auto k = 0; k < len ; k++){
		if (corralateIndex[k] != -1)
			if (corralate[k] > maxCor || init){
				init = false;
				maxCor = corralate[k];
				maxK = k;
			}
	}
	return maxK * step;
}

data_t crossCorrelation(data_t *xSignalArray, data_t *ySignalArray, int lenghtArray,int sepm_Per)
{
	(void)(sepm_Per);
    data_t argmax = 0;
    data_t *a = xSignalArray;
	data_t *b = ySignalArray;
	int step = log2(lenghtArray);
	int maxK = -1;
	int start = 0;
	int stop  = 0;
	while(step>=1){
		maxK =getMaxArg(a,b,start/step,stop/step,step,lenghtArray);
		start = maxK - step * 10 < 0 ? 0 : maxK - step * 10;
		stop  = maxK + step * 10 > (lenghtArray * 2 + 1) ? (lenghtArray * 2 + 1) : maxK + step * 10;
		step /=2;
	}

	argmax = maxK;

	if (argmax > 0 && argmax < lenghtArray * 2){
		data_t x_axis[] = { 0 , 1 , 2};
		data_t y_axis[3];

		for(int i = argmax-1,j=0; i <= argmax+1 ;i++,j++){
			y_axis[j] = calcPoint(a,b,lenghtArray,i);
		}
		data_t eps = 0.0001;
		data_t start = 0;
		data_t stop = 2;
		data_t y_start = InterpolateLagrangePolynomial(start,x_axis,y_axis,3);
		data_t y_stop  = InterpolateLagrangePolynomial(stop,x_axis,y_axis,3);
		data_t max_inter = 0;
		while(eps  < (stop - start)){
			data_t z = (stop - start)/2.0 + start;
			data_t y_sub  = InterpolateLagrangePolynomial(z,x_axis,y_axis,3);
			if (y_sub > y_start || y_sub > y_stop){
				if (y_start > y_stop){
					stop = z;
					y_stop = y_sub;
				}else{
					start = z;
					y_start = y_sub;
				}
				max_inter = z;
			}
			else{
				max_inter = y_stop > y_start? stop : start;
				break;
			}
		}
		argmax = argmax-1 + max_inter;
	}
    return argmax;
}

data_t phaseCalculator(data_t freq_HZ, data_t samplesPerSecond, int numSamples,int sepm_Per, data_t *xSamples, data_t *ySamples)
{
    data_t timeShift, phaseShift, timeLine;
    auto argmax = crossCorrelation(xSamples, ySamples, numSamples,sepm_Per);

    timeLine = ((numSamples - 1) / samplesPerSecond);
    timeShift = ((timeLine * argmax) / (numSamples - 1)) + (-timeLine);
    phaseShift = ((2 * M_PI) * fmod((freq_HZ * timeShift), 1.0)) - M_PI;
    if (phaseShift <= -M_PI/2)
		phaseShift += M_PI;
	else if (phaseShift >= M_PI/2)
		phaseShift -= M_PI;
	return phaseShift;
}

int rpApp_BaDataAnalysisTrap(const rp_ba_buffer_t &buffer,
                                        uint32_t size,
                                        float _freq, // 2*pi*f
                                        int decimation,
                                        float *gain,
                                        float *phase_out,
                                        float input_threshold)
{

		float w_out = _freq*2 * M_PI;

        double ang,  phase;

        double T = ((double)decimation / (double)rpApp_BaGetADCSpeed());

		std::vector<double> u_dut_1;
		u_dut_1.reserve(size);
		std::vector<double> u_dut_2;
		u_dut_2.reserve(size);

		std::vector<double> u_dut_1_s[2];
		u_dut_1_s[0].reserve(size);
		u_dut_1_s[1].reserve(size);
		std::vector<double> u_dut_2_s[2];
		u_dut_2_s[0].reserve(size);
		u_dut_2_s[1].reserve(size);


        double component_lock_in[2][2];
        int ret_value = RP_OK;

        for (size_t i = 0; i < size; i++){
                u_dut_1[i] = buffer.ch2[i];
                u_dut_2[i] = buffer.ch1[i];
        }

        if (size > 0){
                double u1_max = u_dut_1[0];
                double u1_min = u_dut_1[0];
                double u2_max = u_dut_2[0];
                double u2_min = u_dut_2[0];
                for (size_t i = 1; i < size; i++){
                        if (u1_max < u_dut_1[i]) u1_max = u_dut_1[i];
                        if (u2_max < u_dut_2[i]) u2_max = u_dut_2[i];
                        if (u1_min > u_dut_1[i]) u1_min = u_dut_1[i];
                        if (u2_min > u_dut_2[i]) u2_min = u_dut_2[i];
                }
                if ((u1_max - u1_min) < input_threshold) ret_value = RP_EIPV;
                if ((u2_max - u2_min) < input_threshold) ret_value = RP_EIPV;
        }


        for (size_t i = 0; i < size; i++){
                ang = ((double)i * T * (double)w_out);

                u_dut_1_s[0][i] = u_dut_1[i] * sin(ang);
                u_dut_1_s[1][i] = u_dut_1[i] * sin(ang + (M_PI / 2));

                u_dut_2_s[0][i] = u_dut_2[i] * sin(ang);
                u_dut_2_s[1][i] = u_dut_2[i] * sin(ang + (M_PI / 2));
        }

        /* Trapezoidal approximation */
        component_lock_in[0][0] = ba_trapezoidalApprox(u_dut_1_s[0].data(), T, size); //U_X
        component_lock_in[0][1] = ba_trapezoidalApprox(u_dut_1_s[1].data(), T, size); //U_Y
        component_lock_in[1][0] = ba_trapezoidalApprox(u_dut_2_s[0].data(), T, size); //I_X
        component_lock_in[1][1] = ba_trapezoidalApprox(u_dut_2_s[1].data(), T, size); //I_Y

        /* Calculating voltage amplitude and phase */
        auto u_dut_1_ampl = 2.0 * (sqrtf(powf(component_lock_in[0][0], 2.0) + powf(component_lock_in[0][1], 2.0)));

        auto u_dut_1_phase = atan2f(component_lock_in[0][1], component_lock_in[0][0]);

        /* Calculating current amplitude and phase */
        auto u_dut_2_ampl = 2.0 * (sqrtf(powf(component_lock_in[1][0], 2.0) + powf(component_lock_in[1][1], 2.0)));

        auto u_dut_2_phase = atan2f(component_lock_in[1][1], component_lock_in[1][0]);

        phase = u_dut_1_phase - u_dut_2_phase;
        /* Phase has to be limited between M_PI and -M_PI. */
        if (phase <= -M_PI)
                phase += 2*M_PI;
        else if (phase >= M_PI)
                phase -= 2*M_PI;

        *phase_out = phase * (180.0 / M_PI);
		// Old logic
        *gain = u_dut_1_ampl / u_dut_2_ampl;

		// data_t sig1_rms =  RMS(buffer.ch1,size);
		// data_t sig2_rms =  RMS(buffer.ch2,size);
		// *gain = sig2_rms / sig1_rms;
        #ifdef TRACE_ENABLE
            auto mean1 = MEAN(buffer.ch1,buffer.ch1.size());
            auto mean2 = MEAN(buffer.ch2,buffer.ch2.size());
            auto pp1 = P_P(buffer.ch1,buffer.ch1.size());
            auto pp2 = P_P(buffer.ch2,buffer.ch2.size());

            TRACE_SHORT("Signal 1: Amplitude %.9f Phase %.9f Mean %f P-P %f",u_dut_1_ampl,u_dut_1_phase,mean1,pp1)
            TRACE_SHORT("Signal 2: Amplitude %.9f Phase %.9f Mean %f P-P %f",u_dut_2_ampl,u_dut_2_phase,mean2,pp2)
        #endif
        return ret_value;
}


int rpApp_BaDataAnalysisFFT(const rp_ba_buffer_t &buffer,
                                        uint32_t size,
                                        float _freq, // 2*pi*f
                                        int decimation,
                                        float *gain,
                                        float *phase_out,
                                        float input_threshold)
{

        int ret_value = RP_OK;

        if (size > 0){
                double u1_max = buffer.ch1[0];
                double u1_min = buffer.ch1[0];
                double u2_max = buffer.ch2[0];
                double u2_min = buffer.ch2[0];
                for (size_t i = 1; i < size; i++){
                        if (u1_max < buffer.ch1[i]) u1_max = buffer.ch1[i];
                        if (u2_max < buffer.ch2[i]) u2_max = buffer.ch2[i];
                        if (u1_min > buffer.ch1[i]) u1_min = buffer.ch1[i];
                        if (u2_min > buffer.ch2[i]) u2_min = buffer.ch2[i];
                }
                if ((u1_max - u1_min) < input_threshold) ret_value = RP_EIPV;
                if ((u2_max - u2_min) < input_threshold) ret_value = RP_EIPV;
        }

        auto data = g_dsp_logic.createData();
        g_dsp_logic.setSignalLengthDiv2(size);
        g_dsp_logic.window_init(rp_dsp_api::FLAT_TOP);
        g_dsp_logic.fftInit();

        g_dsp_logic.prepareFreqVector(data,adc_rate,decimation);
        for(size_t i = 0 ; i < size; i++){
            data->m_in[0][i] = buffer.ch1[i];
        }

        for(size_t i = 0 ; i < size; i++){
            data->m_in[1][i] = buffer.ch2[i];
        }

        g_dsp_logic.windowFilter(data);
        g_dsp_logic.fft(data);
        double amp[2];
        double phase[2];
        g_dsp_logic.getAmpAndPhase(data,_freq,&amp[0],&phase[0],&amp[1],&phase[1]);

        TRACE_SHORT("A1 %f A2 %f P1 %f P2 %f",amp[0],amp[1],phase[0] * 180 / M_PI ,phase[1] * 180 / M_PI);
        g_dsp_logic.deleteData(data);
        g_dsp_logic.fftClean();

        *phase_out = phase[0] - phase[1];
		*gain = amp[0]/amp[1];

        return ret_value;
}


int rpApp_BaDataAnalysis(const rp_ba_buffer_t &buffer,
					uint32_t size,
					float samplesPerSecond,
					float _freq,
					float samples_period,
					float *gain,
					float *phase_out,
					float input_threshold)
{
	int ret_value = RP_OK;
	std::vector<data_t> buf1;
	buf1.reserve(size);
	std::vector<data_t> buf2;
	buf2.reserve(size);
	// data_t buf1[size];
	// data_t buf2[size];
	data_t max_ch1 = -100000;
	data_t max_ch2 = -100000;
	data_t min_ch1 = 100000;
	data_t min_ch2 = 100000;

	for (size_t i = 0; i < size; i++){
		buf1[i] = buffer.ch1[i];
		buf2[i] = buffer.ch2[i];

		//
		if ((buf1[i]) > max_ch1) {
			max_ch1 = (buf1[i]);
		}
		if ((buf2[i]) > max_ch2) {
			max_ch2 = (buf2[i]);
		}
		if ((buf1[i]) < min_ch1) {
			min_ch1 = (buf1[i]);
		}
		if ((buf2[i]) < min_ch2) {
			min_ch2 = (buf2[i]);
		}
	}

	data_t sig1_rms =  RMS(buffer.ch1,size);
	data_t sig2_rms =  RMS(buffer.ch2,size);

	TRACE_SHORT("freq %f",_freq);
	TRACE_SHORT("RMS 1 %f min %f max %f",sig1_rms, min_ch1, max_ch1);
	TRACE_SHORT("RMS 2 %f min %f max %f",sig2_rms, min_ch2, max_ch2);


	*gain = sig2_rms / sig1_rms;

	if ((max_ch1 - min_ch1) < input_threshold) ret_value = RP_EIPV;
	if ((max_ch2 - min_ch2) < input_threshold) ret_value = RP_EIPV;
	auto phase2 = phaseCalculator(_freq,samplesPerSecond, size ,samples_period,buf1.data(),buf2.data());

	/* Phase has to be limited between M_PI and -M_PI. */
	if (phase2 <= -M_PI)
		phase2 += 2*M_PI;
	else if (phase2 >= M_PI)
		phase2 -= 2*M_PI;

	*phase_out = phase2 *  (180.0 / M_PI) ;
	return ret_value;
}



float rpApp_BaCalibGain(float _freq, float _ampl)
{
    for (size_t i = 3; i < calib_data.size(); i += 3) // 3 - freq, ampl, phase
    {
        if (calib_data[i] >= _freq)
        {
			float f0 = calib_data[i-3];
			float f1 = calib_data[i];
			float t =  (f1 - f0) != 0 ? (_freq - f0)/(f1 - f0) : 0;
            return _ampl - l_inter(calib_data[i - 3 + 1], calib_data[i + 1], t);
        }
    }

    return _ampl;
}

float rpApp_BaCalibPhase(float _freq, float _phase)
{
    for (size_t i = 3; i < calib_data.size(); i += 3) // 3 - freq, ampl, phase
    {
        if (calib_data[i] >= _freq)
        {
			float f0 = calib_data[i-3];
			float f1 = calib_data[i];
			float t =  (f1 - f0) != 0 ? (_freq - f0)/(f1 - f0) : 0;
            return _phase - l_inter(calib_data[i - 3 + 2], calib_data[i + 2], t);
        }
    }

    return _phase;
}


int rpApp_BaResetCalibration()
{
	calib_data.clear();
	remove(BA_CALIB_FILENAME);
    return RP_OK;
}

int rpApp_BaReadCalibration()
{
	int ignored __attribute__((unused));
    // if current mode != calibration then load calibration params
    if (calib_data.empty() && access(BA_CALIB_FILENAME, R_OK) == F_OK)
    {
        // fprintf(stderr, "Calibration data exist\n");
        FILE* calF = fopen(BA_CALIB_FILENAME, "r");
        fseek(calF, 0, SEEK_END);
        int size = ftell(calF);
        fseek(calF, 0, SEEK_SET);
        calib_data.resize(size/sizeof(float));
        ignored = fread((void*)calib_data.data(), sizeof(calib_data[0]), size/sizeof(calib_data[0]), calF);
        fclose(calF);
    }
    else
    {
        return RP_RCA;
    }
    return RP_OK;
}

int rpApp_BaWriteCalib(float _current_freq,float _amplitude,float _phase_out)
{
    FILE* calib_file = nullptr;
    calib_file = fopen(BA_CALIB_FILENAME, "a");
    if (calib_file == nullptr){
        return RP_RCA;
    }

    float data[] = {_current_freq, _amplitude, _phase_out};
    fwrite(data, sizeof(float), 3, calib_file);
    fclose(calib_file);
    calib_file = nullptr;
    return RP_OK;
}

bool rpApp_BaGetCalibStatus(){
	return !calib_data.empty();
}


/* Acquire functions. Callback to the API structure */
int rpApp_BaSafeThreadAcqPrepare()
{
	pthread_mutex_lock(&mutex);
	EXEC_CHECK_MUTEX(rp_AcqReset(), mutex);
	EXEC_CHECK_MUTEX(rp_AcqSetDecimation(RP_DEC_1), mutex);
	EXEC_CHECK_MUTEX(rp_AcqSetTriggerLevel(RP_T_CH_1, 0), mutex);
	EXEC_CHECK_MUTEX(rp_AcqSetTriggerLevel(RP_T_CH_2, 0), mutex);
	EXEC_CHECK_MUTEX(rp_AcqSetTriggerDelay(0), mutex);
	EXEC_CHECK_MUTEX(rp_AcqSetTriggerSrc(RP_TRIG_SRC_DISABLED), mutex);
	pthread_mutex_unlock(&mutex);
	return RP_OK;
}

/* Generate functions  */
int rpApp_BaSafeThreadGen(rp_channel_t _channel, float _frequency, float _ampl, float _dc_bias)
{
	pthread_mutex_lock(&mutex);
	EXEC_CHECK_MUTEX(rp_GenReset(), mutex);
	EXEC_CHECK_MUTEX(rp_GenAmp(_channel, _ampl), mutex); //LCR_AMPLITUDE
	EXEC_CHECK_MUTEX(rp_GenOffset(_channel, _dc_bias), mutex); // 0.25
	EXEC_CHECK_MUTEX(rp_GenWaveform(_channel, RP_WAVEFORM_SINE), mutex);
	EXEC_CHECK_MUTEX(rp_GenFreq(_channel, _frequency), mutex);
	EXEC_CHECK_MUTEX(rp_GenOutEnable(_channel), mutex);
	EXEC_CHECK_MUTEX(rp_GenResetTrigger(_channel), mutex);

	usleep(10000);
	pthread_mutex_unlock(&mutex);
	return RP_OK;
}


int rpApp_BaSafeThreadAcqData(rp_ba_buffer_t &_buffer, int _decimation, int _acq_size, float _trigger)
{
	(void)(_trigger);
	uint32_t pos = 0;
	uint32_t acq_u_size = _acq_size;
	//uint32_t acq_delay = acq_u_size > ADC_BUFFER_SIZE / 2.0 ? acq_u_size - ADC_BUFFER_SIZE / 2.0 : 0;
	uint64_t sleep_time = static_cast<uint64_t>(_acq_size) * _decimation / (rpApp_BaGetADCSpeed() / 1e6);
	sleep_time = sleep_time < 1 ? 1 : sleep_time;
	bool fillState = false;

	rp_acq_trig_state_t trig_state = RP_TRIG_STATE_TRIGGERED;

	pthread_mutex_lock(&mutex);
	EXEC_CHECK_MUTEX(rp_AcqSetDecimationFactor(_decimation), mutex);
	//EXEC_CHECK_MUTEX(rp_AcqSetTriggerDelay(ADC_BUFFER_SIZE / 2.0), mutex);
	EXEC_CHECK_MUTEX(rp_AcqSetTriggerDelay(-ADC_BUFFER_SIZE / 2.0 + acq_u_size), mutex);
	EXEC_CHECK_MUTEX(rp_AcqStart(), mutex);
	usleep(sleep_time);
	// Trigger, it is needed for the RP_DEC_1 decimation
	EXEC_CHECK_MUTEX(rp_AcqSetTriggerSrc(RP_TRIG_SRC_NOW), mutex);

	for (;;) {
		EXEC_CHECK_MUTEX(rp_AcqGetTriggerState(&trig_state), mutex);

		if (trig_state == RP_TRIG_STATE_TRIGGERED) {
			break;
		} else {
			usleep(1);
		}
	}

	while(!fillState){
		EXEC_CHECK_MUTEX(rp_AcqGetBufferFillState(&fillState), mutex);
	}

	EXEC_CHECK_MUTEX(rp_AcqStop(), mutex); // Why the write pointer is not stopped?
	EXEC_CHECK_MUTEX(rp_AcqGetWritePointerAtTrig(&pos), mutex);
	//pos++;
	buffers_t buf;
	buf.size = acq_u_size;
	buf.use_calib_for_volts = true;
	static uint8_t max_channels = rpApp_BaGetADCChannels();
    for(uint8_t ch = 0; ch < max_channels; ch++){
        buf.ch_f[ch] = NULL;
        buf.ch_d[ch] = NULL;
        buf.ch_i[ch] = NULL;
    }

	buf.ch_f[0] = _buffer.ch1.data();
	buf.ch_f[1] = _buffer.ch2.data();
	EXEC_CHECK_MUTEX(rp_AcqGetData(pos, &buf), mutex);
	pthread_mutex_unlock(&mutex);
	return RP_OK;
}

uint32_t increaseSmallBuffer(uint32_t _decimation, uint32_t _currentSize){
    int half_size = ADC_BUFFER_SIZE / 2;
    int rate = half_size / _currentSize;
    if (_decimation < 16){
        rate = rate / _decimation;
        rate = rate < 1 ? 1 : rate;
        _currentSize *= rate;
    }
    return _currentSize;
}

int rpApp_BaGetAmplPhase(float _amplitude_in, float _dc_bias, int _periods_number, rp_ba_buffer_t &_buffer, float* _amplitude, float* _phase, float _freq,float _input_threshold)
{
    float gain = 0;
    float phase_out = 0;
    int acq_size;

    //Generate a sinusoidal wave form
    rpApp_BaSafeThreadGen(RP_CH_1, _freq, _amplitude_in, _dc_bias);
    int size_buff_limit = ADC_BUFFER_SIZE / 4;
	int sampls = size_buff_limit / _periods_number;
	int decimation = adc_rate / (sampls * _freq);
	decimation = decimation < 1 ? 1 : decimation;

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

	acq_size = round((static_cast<float>(_periods_number) * rpApp_BaGetADCSpeed()) / (_freq * decimation));
    auto new_acq_size = increaseSmallBuffer(decimation,acq_size);
    TRACE_SHORT("Decimation: %d Gen freq: %f Buffer size %d Increase size %d",decimation,_freq,acq_size,new_acq_size);

    rpApp_BaSafeThreadAcqData(_buffer,decimation, new_acq_size,_amplitude_in);
    rp_GenOutDisable(RP_CH_1);
    // int ret = rp_BaDataAnalysis(_buffer, acq_size, ADC_SAMPLE_RATE / decimation,_freq, samples_period,  &gain, &phase_out,_input_threshold);
	int ret = rpApp_BaDataAnalysisTrap(_buffer, new_acq_size, _freq, decimation,  &gain, &phase_out,_input_threshold);
	//int ret = rpApp_BaDataAnalysisFFT(_buffer, acq_size, _freq, decimation,  &gain, &phase_out,_input_threshold);

    *_amplitude = 20.*log10f(gain);
    *_phase = phase_out;
    TRACE_SHORT("Gain %f Diff phase %f",*_amplitude,*_phase)

	if (std::isnan(*_amplitude) || std::isinf(*_amplitude)) ret =  RP_EOOR;
    return ret;
}
