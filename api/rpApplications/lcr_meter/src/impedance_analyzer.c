/**
* $Id: $
*
* @brief Red Pitaya application Impedance analyzer module interface
*
* @Author Luka Golinar <luka.golinar@gmail.com>
*
* (c) Red Pitaya  http://www.redpitaya.com
*
* This part of code is written in C programming language.
* Please visit http://en.wikipedia.org/wiki/C_(programming_language)
* for more details on the language used herein.
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <math.h>

#include "impedance_analyzer.h"
#include "utils.h"
#include "calib.h"
#include "../../rpbase/src/common.h"
#include "./common.h"
#include "redpitaya/rp.h"

/* Global variables definition */
int 					min_periodes = 20;
uint32_t 				acq_size = 1024;

pthread_mutex_t 		mutex;
pthread_t 				*imp_thread_handler = NULL;

/* Init impedance params struct */
params_t main_params = 
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, false, false};

/* Decimation constants */
static const uint32_t IMP_DEC_1		= 1;
static const uint32_t IMP_DEC_8		= 8;
static const uint32_t IMP_DEC_64	= 64;
static const uint32_t IMP_DEC_1024  = 1024;
static const uint32_t IMP_DEC_8192	= 8192;
static const uint32_t IMP_DEC_65536 = 65536;

/* Init the main API structure */
int imp_Init(){

	/* Init mutex thread */
	if(pthread_mutex_init(&mutex, NULL)){
		fprintf(stderr, "Failed to init thread: %s\n", strerror(errno));
	}

	pthread_mutex_lock(&mutex);

	if(rp_Init() != RP_OK){
		fprintf(stderr, "Unable to inicialize the RPI API structure "
			"needed by impedance analyzer application: %s\n", strerror(errno));
		return RP_EOOR;
	}
	
	/* Set default values of the impedance structure */
	imp_SetDefaultValues();

	/* Set some default values */
	ECHECK_APP(rp_AcqReset());
	ECHECK_APP(rp_GenReset());
	ECHECK_APP(rp_AcqSetTriggerLevel(0.1));
	ECHECK_APP(rp_AcqSetTriggerDelay(8192));

	pthread_mutex_unlock(&mutex);
	return RP_OK;
}

/* Release resources used the main API structure */
int imp_Release(){
	pthread_mutex_lock(&mutex);
	if(rp_Release() != RP_OK){
		fprintf(stderr, "Unable to release resources used by " 
			"Impedance analyzer meter API: %s\n", strerror(errno));
		return RP_EOOR;
	}

	pthread_mutex_unlock(&mutex);
	pthread_mutex_destroy(&mutex);
	return RP_OK;
}

/* Set default values of all rpi resources */
int imp_Reset(){
	pthread_mutex_lock(&mutex);
	rp_Reset();
	/* Set default values of the lcr_params structure */
	imp_SetDefaultValues(main_params);
	pthread_mutex_unlock(&mutex);
	return RP_OK;
}

int imp_SetDefaultValues(){
	ECHECK_APP(imp_SetAmplitude(1));
	ECHECK_APP(imp_SetDcBias(0));
	ECHECK_APP(imp_SetAveraging(1));
	ECHECK_APP(_setRShunt(R_SHUNT_7_5K));
	ECHECK_APP(imp_SetCalibMode(CALIB_NONE));
	ECHECK_APP(imp_SetRefReal(0.0));
	ECHECK_APP(imp_SetRefImg(0.0));
	ECHECK_APP(imp_SetSteps(0));
	ECHECK_APP(imp_SetStartFreq(1000.0));
	ECHECK_APP(imp_SetEndFreq(20000.0));
	ECHECK_APP(imp_SetScaleType(SCALE_LINEAR));
	ECHECK_APP(imp_SetSweepMode(FREQUENCY_SWEEP));
	ECHECK_APP(imp_SetUserWait(false));
	ECHECK_APP(imp_SetNoCalibration(true));

	return RP_OK;
}

/* Generate functions  */
int _SafeThreadGen(rp_channel_t channel, float ampl, float freq){

	pthread_mutex_lock(&mutex);
	ECHECK_APP(rp_GenFreq(channel, freq));
	ECHECK_APP(rp_GenAmp(channel, ampl));
	ECHECK_APP(rp_GenWaveform(channel, RP_WAVEFORM_SINE));
	ECHECK_APP(rp_GenOutEnable(channel));
	pthread_mutex_unlock(&mutex);

	return RP_OK;
}

/* Acquire functions. Callback to the API structure */
int _SafeThreadAcqData(rp_channel_t channel, 
	float *data, rp_acq_decimation_t decimation){

	pthread_mutex_lock(&mutex);	
	
	rp_acq_trig_state_t state = RP_TRIG_STATE_WAITING;
	
	ECHECK_APP(rp_AcqSetDecimation(decimation));
	ECHECK_APP(rp_AcqSetTriggerSrc(RP_TRIG_SRC_CHA_PE));
	ECHECK_APP(rp_AcqStart());

	while(state == RP_TRIG_STATE_WAITING){
		ECHECK_APP(rp_AcqGetTriggerState(&state));
	}

	ECHECK_APP(rp_AcqGetOldestDataV(channel, &acq_size, data));
	pthread_mutex_unlock(&mutex);

	return RP_OK;
}

int main_sweep(float *ampl_z_out){

	float w_out, start_freq, end_freq;
	float amplitude = main_params.amplitude;
	int averaging = main_params.avg,
		dc_bias = main_params.dc_bias, z_ampl;

	
	int freq_step, i = 0;
	int decimation;
	int steps = main_params.steps;
	calib_t calibration = main_params.calibration;
	rp_acq_decimation_t api_decimation;

	float _Complex Z[averaging+1];

	//Calibration mode
	if(calibration != CALIB_NONE){
		start_freq = START_CALIB_FREQ, end_freq = END_CALIB_FREQ;
		steps = CALIB_SIZE;
		(freq_step = (int)(end_freq - start_freq) / (steps - 1));
	//Lcr meter mode
	}else{
		start_freq = main_params.start_freq, end_freq = start_freq;
		freq_step = 0;
	}

	float frequency[steps + 1];

	if(calibration == CALIB_NONE){
		frequency[0] = start_freq;
	}

	uint32_t r_shunt;
	_getRShunt(&r_shunt);
	
	while(1){

		//Calculate output angular velocity
		w_out = frequency[i] * 2 * M_PI;
		frequency[i] = start_freq + (freq_step * i);

		//Generate a sinusoidal wave form
		int ret_val = _SafeThreadGen(RP_CH_1, amplitude, frequency[i]);
		if(ret_val != RP_OK){
			printf("Error generating api signal: %s\n", rp_GetError(ret_val));
			return RP_EOOR;
		}

		//Get decimation value from frequency
		_getDecimationValue(frequency[i], &api_decimation, &decimation);
		printf("%d\n", decimation);
		
		uint32_t new_size = round((min_periodes * SAMPLE_RATE) /
			(frequency[0] * decimation));

		acq_size = new_size;
		float *ch1_data = (float *)malloc(acq_size * sizeof(float));
		float *ch2_data = (float *)malloc(acq_size * sizeof(float));
		float **analysis_data = multiDimensionVector(acq_size);

		ret_val = _SafeThreadAcqData(RP_CH_1, ch1_data, api_decimation);
		if(ret_val != RP_OK){
			printf("Error acquiring data: %s\n", rp_GetError(ret_val));
			return RP_EOOR;
		}

		ret_val = _SafeThreadAcqData(RP_CH_2, ch2_data, api_decimation);
		if(ret_val != RP_OK){
			printf("Error acquiring data: %s\n", rp_GetError(ret_val));
			return RP_EOOR;
		}
		/* Two dimension vector creation -- u_acq */
		for(int k = 0; k < acq_size; k++){
			analysis_data[0][k] = ch1_data[k];
			analysis_data[1][k] = ch2_data[k];
		}
		ret_val = _data_analysis(analysis_data, acq_size, dc_bias, 
					r_shunt, Z, w_out, decimation);

		z_ampl = sqrtf(powf(creal(*Z), 2) + powf(cimag(*Z), 2));
		break;
	}

	//Disable channel 1 generation module
	ECHECK_APP(rp_GenOutDisable(RP_CH_1)); 
	*ampl_z_out = z_ampl;
	return RP_OK;
}


int _data_analysis(float **data, 
					uint32_t size, 
					float dc_bias, 
					uint32_t r_shunt, 
					float _Complex *Z, 
					float w_out, 
					int decimation){

	/* Forward vector and variable declarations */
	float ang, u_dut_ampl, u_dut_phase_ampl, i_dut_ampl, i_dut_phase_ampl,
		phase_z_rad, z_ampl;

	float T = (decimation / SAMPLE_RATE);

	float *u_dut = (float *)malloc(size * sizeof(float));
	float *i_dut = (float *)malloc(size * sizeof(float));

	float **u_dut_s = multiDimensionVector(size);
	float **i_dut_s = multiDimensionVector(size);

	float **component_lock_in = multiDimensionVector(1);

	for(int i = 0; i < size; i++){
		u_dut[i] = data[0][i] - data[1][i];
		i_dut[i] = data[1][i] / ((r_shunt * 1e6 ) / (r_shunt + 1e6));
	}

	for(int i = 0; i < size; i++){
		ang = (i * T * w_out);
		//Real		
		u_dut_s[0][i] = u_dut[i] * sin(ang);
		u_dut_s[1][i] = u_dut[i] * sin(ang + (M_PI / 2));
		//Imag
		i_dut_s[0][i] = i_dut[i] * sin(ang);
		i_dut_s[1][i] = i_dut[i] * sin(ang + (M_PI / 2));
	}
	
	/* Trapezoidal approximation */
	component_lock_in[0][0] = trapezoidalApprox(u_dut_s[0], T, size);
	component_lock_in[0][1] = trapezoidalApprox(u_dut_s[1], T, size);
	component_lock_in[1][0] = trapezoidalApprox(i_dut_s[0], T, size);
	component_lock_in[1][1] = trapezoidalApprox(i_dut_s[1], T, size);

	/* Calculating volatage and phase */
	u_dut_ampl = 2 * (sqrtf(powf(component_lock_in[0][0], 2.0)) + 
		powf(component_lock_in[0][1], 2.0));

	u_dut_phase_ampl = atan2f(component_lock_in[0][1], 
		component_lock_in[0][0]);

	i_dut_ampl = 2 * (sqrtf(powf(component_lock_in[1][0], 2.0)) + 
		powf(component_lock_in[1][1], 2.0));

	i_dut_phase_ampl = atan2f(component_lock_in[1][1], component_lock_in[1][0]);

	/* Assigning impedance values */
	phase_z_rad = u_dut_phase_ampl - i_dut_phase_ampl;
	z_ampl = u_dut_ampl / i_dut_ampl;

	/* Applying phase limitation (-180 deg, 180 deg) */
	if(phase_z_rad <= -M_PI){
		phase_z_rad = phase_z_rad + (2 * M_PI);
	}else if(phase_z_rad >= M_PI){
		phase_z_rad = phase_z_rad - (2 * M_PI);
	}

	*Z = (z_ampl * cosf(phase_z_rad)) + (z_ampl * sinf(phase_z_rad) * I);

	return RP_OK;
}

/* Main Impedance Analyzer thread */
void *imp_MainThread(void *args){

	/* TODO: File managing system here looks a bit clumsy, 
	 * Make a better implementation. */

	FILE *calib_file;
	float *amplitude_z = malloc(acq_size * sizeof(float));
	calib_t calib_mode = main_params.calibration;

	/* Main sweep function */
	main_sweep((float *)args);

	calib_file = store_calib(calib_mode, amplitude_z);

	fclose(calib_file);
	return RP_OK;
}

/* Main call function */
int lcr_Run(float *amplitude){


	imp_Init();
	int err;
	pthread_t imp_thread_handler;
	err = pthread_create(&imp_thread_handler, 0, imp_MainThread, amplitude);
	if(err != RP_OK){
		printf("Main thread creation failed.\n");
		return RP_EOOR;
	}
	pthread_join(imp_thread_handler, 0);

	return RP_OK;
}

uint32_t imp_shuntAlgorithm(float z_ampl){

	if(z_ampl <= 50) 							return R_SHUNT_30;
	else if(z_ampl <= 100 && z_ampl > 50) 		return R_SHUNT_75;
	else if(z_ampl <= 500 && z_ampl > 100) 		return R_SHUNT_300;
	else if(z_ampl <= 1000 && z_ampl > 500) 	return R_SHUNT_750;
	else if(z_ampl <= 5000 && z_ampl > 1000) 	return R_SHUNT_3K;
	else if(z_ampl <= 10000 && z_ampl > 5000) 	return R_SHUNT_7_5K;
	else if(z_ampl <= 50e3 && z_ampl > 10000) 	return R_SHUNT_30K;
	else if(z_ampl <= 100e3 && z_ampl > 50e3) 	return R_SHUNT_80K;
	else if(z_ampl <= 500e3 && z_ampl > 100e3) 	return R_SHUNT_430K;
	else if(z_ampl > 500e3) 					return R_SHUNT_3M;

	return RP_EOOR;
}

/* Getters and setters */
int imp_SetAmplitude(float ampl){

	if((main_params.dc_bias != -1) && ((main_params.dc_bias + ampl > 1)
	  || (main_params.dc_bias + ampl <= 0))){

		printf("Invalid amplitude value. Max dc_bias plus "
			    "amplitude value must be: 1.0\n");

		return RP_EOOR;
	}
	main_params.amplitude = ampl;
	return RP_OK;
}

int imp_GetAmplitude(float *ampl){
	*ampl = main_params.amplitude;
	return RP_OK;
}

int imp_SetDcBias(float dc_bias){

	if((main_params.amplitude != -1) && ((main_params.amplitude + dc_bias > 1)
	  || (main_params.amplitude + dc_bias <= 0))){

		printf("Invalid dc bias value. Max dc_bias plus "
			    "amplitude value must be: 1.0\n");

		return RP_EOOR;
	}

	main_params.dc_bias = dc_bias;
	return RP_OK;
}

int imp_GetDcBias(float *dc_bias){
	*dc_bias = main_params.dc_bias;
	return RP_OK;
}

int imp_SetAveraging(float avg){
	main_params.avg = avg;
	return RP_OK;
}

int imp_GetAveraging(float *avg){
	*avg = main_params.avg;
	return RP_OK;
}

int _setRShunt(uint32_t r_shunt){
	main_params.r_shunt = r_shunt;
	return RP_OK;
}

int _getRShunt(uint32_t *r_shunt){
	*r_shunt = main_params.r_shunt;
	return RP_OK;
}

int imp_SetCalibMode(calib_t mode){
	main_params.calibration = mode;
	return RP_OK;
}

int imp_GetCalibMode(calib_t *mode){
	*mode = main_params.calibration;
	return RP_OK;
}

int imp_SetRefReal(float ref_real){
	main_params.ref_real = ref_real;
	return RP_OK;
}

int imp_GetRefReal(float *ref_real){
	*ref_real = main_params.ref_real;
	return RP_OK;
}

int imp_SetRefImg(float ref_img){
	main_params.ref_img = ref_img;
	return RP_OK;
}

int imp_GetRefImg(float *ref_img){
	*ref_img = main_params.ref_img;
	return RP_OK;
}

int imp_SetSteps(uint32_t steps){
	main_params.steps = steps;
	return RP_OK;
}

int imp_GetSteps(uint32_t *steps){
	*steps = main_params.steps;
	return RP_OK;
}

int imp_SetStartFreq(float start_freq){
	main_params.start_freq = start_freq;
	return RP_OK;
}

int imp_GetStartFreq(float *start_freq){
	*start_freq = main_params.start_freq;
	return RP_OK;
}

int imp_SetEndFreq(float end_freq){
	main_params.end_freq = end_freq;
	return RP_OK;
}

int imp_GetEndFreq(float *end_freq){
	*end_freq = main_params.end_freq;
	return RP_OK;
}

int imp_SetScaleType(scale_t scale){
	main_params.scale = scale;
	return RP_OK;
}

int imp_GetScaleType(scale_t *scale){
	*scale = main_params.scale;
	return RP_OK;
}

int imp_SetSweepMode(sweep_t sweep){
	main_params.sweep = sweep;
	return RP_OK;
}

int imp_GetSweepMode(sweep_t *sweep){
	*sweep = main_params.sweep;
	return RP_OK;
}

int imp_SetUserWait(bool user_wait){
	main_params.user_wait = user_wait;
	return RP_OK;
}

int imp_SetNoCalibration(bool no_calib){
	main_params.no_calib = no_calib;
	return RP_OK;
}

int imp_GetUserWait(bool *user_wait){
	*user_wait = main_params.user_wait;
	return RP_OK;
}

int main(void){

	imp_Init();
	float amplitude;
	main_sweep(&amplitude);

	printf("%f\n", amplitude);
	return 0;
}