#include <stdexcept>
#include <iostream>
#include <getopt.h>

#include "cli_parse_args.h"

std::string cli_help_string() {
    return "usage:\n"
    "-h, --help: help\n"
    "-m, --min: minimum frequency (default: 0)\n"
    "-M, --max: maximum frequency (default: 62500000)\n"
    "-c, --count: iteration count (default: 1, negative: infinity)\n"
    "-a, --average: average the measurement from 10 times (default: enabled)\n"
    "-n, --no-average: disable average the measurement from 10 times\n"
    "-C, --csv: print values by columns Frequency (Hz), ch0 (dB), ch1 (dB)\n"
    "-L, --csv-limit: print values by columns Frequency (Hz), ch0 min (dB), ch0 max (dB), ch1 min (dB), ch1 max (dB)\n";
}

const struct option long_opt[] = {
        { "help",       no_argument,        0, 'h' },
        { "min",        required_argument,  0, 'm' },
        { "max",        required_argument,  0, 'M' },
        { "count",      required_argument,  0, 'c' },
        { "average",    no_argument,        0, 'a' },
        { "no-average", no_argument,        0, 'n' },
        { "csv",        no_argument,        0, 'C' },
        { "csv-limit",  no_argument,        0, 'L' },
        { 0,            0,                  0,  0  }
    };

bool cli_parse_args(int argc, char * const argv[], cli_args_t &out_args) {
    cli_args_t args;
    
    bool success = true;
    try {
        int short_opt;
        int option_index = 0;
        while ((short_opt = getopt_long(argc, argv, "hm:M:c:anCLt", long_opt, &option_index)) != -1) {            
            switch (short_opt) {
            case 'h':
                args.help = true;
                break;

            case 'm':
                args.freq_min = std::stof(optarg);

                if (args.freq_min < 0. || args.freq_min > 62500000.) {
                    throw std::out_of_range("freq_min");
                }

                break;

            case 'c':
                args.count = std::stoi(optarg);
                break;

            case 'M':
                args.freq_max = std::stof(optarg);

                if (args.freq_max < 0. || args.freq_max > 62500000.) {
                    throw std::out_of_range("freq_max");
                }

                break;

            case 'a':
                args.average_for_10 = true;
                break;

            case 'n':
                args.average_for_10 = false;
                break;

            case 'C':
                args.csv = true;
                break;

            case 'L':
                args.csv_limit = true;
                break;

            // case '?':
            default:
                throw std::logic_error("");
                break;
            }
        }

        if (args.freq_min >= args.freq_max) {
            throw std::logic_error("freq_min >= freq_max");
        }

        if (args.csv && args.csv_limit) {
            throw std::logic_error("--csv and --csv-limit: only one can be used");
        }

        if (args.csv && (args.count != 1)) {
            throw std::logic_error("--count can not be used with --csv");
        }

        out_args = args;
    } catch (const std::logic_error &e) {
        std::cerr << e.what() << std::endl;
        success = false;
    }

    return success;
}
