/**
* $Id: $
*
* @brief Red Pitaya application library Impedance Analyzer module interface
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
#include <string.h>
#include <sys/syslog.h>
#include <complex.h>

#include "calib.h"

int store_calib(const calib_t CALIB_MODE,
				float _Complex *amplitude_z){


	char calib_data_path[100];
	strcpy(calib_data_path, APP_PATH);
	strcat(calib_data_path, stringFromCalib(CALIB_MODE));
	//Open file pointer to store calib data
	FILE *calibration_file = fopen(&calib_data_path[0], "w+");
	
	//Write data to calib_file
	for(int i = 0; i < CALIB_STEPS; i++){
		fprintf(calibration_file, 
			"%f %fi\n", crealf(amplitude_z[i]), cimagf(amplitude_z[i]));

		syslog(LOG_INFO, "%f %fi", 
			crealf(amplitude_z[i]), cimagf(amplitude_z[i]));
	}

	fclose(calibration_file);
	return 0;
}