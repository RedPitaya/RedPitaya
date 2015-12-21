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
#include <complex.h>

#include "lcr_meter.h"
#include "utils.h"
#include "calib.h"
#include "../../rpbase/src/common.h"
#include "./common.h"
#include "redpitaya/rp.h"

/* Global variables definition */
int 					min_periodes = 8;
pthread_mutex_t 		mutex;
int 					overflow_limitation = 0;

/* Init lcr params struct */
lcr_params_t main_params = {0, 0, CALIB_NONE, false, false, true};

/* Main lcr data params */
lcr_main_data_t *calc_data;

struct impendace_params {
	float frequency;
	float _Complex z_out;
	float phase_out;
};

/* R shunt values definition */
const double SHUNT_TABLE[] = 
	{30, 75, 300, 750, 3300, 7500, 30000, 75000, 430000, 3000000};

const int RANGE_FORMAT[] =
	{10.0, 100.0, 1000.0, 10000.0};

const double RANGE_UNITS[] =
	{10e9, 10e6, 10e3, 1, 10e-3, 10e-6};

/* Init the main API structure */
int lcr_Init(){

	/* Init mutex thread */
	if(pthread_mutex_init(&mutex, NULL)){
		RP_LOG(LOG_ERR, "Failed to initialize mutex: %s\n", strerror(errno));
		return RP_EOOR;
	}

	pthread_mutex_lock(&mutex);

	if(rp_Init() != RP_OK){
		RP_LOG(LOG_ERR, "Unable to inicialize the RPI API structure "
			"needed by impedance analyzer application: %s\n", strerror(errno));
		return RP_EOOR;
	}
	
	/* Set default values of the lcr structure */
	lcr_SetDefaultValues();

	EXEC_CHECK(rp_AcqReset());
	EXEC_CHECK(rp_GenReset());

	/* Malloc global vars */
	calc_data = malloc(sizeof(calc_data));

	pthread_mutex_unlock(&mutex);
	return RP_OK;
}

/* Release resources used the main API structure */
int lcr_Release(){
	pthread_mutex_lock(&mutex);
	rp_Release();
	free(calc_data);
	RP_LOG(LOG_INFO, "Releasing Red Pitaya library resources.\n");
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
	calc_data = realloc(&calc_data, sizeof(calc_data));
	pthread_mutex_unlock(&mutex);
	return RP_OK;
}

int lcr_SetDefaultValues(){
	EXEC_CHECK(lcr_setRShunt(2));
	EXEC_CHECK(lcr_SetFrequency(1000.0));
	EXEC_CHECK(lcr_SetCalibMode(CALIB_NONE));
	EXEC_CHECK(lcr_SetMeasTolerance(0));
	EXEC_CHECK(lcr_SetMeasRangeMode(0));
	EXEC_CHECK(lcr_SetRangeFormat(0));
	EXEC_CHECK(lcr_SetRangeUnits(0));
	EXEC_CHECK(lcr_SetMeasSeries(false));
	return RP_OK;
}

/* Generate functions  */
int lcr_SafeThreadGen(rp_channel_t channel, 
	           		  float frequency){

	pthread_mutex_lock(&mutex);
	EXEC_CHECK(rp_GenAmp(channel, LCR_AMPLITUDE));
	EXEC_CHECK(rp_GenOffset(channel, 0.25));
	EXEC_CHECK(rp_GenWaveform(channel, RP_WAVEFORM_SINE));
	EXEC_CHECK(rp_GenFreq(channel, frequency));
	EXEC_CHECK(rp_GenOutEnable(channel));
	usleep(10000);
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

	pthread_mutex_lock(&mutex);
	EXEC_CHECK(rp_AcqReset());	
	EXEC_CHECK(rp_AcqSetDecimation(decimation));
	EXEC_CHECK(rp_AcqSetTriggerLevel(0.5));
	EXEC_CHECK(rp_AcqSetTriggerDelay(acq_size - (ADC_BUFF_SIZE / 2)));
	EXEC_CHECK(rp_AcqSetTriggerSrc(RP_TRIG_SRC_CHA_PE));
	EXEC_CHECK(rp_AcqStart());

	state = RP_TRIG_STATE_TRIGGERED;
	int retiries = 10000; //micro seconds
	while(retiries > 0){
        rp_AcqGetTriggerState(&state);
        if(state == RP_TRIG_STATE_TRIGGERED){
        	break;
        }
        retiries--;
    }

    EXEC_CHECK(rp_AcqGetWritePointerAtTrig(&pos));
    usleep(1000 + (((acq_size * 8) * dec)) / 1000);
	EXEC_CHECK(rp_AcqGetDataV(RP_CH_1, pos, &acq_u_size, data[0]));
	EXEC_CHECK(rp_AcqGetDataV(RP_CH_2, pos, &acq_u_size, data[1]));
	pthread_mutex_unlock(&mutex);

	return RP_OK;
}

int lcr_getImpedance(float frequency, 
	                 float _Complex *z_out, 
	                 float *phase_out){

	float w_out;
	int decimation;
	int acq_size;
	rp_acq_decimation_t api_decimation;

	int r_shunt_index;
	lcr_getRShunt(&r_shunt_index);

	double r_shunt = SHUNT_TABLE[r_shunt_index];

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
		(frequency * decimation));acq_size = round((min_periodes * SAMPLE_RATE) /
		(frequency * decimation));

	float **analysis_data = multiDimensionVector(acq_size);

	ret_val = lcr_SafeThreadAcqData(analysis_data, api_decimation, acq_size, decimation);
	if(ret_val != RP_OK){
		//log error
		return RP_EOOR;
	}

	ret_val = lcr_data_analysis(analysis_data, acq_size, 0, 
				r_shunt, w_out, decimation, z_out, phase_out);

	//Disable channel 1 generation module
	EXEC_CHECK(rp_GenOutDisable(RP_CH_1));
	return RP_OK;
}

/* Main Lcr meter thread */
void *lcr_MainThread(void *args){

	struct impendace_params *args_struct =
		(struct impendace_params *)args;

	int n_shunt_idx = main_params.r_shunt;
	set_IIC_Shunt(n_shunt_idx);

	/* Main lcr meter algorithm */
	if(main_params.calibration){

		lcr_getImpedance(args_struct->frequency, 
			&args_struct->z_out, &args_struct->phase_out);

	}else{	

		double z_abs = 0;
		lcr_getImpedance(args_struct->frequency, 
			&args_struct->z_out, &args_struct->phase_out);

		z_abs = cabs(args_struct->z_out);
		
		syslog(LOG_INFO, "ZABS: %f\n", z_abs);
		syslog(LOG_INFO, "RANGE: %d\n", main_params.range);

		switch(main_params.range){
			//Z
			case 1: 
				z_abs = RANGE_FORMAT[main_params.range_format] * 
					RANGE_UNITS[main_params.range_units];
				break;
			//L
			case 2: 
				z_abs = RANGE_FORMAT[main_params.range_format] * 
					(args_struct->frequency * 2 * M_PI) * RANGE_UNITS
						[main_params.range_units];
				break;
			//C
			case 3: 
				z_abs = 1 / (RANGE_FORMAT[main_params.range_format] * args_struct->frequency *
					2 * M_PI) * RANGE_UNITS[main_params.range_units];
				break;
			//R
			case 4: 
				z_abs = RANGE_FORMAT[main_params.range_format] * 
					RANGE_UNITS[main_params.range_units];
				break;
			//Auto-Mode
			case 0: 
				z_abs = z_abs;
				break;
		}

		lcr_checkRShunt(z_abs, 
			SHUNT_TABLE[n_shunt_idx], &n_shunt_idx);

		main_params.r_shunt = n_shunt_idx;
		set_IIC_Shunt(main_params.r_shunt);

		if(z_abs > 10e6 || z_abs < 1) overflow_limitation++;
		if(overflow_limitation > 10){
			main_params.r_shunt = 2;
			set_IIC_Shunt(main_params.r_shunt);
			overflow_limitation = 0;
		}
	}

	//Exit thread
	pthread_exit(0);
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
		return RP_EOOR;
	}
	pthread_join(lcr_thread_handler, 0);
	if(lcr_CalculateData(args.z_out, args.phase_out) != RP_OK){
		return RP_EOOR;
	}

	return RP_OK;
}

int lcr_Correction(){
	int start_freq 		= START_CORR_FREQ;
	
	int ret_val;
	struct impendace_params args;
	pthread_t lcr_thread_handler;

	float _Complex *amplitude_z = 
		malloc(CALIB_STEPS * sizeof(float _Complex));

	calib_t calib_mode = main_params.calibration;
	
	char command[100];
	//Set system to read-write
	strcpy(command, "rw");
	
	ret_val = system(command);
	if(ret_val != 0){
		return RP_EOOR;
	}

	for(int i = 0; i < CALIB_STEPS; i++){
		
		args.frequency = start_freq * powf(10, i);
		ret_val = pthread_create(&lcr_thread_handler, 0, lcr_MainThread, &args);
		if(ret_val != 0){
			printf("Main thread creation failed.\n");
			return RP_EOOR;
		}

		pthread_join(lcr_thread_handler, 0);
		amplitude_z[i] = args.z_out;
	}
	
	//Store calibration
	store_calib(calib_mode, amplitude_z);
	
	free(amplitude_z);

	//Set system to read-only
	strcpy(command, "ro");
	
	ret_val = system(command);
	if(ret_val != 0){
		return RP_EOOR;
	}

	return RP_OK;
}

int lcr_CalculateData(float _Complex z_measured, float phase_measured){

	bool calibration = false;

	//Client depended parameters
	float R_out, C_out, L_out, ESR_out;

	//Client independed
	float ampl_out, phase_out, Q_out, D_out;

	const char *calibrations[] = 
		{"/opt/redpitaya/www/apps/lcr_meter/CALIB_OPEN",
		 "/opt/redpitaya/www/apps/lcr_meter/CALIB_SHORT"};

	FILE *f_open  = fopen(calibrations[0], "r");
	FILE *f_short = fopen(calibrations[1], "r");

	//Calibration was made
	if((f_open != NULL) && (f_short != NULL)){
		calibration = true;	
	}

	float _Complex z_open[CALIB_STEPS]  = {0, 0, 0, 0};
	float _Complex z_short[CALIB_STEPS] = {0, 0, 0, 0};
	float _Complex z_final;

	/* Read calibration from files */
	if(calibration){
		int line = 0;
		while(!feof(f_open)){
			float z_open_imag, z_open_real;
			fscanf(f_open, "%f %fi", &z_open_real, &z_open_imag);
			z_open[line] = z_open_real + z_open_imag*I;
			line++;
		}

		line = 0;
		while(!feof(f_short)){
			float z_short_imag, z_short_real;
			fscanf(f_short, "%f %fi", &z_short_real, &z_short_imag);
			z_short[line] = z_short_real + z_short_imag*I;
			line++;
		}	
	}

	/* --------------- CALCULATING OUTPUT PARAMETERS --------------- */
	int index = log10(main_params.frequency) - 2;

	//Calibration was made
	if(calibration){
		z_final = z_open[index] - ((z_short[index] - 
			z_measured) / (z_measured - z_open[index]));

	//No calibration was made
	}else{
		z_final = z_measured;
	}

	float w_out = 2 * M_PI * main_params.frequency;
	
	float Y = 1 / z_final;
	float G_p = creal(Y);
	float B_p = cimag(Y);
	float X_s = cimag(z_final);

	/* Series mode */
	if(main_params.series){
		R_out = creal(z_final);
		C_out = -1 / (w_out * X_s);
		L_out = X_s / w_out;
		ESR_out = R_out;
	}else{
		R_out = 1 / G_p;
		C_out = B_p / w_out;
		L_out = -1 * (w_out * B_p);
		ESR_out = 1; //TODO
	}

	Q_out = X_s / R_out;
	D_out = -1 / Q_out;
	ampl_out = cabs(z_final);
	phase_out = phase_measured;

	//Set output structure pointers
	calc_data->lcr_amplitude = ampl_out;
	calc_data->lcr_phase     = phase_out;
	calc_data->lcr_D 		 = D_out;
	calc_data->lcr_Q         = Q_out;
	calc_data->lcr_ESR       = ESR_out;
	calc_data->lcr_L         = L_out;
	calc_data->lcr_C         = C_out;
	calc_data->lcr_R         = R_out;

	//Close files, if calibration	
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
	params->lcr_ESR          = calc_data->lcr_ESR;
	params->lcr_L         	 = calc_data->lcr_L;
	params->lcr_C         	 = calc_data->lcr_C;
	params->lcr_R         	 = calc_data->lcr_R;
	return RP_OK;
}


int lcr_data_analysis(float **data, 
					uint32_t size, 
					float dc_bias, 
					int r_shunt, 
					float w_out, 
					int decimation,
					float _Complex *z_out,
					float *phase_out){

	/* Forward vector and variable declarations */
	float ang, u_dut_ampl, u_dut_phase_ampl, i_dut_ampl, i_dut_phase_ampl,
		phase_z_rad, z_ampl;

	float T = (decimation / SAMPLE_RATE);

	float u_dut[size];
	float i_dut[size];

	float u_dut_s[2][size];
	float i_dut_s[2][size];

	float component_lock_in[2][2];

	for(int i = 0; i < size; i++){
		u_dut[i] = data[0][i] - data[1][i];
		i_dut[i] = data[1][i] / r_shunt;
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
	component_lock_in[0][0] = trapezoidalApprox(u_dut_s[0], T, size); //U_X
	component_lock_in[0][1] = trapezoidalApprox(u_dut_s[1], T, size); //U_Y 
	component_lock_in[1][0] = trapezoidalApprox(i_dut_s[0], T, size); //I_X
	component_lock_in[1][1] = trapezoidalApprox(i_dut_s[1], T, size); //I_Y

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
	if(phase_z_rad <= (-1 * M_PI)){
		phase_z_rad = phase_z_rad + (2 * M_PI);
	}else if(phase_z_rad >= M_PI){
		phase_z_rad = phase_z_rad - (2 * M_PI);
	}

	*z_out = (z_ampl * cosf(phase_z_rad)) + (z_ampl * sinf(phase_z_rad) * I);
	*phase_out = 180.0 * (phase_z_rad / (2 * M_PI));

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

int lcr_setRShunt(int r_shunt){
	main_params.r_shunt = r_shunt;
	return RP_OK;
}

int lcr_getRShunt(int *r_shunt){
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

int lcr_SetMeasSeries(bool series){
	main_params.series = series;
	return RP_OK;
}

int lcr_GetMeasSeries(bool *series){
	*series = main_params.series;
	return RP_OK;
}

int lcr_SetMeasTolerance(int tolerance){
	main_params.tolerance = tolerance;
	return RP_OK;
}

int lcr_GetMeasTolerance(int *tolerance){
	*tolerance = main_params.tolerance;
	return RP_OK;
}

int lcr_SetMeasRangeMode(int range){
	main_params.range = range;
	return RP_OK;
}

int lcr_GetMeasRangeMode(int *range){
	*range = main_params.range;
	return RP_OK;
}

int lcr_SetRangeFormat(int format){
	main_params.range_format = format;
	return RP_OK;
}

int lcr_GetRangeFormat(int *format){
	*format = main_params.range_format;
	return RP_OK;
}

int lcr_SetRangeUnits(int units){
	main_params.range_units = units;
	return RP_OK;
}

int lcr_GetRangeUnits(int *units){
	*units = main_params.range_units;
	return RP_OK;
}

