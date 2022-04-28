#include <iomanip>
#include <iostream>
#include <locale>
#include <algorithm>
#include <array>
#include <vector>
#include <cmath>
#include <csignal>
#include <unistd.h>
#include <version.h>
#include <rp.h>
#include <rp_dsp.h>

#include "cli_parse_args.h"

#if defined Z20_125_4CH
#define MAX_CHANNELS 4
#else
#define MAX_CHANNELS 2
#endif

namespace {

static rp_dsp_api::CDSP g_dsp(MAX_CHANNELS,ADC_BUFFER_SIZE,ADC_SAMPLE_RATE);
static sig_atomic_t     g_quit_requested = 0;

static void quit_handler(int) {
    g_quit_requested = 1;
}

template<typename T>
auto createArray(uint32_t signalLen) -> T** {
    try{
        auto arr = new T*[MAX_CHANNELS];
        for(uint32_t i = 0; i < MAX_CHANNELS; i++){
            arr[i] = new T[signalLen];
        }
        return arr;
    }catch (const std::bad_alloc& e) {
        fprintf(stderr, "createArray() can not allocate mem\n");
        return nullptr;
    }
}

template<typename T>
auto deleteArray(T **arr) -> void {
    if (!arr) return;
    for(uint32_t i = 0; i < MAX_CHANNELS; i++){
        delete[] arr[i];
    }
    delete[] arr;
}


static void spectrum_worker(cli_args_t args) {

    int decimation = ADC_SAMPLE_RATE /  (args.freq_max * 2);

    if (decimation < 16){
        if (decimation >= 8)
            decimation = 8;
        else
            if (decimation >= 4)
                decimation = 4;
            else
                if (decimation >= 2)
                    decimation = 2;
                else 
                    decimation = 1;
    }

    auto tmp_signals = createArray<float>(g_dsp.getOutSignalMaxLength());
    auto max_signals = createArray<float>(g_dsp.getOutSignalMaxLength());
    auto min_signals = createArray<float>(g_dsp.getOutSignalMaxLength());

    auto ch_in = createArray<double>(g_dsp.getSignalMaxLength());
    auto ch_fft = createArray<double>(g_dsp.getOutSignalMaxLength());

    auto peak_pw = new float[MAX_CHANNELS];
    auto peak_pw_freq = new float[MAX_CHANNELS];
    auto peak_pw_max = new float[MAX_CHANNELS];
    auto peak_pw_freq_max = new float[MAX_CHANNELS];
    
    auto peak_set = false;

    for(uint32_t ch = 0; ch < MAX_CHANNELS; ch++){
        peak_pw_max[ch] = std::numeric_limits<float>::lowest();
        peak_pw_freq_max[ch] = std::numeric_limits<float>::lowest();
    }

    int count = args.count;

    if (args.csv) {
        count = 1;
        std::cout << "Frequency (Hz)";
        for(uint32_t ch = 0; ch < MAX_CHANNELS; ch++){
            std::cout << ", ch"<< ch << " (dB)";
        }
        std::cout << "\n";
    } else if (args.csv_limit) {        
        std::cout << "Frequency (Hz)";
        for(uint32_t ch = 0; ch < MAX_CHANNELS; ch++){
            std::cout << ", ch"<< ch << " min (dB)";
            std::cout << ", ch"<< ch << " max (dB)";
        }
        std::cout << "\n";
    }
    
    bool is_infinity = count < 0;
    uint32_t buffer_size = ADC_BUFFER_SIZE;
    while (!g_quit_requested && ((count > 0) || is_infinity)) {
        rp_AcqSetDecimation((rp_acq_decimation_t)decimation);
        rp_AcqSetTriggerDelay(buffer_size - ADC_BUFFER_SIZE / 2.0);
        rp_AcqStart();
        usleep(10);
        rp_AcqSetTriggerSrc(RP_TRIG_SRC_NOW);

         rp_acq_trig_state_t stateTrig = RP_TRIG_STATE_WAITING;
        /* polling until data is ready */
        while(1) {
            if (rp_AcqGetTriggerState(&stateTrig)) exit(1);
            if(stateTrig == RP_TRIG_STATE_TRIGGERED) break;
        }

        bool fillState = false;
        while(!fillState){
            if(rp_AcqGetBufferFillState(&fillState)) exit(1);
        }
        rp_AcqStop();

        
        // Retrieve data and process it
        uint32_t trig_pos;
        rp_AcqGetWritePointerAtTrig(&trig_pos);
#if defined Z20_125_4CH
        rp_AcqGetDataV2D(trig_pos,&buffer_size, ch_in[0], ch_in[1],ch_in[2], ch_in[3]);
#else
        rp_AcqGetDataV2D(trig_pos,&buffer_size, ch_in[0], ch_in[1]);
#endif
        g_dsp.windowFilter(ch_in,&ch_in);        

        g_dsp.fft(ch_in,&ch_fft);
        
        g_dsp.decimate(ch_fft,&tmp_signals,g_dsp.getOutSignalMaxLength(),g_dsp.getOutSignalMaxLength());

        g_dsp.cnvToDBMMaxValueRanged(tmp_signals,&tmp_signals,&peak_pw,&peak_pw_freq,decimation,args.freq_min,args.freq_max);

        // Summary peak calculation
        if (peak_set) {
            if (args.csv_limit) {
                for(uint32_t ch = 0; ch < MAX_CHANNELS; ch++){
                    for (uint32_t i = 0; i < g_dsp.getOutSignalMaxLength(); ++i) {
                        min_signals[ch][i] = std::min(min_signals[ch][i],tmp_signals[ch][i]);
                        max_signals[ch][i] = std::max(max_signals[ch][i],tmp_signals[ch][i]);
                    }
                }
            }
        } else {
            if (args.csv_limit) {
                for(uint32_t ch = 0; ch < MAX_CHANNELS; ch++){
                    for (uint32_t i = 0; i < g_dsp.getOutSignalMaxLength(); ++i) {
                        min_signals[ch][i] = tmp_signals[ch][i];
                        max_signals[ch][i] = tmp_signals[ch][i];
                    }
                }
            } 
            peak_set = true;
        }

        for(uint32_t ch = 0; ch < MAX_CHANNELS; ch++){
            if (peak_pw[ch] >= peak_pw_max[ch]){
                peak_pw_max[ch] = peak_pw[ch];
                peak_pw_freq_max[ch] = peak_pw_freq[ch];
            }
        }

        if (!args.csv && !args.csv_limit) {
            for(uint32_t i = 0; i < MAX_CHANNELS; i++){
                std::cout << "ch" << i << " peak: " << peak_pw_freq[i] << " Hz, " << peak_pw[i] << " dB\n";
            }
        }

        if (!is_infinity) {
            --count;
        }

        usleep(10000);
    }

	const float current_freq_range = ADC_SAMPLE_RATE  / (decimation * 2);
    const float freq_step = current_freq_range / (g_dsp.getOutSignalMaxLength() - 1);
    const size_t freq_index_min = std::floor(args.freq_min / freq_step);
    const size_t freq_index_max = std::ceil(args.freq_max / freq_step);

    if (peak_set) {

        if (args.csv) {
            for (size_t i = freq_index_min; i < (freq_index_max + 1); ++i) {
                std::cout << (freq_step * (freq_index_min + i));
                for(uint32_t ch = 0; ch < MAX_CHANNELS; ch++){
                    std::cout << ", " << tmp_signals[ch][i];
                }
                std::cout << "\n";
            }
        } else if (args.csv_limit) {
            for (size_t i = 0; i < (freq_index_max - freq_index_min + 1); ++i) {
                std::cout << (freq_step * (freq_index_min + i));
                for(uint32_t ch = 0; ch < MAX_CHANNELS; ch++){
                    std::cout << ", " << min_signals[ch][i] << ", " << min_signals[ch][i];
                }
                std::cout << "\n";
            }
        } else {
            for(uint32_t ch = 0; ch < MAX_CHANNELS; ch++){
                std::cout << "ch" << ch << " peak summary: " << peak_pw_freq_max[ch] << " Hz, " << peak_pw_max[ch] << " dB\n";
            }
        }
    }

    std::cout << std::flush;

    deleteArray<float>(tmp_signals);
    deleteArray<float>(max_signals);
    deleteArray<float>(min_signals);
    deleteArray<double>(ch_in);
    deleteArray<double>(ch_fft);

    delete[] peak_pw;
    delete[] peak_pw_max;
    delete[] peak_pw_freq;
    delete[] peak_pw_freq_max;
}

}

int main(int argc, char *argv[]) {
    std::cout.imbue(std::locale::classic());

    // Use fixed float values
    std::cout << std::fixed << std::setprecision(2);    
    cli_args_t args;    
    if (!cli_parse_args(argc, argv, args)) {
        fprintf(stderr,"%s Version: %s-%s\n",argv[0],VERSION_STR, REVISION_STR);        
        std::cerr << cli_help_string() << std::endl;
        return 1;
    }

    if (args.help) {        
        // std::cout has used only for the measurement results
        fprintf(stderr,"%s Version: %s-%s\n",argv[0],VERSION_STR, REVISION_STR);
        std::cerr << cli_help_string() << std::endl;
        return 0;
    }

    // Init
    int error_code = RP_OK;

    error_code = rp_Init();

    if (error_code != RP_OK) {
        std::cerr << "Error: rp_Init, code: " << error_code << std::endl;
        return 1;
    }

    error_code = rp_Reset();

    if (error_code != RP_OK) {
        std::cerr << "Error: rp_Reset, code: " << error_code << std::endl;
        return 1;
    }
    

    error_code = g_dsp.window_init(rp_dsp_api::CDSP::HANNING);

    if (error_code != 0) {
        std::cerr << "Error: rp_spectr_init, code: " << error_code << std::endl;
        rp_Release();
        return 1;
    }

    error_code = g_dsp.fftInit();

    if (error_code != 0) {
        std::cerr << "Error: rp_spectr_fft_init, code: " << error_code << std::endl;
        rp_Release();
        return 1;
    }

    // Handle interrupt signals
    signal(SIGINT, quit_handler);
    signal(SIGTERM, quit_handler);

    spectrum_worker(args);

    rp_Release();

    return 0;
}
