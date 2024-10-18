#include "converter_lib/converter.h"
#include "logger_lib/file_logger.h"
#include "writer_lib/file_helper.h"
#include <algorithm>
#include <iostream>
#include <signal.h>
#include <string>
#include <sys/types.h>

#define MAX(X, Y) ((X > Y) ? X : Y)

using namespace std;

bool g_stopWrite = false;
converter_lib::CConverter::Ptr g_converter = nullptr;

void createTestFiels();

char *getCmdOption(char **begin, char **end, const std::string &option)
{
	char **itr = std::find(begin, end, option);
	if (itr != end && ++itr != end) {
		return *itr;
	}
	return 0;
}

bool cmdOptionExists(char **begin, char **end, const std::string &option)
{
	return std::find(begin, end, option) != end;
}

bool CheckMissing(const char *val, const char *Message)
{
	if (val == NULL) {
		std::cout << "Missing parameters: " << Message << std::endl;
		return true;
	}
	return false;
}

void UsingArgs(char const *progName)
{
	std::cout << "Usage: " << progName << " file_name [-i][-s start][-e end][-f format][-t]\n";
	std::cout << "\t-i get info about file\n";
	std::cout << "\t-s Segment from which the conversion starts\n";
	std::cout << "\t-e Segment where the conversion will end\n";
	std::cout << "\t-f File format. [CSV|WAV|TDMS]. By default used CSV format\n";
	std::cout << "\t-t Creates test data files for DAC streaming\n";
}

void sigHandlerStopCSV(int)
{
	g_converter->stopWriteToCSV();
	g_stopWrite = true;
}

int ParseInt(string value) noexcept
{
	try {
		int x = std::stoi(value);
		if (x <= 0) {
			std::cout << "Error read parameter\n";
			return -1;
		}
		return x;
	} catch (std::exception &e) {
		std::cout << "Error read parameter\n";
		return -1;
	}
}

int main(int argc, char *argv[])
{
	signal(SIGINT, sigHandlerStopCSV);
	if (argc < 2) {
		UsingArgs(argv[0]);
		return -1;
	}
	std::string format = "CSV";
	std::string file_name = argv[1];
	bool check_info = cmdOptionExists(argv, argv + argc, "-i");
	bool gen_test = cmdOptionExists(argv, argv + argc, "-t");

	if (gen_test) {
		createTestFiels();
		return 0;
	}

	if (check_info) {
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
			}
		}
		return 0;
	}
	int32_t s = -2;
	int32_t e = -2;
	if (cmdOptionExists(argv, argv + argc, "-s")) {
		char *start_char = getCmdOption(argv, argv + argc, "-s");
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
		char *end_char = getCmdOption(argv, argv + argc, "-e");
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
		g_converter = converter_lib::CConverter::create();
		g_converter->convertToCSV(file_name, s, e, "");
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
