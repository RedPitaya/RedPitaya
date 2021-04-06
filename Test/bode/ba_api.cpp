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
#include <complex.h>
#include <limits.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <math.h>
#include <pthread.h>
//#include <mutex>
#include "ba_api.h"


#define EXEC_CHECK_MUTEX(x, mutex){ \
 		int retval = (x); \
 		if(retval != RP_OK) { \
            pthread_mutex_unlock((&mutex)); \
 			return retval; \
 		} \
}


static std::vector<float> calib_data;
static pthread_mutex_t mutex;

double RMS(const std::vector<float> &data, int size){
    double result = 0;
    for(int i = 0; i < size; i++){
        result += data[i] * data[i];
    }
    result =  sqrt(result / size);
    return result;
}


float l_inter(float a, float b, float f)
{
    return a + f * (b - a);
}

double InterpolateLagrangePolynomial (double x, double* x_values, double* y_values, int size)
{
	double lagrange_pol = 0;
	double basics_pol;

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



std::pair<double, double> crossCorrelation(double *xSignalArray, double *ySignalArray, int lenghtArray,int sepm_Per)
{
    double max_value = -100000.0;
    double argmax = 0;
    // Stores the final array
    double *crossCorrelate = (double*)malloc(((lenghtArray * 2) + 1) * sizeof(double));
    memset(crossCorrelate, 0, ((lenghtArray * 2) + 1) * sizeof(double));
   
	double *a = xSignalArray;
	double *b = ySignalArray;
	int start_k = lenghtArray - sepm_Per < 0 ? 0 : lenghtArray - sepm_Per;
	int end_k = lenghtArray + sepm_Per > lenghtArray * 2 ? lenghtArray * 2 : lenghtArray + sepm_Per;

	for(int k = start_k ; k < end_k; k++){
		for (int i = 0; i < lenghtArray; i++) {
			int j = k - i;
			if(j < 0 || j >= lenghtArray) continue;
			crossCorrelate[k] +=a[i]*b[lenghtArray - j - 1]; 

		}
	}

	for(int i = 0 ; i < (lenghtArray * 2) ; ++i){
        if ((crossCorrelate[i]) > max_value)
        {
            max_value = (crossCorrelate[i]);
            argmax = i;
        }
	}

	if (argmax > 0 && argmax < lenghtArray * 2 ){
		double x_axis[] = { 0 , 1 , 2};
		double y_axis[3];
		y_axis[0] = crossCorrelate[(int)argmax - 1];
		y_axis[1] = crossCorrelate[(int)argmax];
		y_axis[2] = crossCorrelate[(int)argmax + 1];
		double eps = 0.0001;
		double start = 0;
		double stop = 2;
		double y_start = InterpolateLagrangePolynomial(start,x_axis,y_axis,3);
		double y_stop  = InterpolateLagrangePolynomial(stop,x_axis,y_axis,3);
		double max_inter = 0;
		while(eps  < (stop - start)){
			double z = (stop - start)/2.0 + start;
			double y_sub  = InterpolateLagrangePolynomial(z,x_axis,y_axis,3);
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

    free(crossCorrelate);
    return std::make_pair(max_value, argmax);
}

double phaseCalculator(double freq_HZ, double samplesPerSecond, int numSamples,int sepm_Per, double *xSamples, double *ySamples)
{
    double timeShift, phaseShift, timeLine;
    double argmax;
    auto result = crossCorrelation(xSamples, ySamples, numSamples,sepm_Per);
    argmax = result.second;

    timeLine = ((numSamples - 1) / samplesPerSecond);
    timeShift = ((timeLine * argmax) / (numSamples - 1)) + (-timeLine);
    phaseShift = ((2 * M_PI) * fmod((freq_HZ * timeShift), 1.0)) - M_PI;
    if (phaseShift <= -M_PI/2) 
		phaseShift += M_PI;
	else if (phaseShift >= M_PI/2) 
		phaseShift -= M_PI;
	return phaseShift;
}

int rp_BaDataAnalysis(const rp_ba_buffer_t &buffer,
					uint32_t size,
					float samplesPerSecond,
					float _freq, 
					int   samples_period,
					float *gain,
					float *phase_out,
					float input_threshold) 
{
	int ret_value = RP_OK;
	double buf1[size];
	double buf2[size];
	double max_ch1 = -100000;
	double max_ch2 = -100000;
	double min_ch1 = 100000;
	double min_ch2 = 100000;

	for (size_t i = 0; i < size; i++){
		buf1[i] = buffer.ch1[i];
		buf2[i] = buffer.ch2[i];
		// Filtring 
		//buf1[i]  = buf1[i] * ( 0.54 - 0.46 * cos(2*M_PI*i / (double)(size-1)));
		//buf2[i]  = buf2[i] * ( 0.54 - 0.46 * cos(2*M_PI*i / (double)(size-1)));

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

	double sig1_rms =  RMS(buffer.ch1,size);
	double sig2_rms =  RMS(buffer.ch2,size);
	*gain = sig2_rms / sig1_rms;
	
	if ((max_ch1 - min_ch1) < input_threshold) ret_value = RP_EIPV;
	if ((max_ch2 - min_ch2) < input_threshold) ret_value = RP_EIPV;
	auto phase2 = phaseCalculator(_freq,samplesPerSecond, size ,samples_period,buf1,buf2);

	/* Phase has to be limited between M_PI and -M_PI. */
	if (phase2 <= -M_PI) 
		phase2 += 2*M_PI;
	else if (phase2 >= M_PI) 
		phase2 -= 2*M_PI;

	*phase_out = phase2 *  (180.0 / M_PI) ;
	return ret_value;
}



float rp_BaCalibGain(float _freq, float _ampl)
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

float rp_BaCalibPhase(float _freq, float _phase)
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


int rp_BaResetCalibration()
{
	calib_data.clear();
	remove(BA_CALIB_FILENAME);
    return RP_OK;
}

int rp_BaReadCalibration()
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

int rp_BaWriteCalib(float _current_freq,float _amplitude,float _phase_out)
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

bool rp_BaGetCalibStatus(){
	return !calib_data.empty();
}


/* Acquire functions. Callback to the API structure */
int rp_BaSafeThreadAcqPrepare()
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
int rp_BaSafeThreadGen(rp_channel_t _channel, float _frequency, float _ampl, float _dc_bias)
{
	pthread_mutex_lock(&mutex);
	EXEC_CHECK_MUTEX(rp_GenAmp(_channel, _ampl), mutex); //LCR_AMPLITUDE
	EXEC_CHECK_MUTEX(rp_GenOffset(_channel, _dc_bias), mutex); // 0.25
	EXEC_CHECK_MUTEX(rp_GenWaveform(_channel, RP_WAVEFORM_SINE), mutex);
	EXEC_CHECK_MUTEX(rp_GenFreq(_channel, _frequency), mutex);
	EXEC_CHECK_MUTEX(rp_GenOutEnable(_channel), mutex);
	usleep(10000);
	pthread_mutex_unlock(&mutex);
	return RP_OK;
}


int rp_BaSafeThreadAcqData(rp_ba_buffer_t &_buffer, int _decimation, int _acq_size, float _trigger)
{
	uint32_t pos = 0;
	uint32_t acq_u_size = _acq_size;
	//uint32_t acq_delay = acq_u_size > ADC_BUFFER_SIZE / 2.0 ? acq_u_size - ADC_BUFFER_SIZE / 2.0 : 0; 
	uint64_t sleep_time = static_cast<uint64_t>(_acq_size) * _decimation / (ADC_SAMPLE_RATE / 1e6);
	sleep_time = sleep_time < 1 ? 1 : sleep_time;
	bool fillState = false;
	
	rp_acq_trig_state_t trig_state = RP_TRIG_STATE_TRIGGERED;

	pthread_mutex_lock(&mutex);
	EXEC_CHECK_MUTEX(rp_AcqSetDecimationFactor(_decimation), mutex);
	EXEC_CHECK_MUTEX(rp_AcqSetTriggerDelay(ADC_BUFFER_SIZE / 2.0), mutex);
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
	pos++;
	EXEC_CHECK_MUTEX(rp_AcqGetDataV2(pos, &acq_u_size, _buffer.ch1.data(), _buffer.ch2.data()), mutex);
	pthread_mutex_unlock(&mutex);
	return RP_OK;
}


int rp_BaGetAmplPhase(float _amplitude_in, float _dc_bias, int _periods_number, rp_ba_buffer_t &_buffer, float* _amplitude, float* _phase, float _freq,float _input_threshold)
{
    float gain = 0;
    float phase_out = 0;
    int acq_size;

    //Generate a sinusoidal wave form
    rp_BaSafeThreadGen(RP_CH_1, _freq, _amplitude_in, _dc_bias);
    int size_buff_limit = ADC_BUFFER_SIZE;
	int new_dec = round((static_cast<float>(_periods_number) * ADC_SAMPLE_RATE) / (_freq * size_buff_limit));
	new_dec = new_dec < 1 ? 1 : new_dec;
	
	if (new_dec < 16){
        if (new_dec >= 8)
            new_dec = 8;
        else
            if (new_dec >= 4)
                new_dec = 4;
            else
                if (new_dec >= 2)
                    new_dec = 2;
                else 
                    new_dec = 1;
    }
    if (new_dec > 65536){
        new_dec = 65536;
    }
	int samples_period = round(ADC_SAMPLE_RATE / (_freq * new_dec));

    acq_size = size_buff_limit;
    rp_BaSafeThreadAcqData(_buffer,new_dec, acq_size,_amplitude_in);
    rp_GenOutDisable(RP_CH_1);
    int ret = rp_BaDataAnalysis(_buffer, acq_size, ADC_SAMPLE_RATE / new_dec,_freq, samples_period,  &gain, &phase_out,_input_threshold);
	

    *_amplitude = 10.*logf(gain);
    *_phase = phase_out;
	if (std::isnan(*_amplitude) || std::isinf(*_amplitude)) ret =  RP_EOOR;
    return ret;
}

