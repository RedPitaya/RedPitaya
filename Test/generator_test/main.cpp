#include <cmath>
#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <mutex>
#include <string>

#include <getopt.h>
#include <fcntl.h>
#include <errno.h>
#include <cstring>
#include <sys/stat.h>

#include "rpApp.h"

using std::stoi; 
using std::stof;


enum { ADC_BUFF_SIZE = 16384, SAMPLE_RATE = 125000000, CUT_SIG = 50 };
bool g_verbose;
std::mutex mutex;


int load_fpga(const char* fpga_file)
{
    int fo, fi;
    int fpga_size;
    struct stat st;


    /* Get FPGA size */
    stat(fpga_file, &st);
    fpga_size = st.st_size;
    char fi_buff[fpga_size];

    fo = open("/dev/xdevcfg", O_WRONLY);
    if(fo < 0) {
        fprintf(stderr, "load_fpga() failed to open xdevcfg: %s\n", strerror(errno));
        return 1;
    }

    fi = open(fpga_file, O_RDONLY);
    if(fi < 0) {
        fprintf(stderr, "load_fpga() failed to open FPGA file: %s\n", strerror(errno));
        return 2;
    }

    /* Read FPGA file into fi_buff */
    if(read(fi, &fi_buff, fpga_size) < 0){
        fprintf(stderr, "Unable to read FPGA file: %s\n", strerror(errno));
        return 3;
    }

    /* Write fi_buff into fo */
    if(write(fo, &fi_buff, fpga_size) < 0){
        fprintf(stderr, "Unable to write to /dev/xdevcfg: %s\n", strerror(errno));
        return 4;
    }

    close(fo);
    close(fi);

    return 0;
}


void init()
{
	load_fpga("/opt/redpitaya/fpga/fpga_0.94.bit");	
    rpApp_Init();
}


void deinit()
{
	rpApp_Release();
}


size_t getPeak(const float* signal, size_t size, float amplitude)
{
	size_t peak = 0;
	for (size_t i = 50; i < size - 50; ++i)
	{
		if (signal[i] > amplitude*0.8)
		{
			if (signal[i + 1] > signal[i]) // up
				peak = i;
			else // down
				break;
		}
	}

	return peak;
}


float signal_diff(float* signal1, float* signal2, size_t size, float amplitude, float threshold, int times)
{
	size_t peak_idx1 = std::max_element(signal1 + CUT_SIG, signal1 + size - CUT_SIG) - signal1; //getPeak(signal1, size, amplitude);
	size_t peak_idx2 = std::max_element(signal2 + CUT_SIG, signal2 + size - CUT_SIG) - signal2; //getPeak(signal2, size, amplitude);
	float mean = std::accumulate(signal1 + CUT_SIG, signal1 + size - CUT_SIG, 0.f)/size;

	for (size_t i = peak_idx1; i < size - 50; ++i)
		signal1[i] -= mean;

	if (g_verbose)
		printf("p1 %d %f p2 %d %f\n", peak_idx1, signal1[peak_idx1], peak_idx2, signal2[peak_idx2]);

	int cur_times = times;
	for (size_t i = 0; i < size - std::max(peak_idx1, peak_idx2); ++i)
	{
		float dt = fabs(signal1[i + peak_idx1] - signal2[i + peak_idx2]);
		if (g_verbose)
			printf("%f %f\n", signal1[i + peak_idx1], signal2[i + peak_idx2]);
		if (dt > threshold)
			--cur_times;
		else
			cur_times = times;			

		if (!cur_times)
		{
			FILE* f = fopen("/tmp/in.txt", "w");
			for (size_t i = peak_idx1; i < size - CUT_SIG; ++i)
				fprintf(f, "%f ", signal1[i]);
			fclose(f);

			f = fopen("/tmp/out.txt", "w");
			for (size_t i = peak_idx2; i < size - CUT_SIG; ++i)
				fprintf(f, "%f ", signal2[i]);
			fclose(f);

			return dt;
		}
	}

	return 0.f;
}


void synthesis_sin(float *signal, size_t size, float freq)
{
    for (size_t i = 0; i < size; i++) 
	{
		signal[i] = sinf(2.f*M_PI*(float)i/size);
    }
}

void synthesis_sin2(float *signal, size_t size, float freq, float amplitude, int sample_rate)
{
    for (size_t i = 0; i < size; i++) 
	{
        signal[i] = sinf(2.f*M_PI*(float)i*freq/(float)sample_rate)*amplitude;
    }
}


void getDecimationValue(float frequency, rp_acq_decimation_t *api_dec, int *dec_val)
{
	const int g_dec[] = { 1,  8,  64,  1024,  8192,  65536 };
	int i = 0;

	if (frequency >= 160000)		i = 0;
	else if (frequency >= 2000) 	i = 1;
	else if (frequency >= 250)		i = 2;
	else if (frequency >= 50) 		i = 3;
	else if (frequency >= 10) 		i = 4;
	else if (frequency >= 2) 		i = 5;

	*api_dec = (rp_acq_decimation_t)i;
	*dec_val = g_dec[i];
}


int SafeThreadAcqData(float* data, rp_acq_decimation_t decimation, int acq_size, int dec)
{
	uint32_t acq_u_size = acq_size;
	uint32_t pos;

	mutex.lock();
	rp_AcqReset();
	rp_AcqSetDecimation(decimation);
	rp_AcqSetTriggerLevel(0.5);
	rp_AcqSetTriggerDelay(acq_size - (ADC_BUFF_SIZE/2));
	rp_AcqSetArmKeep(false);
    rp_AcqStart();
    rp_AcqSetTriggerSrc(RP_TRIG_SRC_NOW);
    usleep(1000000);
    rp_AcqStop();
	rp_AcqGetWritePointer(&pos);
	usleep(1234);
	rp_AcqGetWritePointer(&pos);

	rp_AcqGetWritePointerAtTrig(&pos);
	rp_AcqGetDataV(RP_CH_1, pos, &acq_u_size, data);
	mutex.unlock();

	return RP_OK;
}


int SafeThreadGen(rp_channel_t channel, float freq, float amplitude, float* data, size_t size)
{
	mutex.lock();
	rp_GenAmp(channel, amplitude);
	rp_GenOffset(channel, 0);
	rp_GenWaveform(channel, RP_WAVEFORM_ARBITRARY);
	rp_GenArbWaveform(channel, data, size);
	rp_GenFreq(channel, freq);
	rp_GenOutEnable(channel);
	usleep(10000);
	mutex.unlock();

	return RP_OK;
}


int run_test_signals(int freq, float amplitude, float threshold, int times)
{
	// get decimation and signal size
    int decimation;
    rp_acq_decimation_t api_decimation;
    getDecimationValue(freq, &api_decimation, &decimation);
	size_t size = ADC_BUFF_SIZE;
	uint32_t sample_rate = SAMPLE_RATE/decimation;

	float* out = new float[size];
	synthesis_sin(out, size, freq);
	float* out2 = new float[size];
	synthesis_sin2(out2, size, freq, amplitude, sample_rate);
    float* in = new float[size];

	SafeThreadGen(RP_CH_1, freq, amplitude, out, size);
	SafeThreadAcqData(in, api_decimation, size, decimation);
    rp_GenOutDisable(RP_CH_1);

	// diff
	float ret = signal_diff(in, out2, size, amplitude, threshold, times);

	// check error
	if (ret != 0.f)
	{	
		fprintf(stderr, "error: diff = %f, save data to /tmp/in.txt and /tmp/out.txt\n", ret);
	}

	// cleanup
	delete[] in;
	delete[] out;

	return ret != 0.f; // 0 - is good
}


int main(int argc, char** argv)
{
	init();


	float threshold = 0.5, amplitude = 1;
	int times = 5, freq = 1e6;

	static struct option long_options[] =
	{
		{"help", no_argument, 0, 'h'},
		{"verbose", no_argument, 0, 'v'},
		{"threshold", required_argument, 0, 't'},
		{"counts", required_argument, 0, 'c'},
		{"freq", required_argument, 0, 'f'},
		{"amplitude", required_argument, 0, 'a'},
		{0, 0, 0, 0}
	};

	while (1)
	{
		int option_index = 0;
		int c = getopt_long (argc, argv, "ht:c:f:a:", long_options, &option_index);
		if (c == -1)
			break;
		switch (c)
		{
			case 'h': 
				printf("usage:\nthreshold(-t 0.5) freq(-f 1000000) amplitude(-a 1) counts(-c 5) help(-h) verbose(-v false)\n");
				return 0;
			case 'v': g_verbose = true; break;
			case 't': threshold = stof(optarg); break;
			case 'c': times = stoi(optarg); break;
			case 'f': freq = stoi(optarg); break;
			case 'a': amplitude = stof(optarg); break;
		}
	}


	if (g_verbose)
		printf("freq %d amplitude %f threshold %f times %d\n", freq, amplitude, threshold, times);

	int ret = run_test_signals(freq, amplitude, threshold, times);
	if (ret)
		return ret; // error


	deinit();
}
