#include <signal.h>
#include <sys/types.h>
#include <algorithm>
#include <ctime>
#include <format>
#include <iomanip>
#include <iostream>
#include <string>
#include "converter_lib/converter.h"
#include "logger_lib/file_logger.h"
#include "writer_lib/file_helper.h"

#define MAX(X, Y) ((X > Y) ? X : Y)

using namespace std;

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

int ParseInt(string value) noexcept {
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
        std::fstream fs;
        fs.open(file_name, std::ios::binary | std::ofstream::in | std::ofstream::out);
        if (fs.fail()) {
            std::cout << " Error open file: " << file_name << "\n";
        } else {
            auto bi = readBinInfo(&fs);

            aprintf(stdout, "Segments count: %llu\n", bi.segCount);
            aprintf(stdout, "Samples per segment: %llu\n", bi.segSamplesCount);
            aprintf(stdout, "Samples in last segment: %llu\n", bi.segLastSamplesCount);
            aprintf(stdout, "Status of last segment: %s\n", bi.lastSegState ? "OK" : "BROKEN");

            for (int i = 0; i < 4; i++) {
                aprintf(stdout, "\nChannel %d:\n", i + 1);
                string dft = "Unknown";
                if (bi.dataFormatSize[i] == 1)
                    dft = "Int8";
                if (bi.dataFormatSize[i] == 2)
                    dft = "Int16";
                if (bi.dataFormatSize[i] == 4)
                    dft = "Float";
                aprintf(stdout, "\tData format type:\t%s\n", dft.c_str());
                aprintf(stdout, "\tSamples count:\t%llu\n", bi.samples_ch[i]);
                aprintf(stdout, "\tLost samples count: %llu\n", bi.lostCount[i]);
                if (bi.timeCapture[i] != 0) {
                    auto ns = std::chrono::nanoseconds{bi.timeCapture[i]};
                    std::chrono::system_clock::time_point tp{std::chrono::duration_cast<std::chrono::system_clock::duration>(ns)};
                    std::time_t time_t_value = std::chrono::system_clock::to_time_t(tp);
                    char buffer[100];
                    std::tm* tm_info = std::gmtime(&time_t_value);
                    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm_info);
                    auto nsEnd = std::chrono::nanoseconds{bi.timeCaptureLast[i]};
                    std::chrono::system_clock::time_point tpEnd{std::chrono::duration_cast<std::chrono::system_clock::duration>(nsEnd)};
                    std::time_t time_t_valueEnd = std::chrono::system_clock::to_time_t(tpEnd);
                    char bufferEnd[100];
                    std::tm* tm_infoEnd = std::gmtime(&time_t_valueEnd);
                    std::strftime(bufferEnd, sizeof(bufferEnd), "%Y-%m-%d %H:%M:%S", tm_infoEnd);
                    aprintf(stdout, "\tCapture time: %s - %s\n", buffer, bufferEnd);
                }
            }

            if (check_info_all) {
                size_t idx = 1;
                for (auto& seg : bi.segments) {
                    aprintf(stdout, "Segment - %zu. Length: %u\n", idx++, seg.sigmentLength);
                    aprintf(stdout, "\tSamples: CH1 - %u; CH2 - %u; CH3 - %u; CH4 - %u\n", seg.sampleCh[0], seg.sampleCh[1], seg.sampleCh[2], seg.sampleCh[3]);
                    aprintf(stdout, "\tSize: CH1 - %u; CH2 - %u; CH3 - %u; CH4 - %u\n", seg.sizeCh[0], seg.sizeCh[1], seg.sizeCh[2], seg.sizeCh[3]);
                    aprintf(stdout, "\tLost: CH1 - %llu; CH2 - %llu; CH3 - %llu; CH4 - %llu\n", seg.lostCount[0], seg.lostCount[1], seg.lostCount[2], seg.lostCount[3]);
                    aprintf(
                        stdout, "\tFormat: CH1 - %u; CH2 - %u; CH3 - %u; CH4 - %u\n", seg.dataFormatSize[0], seg.dataFormatSize[1], seg.dataFormatSize[2], seg.dataFormatSize[3]);
                    aprintf(stdout, "\tRate: CH1 - %llu; CH2 - %llu; CH3 - %llu; CH4 - %llu\n", seg.oscRate[0], seg.oscRate[1], seg.oscRate[2], seg.oscRate[3]);
                    aprintf(stdout, "\tTime: CH1 - %lld; CH2 - %lld; CH3 - %lld; CH4 - %lld\n", seg.timeCapture[0], seg.timeCapture[1], seg.timeCapture[2], seg.timeCapture[3]);
                }
            }
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
