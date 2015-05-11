/**
* $Id: $
*
* @brief Red Pitaya application lcr module interface
*
* @Author Luka Golinar
*
* (c) Red Pitaya  http://www.redpitaya.com
*
* This part of code is written in C programming language.
* Please visit http://en.wikipedia.org/wiki/C_(programming_language)
* for more details on the language used herein.
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <complex.h>
#include <math.h>

#include "lcrApp.h"
#include "common.h"

/* User view buffer */
int16_t 				*ch1_data, *ch2_data;
int16_t					**analysis_data;

/* Global variables definition */
int 					min_periodes = 10;
uint32_t 				acq_size = 1024;

pthread_mutex_t 		mutex;
pthread_t 				*lcr_thread_handler = NULL;

/* Init lcr params struct */
lcr_params_e 			*main_params;

/* Init the main API structure */
int lcr_Init(){

	/* Init mutex thread */
	if(!pthread_mutex_init(&mutex, NULL)){
		fprintf(stderr, "Failed to init thread: %s\n", strerror(errno));
	}
	pthread_mutex_lock(&mutex);
	if(rp_Init() != RP_OK){
		fprintf(stderr, "Unable to inicialize the RPI API structure "
			"needed by LCR meter application: %s\n", strerror(errno));
		return RP_EOOR;
	}

	/* Set default values of the lcr_params structure */
	lcr_SetDefaultValues();

	pthread_mutex_unlock(&mutex);
	return RP_OK;
}

/* Release resources used the main API structure */
int lcr_Release(){
	pthread_mutex_lock(&mutex);
	if(rp_Release() != RP_OK){
		fprintf(stderr, "Unable to release resources used by " 
			"LCR meter API: %s\n", strerror(errno));
		return RP_EOOR;
	}

	/* Set all bits in the main_params structure to -1 */
	memset(main_params, -1, sizeof(*main_params));

	free(ch1_data);
	free(ch2_data);
	free(analysis_data);

	pthread_mutex_unlock(&mutex);
	pthread_mutex_destroy(&mutex);
	return RP_OK;
}

/* Set default values of all rpi resources */
int lcr_Reset(){
	pthread_mutex_lock(&mutex);
	rp_Reset();
	/* Set default values of the lcr_params structure */
	lcr_SetDefaultValues();
	pthread_mutex_unlock(&mutex);
	return RP_OK;
}

int lcr_SetDefaultValues(){
	ECHECK_APP(lcr_SetAmplitude(1.0));
	ECHECK_APP(lcr_SetDcBias(0.0));
	ECHECK_APP(lcr_SetAveraging(10));
	ECHECK_APP(lcr_SetCalibMode(1));
	ECHECK_APP(lcr_SetRefReal(0.0));
	ECHECK_APP(lcr_SetRefImg(0.0));
	ECHECK_APP(lcr_SetSteps(100));
	ECHECK_APP(lcr_SetStartFreq(1000.0));
	ECHECK_APP(lcr_SetEndFreq(10000));
	ECHECK_APP(lcr_SetScaleType(0));
	ECHECK_APP(lcr_SetSweepMode(0));
	ECHECK_APP(lcr_SetUserWait(false));
	return RP_OK;
}

/* Generate functions  */
int lcr_SafeThreadGen(rp_channel_t channel, float ampl, float freq){

	pthread_mutex_lock(&mutex);
	ECHECK_APP(rp_GenFreq(channel, freq));
	ECHECK_APP(rp_GenAmp(channel, ampl));
	ECHECK_APP(rp_GenWaveform(channel, RP_WAVEFORM_SINE));
	ECHECK_APP(rp_GenOutEnable(channel));
	pthread_mutex_unlock(&mutex);

	return RP_OK;
}

/* Acquire functions. Callback to the API structure */
int lcr_SafeThreadAcqData(rp_channel_t channel, int16_t *data){

	pthread_mutex_lock(&mutex);
	ECHECK_APP(rp_AcqGetOldestDataV(channel, &acq_size, (float *)data));
	pthread_mutex_unlock(&mutex);
	return RP_OK;
}

int lcr_data_analysis(int16_t **data, uint32_t size, float dc_bias, 
		float r_shunt, float complex *Z, float w_out, int decimation){


	/* Forward vector and variable declarations */
	float ang, u_dut_ampl, u_dut_phase_ampl, i_dut_ampl, i_dut_phase_ampl,
		phase_z_rad, z_ampl;

	int COORDINATES = 2;
	float T = (decimation / SAMPLE_RATE);

	int16_t *u_dut = malloc(size * sizeof(int16_t));
	int16_t *i_dut = malloc(size * sizeof(int16_t));

	int16_t **u_dut_s = multiDimensionVector(COORDINATES, size);
	int16_t **i_dut_s = multiDimensionVector(COORDINATES, size);
	int16_t **component_lock_in = multiDimensionVector(COORDINATES, size);

	for(int i = 0; i < size; i++){
		u_dut[i] = data[0][i] - data[1][i];
		i_dut[i] = data[1][i] / r_shunt; 
	}

	for(int i = 0; i < size; i++){
		ang = (T * w_out * i);
		//X		
		u_dut_s[0][i] = u_dut[i] * sin(ang);
		i_dut_s[0][i] = i_dut[i] * sin(ang);
		//Y
		u_dut_s[1][i] = u_dut[i] * sin(ang + (M_PI / 2)); 
		i_dut_s[1][i] = i_dut[i] * sin(ang + (M_PI / 2));
	}

	/* Trapezoidal approximation */
	component_lock_in[0][0] = trapezoidalApprox(u_dut_s[0], T, size);
	component_lock_in[0][1] = trapezoidalApprox(u_dut_s[1], T, size);
	component_lock_in[1][0] = trapezoidalApprox(i_dut_s[0], T, size);
	component_lock_in[1][0] = trapezoidalApprox(i_dut_s[1], T, size);

	/* Calculating volatage and phase */
	u_dut_ampl = 2 * (sqrt(pow(component_lock_in[0][0], 2)) + pow(component_lock_in[0][1], 2));
	u_dut_phase_ampl = atan2(component_lock_in[0][0], component_lock_in[0][1]);

	i_dut_ampl = 2 * (sqrt(pow(component_lock_in[1][0], 2)) + pow(component_lock_in[1][1], 2));
	i_dut_phase_ampl = atan2(component_lock_in[1][0], component_lock_in[1][1]);

	/* Assigning impedance values */
	phase_z_rad = u_dut_phase_ampl - i_dut_phase_ampl;
	z_ampl = u_dut_ampl + i_dut_ampl;

	/* Applying phase limitation (-180 deg, 180 deg) */
	if(phase_z_rad <= -M_PI){
		phase_z_rad = phase_z_rad + (2 * M_PI);
	}else if(phase_z_rad >= M_PI){
		phase_z_rad = phase_z_rad - (2 * M_PI);
	}

	*Z = (z_ampl * cos(phase_z_rad)) + (z_ampl * sin(phase_z_rad) * I);

	return RP_OK;
}

int lcr_FreqSweep(int16_t **calib_data){

	/* Forward variable declaration */
	//float complex Z_load_ref = main_params->ref_real + main_params->ref_img;
	float log_freq, a, b, c, w_out;
	float start_freq = main_params->start_freq, end_freq = main_params->end_freq,
		ampl = main_params->amplitude, averaging = main_params->avg,
		dc_bias = main_params->dc_bias;

	lcr_r_shunt_e r_shunt;
	lcr_scale_e scale_type = main_params->scale;
	int steps = main_params->steps;
	int freq_step;
	int decimation;

	/* Forward memory allocation */
	float *frequency 	= (float *)malloc(steps * sizeof(float));
	float complex *Z 	= (float complex *)malloc((averaging + 1) * sizeof(float complex));

	if(start_freq < end_freq){
		printf("End frequency must be greater than the starting frequency.\n");
		return RP_EOOR;
	}

	/* Check for logarithmic scale */
	if(scale_type == LCR_SCALE_LOGARITHMIC){
		a = log10(start_freq);
		b = log10(end_freq);
		(steps == 1) ? (c = (b - a)) : (c = (b - a) / (steps - 1));
	}

	/* Frequency iteration step */
	(steps == 1) ? (freq_step = (int)(end_freq - start_freq)) : 
		(freq_step = (int)(end_freq - start_freq) / (steps - 1));


	/* Main frequency sweep loop */
	for(int i = 0; i < steps; i++){
		if(scale_type == LCR_SCALE_LOGARITHMIC){
			log_freq = powf(10, (c * i + a));
			frequency[i] = log_freq;
		}else{
			frequency[i] = start_freq + (freq_step * i);
		}

		/* Angular velocity calculation */
		w_out = frequency[i] * 2 * M_PI;

		/* Generating a sinusoidal form with the given frequency */
		int ret_gen = lcr_SafeThreadGen(RP_CH_1, ampl, frequency[i]);

		if(ret_gen != RP_OK){
			printf("Error generating signal.\n");
			return RP_EOOR;
		}

		for(int j = 0; j < averaging; j++){
			if(frequency[i] >= 160000){
				decimation = LCR_DEC_1;
			}else if(frequency[i] >= 20000){
				decimation = LCR_DEC_8;
			}else if(frequency[i] >= 2500){
				decimation = LCR_DEC_64;
			}else if(frequency[i] >= 1024){
				decimation = LCR_DEC_8192;
			}else if(frequency[i]){
				decimation = LCR_DEC_65536;
			}

			float new_size = round((min_periodes * 125e6) / frequency[i] * decimation);

			/* Realloc buffer, if view size has changed */
			if(new_size != acq_size){
				ch1_data = realloc(ch1_data, new_size);
				ch2_data = realloc(ch2_data, new_size);
				acq_size = new_size;
			}

			/* Signal acquisition for both channels */
			int ret_val;

			ret_val = lcr_SafeThreadAcqData(RP_CH_1, ch1_data);
			if(ret_val != RP_OK){
				printf("Error acquiring data.\n");
				return RP_EOOR;
			}

			ret_val = lcr_SafeThreadAcqData(RP_CH_2, ch2_data);
			if(ret_val != RP_OK){
				printf("Error acquiring data.\n");
				return RP_EOOR;
			}

			/* Two dimension vector creation -- u_acq */
			for(int i = 0; i < acq_size; i++){
				analysis_data[0][i] = ch1_data[i];
				analysis_data[1][i] = ch2_data[i];
			}

			/* TODO: Function for settting R_SHUNT */

			lcr_GetRShunt(&r_shunt);

			/* Calculate output data */
			ret_val = lcr_data_analysis(analysis_data, acq_size, dc_bias, 
						r_shunt, Z, w_out, decimation);

			if(ret_val != RP_OK){
				printf("Lcr data analysis failed to properly execute.\n");
				return RP_EOOR;
			}

			/* Saving calibration data */
			calib_data[0][j] = creal(*Z);
			calib_data[1][j] = cimag(*Z);
		}
	}
	
	printf("%d%f%d\n", freq_step, w_out, decimation);

	return RP_OK;
}

int lcr_MeasSweep(int16_t **calib_data){

	return RP_OK;
}

/* Main LCR thread */
void *lcr_MainThread(){

	/* Channel memory allocation */
	ch1_data = malloc((acq_size) * sizeof(int16_t));
	ch2_data = malloc((acq_size) * sizeof(int16_t));

	for(int i = LCR_CALIB_NONE; i < (main_params->mode); i++){

		
		if(main_params->sweep == LCR_MEASURMENT_SWEEP){
			
		}else{
			
		}


	}
	

	return RP_OK;
}

/* Main call function */
int lcr_Run(){

	int err;
	lcr_thread_handler = (pthread_t *)malloc(sizeof(pthread_t));

	err = pthread_create(lcr_thread_handler, NULL, &lcr_MainThread, NULL);
	if(err != RP_OK){
		printf("Main thread creation failed.\n");
		return RP_EOOR;
	}

	return RP_OK;
}

/* lcr helper functions */
int lcr_GetRshuntFactor(float *r_shunt_factor){
	
	lcr_r_shunt_e r_shunt;
	ECHECK_APP(lcr_GetRShunt(&r_shunt));

	switch(r_shunt){
		case LCR_R_SHUNT_30:
			*r_shunt_factor = 30;
			return RP_OK;
		case LCR_R_SHUNT_75:
			*r_shunt_factor = 75;
			return RP_OK;
		case LCR_R_SHUNT_300:
			*r_shunt_factor = 300;
			return RP_OK;
		case LCR_R_SHUNT_750:
			*r_shunt_factor = 750;
			return RP_OK;
		case LCR_R_SHUNT_3K:
			*r_shunt_factor = 3000;
			return RP_OK;
		case LCR_R_SHUNT_7_5K:
			*r_shunt_factor = 7500;
			return RP_OK;
		case LCR_R_SHUNT_30K:
			*r_shunt_factor = 30000;
			return RP_OK;
		case LCR_R_SHUNT_80K:
			*r_shunt_factor = 80000;
			return RP_OK;
		case LCR_R_SHUNT_430K:
			*r_shunt_factor = 430000;
			return RP_OK;
		case LCR_R_SHUNT_3M:
			*r_shunt_factor = 30000000;
			return RP_OK;
		default:
			return RP_EOOR;
	}
}

/* Getters and setters */
int lcr_SetAmplitude(float ampl){

	if((main_params->dc_bias != -1) && ((main_params->dc_bias + ampl > 1)
	  || (main_params->dc_bias + ampl <= 0))){

		printf("Invalid amplitude value. Max dc_bias plus "
			    "amplitude value must be: 1.0\n");

		return RP_EOOR;
	}

	main_params->amplitude = ampl;
	
	return RP_OK;
}

int lcr_GetAmplitude(float *ampl){
	ampl = &main_params->amplitude;
	return RP_OK;
}

int lcr_SetDcBias(float dc_bias){

	if((main_params->amplitude != -1) && ((main_params->amplitude + dc_bias > 1)
	  || (main_params->amplitude + dc_bias <= 0))){

		printf("Invalid dc bias value. Max dc_bias plus "
			    "amplitude value must be: 1.0\n");

		return RP_EOOR;
	}
	main_params->dc_bias = dc_bias;
	
	return RP_OK;
}

int lcr_GetDcBias(float *dc_bias){
	dc_bias = &main_params->dc_bias;
	return RP_OK;
}

int lcr_SetAveraging(float avg){

	main_params->avg = avg;
	return RP_OK;
}

int lcr_GetAveraging(float *avg){
	avg = &main_params->avg;
	return RP_OK;
}

int lcr_SetRshunt(lcr_r_shunt_e r_shunt){
	main_params->r_shunt = r_shunt;
	return RP_OK;
}

int lcr_GetRShunt(lcr_r_shunt_e *r_shunt){
	r_shunt = &main_params->r_shunt;
	return RP_OK;
}

int lcr_SetCalibMode(lcr_calib_e mode){
	main_params->mode = mode;
	return RP_OK;
}

int lcr_GetCalibMode(lcr_calib_e *mode){
	mode = &main_params->mode;
	return RP_OK;
}

int lcr_SetRefReal(float ref_real){
	main_params->ref_real = ref_real;
	return RP_OK;
}

int lcr_GetRefReal(float *ref_real){
	ref_real = &main_params->ref_real;
	return RP_OK;
}

int lcr_SetRefImg(float ref_img){
	main_params->ref_img = ref_img;
	return RP_OK;
}

int lcr_GetRefImg(float *ref_img){
	ref_img = &main_params->ref_img;
	return RP_OK;
}

int lcr_SetSteps(uint32_t steps){
	main_params->steps = steps;
	return RP_OK;
}

int lcr_GetSteps(uint32_t *steps){
	steps = &main_params->steps;
	return RP_OK;
}

int lcr_SetStartFreq(float start_freq){
	main_params->start_freq = start_freq;
	return RP_OK;
}

int lcr_GetStartFreq(float *start_freq){
	start_freq = &main_params->start_freq;
	return RP_OK;
}

int lcr_SetEndFreq(float end_freq){
	main_params->end_freq = end_freq;
	return RP_OK;
}

int lcr_GetEndFreq(float *end_freq){
	end_freq = &main_params->end_freq;
	return RP_OK;
}

int lcr_SetScaleType(lcr_scale_e scale){
	main_params->scale = scale;
	return RP_OK;
}

int lcr_GetScaleType(lcr_scale_e *scale){
	scale = &main_params->scale;
	return RP_OK;
}

int lcr_SetSweepMode(lcr_sweep_e sweep){
	main_params->sweep = sweep;
	return RP_OK;
}

int lcr_GetSweepMode(lcr_sweep_e *sweep){
	sweep = &main_params->sweep;
	return RP_OK;
}

int lcr_SetUserWait(bool user_wait){
	main_params->user_wait = user_wait;
	return RP_OK;
}

int lcr_GetUserWait(bool *user_wait){
	user_wait = &main_params->user_wait;
	return RP_OK;
}

int lcr_SetUserView(uint32_t view){
	if(view < 10){
		printf("Invalid view size. Must be greater than 10.\n");
		return RP_EOOR;
	}
	acq_size = view;
	return RP_OK;
}

int lcr_GetUserView(uint32_t *view){
	view = &acq_size;
	return RP_OK;
}

int main(int argc, char **argv){

	return 0;
}



