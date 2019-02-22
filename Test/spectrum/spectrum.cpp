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
    #include <../src/spec_fpga.h>
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

static void spectrum_worker(cli_args_t args) {
    // Calculate the frequency range
    constexpr std::array<float, 6> freq_range_values = {62500000., 7800000., 976000., 61000., 7600., 953.};
    size_t freq_range_index = 0;

    for (size_t i = 1; i < freq_range_values.size(); ++i) {
        if (args.freq_max <= freq_range_values[i]) {
            freq_range_index = i;
        } else {
            break;
        }
    }

    // const std::array<std::string, 3> unit_str = {"Hz", "kHz", "MHz"};
    std::array<signal_array_t, spectrum_signal_count> tmp_signals = {};

    for (signal_array_t &signal_array : tmp_signals) {
        signal_array = signal_array_t(spectrum_signal_size, 0);
    }

    std::vector<double> cha_in(SPECTR_FPGA_SIG_LEN, 0);
    std::vector<double> chb_in(SPECTR_FPGA_SIG_LEN, 0);
    std::vector<double> cha_fft(c_dsp_sig_len, 0);
    std::vector<double> chb_fft(c_dsp_sig_len, 0);

    // FPGA parameters
    spectr_fpga_reset();
    spectr_fpga_update_params(0, 0, 0, 0, 0, freq_range_index, args.average_for_10 ? 1 : 0);
    // int unit = spectr_fpga_cnv_freq_range_to_unit(freq_range_index);

    // API compatible
    float *tmp_signal_0 = tmp_signals[0].data();
    float *tmp_signal_1 = tmp_signals[1].data();
    float *tmp_signal_2 = tmp_signals[2].data();
    double *p_cha_fft = cha_fft.data();
    double *p_chb_fft = chb_fft.data();
    double *p_cha_in = cha_in.data();
    double *p_chb_in = chb_in.data();

    const float freq_step = freq_range_values[freq_range_index] / (spectrum_signal_size - 1);
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

    while (!g_quit_requested && ((count > 0) || is_infinity)) {
        // Start the writting machine
        spectr_fpga_arm_trigger();
        usleep(10);
        spectr_fpga_set_trigger(1);

        // Polling until data is ready
        while (!spectr_fpga_triggered()) {
            usleep(1);
        }

        // Retrieve data and process it
        spectr_fpga_get_signal(&p_cha_in, &p_chb_in);
        rp_spectr_prepare_freq_vector(&tmp_signal_0, spectr_get_fpga_smpl_freq(), freq_range_index);
        rp_spectr_hann_filter(p_cha_in, p_chb_in, &p_cha_in, &p_chb_in);
        rp_spectr_fft(p_cha_in, p_chb_in, &p_cha_fft, &p_chb_fft);
        rp_spectr_decimate(p_cha_fft, p_chb_fft, &tmp_signal_1, &tmp_signal_2, c_dsp_sig_len, spectrum_signal_size);

        // Unused
        rp_spectr_worker_res_t tmp_result;
        rp_spectr_cnv_to_dBm(tmp_signal_1,
                                tmp_signal_2,
                                &tmp_signal_1,
                                &tmp_signal_2,
                                &tmp_result.peak_pw_cha,
                                &tmp_result.peak_pw_freq_cha, &tmp_result.peak_pw_chb,
                                &tmp_result.peak_pw_freq_chb,
                                freq_range_index);

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

    if (!args.test) {
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
    }

    error_code = spectr_fpga_init();

    if (error_code != 0) {
        std::cerr << "Error: spectr_fpga_init, code: " << error_code << std::endl;

        if (!args.test) {
            rp_Release();
        }

        return 1;
    }

    error_code = rp_spectr_hann_init();

    if (error_code != 0) {
        std::cerr << "Error: rp_spectr_hann_init, code: " << error_code << std::endl;
        spectr_fpga_exit();

        if (!args.test) {
            rp_Release();
        }

        return 1;
    }

    error_code = rp_spectr_fft_init();

    if (error_code != 0) {
        std::cerr << "Error: rp_spectr_fft_init, code: " << error_code << std::endl;
        rp_spectr_hann_clean();
        spectr_fpga_exit();

        if (!args.test) {
            rp_Release();
        }

        return 1;
    }

    // Handle interrupt signals
    signal(SIGINT, quit_handler);
    signal(SIGTERM, quit_handler);

    std::array<signal_array_t, spectrum_signal_count> spectrum_signals = {};

    for (signal_array_t &signal_array : spectrum_signals) {
        signal_array = signal_array_t(spectrum_signal_size, 0);
    }

    // Signals directly pointing at the FPGA mem space
    int *rp_fpga_cha_signal = nullptr;
    int *rp_fpga_chb_signal = nullptr;
    spectr_fpga_get_sig_ptr(&rp_fpga_cha_signal, &rp_fpga_chb_signal);

    spectrum_worker(args);

    rp_spectr_fft_clean();
    rp_spectr_hann_clean();
    spectr_fpga_exit();

    if (!args.test) {
        rp_Release();
    }

    return 0;
}
