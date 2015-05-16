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


/* Global variables definition */
int 					min_periodes = 10;
uint32_t 				acq_size = 1024;

pthread_mutex_t 		mutex;
pthread_t 				*lcr_thread_handler = NULL;

bool print_data = false;

/* Init lcr params struct */
lcr_params_t p_params = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, false};
lcr_params_t *main_params = &p_params; 

/* R_shunt constans */
static const uint32_t R_SHUNT_30	 = 30;
static const uint32_t R_SHUNT_75     = 75;
static const uint32_t R_SHUNT_300    = 300;
static const uint32_t R_SHUNT_750    = 750;
static const uint32_t R_SHUNT_3K     = 3000;
static const uint32_t R_SHUNT_7_5K   = 7500;
static const uint32_t R_SHUNT_30K    = 30000;
static const uint32_t R_SHUNT_80K    = 80000;
static const uint32_t R_SHUNT_430K   = 430000;
static const uint32_t R_SHUNT_3M     = 3000000;

/* Decimation constants */
static const uint32_t LCR_DEC_1		= 1;
static const uint32_t LCR_DEC_8		= 8;
static const uint32_t LCR_DEC_64	= 64;
static const uint32_t LCR_DEC_1024  = 1024;
static const uint32_t LCR_DEC_8192	= 8192;
static const uint32_t LCR_DEC_65536 = 65536;

/* Init the main API structure */
int lcr_Init(){

	/* Init mutex thread */
	if(pthread_mutex_init(&mutex, NULL)){
		fprintf(stderr, "Failed to init thread: %s\n", strerror(errno));
	}

	pthread_mutex_lock(&mutex);

	if(rp_Init() != RP_OK){
		fprintf(stderr, "Unable to inicialize the RPI API structure "
			"needed by LCR meter application: %s\n", strerror(errno));
		return RP_EOOR;
	}
	/* Set default values of the lcr_params structure */
	lcr_SetDefaultValues(main_params);
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

int lcr_SetDefaultValues(lcr_params_t *params){
	ECHECK_APP(lcr_SetAmplitude(params, 1));
	ECHECK_APP(lcr_SetDcBias(params, 0));
	ECHECK_APP(lcr_SetAveraging(params, 1));
	ECHECK_APP(lcr_SetCalibMode(params, 0));
	ECHECK_APP(lcr_SetRefReal(params, 0.0));
	ECHECK_APP(lcr_SetRefImg(params, 0.0));
	ECHECK_APP(lcr_SetSteps(params, 10));
	ECHECK_APP(lcr_SetStartFreq(params, 1000.0));
	ECHECK_APP(lcr_SetEndFreq(params, 10000.0));
	ECHECK_APP(lcr_SetScaleType(params, 0));
	ECHECK_APP(lcr_SetSweepMode(params, 0));
	ECHECK_APP(lcr_SetUserWait(params, false));
	return RP_OK;
}

/* Generate functions  */
int lcr_SafeThreadGen(rp_channel_t channel, float ampl, float freq){

	//pthread_mutex_lock(&mutex);
	(rp_GenFreq(channel, freq));
	(rp_GenAmp(channel, ampl));
	(rp_GenWaveform(channel, RP_WAVEFORM_SINE));
	(rp_GenOutEnable(channel));
	//pthread_mutex_unlock(&mutex);

	return RP_OK;
}

/* Acquire functions. Callback to the API structure */
int lcr_SafeThreadAcqData(rp_channel_t channel, 
	float *data, rp_acq_decimation_t decimation){

	uint32_t pos, curr_pos;
	pthread_mutex_lock(&mutex);
	
	ECHECK_APP(rp_AcqSetDecimation(decimation));
	ECHECK_APP(rp_AcqStart());
	ECHECK_APP(rp_AcqGetWritePointer(&pos));
	
	do {
		ECHECK_APP(rp_AcqGetWritePointer(&curr_pos));
	}while(curr_pos > (pos + ADC_BUFF_SIZE));

	
	(rp_AcqGetLatestDataV(channel, &acq_size, (float *)data));
	pthread_mutex_unlock(&mutex);
	return RP_OK;
}

float lcr_data_analysis(float **data, uint32_t size, float dc_bias, 
		float r_shunt, float complex *Z, float w_out, int decimation){


	/* Forward vector and variable declarations */
	float ang, u_dut_ampl, u_dut_phase_ampl, i_dut_ampl, i_dut_phase_ampl,
		phase_z_rad, z_ampl;

	int COORDINATES = 2;
	float T = (decimation / SAMPLE_RATE);

	float *u_dut = malloc(size * sizeof(float));
	float *i_dut = malloc(size * sizeof(float));

	float **u_dut_s = multiDimensionVector(COORDINATES, size);
	float **i_dut_s = multiDimensionVector(COORDINATES, size);
	float **component_lock_in = multiDimensionVector(COORDINATES, size);

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

	return z_ampl;
}

int lcr_FreqSweep(float **calib_data){

	/* Forward variable declaration */
	//float complex Z_load_ref = main_params->ref_real + main_params->ref_img;
	float log_freq, a, b, c, w_out;
	float start_freq = main_params->start_freq, end_freq = main_params->end_freq,
		ampl = main_params->amplitude, averaging = main_params->avg,
		dc_bias = main_params->dc_bias;//, z_ampl;

	float r_shunt = R_SHUNT_430K;
	lcr_scale_e scale_type = main_params->scale;
	int steps = main_params->steps;
	int freq_step;
	int decimation;
	rp_acq_decimation_t api_decimation;

	/* Testing purposes */
	scale_type = 0;
	start_freq = 1000;
	end_freq = 10000;
	steps = 10;

	/* Forward memory allocation */
	float *frequency 		= (float *)malloc(steps * sizeof(float));
	float complex *Z 		= (float complex *)malloc((averaging + 1) * sizeof(float complex));
	float **analysis_data 	= (float **)multiDimensionVector(2, acq_size);

	/* Channel memory allocation */
	float *ch1_data = malloc((acq_size) * sizeof(float));
	float *ch2_data = malloc((acq_size) * sizeof(float));

	if(start_freq > end_freq){
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
		/* R shunt algorithm calculation */
		//(i != 0) ? (lcr_SetRshunt(main_params, calculateShunt(z_ampl)))
			//: (lcr_SetRshunt(main_params, 0));
		//lcr_GetRshuntFactor(&r_shunt);

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
				api_decimation = RP_DEC_1;

			}else if(frequency[i] >= 20000){

				decimation = LCR_DEC_8;
				api_decimation = RP_DEC_8;

			}else if(frequency[i] >= 2500){

				decimation = LCR_DEC_64;
				api_decimation = RP_DEC_64;

			}else if(frequency[i] >= 160){

				decimation = LCR_DEC_1024;
				api_decimation = RP_DEC_1024;

			}else if(frequency[i] >= 20){

				decimation = LCR_DEC_8192;
				api_decimation = RP_DEC_8192;

			}else if(frequency[i] >= 2.5){

				decimation = LCR_DEC_65536;
				api_decimation = RP_DEC_65536;
			}



			uint32_t new_size = round((min_periodes * SAMPLE_RATE) / 
				(frequency[i] * decimation));

			/* Realloc buffer, if view size has changed */
			if(new_size != acq_size){
				ch1_data = realloc(ch1_data, new_size * sizeof(float));
				ch2_data = realloc(ch2_data, new_size * sizeof(float));
				analysis_data = multiDimensionVector(2, new_size);
				acq_size = new_size;
			}
			/* TODO Make dynamic memory allocation */
			/* Signal acquisition for both channels */
			int ret_val;
			ret_val = lcr_SafeThreadAcqData(RP_CH_1, ch1_data, api_decimation);
			if(ret_val != RP_OK){
				printf("Error acquiring data.\n");
				return RP_EOOR;
			}

			ret_val = lcr_SafeThreadAcqData(RP_CH_2, ch2_data, api_decimation);
			if(ret_val != RP_OK){
				printf("Error acquiring data.\n");
				return RP_EOOR;
			}
			if(i == steps - 1){
				print_data = true;
			}
			/* Two dimension vector creation -- u_acq */
			for(int k = 0; k < new_size; k++){
				analysis_data[0][k] = ch1_data[k];
				
				//if(i == 99) printf("%f,\n", ch2_data[k]);
				analysis_data[1][k] = ch2_data[k];
			}
			/* Calculate output data */
			lcr_data_analysis(analysis_data, acq_size, dc_bias, 
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
	print_data = false;
	return RP_OK;
}

int lcr_MeasSweep(float **calib_data){

	return RP_OK;
}

/* Main LCR thread */
void *lcr_MainThread(){
	
	float **data = multiDimensionVector(2, acq_size);

	lcr_FreqSweep(data);
	

	return RP_OK;
}

/* Main call function */
int lcr_Run(){

	//int err;
	//lcr_thread_handler = (pthread_t *)malloc(sizeof(pthread_t));
	//err = pthread_create(lcr_thread_handler, NULL, &lcr_MainThread, NULL);
	
	lcr_MainThread();

	//if(err != RP_OK){
		//printf("Main thread creation failed.\n");
		//return RP_EOOR;
	//}

	return RP_OK;
}

/* Lcr helper functions */
int lcr_GetRshuntFactor(float *r_shunt_factor){
	
	lcr_r_shunt_e r_shunt;
	ECHECK_APP(lcr_GetRShunt(main_params, &r_shunt));

	switch(r_shunt){
		case LCR_R_SHUNT_30:
			*r_shunt_factor = R_SHUNT_30;
			return RP_OK;
		case LCR_R_SHUNT_75:
			*r_shunt_factor = R_SHUNT_75;
			return RP_OK;
		case LCR_R_SHUNT_300:
			*r_shunt_factor = R_SHUNT_300;
			return RP_OK;
		case LCR_R_SHUNT_750:
			*r_shunt_factor = R_SHUNT_750;
			return RP_OK;
		case LCR_R_SHUNT_3K:
			*r_shunt_factor = R_SHUNT_3K;
			return RP_OK;
		case LCR_R_SHUNT_7_5K:
			*r_shunt_factor = R_SHUNT_7_5K;
			return RP_OK;
		case LCR_R_SHUNT_30K:
			*r_shunt_factor = R_SHUNT_30K;
			return RP_OK;
		case LCR_R_SHUNT_80K:
			*r_shunt_factor = R_SHUNT_80K;
			return RP_OK;
		case LCR_R_SHUNT_430K:
			*r_shunt_factor = R_SHUNT_430K;
			return RP_OK;
		case LCR_R_SHUNT_3M:
			*r_shunt_factor = R_SHUNT_3M;
			return RP_OK;
		default:
			return RP_EOOR;
	}
}

int calculateShunt(float z_ampl){

	if(z_ampl <= 50) 							return 0;
	else if(z_ampl <= 100 && z_ampl > 50) 		return 1;
	else if(z_ampl <= 500 && z_ampl > 100) 		return 2;
	else if(z_ampl <= 1000 && z_ampl > 500) 	return 3;
	else if(z_ampl <= 5000 && z_ampl > 1000) 	return 4;
	else if(z_ampl <= 10000 && z_ampl > 5000) 	return 5;
	else if(z_ampl <= 50e3 && z_ampl > 10000) 	return 6;
	else if(z_ampl < 100e3 && z_ampl > 50e3) 	return 7;
	else if(z_ampl <= 500e3 && z_ampl > 100e3) 	return 8;
	else if(z_ampl > 500e3) 					return 9;

	return RP_EOOR;
}

/* Getters and setters */
int lcr_SetAmplitude(lcr_params_t *params, float ampl){

	if((params->dc_bias != -1) && ((params->dc_bias + ampl > 1)
	  || (params->dc_bias + ampl <= 0))){

		printf("Invalid amplitude value. Max dc_bias plus "
			    "amplitude value must be: 1.0\n");

		return RP_EOOR;
	}
	params->amplitude = ampl;
	
	return RP_OK;
}

int lcr_GetAmplitude(lcr_params_t *params, float *ampl){
	ampl = &params->amplitude;
	return RP_OK;
}

int lcr_SetDcBias(lcr_params_t *params, float dc_bias){

	if((params->amplitude != -1) && ((params->amplitude + dc_bias > 1)
	  || (params->amplitude + dc_bias <= 0))){

		printf("Invalid dc bias value. Max dc_bias plus "
			    "amplitude value must be: 1.0\n");

		return RP_EOOR;
	}
	params->dc_bias = dc_bias;
	
	return RP_OK;
}

int lcr_GetDcBias(lcr_params_t *params, float *dc_bias){
	dc_bias = &params->dc_bias;
	return RP_OK;
}

int lcr_SetAveraging(lcr_params_t *params, float avg){

	params->avg = avg;
	return RP_OK;
}

int lcr_GetAveraging(lcr_params_t *params, float *avg){
	avg = &params->avg;
	return RP_OK;
}

int lcr_SetRshunt(lcr_params_t *params, lcr_r_shunt_e r_shunt){
	params->r_shunt = r_shunt;
	return RP_OK;
}

int lcr_GetRShunt(lcr_params_t *params, lcr_r_shunt_e *r_shunt){
	r_shunt = &params->r_shunt;
	return RP_OK;
}

int lcr_SetCalibMode(lcr_params_t *params, lcr_calib_e mode){
	params->mode = mode;
	return RP_OK;
}

int lcr_GetCalibMode(lcr_params_t *params, lcr_calib_e *mode){
	mode = &params->mode;
	return RP_OK;
}

int lcr_SetRefReal(lcr_params_t *params, float ref_real){
	params->ref_real = ref_real;
	return RP_OK;
}

int lcr_GetRefReal(lcr_params_t *params, float *ref_real){
	ref_real = &params->ref_real;
	return RP_OK;
}

int lcr_SetRefImg(lcr_params_t *params, float ref_img){
	params->ref_img = ref_img;
	return RP_OK;
}

int lcr_GetRefImg(lcr_params_t *params, float *ref_img){
	ref_img = &params->ref_img;
	return RP_OK;
}

int lcr_SetSteps(lcr_params_t *params, uint32_t steps){
	params->steps = steps;
	return RP_OK;
}

int lcr_GetSteps(lcr_params_t *params, uint32_t *steps){
	steps = &params->steps;
	return RP_OK;
}

int lcr_SetStartFreq(lcr_params_t *params, float start_freq){
	params->start_freq = start_freq;
	return RP_OK;
}

int lcr_GetStartFreq(lcr_params_t *params, float *start_freq){
	start_freq = &params->start_freq;
	return RP_OK;
}

int lcr_SetEndFreq(lcr_params_t *params, float end_freq){
	params->end_freq = end_freq;
	return RP_OK;
}

int lcr_GetEndFreq(lcr_params_t *params, float *end_freq){
	end_freq = &params->end_freq;
	return RP_OK;
}

int lcr_SetScaleType(lcr_params_t *params, lcr_scale_e scale){
	params->scale = scale;
	return RP_OK;
}

int lcr_GetScaleType(lcr_params_t *params, lcr_scale_e *scale){
	scale = &params->scale;
	return RP_OK;
}

int lcr_SetSweepMode(lcr_params_t *params, lcr_sweep_e sweep){
	params->sweep = sweep;
	return RP_OK;
}

int lcr_GetSweepMode(lcr_params_t *params, lcr_sweep_e *sweep){
	sweep = &params->sweep;
	return RP_OK;
}

int lcr_SetUserWait(lcr_params_t *params, bool user_wait){
	params->user_wait = user_wait;
	return RP_OK;
}

int lcr_GetUserWait(lcr_params_t *params, bool *user_wait){
	user_wait = &params->user_wait;
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

	lcr_Init();
	lcr_Run();
	lcr_Release();
	return 0;
}



