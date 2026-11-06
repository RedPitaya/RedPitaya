#include <signal.h>
#include <sys/types.h>
#include <algorithm>
#include <ctime>
#include <format>
#include <iomanip>
#include <iostream>
#include <string>
#include "bin_lib/bin_file.h"
#include "converter_lib/converter.h"
#include "logger_lib/file_logger.h"
#include "writer_lib/file_helper.h"

#define MAX(X, Y) ((X > Y) ? X : Y)

bool g_stopWrite = false;
converter_lib::CConverter::Ptr g_converter = nullptr;

void createTestFiels();

char* getCmdOption(char** begin, char** end, const std::string& option) {
    char** itr = std::find(begin, end, option);
    if (itr != end && ++itr != end) {
        return *itr;
    }
    return 0;
}

bool cmdOptionExists(char** begin, char** end, const std::string& option) {
    return std::find(begin, end, option) != end;
}

bool CheckMissing(const char* val, const char* Message) {
    if (val == NULL) {
        std::cout << "Missing parameters: " << Message << std::endl;
        return true;
    }
    return false;
}

void UsingArgs(char const* progName) {
    std::cout << "Usage: " << progName << " file_name [-i][-s start][-e end][-f format][-t][-ci][-cb|-cs|-cn]\n";
    std::cout << "\t-i get info about BIN file\n";
    std::cout << "\t-ia get info about BIN file. Including information on all segments.\n";
    std::cout << "\t-s Segment from which the conversion starts\n";
    std::cout << "\t-e Segment where the conversion will end\n";
    std::cout << "\t-f File format. [CSV|WAV|TDMS]. By default used CSV format\n";
    std::cout << "\t-t Creates test data files for DAC streaming\n";
    std::cout << "\t-ci Adds an index column to the CSV file.\n";
    std::cout << "\t-cb Adds a block time column to the CSV file.\n";
    std::cout << "\t-cs Adds a sample time column to the CSV file.\n";
    std::cout << "\t-cn Adds a sample time column in nanoseconds to the CSV file.\n";
}

void sigHandlerStopCSV(int) {
    g_converter->stopWriteToCSV();
    g_stopWrite = true;
}

int ParseInt(std::string value) noexcept {
    try {
        int x = std::stoi(value);
        if (x <= 0) {
            std::cout << "Error read parameter\n";
            return -1;
        }
        return x;
    } catch (std::exception& e) {
        std::cout << "Error read parameter\n";
        return -1;
    }
}

int main(int argc, char* argv[]) {
    signal(SIGINT, sigHandlerStopCSV);
    if (argc < 2) {
        UsingArgs(argv[0]);
        return -1;
    }
    std::string format = "CSV";
    std::string file_name = argv[1];
    bool check_info = cmdOptionExists(argv, argv + argc, "-i");
    bool check_info_all = cmdOptionExists(argv, argv + argc, "-ia");
    bool gen_test = cmdOptionExists(argv, argv + argc, "-t");

    if (gen_test) {
        createTestFiels();
        return 0;
    }

    if (check_info || check_info_all) {
        // The BIN stream file (bin_lib): the channels, their samples and losses, block by block with -ia
        bin_lib::FileReader reader;
        if (!reader.open(file_name)) {
            std::cout << " Error open file: " << file_name << ": " << reader.error() << "\n";
            return 0;
        }
        const auto& info = reader.info();
        uint64_t samples[4] = {0, 0, 0, 0};
        uint64_t lost[4] = {0, 0, 0, 0};
        uint64_t wait[4] = {0, 0, 0, 0};
        uint64_t blocks = 0;
        bin_lib::BlockInfo block;
        std::vector<uint8_t> data;
        while (reader.nextBlock(block, data)) {
            const auto& ch = info.channels[block.channel];
            if (ch.id >= 1 && ch.id <= 4) {
                samples[ch.id - 1] += block.samples;
                lost[ch.id - 1] += block.lost;
                wait[ch.id - 1] += block.wait;
            }
            if (check_info_all)
                aprintf(stdout, "Block %llu. Channel %u, samples %llu, lost %llu, wait %llu, position %llu, bytes %u, trigger count %u\n", (unsigned long long)blocks + 1,
                        ch.id, (unsigned long long)block.samples, (unsigned long long)block.lost, (unsigned long long)block.wait, (unsigned long long)block.position,
                        block.dataSize, block.triggerCount);
            blocks++;
        }
        aprintf(stdout, "Blocks count: %llu\n", (unsigned long long)blocks);
        aprintf(stdout, "Clock: %llu Hz, decimation %u\n", (unsigned long long)info.clock, info.decimation);
        for (int i = 0; i < 4; i++) {
            aprintf(stdout, "\nChannel %d:\n", i + 1);
            std::string dft = "Unknown";
            for (const auto& ch : info.channels) {
                if (ch.id != i + 1)
                    continue;
                if (ch.type == bin_lib::SampleType::INT8)
                    dft = "Int8";
                if (ch.type == bin_lib::SampleType::INT16)
                    dft = "Int16";
                if (ch.type == bin_lib::SampleType::FLOAT32)
                    dft = "Float";
            }
            aprintf(stdout, "\tData format type:\t%s\n", dft.c_str());
            aprintf(stdout, "\tSamples count:\t%llu\n", (unsigned long long)samples[i]);
            aprintf(stdout, "\tLost samples count: %llu\n", (unsigned long long)lost[i]);
            aprintf(stdout, "\tTrigger wait samples count: %llu\n", (unsigned long long)wait[i]);
        }
        if (info.startTime != 0) {
            auto ns = std::chrono::nanoseconds{info.startTime};
            std::chrono::system_clock::time_point tp{std::chrono::duration_cast<std::chrono::system_clock::duration>(ns)};
            std::time_t t = std::chrono::system_clock::to_time_t(tp);
            char buffer[100];
            std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::gmtime(&t));
            aprintf(stdout, "\nCapture start: %s\n", buffer);
        }
        return 0;
    }
    int32_t s = -2;
    int32_t e = -2;
    if (cmdOptionExists(argv, argv + argc, "-s")) {
        char* start_char = getCmdOption(argv, argv + argc, "-s");
        if (CheckMissing(start_char, "start segment")) {
            UsingArgs(argv[0]);
            return -1;
        }
        s = ParseInt(start_char);
        if (s == -1) {
            UsingArgs(argv[0]);
            return -1;
        }
    }

    if (cmdOptionExists(argv, argv + argc, "-e")) {
        char* end_char = getCmdOption(argv, argv + argc, "-e");
        if (CheckMissing(end_char, "end segment")) {
            UsingArgs(argv[0]);
            return -1;
        }
        e = ParseInt(end_char);
        if (e == -1) {
            UsingArgs(argv[0]);
            return -1;
        }
    }

    if (cmdOptionExists(argv, argv + argc, "-f")) {
        format = getCmdOption(argv, argv + argc, "-f");
    }

    if (s > 0 && e > 0 && s > e) {
        std::cout << "The start segment must be less than or equal to the end.\n";
        return -1;
    }

    if (format == "CSV") {
        FH_CSVMode csv_mode = FH_CSV_NONE;
        if (cmdOptionExists(argv, argv + argc, "-ci")) {
            csv_mode = (FH_CSVMode)(csv_mode | FH_CSV_ADD_INDEX);
        }
        if (cmdOptionExists(argv, argv + argc, "-cb")) {
            csv_mode = (FH_CSVMode)(csv_mode | FH_CSV_ADD_TIME_COL_FOR_BLOCK);
        }
        if (cmdOptionExists(argv, argv + argc, "-cs")) {
            csv_mode = (FH_CSVMode)(csv_mode | FH_CSV_ADD_TIME_COL);
        }
        if (cmdOptionExists(argv, argv + argc, "-cn")) {
            csv_mode = (FH_CSVMode)(csv_mode | FH_CSV_ADD_TIME_COL_NS);
        }
        g_converter = converter_lib::CConverter::create();
        g_converter->convertToCSV(file_name, s, e, "", csv_mode);
    }

    if (format == "WAV") {
        g_converter = converter_lib::CConverter::create();
        g_converter->convertToWAV(file_name, s, e, "");
    }

    if (format == "TDMS") {
        g_converter = converter_lib::CConverter::create();
        g_converter->convertToTDMS(file_name, s, e, "");
    }
    return 0;
}
