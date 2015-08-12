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
#include <complex.h>
#include <math.h>

#include "impedance_analyzer.h"
#include "common.h"


/* Global variables definition */
int 					min_periodes = 20;
uint32_t 				acq_size = 1024;

pthread_mutex_t 		mutex;
pthread_t 				*imp_thread_handler = NULL;

/* Init impedance params struct */
imp_params_t main_params = 
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
	ECHECK_APP(imp_SetRshunt(R_SHUNT_7_5K));
	ECHECK_APP(imp_SetCalibMode(IMP_CALIB_OPEN));
	ECHECK_APP(imp_SetRefReal(0.0));
	ECHECK_APP(imp_SetRefImg(0.0));
	ECHECK_APP(imp_SetSteps(10));
	ECHECK_APP(imp_SetStartFreq(900.0));
	ECHECK_APP(imp_SetEndFreq(20000.0));
	ECHECK_APP(imp_SetScaleType(IMP_SCALE_LINEAR));
	ECHECK_APP(imp_SetSweepMode(IMP_FREQUENCY_SWEEP));
	ECHECK_APP(imp_SetUserWait(false));
	ECHECK_APP(imp_SetNoCalibration(true));

	return RP_OK;
}

/* Generate functions  */
int imp_SafeThreadGen(rp_channel_t channel, float ampl, float freq){

	pthread_mutex_lock(&mutex);
	ECHECK_APP(rp_GenFreq(channel, freq));
	ECHECK_APP(rp_GenAmp(channel, ampl));
	ECHECK_APP(rp_GenWaveform(channel, RP_WAVEFORM_SINE));
	ECHECK_APP(rp_GenOutEnable(channel));
	pthread_mutex_unlock(&mutex);

	return RP_OK;
}

/* Acquire functions. Callback to the API structure */
int imp_SafeThreadAcqData(rp_channel_t channel, 
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

int imp_Sweep(float *ampl_z_out){

	/* Forward variable declaration */
	float log_freq, a, b, c, w_out, start_freq, end_freq;
	float ampl = main_params.amplitude, averaging = main_params.avg,
		dc_bias = main_params.dc_bias, z_ampl;

	imp_scale_t scale_type = main_params.scale;
	int freq_step, steps, decimation;
	rp_acq_decimation_t api_decimation;
	imp_calib_t calibration = main_params.mode;
	//bool rep_meas = true;
	int i = 0;

	/* Forward memory allocation */
	float complex *Z 		= (float complex *)malloc((averaging + 1) * sizeof(float complex));
	float **analysis_data 	= (float **)multiDimensionVector(acq_size);
	float **z_avg = (float **)multiDimensionVector(acq_size);

	/* Channel memory allocation */
	float *ch1_data = malloc((acq_size) * sizeof(float));
	float *ch2_data = malloc((acq_size) * sizeof(float));

	if(calibration != IMP_CALIB_NONE){
		start_freq = START_CALIB_FREQ, end_freq = END_CALIB_FREQ;
		steps = CALIB_SIZE;
	}else{
		start_freq = main_params.start_freq, end_freq = main_params.end_freq;
		steps = main_params.steps;
	}

	float *frequency = (float *)malloc(steps * sizeof(float));

	if(start_freq > end_freq){
		printf("End frequency must be greater than the starting frequency.\n");
		return RP_EOOR;
	}
	/* Check for logarithmic scale */
	if(scale_type == IMP_SCALE_LOGARITHMIC){
		a = log10(start_freq);
		b = log10(end_freq);
		(steps == 1) ? (c = (b - a)) : (c = (b - a) / (steps - 1));
	}

	/* Frequency iteration step ( Preventing division by zero ) */
	if(main_params.sweep){
		(steps == 1) ? (freq_step = (int)(end_freq - start_freq)) : 
			(freq_step = (int)(end_freq - start_freq) / (steps - 1));		
	}else{
		freq_step = 0;
	}

	//set_IIC_Shunt(-1);

	uint32_t r_shunt;
	/* Get R shunt enumerator value */
	ECHECK_APP(imp_GetRShunt(&r_shunt));

	/* Set shunt value before measurment */
	//set_IIC_Shunt(r_shunt);

	/* Main frequency sweep loop */
	while(i < steps){

		/* Repeat step if r_shunt changed */
		if(scale_type == IMP_SCALE_LOGARITHMIC){
			log_freq = powf(10, (c * i + a));
			frequency[i] = log_freq;
		}else{
			frequency[i] = start_freq + (freq_step * i);
		}

		/* Angular velocity calculation */
		w_out = frequency[i] * 2 * M_PI;

		/* Generating a sinusoidal form with the given frequency */
		int ret_gen = imp_SafeThreadGen(RP_CH_1, ampl, frequency[i]);

		if(ret_gen != RP_OK){
			printf("Error generating signal.\n");
			return RP_EOOR;
		}

		for(int j = 0; j < averaging; j++){

			if(frequency[i] >= 160000){
				decimation = IMP_DEC_1;
				api_decimation = RP_DEC_1;
			}else if(frequency[i] >= 20000){
				decimation = IMP_DEC_8;
				api_decimation = RP_DEC_8;
			}else if(frequency[i] >= 2500){
				decimation = IMP_DEC_64;
				api_decimation = RP_DEC_64;
			}else if(frequency[i] >= 160){
				decimation = IMP_DEC_1024;
				api_decimation = RP_DEC_1024;
			}else if(frequency[i] >= 20){
				decimation = IMP_DEC_8192;
				api_decimation = RP_DEC_8192;
			}else if(frequency[i] >= 2.5){
				decimation = IMP_DEC_65536;
				api_decimation = RP_DEC_65536;
			}

			uint32_t new_size = round((min_periodes * SAMPLE_RATE) / 
				(frequency[i] * decimation));

			/* Realloc buffer, if view size has changed */
			if(new_size != acq_size){
				ch1_data = realloc(ch1_data, new_size * sizeof(float));
				ch2_data = realloc(ch2_data, new_size * sizeof(float));
				analysis_data = multiDimensionVector(new_size);
				z_avg = multiDimensionVector(new_size);
				ampl_z_out = realloc(ampl_z_out, new_size * sizeof(float));
				acq_size = new_size;
			}

			/* TODO: dynamic memory allocation */
			/* Signal acquisition for both channels */
			int ret_val;

			ret_val = imp_SafeThreadAcqData(RP_CH_1, ch1_data, api_decimation);
			if(ret_val != RP_OK){
				printf("Error acquiring data.\n");
				return RP_EOOR;
			}

			ret_val = imp_SafeThreadAcqData(RP_CH_2, ch2_data, api_decimation);
			if(ret_val != RP_OK){
				printf("Error acquiring data.\n");
				return RP_EOOR;
			}

			/* Two dimension vector creation -- u_acq */
			for(int k = 0; k < acq_size; k++){
				analysis_data[0][k] = ch1_data[k];
				analysis_data[1][k] = ch2_data[k];
			}

			/* Calculate output data */
			imp_data_analysis(analysis_data, acq_size, dc_bias, 
						r_shunt, Z, w_out, decimation);
			
			if(ret_val != RP_OK){
				printf("Impedance analyzer data analysis failed to properly "
					"execute.\n");
				return RP_EOOR;
			}

			/* Saving calibration data */
			z_avg[0][j] = creal(*Z);
			z_avg[1][j] = cimag(*Z);

		}

		ECHECK_APP(rp_GenOutDisable(RP_CH_1)); 
		
		/* Out data */
		z_ampl = sqrtf(powf(vectorMean(z_avg[0], averaging), 2) + 
			powf(vectorMean(z_avg[1], averaging), 2));

		i++;
		ampl_z_out[i] = z_ampl;

	}

#if 0
		if(((z_ampl >= (r_shunt * 3)) || (z_ampl 
			<= (r_shunt / 3))) && rep_meas){

			if(((z_ampl > 1e6) && (r_shunt == R_SHUNT_3M)) ||
				((z_ampl < 30) && (r_shunt == R_SHUNT_30))){

				rep_meas = false;
				continue;

			}else{

				//r_shunt = imp_shuntAlgorithm(z_ampl);
				/* Set shunt value before measurment */
				//set_IIC_Shunt(r_shunt);	
			}

		/* Save *Z */
		}else{
			ampl_z_out[i] = z_ampl;
			rep_meas = true;
			/* Increment counter */
			i++;	
		}
	}

	/* Set shunt value after measurment */
	//set_IIC_Shunt(R_SHUNT_7_5K);
#endif
	//rep_meas = false;

	return RP_OK;
}

float imp_data_analysis(float **data, 
						uint32_t size, 
						float dc_bias, 
						uint32_t r_shunt, 
						float complex *Z, 
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
		i_dut[i] = data[1][i] / R_SHUNT_7_5K;//((r_shunt * 1e6 ) / (r_shunt + 1e6));
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

	return z_ampl;
}

/* Main Impedance Analyzer thread */
void *imp_MainThread(){

	/* TODO: File managing system here looks a bit clumsy, 
	 * Make a better implementation. */

	FILE *calib_file;
	float *amplitude_z = malloc(acq_size * sizeof(float));
	imp_calib_t calib_mode = main_params.mode;
	bool no_calib = main_params.no_calib;

	/* Stat structure */
	struct stat st = {0};

	/* Main sweep function */
	imp_Sweep(amplitude_z);

	/* Data directory */
	char *data_path = "/tmp/imp_data/calibration";
	char *command = " mkdir -p /tmp/imp_data/calibration";

	/* Create calibration data directory if it doesn't exist yet */
	if(calib_mode){
		if(stat(data_path, &st) == -1){
			/* Function call for creating a directory TODO: Use something else 
			* other than system call */
			//createPath(&data_path[0]);

			if(system(command) < 0){
				fprintf(stderr, "Error executing system "
					"command: %s\n", strerror(errno));
			}
		}
	}

	/* Write calibration data into files on the system */
	if(calib_mode == IMP_CALIB_OPEN){
		calib_file = fopen("/tmp/imp_data/calibration/calib_open", "w+");

	}else if(calib_mode == IMP_CALIB_SHORT){
		calib_file = fopen("/tmp/imp_data/calibration/calib_short", "w+");
	}

	/* If we are in calibration mode, write data to a file */
	if(calib_mode != IMP_CALIB_NONE){
		for(int i = 0; i < CALIB_SIZE; i++){
			fprintf(calib_file, "%.10f\n", amplitude_z[i]);
		}	
	}	

	if(calib_mode != IMP_CALIB_NONE) fclose(calib_file);

	float *calib_data_open = malloc(CALIB_SIZE * sizeof(float));
	float *calib_data_short = malloc(CALIB_SIZE * sizeof(float));

	/* Interpolate output data with the calibration data if *
	 * if user did calibration and has not requested data with no calibration */
	if(calib_mode == IMP_CALIB_NONE && !no_calib) {
		imp_Interpolate(calib_data_open, IMP_CALIB_OPEN);
		imp_Interpolate(calib_data_short, IMP_CALIB_SHORT);
	}
	
	/* Calculate some more data from ampz TODO: Function. 
	 * Return data to client. Return sucessfuly calibration parameter
	 * if CALIB mode selected. */

	return RP_OK;
}

/* Main call function */
int imp_Run(){

	int err;
	pthread_t imp_thread_handler;
	err = pthread_create(&imp_thread_handler, 0, imp_MainThread, 0);
	if(err != RP_OK){
		printf("Main thread creation failed.\n");
		return RP_EOOR;
	}
	pthread_join(imp_thread_handler, 0);
	
	return RP_OK;
}

int imp_Interpolate(float *calib_data, imp_calib_t calib_mode){

	float start_freq, end_freq, start_interval, end_interval; 
	uint32_t steps;

	ECHECK_APP(imp_GetSteps(&steps));
	FILE *calib_file;
	float *t_calib = malloc(CALIB_SIZE * sizeof(float));
	float *sub_calib = malloc(steps * sizeof(float));

	/* Data calculation */
	switch(calib_mode){
		case IMP_CALIB_OPEN:
			calib_file = fopen("/tmp/imp_data/calibration/calib_open", "r");
			break;
		case IMP_CALIB_SHORT:
			calib_file = fopen("/tmp/imp_data/calibration/calib_short", "r");
			break;
		default:
			break;
	}

	int c = 0;
	/* Read calibration file */
	while(!feof(calib_file)){
		fscanf(calib_file, "%f", &t_calib[c++]);
	}

	imp_GetStartFreq(&start_freq);
	imp_GetEndFreq(&end_freq);

	/* Get interpolation interval */
	findInterpFreq(start_freq, &start_interval, true);
	findInterpFreq(end_freq, &end_interval, false);	

	/* Interpolate each step inside */
	findIntrpInterv(&t_calib[0], &sub_calib[0], 
		start_interval, end_interval);


	//TODO: Interpolate
	fclose(calib_file);

	return RP_OK;
}

/* Finds the index of the given interval number 
 * between an array of z amplitudes. Indexes are calibration
 * frequencys. */
int findInterpFreq(float input_freq,
				   float *out_index, 
				   bool start_interval){

	float diff, freq_step;

	/* Max calibration frequency */
	float smallest_diff = END_CALIB_FREQ;

	for(int i = 1; i < CALIB_SIZE+1; i++){

		freq_step = 100 + (i-1) * 10100;
		diff = abs(freq_step -  input_freq);

		if(start_interval){
			if((diff < smallest_diff) && (freq_step < input_freq)){
				*out_index = i;
				smallest_diff = diff;
			}
		}else{
			if((diff < smallest_diff) && (freq_step > input_freq)){
				*out_index = i;
				smallest_diff = diff;	
			}
		}
	}
	return RP_OK;
}

/* This functions finds the Z amplitude 
 * interval on which we are going to interpolate */
int findIntrpInterv(float *in_z_ampl,
					float *out_sub_arry,
					int start_interval, 
					int end_interval){

	/* Calculate sub array size */
	uint32_t sub_size = (start_interval - end_interval) * sizeof(float);

	/* Get pointer to sub array*/
	memcpy(out_sub_arry, &in_z_ampl[start_interval], sub_size);

	return RP_OK;
}

/* [ f1 , f2 , f3 , f4 , f5 , f6 , f7 , f8 , ... ]
 * [ x1 , x2 , x3 , x4 , x5 , x6 , x7 , x8 , ... ]
 * */
int interpolationFunc(float *calib_data, float frequency){

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

int imp_SetRshunt(uint32_t r_shunt){
	main_params.r_shunt = r_shunt;
	return RP_OK;
}

int imp_GetRShunt(uint32_t *r_shunt){
	*r_shunt = main_params.r_shunt;
	return RP_OK;
}

int imp_SetCalibMode(imp_calib_t mode){
	main_params.mode = mode;
	return RP_OK;
}

int imp_GetCalibMode(imp_calib_t *mode){
	*mode = main_params.mode;
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

int imp_SetScaleType(imp_scale_t scale){
	main_params.scale = scale;
	return RP_OK;
}

int imp_GetScaleType(imp_scale_t *scale){
	*scale = main_params.scale;
	return RP_OK;
}

int imp_SetSweepMode(imp_sweep_t sweep){
	main_params.sweep = sweep;
	return RP_OK;
}

int imp_GetSweepMode(imp_sweep_t *sweep){
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

int main(int argc, char **argv){

	imp_Init();
	imp_Run();
	imp_Release();
	return 0;
}