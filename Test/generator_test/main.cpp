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


double sigSum(const float* sig1, const float* sig2, size_t size)
{
	double sum = 0;
	for (size_t i = 0; i < size; ++i)
		sum += sig1[i]*sig2[i];

	return sum;
}

void writeSigToFile(const float* sig, size_t size, const char* path)
{
	FILE* f = fopen(path, "w");
	for (size_t i = 0; i < size; ++i)
		fprintf(f, "%f ", sig[i]);
	fclose(f);
}

double signal_corr(const float* signal1, const float* signal2, size_t size, float amplitude, float threshold)
{
	double K = sigSum(signal2, signal2, size);
	double k0 = sigSum(signal1, signal2, size);
	if (g_verbose)
		printf("SUM(OUT*OUT) = %f\n", K);
	size_t start = 0;

	for (size_t shift = 1; shift < size; ++shift)
	{
		double k = sigSum(signal1, signal2 + shift, size - shift);
		if (k > k0)
		{
			k0 = k;
			start = shift;
		}
	}
	if (g_verbose)
		printf("SUM(IN*OUT) = %f\n", k0);

	writeSigToFile(signal1 + start, size - start, "/tmp/in.txt");
	writeSigToFile(signal2 + start, size - start, "/tmp/out.txt");

	return fabs(1. - K/k0);
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


double run_test_signals(int freq, float amplitude, float threshold)
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

	double ret = signal_corr(in, out2, size, amplitude, threshold);
	if (g_verbose)
		printf("SUM(OUT*OUT) / SUM(IN*OUT) = %lf\n", ret);

	// cleanup
	delete[] in;
	delete[] out;

	return ret;
}


int main(int argc, char** argv)
{
	init();


	double threshold = 0.5, amplitude = 0.9;
	int freq = 1e6;

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
				printf("usage:\nthreshold(-t 0.5) freq(-f 1000000) amplitude(-a 0.9) help(-h) verbose(-v false)\n");
				return 0;
			case 'v': g_verbose = true; break;
			case 't': threshold = stof(optarg); break;
			case 'f': freq = stoi(optarg); break;
			case 'a': amplitude = stof(optarg); break;
		}
	}


	if (g_verbose)
		printf("freq %d amplitude %f threshold %f\n", freq, amplitude, threshold);

	double ret = run_test_signals(freq, amplitude, threshold);
	if (ret > threshold)
	{
		fprintf(stderr, "error: threshold < %lf\nwrite signals to /tmp/in.txt and /tmp/out.txt\n", ret);
		return 1;
	}

	deinit();
}
