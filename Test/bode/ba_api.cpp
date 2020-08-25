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

float ba_trapezoidalApprox(double *data, float T, int size) 
{
	double result = 0;

	for(int i = 0; i < size - 1; i++){
		result += data[i] + data[i+1];
	}
	result = ((T / 2.0) * result);
	return result;
}


float l_inter(float a, float b, float f)
{
    return a + f * (b - a);
}

void ba_getDecimationValue(float frequency, rp_acq_decimation_t *api_dec, int *dec_val)
{
	const int g_dec[] = { 1,  8,  64,  1024,  8192,  65536 };
	int f = 0;

	if (frequency >= 160000) f = 0;
	else if (frequency >= 20000) f = 1;
	else if (frequency >= 2500) f = 2;
	else if (frequency >= 160) f = 3;
	else if (frequency >= 20) f = 4;
	else if (frequency >= 0) f = 5;

	*api_dec = (rp_acq_decimation_t)f;
	*dec_val = g_dec[f];
}

int rp_BaDataAnalysis(const rp_ba_buffer_t &buffer,
					uint32_t size,
					float w_out, // 2*pi*f
					int decimation,
					float *gain,
					float *phase_out,
					float input_threshold) 
{

	double ang, u_dut_1_ampl, u_dut_1_phase, u_dut_2_ampl, u_dut_2_phase, phase;

	double T = (decimation / ADC_SAMPLE_RATE);
	double u_dut_1[size];
	double u_dut_2[size];

	double u_dut_1_s[2][size];
	double u_dut_2_s[2][size];

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
		ang = (i * T * w_out);

		u_dut_1_s[0][i] = u_dut_1[i] * sin(ang);
		u_dut_1_s[1][i] = u_dut_1[i] * sin(ang + (M_PI / 2));

		u_dut_2_s[0][i] = u_dut_2[i] * sin(ang);
		u_dut_2_s[1][i] = u_dut_2[i] * sin(ang + (M_PI / 2));
	}

	/* Trapezoidal approximation */
	component_lock_in[0][0] = ba_trapezoidalApprox(u_dut_1_s[0], T, size); //U_X
	component_lock_in[0][1] = ba_trapezoidalApprox(u_dut_1_s[1], T, size); //U_Y
	component_lock_in[1][0] = ba_trapezoidalApprox(u_dut_2_s[0], T, size); //I_X
	component_lock_in[1][1] = ba_trapezoidalApprox(u_dut_2_s[1], T, size); //I_Y

	/* Calculating voltage amplitude and phase */
	u_dut_1_ampl = 2.0 * (sqrtf(powf(component_lock_in[0][0], 2.0) + powf(component_lock_in[0][1], 2.0)));

	u_dut_1_phase = atan2f(component_lock_in[0][1], component_lock_in[0][0]);

	/* Calculating current amplitude and phase */
	u_dut_2_ampl = 2.0 * (sqrtf(powf(component_lock_in[1][0], 2.0) + powf(component_lock_in[1][1], 2.0)));

	u_dut_2_phase = atan2f(component_lock_in[1][1], component_lock_in[1][0]);

	phase = u_dut_1_phase - u_dut_2_phase;

	/* Phase has to be limited between M_PI and -M_PI. */
	if (phase <= -M_PI) 
		phase += 2*M_PI;
	else if (phase >= M_PI) 
		phase -= 2*M_PI;

	*phase_out = phase * (180.0 / M_PI);
	*gain = u_dut_1_ampl / u_dut_2_ampl;

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
            //fprintf(stdout, "%f %f\n", _ampl, _ampl - l_inter(calib_data[i - 3 + 1], calib_data[i + 1], _ampl));
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


int rp_BaSafeThreadAcqData(rp_ba_buffer_t &_buffer, rp_acq_decimation_t _decimation, int _acq_size, int _dec, float _trigger)
{
	uint32_t pos = 0;
	uint32_t acq_u_size = _acq_size;
	uint32_t acq_delay = acq_u_size > ADC_BUFFER_SIZE / 2.0 ? acq_u_size - ADC_BUFFER_SIZE / 2.0 : 0; 
	uint64_t sleep_time = static_cast<uint64_t>(_acq_size) * _dec / (ADC_SAMPLE_RATE / 1e6);
	sleep_time = sleep_time < 1 ? 1 : sleep_time;
	bool fillState = false;
	sleep_time += 1;

	rp_acq_trig_state_t trig_state = RP_TRIG_STATE_TRIGGERED;

	pthread_mutex_lock(&mutex);
	EXEC_CHECK_MUTEX(rp_AcqSetDecimation(_decimation), mutex);
	EXEC_CHECK_MUTEX(rp_AcqSetTriggerDelay(acq_delay), mutex);
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
	EXEC_CHECK_MUTEX(rp_AcqGetWritePointer(&pos), mutex);

	// Calculate the beginning of the buffer
	if (pos >= acq_u_size) {
		pos -= acq_u_size;
	} else {
		pos = ADC_BUFFER_SIZE - acq_u_size + pos;
	}

	EXEC_CHECK_MUTEX(rp_AcqGetDataV2(pos, &acq_u_size, _buffer.ch1.data(), _buffer.ch2.data()), mutex);
	pthread_mutex_unlock(&mutex);
	return RP_OK;
}


int rp_BaGetAmplPhase(float _amplitude_in, float _dc_bias, int _periods_number, rp_ba_buffer_t &_buffer, float* _amplitude, float* _phase, float _freq,float _input_threshold)
{
    float gain = 0;
    float phase_out = 0;

    float w_out;
    int decimation;
    int acq_size;

    rp_acq_decimation_t api_decimation;
    ba_getDecimationValue(_freq, &api_decimation, &decimation);

    //Calculate output angular velocity
    w_out = _freq*2 * M_PI;

    //Generate a sinusoidal wave form
    rp_BaSafeThreadGen(RP_CH_1, _freq, _amplitude_in, _dc_bias);
    acq_size = round((static_cast<float>(_periods_number) * ADC_SAMPLE_RATE) / (_freq * decimation));
   //    fprintf(stderr,"Sample_rate %f %d\n",SAMPLE_RATE,acq_size);
    rp_BaSafeThreadAcqData(_buffer, api_decimation, acq_size, decimation,_amplitude_in);
    rp_GenOutDisable(RP_CH_1);
    int ret = rp_BaDataAnalysis(_buffer, acq_size, w_out, decimation, &gain, &phase_out,_input_threshold);

    *_amplitude = 10.*logf(gain);
    *_phase = phase_out;
    //fprintf(stderr, "freq = %f dbm = %f phase = %f \n", _freq, *_amplitude, phase_out);
	if (std::isnan(*_amplitude) || std::isinf(*_amplitude)) ret =  RP_EOOR;
    return ret;
}

