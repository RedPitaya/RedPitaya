/**
 * $Id: $
 *
 * @brief Red Pitaya library Bode analyzer module interface
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */
#include <complex.h>
#include <limits.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <math.h>
#include <pthread.h>
//#include <mutex>
#include "ba_api.h"


#define EXEC_CHECK_MUTEX(x, mutex){ \
 		int retval = (x); \
 		if(retval != RP_OK) { \
            pthread_mutex_unlock((&mutex)); \
 			return retval; \
 		} \
}


static std::vector<float> calib_data;
static pthread_mutex_t mutex;

// float ba_trapezoidalApprox(double *data, float T, int size) 
// {
// 	double result = 0;
// 	for(int i = 0; i < size - 1; i++){
// 		result += (data[i] + data[i+1]);
// 	}
// 	result = ((T / 2.0) * result);
	
// 	// double trapezoidal_integral = 0;
//     // for(int step = 0; step < size - 1; step++) {
//     //     const double x1 = step * T;
//     //     const double x2 = (step+1) * T;

//     //     trapezoidal_integral += 0.5*(x2-x1)*(data[step] + data[step+1]);
//     // }

//     // return trapezoidal_integral;

// 	return result;
// }

// float trapezoidalApprox(double *data, float T, int size){
//     double result = 0;

//     for(int i = 0; i < size - 1; i++){
//         result += data[i] + data[i+1];
//     }
//     result = ((T / 2.0) * result);
//     return result;
// }

double RMS(const std::vector<float> &data, int size){
    double result = 0;
    for(int i = 0; i < size; i++){
        result += data[i] * data[i];
    }
    result =  sqrt(result / size);
    return result;
}


float l_inter(float a, float b, float f)
{
    return a + f * (b - a);
}

// void ba_getDecimationValue(float frequency, rp_acq_decimation_t *api_dec, int *dec_val)
// {
// 	const int g_dec[] = { 1,  8,  64,  1024,  8192,  65536 };
// 	int f = 0;

// 	if (frequency >= 160000) f = 0;
// 	else if (frequency >= 20000) f = 1;
// 	else if (frequency >= 2500) f = 2;
// 	else if (frequency >= 160) f = 3;
// 	else if (frequency >= 20) f = 4;
// 	else if (frequency >= 0) f = 5;

// 	*api_dec = (rp_acq_decimation_t)f;
// 	*dec_val = g_dec[f];
// }

double InterpolateLagrangePolynomial (double x, double* x_values, double* y_values, int size)
{
	double lagrange_pol = 0;
	double basics_pol;

	for (int i = 0; i < size; i++)
	{
		basics_pol = 1;
		for (int j = 0; j < size; j++)
		{
			if (j == i) continue;
			basics_pol *= (x - x_values[j])/(x_values[i] - x_values[j]);		
		}
		lagrange_pol += basics_pol*y_values[i];
	}
	return lagrange_pol;
}



std::pair<double, double> crossCorrelation(double *xSignalArray, double *ySignalArray, int lenghtArray,int sepm_Per)
{
    double max_value = -100000.0;
    double argmax = 0;
    // Stores the final array
    double *crossCorrelate = (double*)malloc(((lenghtArray * 2) + 1) * sizeof(double));
    memset(crossCorrelate, 0, ((lenghtArray * 2) + 1) * sizeof(double));
    // Traver the two given arrays
    // for (int i = 0; i < lenghtArray; i++) {
    //     for (int j = 0; j < lenghtArray; j++) {
    //    crossCorrelate[i + j] += (xSignalArray[i] * ySignalArray[lenghtArray - j - 1]);
    //     }
    // }

	// double *a = xSignalArray;
    // for (int i = 0; i < lenghtArray; i++) {
	// 	double *b = (ySignalArray + lenghtArray - 1);
	// 	double *c = crossCorrelate + i;
    //     for (int j = 0; j < lenghtArray; j++) {
    //         // Update the convolution array
    //         *c += ((*a) * (*b));
	// 		--b;
	// 		++c;
    //     }
	// 	++a;
    // }

	double *a = xSignalArray;
	double *b = ySignalArray;
	int start_k = lenghtArray - sepm_Per < 0 ? 0 : lenghtArray - sepm_Per;
	int end_k = lenghtArray + sepm_Per > lenghtArray * 2 ? lenghtArray * 2 : lenghtArray + sepm_Per;

	for(int k = start_k ; k < end_k; k++){
		for (int i = 0; i < lenghtArray; i++) {
			int j = k - i;
			if(j < 0 || j >= lenghtArray) continue;
			crossCorrelate[k] +=a[i]*b[lenghtArray - j - 1]; 

		}
	}


	for(int i = 0 ; i < (lenghtArray * 2) ; ++i){
        if ((crossCorrelate[i]) > max_value)
        {
            max_value = (crossCorrelate[i]);
            argmax = i;
        }
	}

	if (argmax > 0 && argmax < lenghtArray * 2 ){
		double x_axis[] = { 0 , 1 , 2};
		double y_axis[3];
		y_axis[0] = crossCorrelate[(int)argmax - 1];
		y_axis[1] = crossCorrelate[(int)argmax];
		y_axis[2] = crossCorrelate[(int)argmax + 1];
		double eps = 0.0001;
		double start = 0;
		double stop = 2;
		double y_start = InterpolateLagrangePolynomial(start,x_axis,y_axis,3);
		double y_stop  = InterpolateLagrangePolynomial(stop,x_axis,y_axis,3);
		//fprintf(stderr,"Y(%f,%f,%f)\n",y_axis[0],y_axis[1],y_axis[2]);
		double max_inter = 0;
		while(eps  < (stop - start)){
			double z = (stop - start)/2.0 + start;
			double y_sub  = InterpolateLagrangePolynomial(z,x_axis,y_axis,3);
			//fprintf(stderr,"start %f stop %f z %f (%f,%f,%f)\n",start,stop,z,y_start,y_stop,y_sub);
			if (y_sub > y_start || y_sub > y_stop){
				if (y_start > y_stop){
					stop = z;
					y_stop = y_sub;
				}else{
					start = z;
					y_start = y_sub;
				}
				max_inter = z;
			}
			else{
				max_inter = y_stop > y_start? stop : start;
				break;
			}
		}
		//argmax = InterpolateLagrangePolynomial(max_inter,x_axis,y_axis,3);
		argmax = argmax-1 + max_inter;
	}

    free(crossCorrelate);
    auto answer = std::make_pair(max_value, argmax);
    return answer;
}

double phaseCalculator(double freq_HZ, double samplesPerSecond, int numSamples,int sepm_Per, double *xSamples, double *ySamples)
{
    double maxValue, timeShift, phaseShift, timeLine;
    double argmax;
    auto result = crossCorrelation(xSamples, ySamples, numSamples,sepm_Per);
    maxValue = result.first;
    argmax = result.second;

    timeLine = ((numSamples - 1) / samplesPerSecond);
    timeShift = ((timeLine * argmax) / (numSamples - 1)) + (-timeLine);
    phaseShift = ((2 * M_PI) * fmod((freq_HZ * timeShift), 1.0)) - M_PI;
    if (phaseShift <= -M_PI/2) 
		phaseShift += M_PI;
	else if (phaseShift >= M_PI/2) 
		phaseShift -= M_PI;
	//fprintf(stderr,"freq_HZ %f  samplesPerSecond %f\n",freq_HZ,samplesPerSecond);
	//fprintf(stderr,"argmax %f  maxValue %f timeShift %f phaseShift %f \n",argmax,maxValue,timeShift,phaseShift * 180.0 / M_PI);
	return phaseShift;
}


// double InterpolateLagrangePolynomial (double x, double* x_values, double* y_values, int size)
// {
// 	double lagrange_pol = 0;
// 	double basics_pol;

// 	for (int i = 0; i < size; i++)
// 	{
// 		basics_pol = 1;
// 		for (int j = 0; j < size; j++)
// 		{
// 			if (j == i) continue;
// 			basics_pol *= (x - x_values[j])/(x_values[i] - x_values[j]);		
// 		}
// 		lagrange_pol += basics_pol*y_values[i];
// 	}
// 	return lagrange_pol;
// }

// int calcSinInterplated(int pos,int window_size, double offset , double* y_values, int size){
// 	if (offset >= 1){
// 		int index = offset;
// 		return y_values[pos + index] > y_values[pos - index] ? 1 : -1;
// 	}

// 	double x[window_size * 2 + 1];
// 	double y[window_size * 2 + 1];
// 	for(int i = -window_size; i <= window_size;i++){
// 		x[i+window_size] = (double)i/(double)window_size;
// 		y[i+window_size] = y_values[pos + i];
// 	//	fprintf(stderr,"%f %f\n",x[i+window_size],y[i+window_size]);
// 	}
// 	fprintf(stderr,"Offset %f\n",offset);
// 	auto max_1 =  InterpolateLagrangePolynomial(offset,x,y,window_size * 2 + 1);
// 	auto max_2 =  InterpolateLagrangePolynomial(-offset,x,y,window_size * 2 + 1);
// 	fprintf(stderr,"Max1 %f max2 %f\n",max_1,max_2);
// 	return max_1 > max_2 ? -1 : 1;
// }

int rp_BaDataAnalysis(const rp_ba_buffer_t &buffer,
					uint32_t size,
					float samplesPerSecond,
					float _freq, 
					int   samples_period,
					float *gain,
					float *phase_out,
					float input_threshold) 
{
	// {
	// 	double T = (decimation / ADC_SAMPLE_RATE);
		
	// 	double u_dut[size];
	// 	double i_dut[size];
	// 	// double r_RC = (r_shunt * (1.0 / (w_out * c_calib)))
	// 	// 		/ (r_shunt + (1.0 / (w_out * c_calib)));
		
	// 	for(int i = 0; i < size; i++) {
	// 		i_dut[i] =  buffer.ch2[i] / 1;
	// 		u_dut[i] = buffer.ch1[i] - buffer.ch2[i];
	// 	}

	// 	double u_dut_s[2][size];
	// 	double i_dut_s[2][size];
	// 	double ang = 0;
	// 	double deltaPhi = T * w_out;
	// 	// To complex
	// 	for(int i = 0; i < size; ang += deltaPhi, i++) {
	// 		// Voltage
	// 		u_dut_s[1][i] = u_dut[i] * cos(ang); // Imag
	// 		u_dut_s[0][i] = u_dut[i] * sin(ang); // Real
	// 		// Current
	// 		i_dut_s[1][i] = i_dut[i] * cos(ang); // Imag
	// 		i_dut_s[0][i] = i_dut[i] * sin(ang); // Real
	// 	}

	// 	/** The integral over the interval multiple of the period is equivalent
	// 	** to  the constant component multiply to interval length **/
	// 	/* Trapezoidal approximation */
	// 	double U_X = trapezoidalApprox(u_dut_s[0], T, size);
	// 	double U_Y = trapezoidalApprox(u_dut_s[1], T, size);
	// 	double I_X = trapezoidalApprox(i_dut_s[0], T, size);
	// 	double I_Y = trapezoidalApprox(i_dut_s[1], T, size);

	// 	/* Calculating current amplitude and phase */
	// 	double i_dut_ampl = 2.0 * (sqrtf(powf(I_X, 2.0) + powf(I_Y, 2.0)));
	// 	double i_dut_phase_ampl = atan2f(I_Y, I_X);

	// 	/* Calculating voltage amplitude and phase */
	// 	double u_dut_ampl = 2.0 * (sqrtf(powf(U_X, 2.0) + powf(U_Y, 2.0)));
	// 	double u_dut_phase_ampl = atan2f(U_Y, U_X);
	
	// 	double phase_z_rad =  u_dut_phase_ampl - i_dut_phase_ampl;
	//     double z_ampl = u_dut_ampl / i_dut_ampl;
	
	// 	phase_z_rad = phase_z_rad * (180.0 / M_PI);
	// 	fprintf(stderr, "u_dut_ampl %f u_dut_phase_ampl %f\n",z_ampl,phase_z_rad);
	// }
	double phase = 0;
	//double phase_offset = 0;

	// double T = (decimation / ADC_SAMPLE_RATE);
	// double u_dut_1[size];
	// double u_dut_2[size];

	// double u_dut_1_s[2][size];
	// double u_dut_2_s[2][size];

	// double component_lock_in[2][2];
	int ret_value = RP_OK;

	// for (size_t i = 0; i < size; i++){
	// 	u_dut_1[i] = buffer.ch2[i];
	// 	u_dut_2[i] = buffer.ch1[i];
	// }

	// if (size > 0){
	// 	double u1_max = u_dut_1[0];
	// 	double u1_min = u_dut_1[0];
	// 	double u2_max = u_dut_2[0];
	// 	double u2_min = u_dut_2[0];
	// 	for (size_t i = 1; i < size; i++){
	// 		if (u1_max < u_dut_1[i]) u1_max = u_dut_1[i];
	// 		if (u2_max < u_dut_2[i]) u2_max = u_dut_2[i];
	// 		if (u1_min > u_dut_1[i]) u1_min = u_dut_1[i];
	// 		if (u2_min > u_dut_2[i]) u2_min = u_dut_2[i];
	// 	}
	// 	if ((u1_max - u1_min) < input_threshold) ret_value = RP_EIPV;
	// 	if ((u2_max - u2_min) < input_threshold) ret_value = RP_EIPV;

	// }

	// for (size_t i = 0; i < size; i++){
	// 	ang = (i * T * w_out);

	// 	u_dut_1_s[0][i] = u_dut_1[i] * sin(ang);
	// 	u_dut_1_s[1][i] = u_dut_1[i] * sin(ang + (M_PI / 2));

	// 	u_dut_2_s[0][i] = u_dut_2[i] * sin(ang);
	// 	u_dut_2_s[1][i] = u_dut_2[i] * sin(ang + (M_PI / 2));
	// }

	// /* Trapezoidal approximation */
	// component_lock_in[0][0] = ba_trapezoidalApprox(u_dut_1_s[0], T, size); //U_X
	// component_lock_in[0][1] = ba_trapezoidalApprox(u_dut_1_s[1], T, size); //U_Y
	// component_lock_in[1][0] = ba_trapezoidalApprox(u_dut_2_s[0], T, size); //I_X
	// component_lock_in[1][1] = ba_trapezoidalApprox(u_dut_2_s[1], T, size); //I_Y
	// fprintf(stderr,"%f %f %f %f\n",component_lock_in[0][0],component_lock_in[0][1],component_lock_in[1][0],component_lock_in[1][1]);
	// /* Calculating voltage amplitude and phase */
	// u_dut_1_ampl = 2.0 * (sqrtf(powf(component_lock_in[0][0], 2.0) + powf(component_lock_in[0][1], 2.0)));

	// u_dut_1_phase = atan2f(component_lock_in[0][1], component_lock_in[0][0]);

	// /* Calculating current amplitude and phase */
	// u_dut_2_ampl = 2.0 * (sqrtf(powf(component_lock_in[1][0], 2.0) + powf(component_lock_in[1][1], 2.0)));

	// u_dut_2_phase = atan2f(component_lock_in[1][1], component_lock_in[1][0]);

	// phase = u_dut_1_phase - u_dut_2_phase;


	//double buf2_offset[size];
	// double scale = 1;
	// int old_size = size;
	// if (samples_period < 30) {
	// 	scale = (30.0 / (double)samples_period);
	// 	size /= scale; 	
	// }
	// int new_len = size * scale;
	double buf1[size];
	double buf2[size];
	//double x_axis[new_len];
	// fprintf(stderr,"Test %d\n",new_len);
	// if (scale > 1){
	// 	for (size_t i = 0; i < old_size; i++){
	// 		x_axis[i] = i;
	// 	}
	// 	fprintf(stderr,"Test1.1 %d\n",new_len);
	// 	for (size_t i = 0; i < new_len; i++){
	// 		buf1[i] = InterpolateLagrangePolynomial(((double)i)/scale,x_axis,buf1,old_size);
	// 		buf2[i] = InterpolateLagrangePolynomial(((double)i)/scale,x_axis,buf2,old_size);
	// 	}
	// 	fprintf(stderr,"Test1.2 %d\n",new_len);
	// 	size = new_len;
	// }else{
	double max_ch1 = -100000;
	double max_ch2 = -100000;
	double min_ch1 = 100000;
	double min_ch2 = 100000;
	
	;

	for (size_t i = 0; i < size; i++){
		buf1[i] = buffer.ch1[i];
		buf2[i] = buffer.ch2[i];
		// Filtring 
		//buf1[i]  = buf1[i] * ( 0.54 - 0.46 * cos(2*M_PI*i / (double)(size-1)));
		//buf2[i]  = buf2[i] * ( 0.54 - 0.46 * cos(2*M_PI*i / (double)(size-1)));

		// 
		if ((buf1[i]) > max_ch1) {
			max_ch1 = (buf1[i]);
		}
		if ((buf2[i]) > max_ch2) {
			max_ch2 = (buf2[i]);
		}
		if ((buf1[i]) < min_ch1) {
			min_ch1 = (buf1[i]);
		}
		if ((buf2[i]) < min_ch2) {
			min_ch2 = (buf2[i]);
		}

		//	buf2_offset[i] = buffer.ch2[(i + samples_period / 4) % size];
	}
	// }
	// fprintf(stderr,"Test2\n");

	

// 	double dot = 0;
// 	double norm1 = 0;
// 	double norm2 = 0;

// 	// for (size_t i = 0; i < size; i++){
// 	// 	dot += buf1[i] * buf2[i];
// 	// 	norm1 += (buf1[i] * buf1[i]);
// 	// 	norm2 += (buf2[i] * buf2[i]);
// 	// }

//   	norm1 = sqrt(norm1);
//  	norm2 = sqrt(norm2);
//  	if (norm1 * norm2 != 0){
//  		phase =  acos(( dot / (norm1 * norm2)));
// // //		fprintf(stderr,"P = %f, DEG = %f\n",phase , phase*180/M_PI);
//  	}else{
// 		fprintf(stderr,"Norm is zero\n");
//  		ret_value = RP_EIPV;
//  	}

	double sig1_rms =  RMS(buffer.ch1,size);
	double sig2_rms =  RMS(buffer.ch2,size);
	*gain = sig2_rms / sig1_rms;
	
	// for(int i = 0; i < size ; i++){
	// 	buf1[i] = buf1[i] * (1.0 / max_ch1); 
	// 	buf2[i] = buf2[i] * (1.0 / max_ch2); 
	// }
	if ((max_ch1 - min_ch1) < input_threshold) ret_value = RP_EIPV;
	if ((max_ch2 - min_ch2) < input_threshold) ret_value = RP_EIPV;
	//fprintf(stderr,"pp_ch1 %f pp_ch2 %f\n", (max_ch1 - min_ch1), (max_ch2 - min_ch2));
	int s = size;
	// if (samples_period * 4 < size){
	// 	s = samples_period * 4;
	// 	// if (s < 100 && size > 100)
	// 	// 	s = 100;
	// }
	auto phase2 = phaseCalculator(_freq,samplesPerSecond, size ,samples_period,buf1,buf2);
	// std::vector<int> points;
	// int dirs_pos = 0;
	// int dirs_neg = 0;


	// for( int j = 2 ; j < size / samples_period - 2; j++){
	// 	float min_val = -1000;
	// 	int index = -1;
	// 	for( int i = 0 ; i < samples_period ; i++){
	// 		auto val = buf1[j * samples_period + i];
	// 		//fprintf(stderr,"%d %d %d %f %d\n",j * samples_period + i, i , j , val,samples_period);
	// 		if(val > min_val){
	// 			index = j * samples_period + i;
	// 			min_val = val;
	// 		}
	// 	}
	// 	if (index != -1){
	// 		points.push_back(index);
	// 	}
	// }

	// for(int i = 0; i < points.size() && i < 10;i++){
	// 	//fprintf(stderr,"MaxPos %d %f\n",points[i],buf1[points[i]]);
	// 	auto offset = phase / (2 * M_PI) * samples_period;
	// 	if (calcSinInterplated(points[i],samples_period,offset,buf2,size) > 0){
	// 		dirs_pos++;
	// 	}else{
	// 		dirs_neg++;
	// 	}
	// }


	/* Phase has to be limited between M_PI and -M_PI. */
	if (phase2 <= -M_PI) 
		phase2 += 2*M_PI;
	else if (phase2 >= M_PI) 
		phase2 -= 2*M_PI;

	*phase_out = phase2 *  (180.0 / M_PI) ;//* (phase2 >= 0 ? 1 : -1);
	//*gain = u_dut_1_ampl / u_dut_2_ampl;
	//fprintf(stderr,"phase %f phase2 %f offset %f semples %d size %d\n\n",phase,phase2  , phase / (2 * M_PI) * samples_period ,samples_period ,size);
	return ret_value;
}



float rp_BaCalibGain(float _freq, float _ampl)
{
    for (size_t i = 3; i < calib_data.size(); i += 3) // 3 - freq, ampl, phase
    {
        if (calib_data[i] >= _freq)
        {
			float f0 = calib_data[i-3];
			float f1 = calib_data[i];
			float t =  (f1 - f0) != 0 ? (_freq - f0)/(f1 - f0) : 0;
            //fprintf(stdout, "%f %f\n", _ampl, _ampl - l_inter(calib_data[i - 3 + 1], calib_data[i + 1], _ampl));
            return _ampl - l_inter(calib_data[i - 3 + 1], calib_data[i + 1], t);
        }
    }

    return _ampl;
}

float rp_BaCalibPhase(float _freq, float _phase)
{
    for (size_t i = 3; i < calib_data.size(); i += 3) // 3 - freq, ampl, phase
    {
        if (calib_data[i] >= _freq)
        {
			float f0 = calib_data[i-3];
			float f1 = calib_data[i];
			float t =  (f1 - f0) != 0 ? (_freq - f0)/(f1 - f0) : 0;
            return _phase - l_inter(calib_data[i - 3 + 2], calib_data[i + 2], t);
        }
    }

    return _phase;
}


int rp_BaResetCalibration()
{
	calib_data.clear();
	remove(BA_CALIB_FILENAME);
    return RP_OK;
}

int rp_BaReadCalibration()
{
	int ignored __attribute__((unused));
    // if current mode != calibration then load calibration params
    if (calib_data.empty() && access(BA_CALIB_FILENAME, R_OK) == F_OK)
    {
        // fprintf(stderr, "Calibration data exist\n");
        FILE* calF = fopen(BA_CALIB_FILENAME, "r");
        fseek(calF, 0, SEEK_END);
        int size = ftell(calF);
        fseek(calF, 0, SEEK_SET);
        calib_data.resize(size/sizeof(float));
        ignored = fread((void*)calib_data.data(), sizeof(calib_data[0]), size/sizeof(calib_data[0]), calF);
        fclose(calF);
    }
    else
    {
        return RP_RCA;
    }
    return RP_OK;
}

int rp_BaWriteCalib(float _current_freq,float _amplitude,float _phase_out)
{
    FILE* calib_file = nullptr;
    calib_file = fopen(BA_CALIB_FILENAME, "a");
    if (calib_file == nullptr){
        return RP_RCA;
    }
    
    float data[] = {_current_freq, _amplitude, _phase_out};
    fwrite(data, sizeof(float), 3, calib_file);
    fclose(calib_file);
    calib_file = nullptr;
    return RP_OK;
}

bool rp_BaGetCalibStatus(){
	return !calib_data.empty();
}


/* Acquire functions. Callback to the API structure */
int rp_BaSafeThreadAcqPrepare()
{
	pthread_mutex_lock(&mutex);
	EXEC_CHECK_MUTEX(rp_AcqReset(), mutex);
	EXEC_CHECK_MUTEX(rp_AcqSetDecimation(RP_DEC_1), mutex);
	EXEC_CHECK_MUTEX(rp_AcqSetTriggerLevel(RP_T_CH_1, 0), mutex);
	EXEC_CHECK_MUTEX(rp_AcqSetTriggerLevel(RP_T_CH_2, 0), mutex);
	EXEC_CHECK_MUTEX(rp_AcqSetTriggerDelay(0), mutex);
	EXEC_CHECK_MUTEX(rp_AcqSetTriggerSrc(RP_TRIG_SRC_DISABLED), mutex);
	pthread_mutex_unlock(&mutex);
	return RP_OK;
}

/* Generate functions  */
int rp_BaSafeThreadGen(rp_channel_t _channel, float _frequency, float _ampl, float _dc_bias)
{
	pthread_mutex_lock(&mutex);
	EXEC_CHECK_MUTEX(rp_GenAmp(_channel, _ampl), mutex); //LCR_AMPLITUDE
	EXEC_CHECK_MUTEX(rp_GenOffset(_channel, _dc_bias), mutex); // 0.25
	EXEC_CHECK_MUTEX(rp_GenWaveform(_channel, RP_WAVEFORM_SINE), mutex);
	EXEC_CHECK_MUTEX(rp_GenFreq(_channel, _frequency), mutex);
	EXEC_CHECK_MUTEX(rp_GenOutEnable(_channel), mutex);
	usleep(10000);
	pthread_mutex_unlock(&mutex);
	return RP_OK;
}


int rp_BaSafeThreadAcqData(rp_ba_buffer_t &_buffer, int _decimation, int _acq_size, float _trigger)
{
	uint32_t pos = 0;
	uint32_t acq_u_size = _acq_size;
	//uint32_t acq_delay = acq_u_size > ADC_BUFFER_SIZE / 2.0 ? acq_u_size - ADC_BUFFER_SIZE / 2.0 : 0; 
	uint64_t sleep_time = static_cast<uint64_t>(_acq_size) * _decimation / (ADC_SAMPLE_RATE / 1e6);
	sleep_time = sleep_time < 1 ? 1 : sleep_time;
	bool fillState = false;
	
	rp_acq_trig_state_t trig_state = RP_TRIG_STATE_TRIGGERED;

	pthread_mutex_lock(&mutex);
	EXEC_CHECK_MUTEX(rp_AcqSetDecimationFactor(_decimation), mutex);
	EXEC_CHECK_MUTEX(rp_AcqSetTriggerDelay(ADC_BUFFER_SIZE / 2.0), mutex);
	EXEC_CHECK_MUTEX(rp_AcqStart(), mutex);
	usleep(sleep_time);
	// Trigger, it is needed for the RP_DEC_1 decimation
	EXEC_CHECK_MUTEX(rp_AcqSetTriggerSrc(RP_TRIG_SRC_NOW), mutex);
	
	for (;;) {
		EXEC_CHECK_MUTEX(rp_AcqGetTriggerState(&trig_state), mutex);

		if (trig_state == RP_TRIG_STATE_TRIGGERED) {
			break;
		} else {
			usleep(1);
		}
	}

	while(!fillState){
		
		EXEC_CHECK_MUTEX(rp_AcqGetBufferFillState(&fillState), mutex);
	}

	EXEC_CHECK_MUTEX(rp_AcqStop(), mutex); // Why the write pointer is not stopped?
	EXEC_CHECK_MUTEX(rp_AcqGetWritePointerAtTrig(&pos), mutex);
	pos++;
	// Calculate the beginning of the buffer
	// if (pos >= acq_u_size) {
	// 	pos -= acq_u_size;
	// } else {
	// 	pos = ADC_BUFFER_SIZE - acq_u_size + pos;
	// }

	EXEC_CHECK_MUTEX(rp_AcqGetDataV2(pos, &acq_u_size, _buffer.ch1.data(), _buffer.ch2.data()), mutex);
	//fprintf(stderr,"Size %d\n",acq_u_size);
	//for(int i = 0 ; i < acq_u_size ;i++){
	//	_buffer.ch1[i] = sin(8 * M_PI * (float)i/(float(acq_u_size))) * 2;
	//	_buffer.ch2[i] = sin(8 * M_PI * (float)i/(float(acq_u_size)) + M_PI/4);
	//	fprintf(stderr,"(%f,%f)",_buffer.ch1[i],_buffer.ch2[i]);
	//}
	//fprintf(stderr,"\n");
	pthread_mutex_unlock(&mutex);
	return RP_OK;
}


int rp_BaGetAmplPhase(float _amplitude_in, float _dc_bias, int _periods_number, rp_ba_buffer_t &_buffer, float* _amplitude, float* _phase, float _freq,float _input_threshold)
{
    float gain = 0;
    float phase_out = 0;

    //float w_out = 0;
    //int decimation;
    int acq_size;

    //rp_acq_decimation_t api_decimation;
    //ba_getDecimationValue(_freq, &api_decimation, &decimation);

    //Calculate output angular velocity
    //w_out = _freq * 2 * M_PI;
    //fprintf(stderr,"_freq %f\n",_freq);
    //Generate a sinusoidal wave form
    rp_BaSafeThreadGen(RP_CH_1, _freq, _amplitude_in, _dc_bias);
    int size_buff_limit = ADC_BUFFER_SIZE;
	int new_dec = round((static_cast<float>(_periods_number) * ADC_SAMPLE_RATE) / (_freq * size_buff_limit));
	new_dec = new_dec < 1 ? 1 : new_dec;
	
	if (new_dec < 16){
        if (new_dec >= 8)
            new_dec = 8;
        else
            if (new_dec >= 4)
                new_dec = 4;
            else
                if (new_dec >= 2)
                    new_dec = 2;
                else 
                    new_dec = 1;
    }
    if (new_dec > 65536){
        new_dec = 65536;
    }
	//acq_size =  round((static_cast<float>(_periods_number) * ADC_SAMPLE_RATE) / (_freq * new_dec));
	int samples_period = round(ADC_SAMPLE_RATE / (_freq * new_dec));
	
	//fprintf(stderr,"New dec %d _periods_number %d\n",new_dec,_periods_number);
	

    acq_size = size_buff_limit;
   //    fprintf(stderr,"Sample_rate %f %d\n",SAMPLE_RATE,acq_size);
    rp_BaSafeThreadAcqData(_buffer,new_dec, acq_size,_amplitude_in);
    rp_GenOutDisable(RP_CH_1);
    int ret = rp_BaDataAnalysis(_buffer, acq_size, ADC_SAMPLE_RATE / new_dec,_freq, samples_period,  &gain, &phase_out,_input_threshold);
	

    *_amplitude = 10.*logf(gain);
    *_phase = phase_out;
    //fprintf(stderr, "freq = %f dbm = %f phase = %f \n", _freq, *_amplitude, phase_out);
	if (std::isnan(*_amplitude) || std::isinf(*_amplitude)) ret =  RP_EOOR;
    return ret;
}

