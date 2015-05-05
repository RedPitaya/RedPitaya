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
float *ch1_data, *ch2_data;

/* Global variables definition */
int min_periodes = 10;
const int view_size = 0;

pthread_mutex_t mutex;
pthread_t 		*lcr_thread_handler = NULL;

/* Init lcr params struct */
lcr_params_e *main_params;

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
	ECHECK_APP(lcr_SetRshunt(0.0));
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
int SafeThreadGen(rp_channel_t channel, float ampl, float start_freq, float end_freq, float *data){
	pthread_mutex_lock(&mutex);
	ECHECK_APP(rp_GenAmp(channel, ampl));
	ECHECK_APP(rp_GenFreq(channel, start_freq));
	//ECHECK_APP(rp_GenEndFreq(channel, end_freq));
	/* TODO: Need to add synthesis for LCR waveform in API */

	pthread_mutex_unlock(&mutex);
	return RP_OK;
}

/* Acquire functions. Callback to the API structure */
int lcr_SafeThreadAcqData(rp_channel_t channel, float *data){
	pthread_mutex_lock(&mutex);
	ECHECK_APP(rp_AcqGetOldestDataV(channel, (uint32_t*)view_size, data));
	pthread_mutex_unlock(&mutex);
	return RP_OK;
}

void *lcr_FreqSweep(){

	/* Forward variable declaration */
	//float complex Z_load_ref = main_params->ref_real + main_params->ref_img;
	float log_freq, a, b, c, w_out;
	float start_freq = main_params->start_freq, end_freq = main_params->end_freq,
					   		ampl = main_params->amplitude, averaging = main_params->avg;

	lcr_scale_e scale_type = main_params->scale;
	int steps = main_params->steps;
	int stepsTe, freq_step;

	rp_acq_decimation_t decimation;

	if(start_freq < end_freq){
		printf("End frequency must be greater than the starting frequency.\n");
	}

	if(steps < 10){
		stepsTe = steps;
	}
	int trans_eff_c = stepsTe;

	/* Check for logarithmic scale */
	if(scale_type == LCR_SCALE_LOGARITHMIC){
		a = log10(start_freq);
		b = log10(end_freq);
		(steps == 1) ? (c = (b - a)) : (c = (b - a) / (steps - 1));
	}

	/* Frequency iteration step */
	(steps == 1) ? (freq_step = (int)(end_freq - start_freq)) : 
		(freq_step = (int)(end_freq - start_freq) / (steps - 1));


	/* Forward memory allocation */
	float *frequency = (float *)malloc(steps * sizeof(float));


	/* Main frequency sweep loop */
	for(int i = 0; i < steps; i++){
		if(scale_type == LCR_SCALE_LOGARITHMIC){
			log_freq = powf(10, (c * i + a));
			frequency[i] = log_freq;
		}else{
			frequency[i] = start_freq + (freq_step * i);
		}

		if(trans_eff_c > 0){
			frequency[i] = start_freq - (start_freq / 2) + 
				((start_freq / 2) * trans_eff_c / stepsTe);
			trans_eff_c--;
		}else if(!trans_eff_c){
			frequency[i] = start_freq;
		}

		/* Angular velocity calculation */
		w_out = frequency[i] * 2 * M_PI;

		pthread_mutex_lock(&mutex);
		/* Generating a sinusoidal form with the given frequency */
		rp_GenFreq(RP_CH_1, 10000.0);
		rp_GenAmp(RP_CH_1, ampl);
		rp_GenWaveform(RP_CH_1, RP_WAVEFORM_SINE);
		rp_GenOutEnable(RP_CH_1);

		rp_GenFreq(RP_CH_2, 10000.0);
		rp_GenAmp(RP_CH_2, ampl);
		rp_GenWaveform(RP_CH_2, RP_WAVEFORM_SINE);
		rp_GenOutEnable(RP_CH_2);
		pthread_mutex_unlock(&mutex);

		for(int j = 0; j < averaging; j++){
			char freq_c = (char)frequency[j];
			switch(freq_c){
				case (freq_c >= 160000):
					decimation = RP_DEC_1;
					break;
			}
		}

	}
	
	printf("%d%d%f%d\n", stepsTe, freq_step, w_out, decimation);

	return RP_OK;
}

void *lcr_MeasSweep(){

	return RP_OK;
}

/* Main LCR thread */
int lcr_MainThread(){

	int err;

	lcr_thread_handler = (pthread_t *)malloc(sizeof(pthread_t));
	if(main_params->sweep == LCR_MEASURMENT_SWEEP){
		err = pthread_create(lcr_thread_handler, NULL, &lcr_MeasSweep, NULL);
		if(err != RP_OK){
			printf("Main thread creation failed.\n");
			return RP_EOOR;
		}
	}else{
		err = pthread_create(lcr_thread_handler, NULL, &lcr_FreqSweep, NULL);
		if(err != RP_OK){
			printf("Main thread creation failed.\n");
			return RP_EOOR;
		}
	}

	return RP_OK;
}

/* Main call function */
int lcr_Run(){

	if(lcr_MainThread() != RP_OK){
		printf("Error.\n");
		return RP_EOOR;
	}

	return RP_OK;
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

int lcr_SetRshunt(float r_shunt){

	main_params->r_shunt = r_shunt;
	return RP_OK;
}

int lcr_GetRshunt(float *r_shunt){
	r_shunt = &main_params->r_shunt;
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

int main(int argc, char **argv){

	return 0;
}



