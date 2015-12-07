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
#include <complex.h>

#include "lcr_meter.h"
#include "utils.h"
#include "calib.h"
#include "../../rpbase/src/common.h"
#include "./common.h"
#include "redpitaya/rp.h"

/* Global variables definition */
int 					min_periodes = 10;

pthread_mutex_t 		mutex;
pthread_t 				*imp_thread_handler = NULL;

/* Init lcr params struct */
lcr_params_t main_params = {0, 0, 0, false};

/* Main lcr data params */
lcr_main_data_t *calc_data;

struct impendace_params {
	float frequency;
	float _Complex Z_out;
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

	/* Malloc global vars */
	calc_data = malloc(sizeof(calc_data));

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
	ECHECK_APP(rp_GenOffset(channel, 0.25));
	ECHECK_APP(rp_GenWaveform(channel, RP_WAVEFORM_SINE));
	ECHECK_APP(rp_GenFreq(channel, frequency));
	ECHECK_APP(rp_GenOutEnable(channel));
	pthread_mutex_unlock(&mutex);

	return RP_OK;
}

/* Acquire functions. Callback to the API structure */
int lcr_SafeThreadAcqData(float **data, 
						  rp_acq_decimation_t decimation,
						  int acq_size,
						  int dec){
	
	rp_acq_trig_state_t state;
	uint32_t pos;
	uint32_t acq_u_size = acq_size;

	ECHECK_APP(rp_AcqReset());	
	ECHECK_APP(rp_AcqSetDecimation(decimation));
	ECHECK_APP(rp_AcqSetTriggerLevel(0.4));
	ECHECK_APP(rp_AcqSetTriggerDelay(acq_size - (ADC_BUFF_SIZE / 2)));
	ECHECK_APP(rp_AcqSetTriggerSrc(RP_TRIG_SRC_CHA_PE));
	ECHECK_APP(rp_AcqStart());

	state = RP_TRIG_STATE_TRIGGERED;
	int retiries = 10000; //micro seconds
	while(retiries > 0){
        rp_AcqGetTriggerState(&state);
        if(state == RP_TRIG_STATE_TRIGGERED){
        	break;
        }
        retiries--;
    }

    ECHECK_APP(rp_AcqGetWritePointerAtTrig(&pos));
    usleep(100 + (((acq_size * 8) * dec)) / 1000);
	ECHECK_APP(rp_AcqGetDataV(RP_CH_1, pos, &acq_u_size, data[0]));
	ECHECK_APP(rp_AcqGetDataV(RP_CH_2, pos, &acq_u_size, data[1]));

	uint32_t pos_after_trig;
	rp_AcqGetWritePointer(&pos_after_trig);

	return RP_OK;
}

int lcr_getImpedance(float frequency, float _Complex *Z_out){

	float w_out;

	int decimation;
	int acq_size;
	rp_acq_decimation_t api_decimation;

	uint32_t r_shunt;
	lcr_getRShunt(&r_shunt);

	//Calculate output angular velocity
	w_out = frequency * 2 * M_PI;

	//Generate a sinusoidal wave form
	int ret_val = lcr_SafeThreadGen(RP_CH_1, frequency);
	if(ret_val != RP_OK){
		printf("Error generating api signal: %s\n", rp_GetError(ret_val));
		return RP_EOOR;
	}

	//Get decimation value from frequency
	lcr_getDecimationValue(frequency, &api_decimation, &decimation);
	
	acq_size = round((min_periodes * SAMPLE_RATE) /
		(frequency * decimation));

	float **analysis_data = multiDimensionVector(acq_size);

	ret_val = lcr_SafeThreadAcqData(analysis_data, api_decimation, acq_size, decimation);
	if(ret_val != RP_OK){
		//log error
		return RP_EOOR;
	}

	ret_val = lcr_data_analysis(analysis_data, acq_size, 0, 
				r_shunt, Z_out, w_out, decimation);

	//Disable channel 1 generation module
	ECHECK_APP(rp_GenOutDisable(RP_CH_1));

	return RP_OK;
}

/* Main Lcr meter thread */
void *lcr_MainThread(void *args){

	struct impendace_params *args_struct =
		(struct impendace_params *)args;

	set_IIC_Shunt(R_SHUNT_30);
	//uint32_t new_shunt;

lcr_getImpedance(args_struct->frequency, &args_struct->Z_out);
	/*Main lcr meter algorithm
	
	while(1){
		
		
		if(!main_params.calibration) break;
		
		float z_ampl = sqrt(powf(creal(args_struct->Z_out), 2) + 
			powf(cimag(args_struct->Z_out), 2));

		int ret_val = lcr_switchRShunt(z_ampl, &new_shunt);
		//Change shunt
		if(ret_val == 2){
			//Change r_shunt, set new r_shunt
			set_IIC_Shunt(new_shunt);
			lcr_setRShunt(new_shunt);
		}else{
			break;
		}
	}
	*/
	return RP_OK;
}

/* Main call function */
int lcr_Run(){

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

	if(lcr_CalculateData(args.Z_out) != RP_OK){
		return RP_EOOR;
	}

	return RP_OK;
}

int lcr_Correction(){
	int start_freq 		= START_CORR_FREQ;
	
	int err;
	struct impendace_params args;
	pthread_t lcr_thread_handler;

	float _Complex *amplitude_z = 
		malloc(CALIB_STEPS * sizeof(float _Complex));

	calib_t calib_mode = main_params.calibration;
	for(int i = 0; i < CALIB_STEPS; i++){
		
		args.frequency = start_freq * powf(10, i);
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
	free(amplitude_z);

	return RP_OK;
}

int lcr_CalculateData(float _Complex Z_measured){

	//float phasez = 0;
	bool calibration = false;

	const char *calibrations[] = 
		{"/opt/redpitaya/www/apps/lcr_meter/CALIB_OPEN",
		 "/opt/redpitaya/www/apps/lcr_meter/CALIB_SHORT"};

	FILE *f_open  = fopen(calibrations[0], "r");
	FILE *f_short = fopen(calibrations[1], "r");

	//Calibration was made
	if((f_open != NULL) && (f_short != NULL)){
		calibration = true;	
	}

	float _Complex Z_open[CALIB_STEPS]  = {0, 0, 0, 0};
	float _Complex Z_short[CALIB_STEPS] = {0, 0, 0, 0};
	float _Complex Z_final;

	/* Read calibration from files */
	if(calibration){
		int line = 0;
		while(!feof(f_open)){
			float z_open_imag, z_open_real;
			fscanf(f_open, "%f %fi", &z_open_real, &z_open_imag);
			Z_open[line] = z_open_real + z_open_imag*I;
			line++;
		}

		line = 0;
		while(!feof(f_short)){
			float z_short_imag, z_short_real;
			fscanf(f_short, "%f %fi", &z_short_real, &z_short_imag);
			Z_short[line] = z_short_real + z_short_imag*I;
			line++;
		}	
	}

	/* --------------- CALCULATING OUTPUT PARAMETERS --------------- */
	int index = log10(main_params.frequency) - 2;

	//Calibration was made
	if(calibration){
		
		Z_final = Z_open[index] - ((Z_short[index] - 
			Z_measured) / (Z_measured - Z_open[index]));
/*
		syslog(LOG_INFO, "SHORT R: %f | SHORT I: %f\n", 
			creal(Z_short[index]), cimagf(Z_short[index]));

		syslog(LOG_INFO, "OPEN R: %f | OPEN I: %f\n", 
			crealf(Z_open[index]), cimagf(Z_open[index]));

		syslog(LOG_INFO, "JAO FINAL R: %f | FINAL I: %f\n\n",
			creal(Z_final), cimag(Z_final));
*/
	//No calibration was made
	}else{
		Z_final = Z_measured;
	}

	float w_out = 2 * M_PI * main_params.frequency;
	float Y = 1 / Z_final;
	//float Y_abs = sqrtf(powf(creal(Y), 2) + powf(cimag(Y), 2));
	
	//float B_p = cimag(Y);
	float G_p = creal(Y);

	float R_s = creal(Z_final);
	float X_s = cimag(Z_final);

	/* Calculate Z */
	calc_data->lcr_amplitude = 
		sqrtf(powf(creal(Z_final), 2) + powf(cimag(Z_final), 2));

	/* Calculate L */
	calc_data->lcr_L = X_s / w_out;

	/* Calculate C */
	calc_data->lcr_C = -1 / (w_out * X_s);

	/* Calculate R */
	calc_data->lcr_R = 1 / G_p;

	/* Calculate phase */
	calc_data->lcr_phase = 
		(180 / M_PI) * atan2f(cimag(Z_final), creal(Z_final));

	/* Calculate Q */
	calc_data->lcr_Q = X_s / R_s;

	/* Calculate D */
	calc_data->lcr_D = -1 / calc_data->lcr_Q;  

	/* Calculate ESR */
	
	if(calibration){
		fclose(f_short);
		fclose(f_open);
	}

	return RP_OK;
}

int lcr_CopyParams(lcr_main_data_t *params){
	params->lcr_amplitude    = calc_data->lcr_amplitude;
	params->lcr_phase        = calc_data->lcr_phase;
	params->lcr_D         	 = calc_data->lcr_D;
	params->lcr_Q            = calc_data->lcr_Q;
	params->lcr_E         	 = calc_data->lcr_E;
	params->lcr_L         	 = calc_data->lcr_L;
	params->lcr_C         	 = calc_data->lcr_C;
	params->lcr_R         	 = calc_data->lcr_R;
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

	float u_dut[size];
	float i_dut[size];

	float u_dut_s[2][size];
	float i_dut_s[2][size];

	float component_lock_in[2][1];

	for(int i = 0; i < size; i++){
		u_dut[i] = data[0][i] - data[1][i];
		i_dut[i] = data[1][i] / r_shunt;
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
	component_lock_in[0][0] = trapezoidalApprox(u_dut_s[0], T, size); //X
	component_lock_in[0][1] = trapezoidalApprox(u_dut_s[1], T, size); //Y 
	component_lock_in[1][0] = trapezoidalApprox(i_dut_s[0], T, size); //X
	component_lock_in[1][1] = trapezoidalApprox(i_dut_s[1], T, size); //Y

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

int lcr_SetCalibMode(calib_t calibrated){
	main_params.calibration = calibrated;
	return RP_OK;
}

int lcr_GetCalibMode(calib_t *mode){
	*mode = main_params.calibration;
	return RP_OK;
}