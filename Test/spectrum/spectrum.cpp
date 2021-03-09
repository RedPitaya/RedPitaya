#include <iomanip>
#include <iostream>
#include <locale>
#include <algorithm>
#include <array>
#include <vector>
#include <cmath>
#include <csignal>
#include <unistd.h>
#include <redpitaya/version.h>
#include <redpitaya/rp.h>
#include "cli_parse_args.h"

extern "C" {
    #include <../src/spec_dsp.h>
}

namespace {
using signal_array_t = std::vector<float>;

struct rp_spectr_worker_res_t {
    float peak_pw_cha;
    float peak_pw_freq_cha;
    float peak_pw_chb;
    float peak_pw_freq_chb;
};

static constexpr size_t spectrum_signal_count = 3;
static constexpr size_t spectrum_signal_size = 8 * 1024;
static sig_atomic_t g_quit_requested = 0;

static void quit_handler(int) {
    g_quit_requested = 1;
}

int spectr_fpga_cnv_freq_range_to_unit(int freq_range)
{
    /* Input freq. range: 0, 1, 2, 3, 4, 5 translates to:
     * Output: 0 - [MHz], 1 - [kHz], 2 - [Hz] */
    switch(freq_range) {
    case 0:
    case 1:
        return 2;
        break;
    case 2:
    case 3:
    case 4:
        return 1;
        break;
    case 5:
        return 0;
        break;
    default:
        return -1;
        break;
    };

    return -1;
}

static void spectrum_worker(cli_args_t args) {

    int decimation =     ADC_SAMPLE_RATE /  (args.freq_max * 2);

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


	auto current_freq_range = ADC_SAMPLE_RATE  / (decimation * 2);
	

    // const std::array<std::string, 3> unit_str = {"Hz", "kHz", "MHz"};
    std::array<signal_array_t, spectrum_signal_count> tmp_signals = {};

    for (signal_array_t &signal_array : tmp_signals) {
        signal_array = signal_array_t(spectrum_signal_size, 0);
    }

    std::vector<double> cha_in(rp_get_spectr_signal_max_length(), 0);
    std::vector<double> chb_in(rp_get_spectr_signal_max_length(), 0);
    std::vector<double> cha_fft(rp_get_spectr_out_signal_max_length(), 0);
    std::vector<double> chb_fft(rp_get_spectr_out_signal_max_length(), 0);

    // API compatible
    // float *tmp_signal_0 = tmp_signals[0].data();
    float *tmp_signal_1 = tmp_signals[1].data();
    float *tmp_signal_2 = tmp_signals[2].data();
    double *p_cha_fft = cha_fft.data();
    double *p_chb_fft = chb_fft.data();
    double *p_cha_in = cha_in.data();
    double *p_chb_in = chb_in.data();

    const float freq_step = current_freq_range / (spectrum_signal_size - 1);
    const size_t freq_index_min = std::floor(args.freq_min / freq_step);
    const size_t freq_index_max = std::ceil(args.freq_max / freq_step);

    // Peak values by frequency
    std::vector<signal_array_t> max_values(2);
    std::vector<signal_array_t> min_values(2);

    bool peak_set = false;
    float peak_ch0_freq = std::numeric_limits<float>::lowest();
    float peak_ch0_value = std::numeric_limits<float>::lowest();
    float peak_ch1_freq = std::numeric_limits<float>::lowest();
    float peak_ch1_value = std::numeric_limits<float>::lowest();

    int count = args.count;

    if (args.csv) {
        count = 1;
        std::cout << "Frequency (Hz), ch0 (dB), ch1 (dB)\n";
    } else if (args.csv_limit) {
        std::cout << "Frequency (Hz), ch0 min (dB), ch0 max (dB), ch1 min (dB), ch1 max (dB)\n";
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
        rp_AcqGetDataV2D(trig_pos,&buffer_size, p_cha_in, p_chb_in);
    //    rp_spectr_prepare_freq_vector(&tmp_signal_0, ADC_SAMPLE_RATE, decimation);
        rp_spectr_window_filter(p_cha_in, p_chb_in, &p_cha_in, &p_chb_in);
        rp_spectr_fft(p_cha_in, p_chb_in, &p_cha_fft, &p_chb_fft);
        rp_spectr_decimate(p_cha_fft, p_chb_fft, &tmp_signal_1, &tmp_signal_2, rp_get_spectr_out_signal_length(), spectrum_signal_size);

        // Unused
        rp_spectr_worker_res_t tmp_result;
        rp_spectr_cnv_to_dBm(tmp_signal_1,
                                tmp_signal_2,
                                &tmp_signal_1,
                                &tmp_signal_2,
                                &tmp_result.peak_pw_cha,
                                &tmp_result.peak_pw_freq_cha, &tmp_result.peak_pw_chb,
                                &tmp_result.peak_pw_freq_chb,
                                decimation);

        // Peak calculation
        const auto it_ch0_max = std::max_element(std::begin(tmp_signals[1]) + freq_index_min, std::begin(tmp_signals[1]) + freq_index_max + 1);
        const auto it_ch1_max = std::max_element(std::begin(tmp_signals[2]) + freq_index_min, std::begin(tmp_signals[2]) + freq_index_max + 1);
        const float ch0_max = *it_ch0_max;
        const float ch0_max_freq = freq_step * std::distance(std::begin(tmp_signals[1]), it_ch0_max);
        const float ch1_max = *it_ch1_max;
        const float ch1_max_freq = freq_step * std::distance(std::begin(tmp_signals[2]), it_ch1_max);

        // Summary peak calculation
        if (peak_set) {
            if (args.csv_limit) {
                for (size_t i = 0; i < (freq_index_max - freq_index_min + 1); ++i) {
                    min_values[0][i] = std::min(min_values[0][i], tmp_signals[1][freq_index_min + i]);
                    max_values[0][i] = std::max(max_values[0][i], tmp_signals[1][freq_index_min + i]);
                    min_values[1][i] = std::min(min_values[1][i], tmp_signals[2][freq_index_min + i]);
                    max_values[1][i] = std::max(max_values[1][i], tmp_signals[2][freq_index_min + i]);
                }
            } else {
                if (ch0_max >= peak_ch0_value) {
                    peak_ch0_value = ch0_max;
                    peak_ch0_freq = ch0_max_freq;
                }

                if (ch1_max >= peak_ch1_value) {
                    peak_ch1_value = ch1_max;
                    peak_ch1_freq = ch1_max_freq;
                }
            }
        } else {
            if (args.csv_limit) {
                min_values[0].assign(std::begin(tmp_signals[1]) + freq_index_min, std::begin(tmp_signals[1]) + freq_index_max + 1);
                max_values[0].assign(std::begin(tmp_signals[1]) + freq_index_min, std::begin(tmp_signals[1]) + freq_index_max + 1);
                min_values[1].assign(std::begin(tmp_signals[2]) + freq_index_min, std::begin(tmp_signals[2]) + freq_index_max + 1);
                max_values[1].assign(std::begin(tmp_signals[2]) + freq_index_min, std::begin(tmp_signals[2]) + freq_index_max + 1);
            } else {
                peak_ch0_value = ch0_max;
                peak_ch0_freq = ch0_max_freq;
                peak_ch1_value = ch1_max;
                peak_ch1_freq = ch1_max_freq;
            }

            peak_set = true;
        }

        if (!args.csv && !args.csv_limit) {
            std::cout << "ch0 peak: " << ch0_max_freq << " Hz, " << ch0_max << " dB\n";
            std::cout << "ch1 peak: " << ch1_max_freq << " Hz, " << ch1_max << " dB\n";
        }

        if (!is_infinity) {
            --count;
        }

        usleep(10000);
    }

    if (peak_set) {
        if (args.csv) {
            for (size_t i = freq_index_min; i < (freq_index_max + 1); ++i) {
                std::cout << (freq_step * i) << ", " << tmp_signals[1][i] << ", " << tmp_signals[2][i] << "\n";
            }
        } else if (args.csv_limit) {
            for (size_t i = 0; i < (freq_index_max - freq_index_min + 1); ++i) {
                std::cout << (freq_step * (freq_index_min + i)) << ", "
                    << min_values[0][i] << ", "
                    << max_values[0][i] << ", "
                    << min_values[1][i] << ", "
                    << max_values[1][i] << "\n";
            }
        } else {
            std::cout << "ch0 peak summary: " << peak_ch0_freq << " Hz, " << peak_ch0_value << " dB\n";
            std::cout << "ch1 peak summary: " << peak_ch1_freq << " Hz, " << peak_ch1_value << " dB\n";
        }
    }

    std::cout << std::flush;
}
}

int main(int argc, char *argv[]) {
    std::cout.imbue(std::locale::classic());

    // Use fixed float values
    std::cout << std::fixed << std::setprecision(2);

    cli_args_t args;

    if (!cli_parse_args(argc, argv, args)) {
        std::cerr << cli_help_string() << std::endl;
        return 1;
    }

    if (args.help) {
        // std::cout has used only for the measurement results
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
    

    error_code = rp_spectr_window_init(HANNING);

    if (error_code != 0) {
        std::cerr << "Error: rp_spectr_init, code: " << error_code << std::endl;
        rp_Release();
        return 1;
    }

    error_code = rp_spectr_fft_init();

    if (error_code != 0) {
        std::cerr << "Error: rp_spectr_fft_init, code: " << error_code << std::endl;
        rp_spectr_window_clean();
        rp_Release();
        return 1;
    }

    // Handle interrupt signals
    signal(SIGINT, quit_handler);
    signal(SIGTERM, quit_handler);

    std::array<signal_array_t, spectrum_signal_count> spectrum_signals = {};

    for (signal_array_t &signal_array : spectrum_signals) {
        signal_array = signal_array_t(spectrum_signal_size, 0);
    }

    spectrum_worker(args);

    rp_spectr_fft_clean();
    rp_spectr_window_clean();
    rp_Release();

    return 0;
}
