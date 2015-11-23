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

#include "calib.h"

FILE* store_calib(const calib_t CALIB_MODE,
				  const float *amplitude_z){


	char calib_data_path[100];
	strcpy(calib_data_path, "/tmp/");
	strcat(calib_data_path, stringFromCalib(CALIB_MODE));

	//Open file pointer to store calib data
	FILE *calibration_data = fopen(&calib_data_path[0], "w+");
	
	//Write data to calib_file
	for(int i = 0; i < CALIB_SIZE; i++){
		fprintf(calibration_data, "%.10f\n", amplitude_z[i]);
	}


	return calibration_data;
}