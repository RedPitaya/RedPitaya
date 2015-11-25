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
#include <sys/syslog.h>

#include "impedance_analyzer.h"
#include "utils.h"
#include "calib.h"
#include "../../rpbase/src/common.h"
#include "./common.h"
#include "redpitaya/rp.h"

/* Global variables definition */

#define CORR_S_FREQ 	100
#define CORR_E_FREQ		1e6

int 					min_periodes = 20;
uint32_t 				acq_size = 1024;

pthread_mutex_t 		mutex;
pthread_t 				*imp_thread_handler = NULL;

/* Init impedance params struct */
lcr_params_t main_params = {0, 0, 0, false};


struct impendace_params {
	float frequency;
	float Z_out;
};

/* Init the main API structure */
int lcr_Init(){

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
	
	/* Set default values of the lcr structure */
	lcr_SetDefaultValues();

	ECHECK_APP(rp_AcqReset());
	ECHECK_APP(rp_GenReset());
	ECHECK_APP(rp_AcqSetTriggerDelay(ADC_BUFF_SIZE / 2));
	ECHECK_APP(rp_AcqSetTriggerLevel(0));

	pthread_mutex_unlock(&mutex);
	return RP_OK;
}

/* Release resources used the main API structure */
int lcr_Release(){
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
int lcr_Reset(){
	pthread_mutex_lock(&mutex);
	rp_Reset();
	/* Set default values of the lcr_params structure */
	lcr_SetDefaultValues(main_params);
	pthread_mutex_unlock(&mutex);
	return RP_OK;
}

int lcr_SetDefaultValues(){
	ECHECK_APP(lcr_setRShunt(R_SHUNT_30));
	ECHECK_APP(lcr_SetFrequency(1000.0));
	return RP_OK;
}

/* Generate functions  */
int lcr_SafeThreadGen(rp_channel_t channel, 
	           	float frequency){

	pthread_mutex_lock(&mutex);
	ECHECK_APP(rp_GenAmp(channel, LCR_AMPLITUDE));
	ECHECK_APP(rp_GenWaveform(channel, RP_WAVEFORM_SINE));
	ECHECK_APP(rp_GenFreq(channel, frequency));
	ECHECK_APP(rp_GenOutEnable(channel));
	pthread_mutex_unlock(&mutex);

	return RP_OK;
}

/* Acquire functions. Callback to the API structure */
int lcr_SafeThreadAcqData(rp_channel_t channel, 
					float *data, 
					rp_acq_decimation_t decimation){
	
	rp_acq_trig_state_t state;
	pthread_mutex_lock(&mutex);	
	ECHECK_APP(rp_AcqSetDecimation(decimation));
	ECHECK_APP(rp_AcqStart());
	ECHECK_APP(rp_AcqSetTriggerSrc(RP_TRIG_SRC_CHA_PE));
	state = RP_TRIG_STATE_WAITING;
	while(state == RP_TRIG_STATE_WAITING){
		ECHECK_APP(rp_AcqGetTriggerState(&state));
	}
	ECHECK_APP(rp_AcqGetOldestDataV(channel, &acq_size, data));
	pthread_mutex_unlock(&mutex);

	return RP_OK;
}

int lcr_getImpedance(float frequency, float *Z_out){
#if 0
	float w_out, start_freq, end_freq;
	float amplitude = main_params.amplitude;
	int averaging = main_params.avg,
		dc_bias = main_params.dc_bias;

	float z_ampl;

	
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


	//Calculate output angular velocity
	w_out = frequency[0] * 2 * M_PI;
	frequency[i] = start_freq + (freq_step * i);

	//Generate a sinusoidal wave form
	int ret_val = _SafeThreadGen(RP_CH_1, amplitude, frequency[0]);
	if(ret_val != RP_OK){
		printf("Error generating api signal: %s\n", rp_GetError(ret_val));
		return RP_EOOR;
	}

	//Get decimation value from frequency
	_getDecimationValue(frequency[0], &api_decimation, &decimation);
	
	acq_size = round((min_periodes * SAMPLE_RATE) /
		(frequency[0] * decimation));

	float *ch1_data = (float *)malloc(acq_size * sizeof(float));
	float *ch2_data = (float *)malloc(acq_size * sizeof(float));
	float **analysis_data = multiDimensionVector(acq_size);

	syslog(LOG_INFO, "Problem\n");
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
	char file1[20];
	char file2[20];
	sprintf(file1, "/tmp/datach1-%d",i);
	sprintf(file2, "/tmp/datach2-%d",i);

	FILE *f1 = fopen(file1, "w+");
	FILE *f2 = fopen(file2, "w+");

	/* Two dimension vector creation -- u_acq */
	for(int k = 0; k < acq_size; k++){
		analysis_data[0][k] = ch1_data[k];
		fprintf(f1, "%f\n", ch1_data[k]);
		analysis_data[1][k] = ch2_data[k];
		fprintf(f2, "%f\n", ch2_data[k]);
	}
	ret_val = _data_analysis(analysis_data, acq_size, dc_bias, 
				r_shunt, Z, w_out, decimation);

	syslog(LOG_INFO, "Problem\n");
	z_ampl = sqrtf(powf(creal(*Z), 2) + powf(cimag(*Z), 2));

	FILE *f3 = fopen("/tmp/z_ampl", "a+");
	fprintf(f3, "%f\n", z_ampl);

	fclose(f1);
	fclose(f2);
	fclose(f3);
	
	//Disable channel 1 generation module
	ECHECK_APP(rp_GenOutDisable(RP_CH_1));
#endif

	//Dummy test value - rp_GetOldestDataV is not working correctly.
	switch((int)frequency){
		case 100:
			*Z_out = 365.61868;
			break;
		case 1000:
			*Z_out = 44.65929;
			break;
		case 10000:
			*Z_out = 9.81285;
			break;
		case 100000:
			*Z_out = 36.57750;
			break;
	}

	//syslog(LOG_INFO, "%f\n", z_ampl);
	return RP_OK;
}

/* Main Lcr meter thread */
void *lcr_MainThread(void *args){

	struct impendace_params *args_struct =
		(struct impendace_params *)args;

	//Main lcr meter algorithm
	lcr_getImpedance(args_struct->frequency, &args_struct->Z_out);
	return RP_OK;
}

/* Main call function */
int lcr_Run(float *amplitude){

	int err;

	struct impendace_params args;
	args.frequency = main_params.frequency;

	pthread_t lcr_thread_handler;
	err = pthread_create(&lcr_thread_handler, 0, lcr_MainThread, &args);
	if(err != RP_OK){
		printf("Main thread creation failed.\n");
		return RP_EOOR;
	}
	pthread_join(lcr_thread_handler, 0);

	*amplitude = args.Z_out;

	return RP_OK;
}


int lcr_Correction(){

	int steps = 		CALIB_STEPS;
	int start_freq = 	CORR_S_FREQ;
	int end_freq = 		CORR_E_FREQ;
	int freq_step;

	int err;
	struct impendace_params args;
	pthread_t lcr_thread_handler;

	float *amplitude_z = malloc(acq_size * sizeof(float));
	calib_t calib_mode = main_params.calibration;

	for(int i = 0; i < steps; i++){
		
		freq_step = (end_freq - start_freq) / (steps - 1);
		args.frequency = freq_step;

		err = pthread_create(&lcr_thread_handler, 0, lcr_MainThread, &args);
		if(err != RP_OK){
			printf("Main thread creation failed.\n");
			return RP_EOOR;
		}
		pthread_join(lcr_thread_handler, 0);
		
		amplitude_z[i] = args.Z_out;
	}

	//Store calibration
	store_calib(calib_mode, amplitude_z);

	return RP_OK;
}


int lcr_data_analysis(float **data, 
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
		i_dut[i] = data[1][i] / ((30 * 1e6 ) / (30 + 1e6));
	}

	for(int i = 0; i < size; i++){
		ang = (i * T * w_out);
		//Real		
		u_dut_s[0][i] = u_dut[i] * sin(ang);
		u_dut_s[1][i] = u_dut[i] * sin(ang - (M_PI / 2));
		//Imag
		i_dut_s[0][i] = i_dut[i] * sin(ang);
		i_dut_s[1][i] = i_dut[i] * sin(ang - (M_PI / 2));
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



/* Getters and setters */
int lcr_SetFrequency(float frequency){
	main_params.frequency = frequency;
	return RP_OK;
}

int lcr_GetFrequency(float *frequency){
	*frequency = main_params.frequency;
	return RP_OK;
}

int lcr_setRShunt(uint32_t r_shunt){
	main_params.r_shunt = r_shunt;
	return RP_OK;
}

int lcr_getRShunt(uint32_t *r_shunt){
	*r_shunt = main_params.r_shunt;
	return RP_OK;
}

int lcr_SetCalibMode(calib_t mode){
	main_params.calibration = mode;
	return RP_OK;
}

int lcr_GetCalibMode(calib_t *mode){
	*mode = main_params.calibration;
	return RP_OK;
}