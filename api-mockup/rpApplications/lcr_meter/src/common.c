/**
 * $Id: $
 *
 * @brief Red Pitaya Applications library common module interface
 *
 * @Author Luka Golinar, RedPitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 */

#include <stdio.h>
#include <stdlib.h>

#include "common.h"


float vectorMax(float *data, int view_size){
	float max = -62.5e6;
	int i;
	for(i = 0; i < view_size; i++){
		(data[i] > max) ? (max = data[i]) : (max = max);
	}
	return max;
}

float trapezoidalApprox(int16_t *data, float t, int view_size){
	float result = 0;
	int i;
	for(i = 0; i < view_size - 1; i++){
		result += data[i] + data[i+1];
	}
	return ((t / 2) * result);
}

float vectorMean(float *data, int view_size){
	int i;
	float result = 0;
	for(i = 0; i < view_size; i++){
		result += data[i];
	}
	return (result / i);
}

/* TODO: SWD, function placement open discussion. Directly in 
   FS/MS or in common as a helper tool */
int16_t **multiDimensionVector(int first_dimension, int second_dimenson){

	/* Allocate first dimension */
	int16_t **data_out = malloc(first_dimension * sizeof(int16_t));

	/* For each element in the first dimension, 
	 * allocate a size equal to the second dimension*/
	for(int i = 0; i < first_dimension; i++){
		data_out[i] = malloc(second_dimenson * sizeof(int16_t));
	}
	
	return data_out;
}  