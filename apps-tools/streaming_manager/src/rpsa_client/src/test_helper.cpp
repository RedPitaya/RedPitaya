#include "test_helper.h"
#include "logger_lib/file_logger.h"
#include <atomic>
#include <chrono>
#include <math.h>

ClientOpt::Options g_hoption;
std::atomic<bool> g_helper_exit_flag;
std::mutex g_helper_mutex;

std::mutex g_statMutex;
long long int g_timeBeginTotal;
long long int g_timeBegin;
std::map<std::string, long long int> g_timeHostBegin;
std::map<std::string, uint64_t> g_BytesCount;
std::map<std::string, uint64_t> g_BytesCountTotal;
std::map<std::string, uint64_t> g_lostRate;
std::map<std::string, uint64_t> g_lostRateTotal;
std::map<std::string, uint64_t> g_lostFileRate;
std::map<std::string, uint64_t> g_lostFileRateTotal;

std::map<std::string, uint64_t> g_packCounter_ch1;
std::map<std::string, uint64_t> g_packCounter_ch2;
std::map<std::string, uint64_t> g_packCounter_ch3;
std::map<std::string, uint64_t> g_packCounter_ch4;
std::map<std::string, uint64_t> g_packCounterTotal_ch1;
std::map<std::string, uint64_t> g_packCounterTotal_ch2;
std::map<std::string, uint64_t> g_packCounterTotal_ch3;
std::map<std::string, uint64_t> g_packCounterTotal_ch4;

std::map<std::string, int64_t> g_brokenBuffer;

auto setOptions(ClientOpt::Options option) -> void
{
	g_hoption = option;
	g_helper_exit_flag = false;
}

auto resetStreamingCounter() -> void
{
	const std::lock_guard lock(g_statMutex);
	g_timeBegin = 0;
	g_timeBeginTotal = 0;
	g_timeHostBegin.clear();
	g_BytesCount.clear();
	g_lostRate.clear();
	g_lostRateTotal.clear();
	g_BytesCountTotal.clear();
	g_packCounter_ch1.clear();
	g_packCounter_ch2.clear();
	g_packCounter_ch3.clear();
	g_packCounter_ch4.clear();
	g_packCounterTotal_ch1.clear();
	g_packCounterTotal_ch2.clear();
	g_packCounterTotal_ch3.clear();
	g_packCounterTotal_ch4.clear();
	g_lostFileRate.clear();
	g_lostFileRateTotal.clear();
	g_brokenBuffer.clear();
}

auto addStatisticSteaming(std::string &host,
						  uint64_t bytesCount,
						  uint64_t samp_ch1,
						  uint64_t samp_ch2,
						  uint64_t samp_ch3,
						  uint64_t samp_ch4,
						  uint64_t lost,
						  uint64_t fileLost,
						  int64_t brokenBuff) -> void
{
	const std::lock_guard lock(g_statMutex);

	if (g_timeHostBegin.count(host) == 0) {
		auto curTime = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
		auto value = curTime.time_since_epoch();
		g_timeHostBegin[host] = value.count();
	}

	if (g_timeBeginTotal == 0) {
		auto curTime = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
		auto value = curTime.time_since_epoch();
		g_timeBeginTotal = value.count();
	}

	g_BytesCount[host] += bytesCount;
	g_BytesCountTotal[host] += bytesCount;

	g_packCounter_ch1[host] += samp_ch1;
	g_packCounter_ch2[host] += samp_ch2;
	g_packCounter_ch3[host] += samp_ch3;
	g_packCounter_ch4[host] += samp_ch4;

	g_packCounterTotal_ch1[host] += samp_ch1;
	g_packCounterTotal_ch2[host] += samp_ch2;
	g_packCounterTotal_ch3[host] += samp_ch3;
	g_packCounterTotal_ch4[host] += samp_ch4;

	g_lostFileRate[host] += fileLost;
	g_lostFileRateTotal[host] += fileLost;

	g_lostRate[host] += lost;
	g_lostRateTotal[host] += lost;

	if (brokenBuff != -1) {
		g_brokenBuffer[host] += brokenBuff;
	}
}

auto createStr(std::string str, size_t len) -> std::string
{
	std::string s(len, ' ');
	for (size_t i = 0; i < str.length() && i < len; i++) {
		s[i] = str[i];
	}
	return s;
}

auto convertBtoS(uint64_t value) -> std::string
{
	double d = value;
	std::string s = "";
	if (value >= 1024 * 1024) {
		d = round(((double) value * 1000.0) / (1024 * 1024)) / 1000;
		s = to_string_with_precision(d, 3) + " Mb";
	} else if (value >= 1024) {
		d = round(((double) value * 1000.0) / (1024)) / 1000;
		s = to_string_with_precision(d, 3) + " kb";
	} else {
		s = std::to_string(value) + " b";
	}
	return s;
}

auto convertBtoST(uint64_t value) -> std::string
{
	std::string s = "";
	auto h = value / (60 * 60 * 1000);
	auto m = (value - h * 60 * 60 * 1000) / (60 * 1000);
	auto sec = (value - h * 60 * 60 * 1000 - m * 60 * 1000) / (1000);
	auto ms = value % 1000;
	s = std::to_string(h) + ":" + std::to_string(m) + ":" + std::to_string(sec) + "." + std::to_string(ms);
	return s;
}

auto convertBtoSpeed(uint64_t value, uint64_t time) -> std::string
{
	double d = value;
	double t = time;
	t = t / 1000;
	d = d / t;
	std::string s = "";
	if (value >= 1024 * 1024) {
		d = round(((double) d * 1000.0) / (1024 * 1024)) / 1000;
		s = to_string_with_precision(d, 3) + " MB/s";
	} else if (value >= 1024) {
		d = round(((double) d * 1000.0) / (1024)) / 1000;
		s = to_string_with_precision(d, 3) + " kB/s";
	} else {
		s = std::to_string(d) + " B/s";
	}
	return s;
}

auto printStatisitc(bool force) -> void
{
	const std::lock_guard lock(g_statMutex);

	auto curTime = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
	auto value = curTime.time_since_epoch();
	if ((value.count() - g_timeBegin) >= 5000 || force) {
		std::vector<std::string> keys;
		std::transform(g_timeHostBegin.begin(),
					   g_timeHostBegin.end(),
					   std::back_inserter(keys),
					   [](const std::map<std::string, long long int>::value_type &pair) { return pair.first; });

		if (keys.size() > 0) {
			std::stringstream ss;
			ss << "\n" << getTS() << "\n";

			ss << "========================================================================================================================"
				  "=====================================\n";
			ss << "Host              | Bytes all         | Bandwidth         |    Samples CH1    |    Samples CH2    |    Samples CH3    | "
				  "   Samples CH4    |      Lost        |\n";

			bool first = true;
			for (auto const &host : keys) {
				if (first) {
					first = false;
					ss << "----------------------------------------------------------------------------------------------------------------"
						  "---------------------------------------------|\n";
				}
				auto bw = convertBtoSpeed(g_BytesCount[host], value.count() - g_timeHostBegin[host]);
				ss << createStr(host, 18) << "|";
				ss << " " << createStr(convertBtoS(g_BytesCountTotal[host]), 18) << "|";
				ss << " " << createStr(bw, 18) << "|";
				ss << " " << createStr(std::to_string(g_packCounter_ch1[host]), 18) << "|";
				ss << " " << createStr(std::to_string(g_packCounter_ch2[host]), 18) << "|";
				ss << " " << createStr(std::to_string(g_packCounter_ch3[host]), 18) << "|";
				ss << " " << createStr(std::to_string(g_packCounter_ch4[host]), 18) << "|";
				ss << "                  |\n";
				ss << "                  "
					  "+...................+...................+...................+...................+...................+..............."
					  "....+";
				ss << " " + createStr(std::to_string(g_lostRate[host]), 17) << "|\n";
				ss << "                  |";
				ss << "Lost in file: " << createStr(std::to_string(g_lostFileRate[host]), 105) << "|";
				ss << "                  |\n";
				ss << "                  "
					  "+...................+...................+...................+...................+...................+..............."
					  "....+";
				ss << "                  |\n";
				g_BytesCount[host] = 0;
				g_timeHostBegin[host] = value.count();
				g_packCounter_ch1[host] = 0;
				g_packCounter_ch2[host] = 0;
				g_packCounter_ch3[host] = 0;
				g_packCounter_ch4[host] = 0;
				g_lostFileRate[host] = 0;
				g_lostRate[host] = 0;
			}
			ss << "========================================================================================================================"
				  "=====================================\n";
			aprintf(stdout, "%s", ss.str().c_str());
		}

		g_timeBegin = value.count();
	}
}

auto printFinalStatisitc() -> void
{
	const std::lock_guard lock(g_statMutex);

	auto curTime = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
	auto value = curTime.time_since_epoch();
	std::vector<std::string> keys;
	std::transform(g_timeHostBegin.begin(),
				   g_timeHostBegin.end(),
				   std::back_inserter(keys),
				   [](const std::map<std::string, long long int>::value_type &pair) { return pair.first; });

	if (keys.size() > 0) {
		std::stringstream ss;
		ss << "\n" << getTS() << " Total time: " << convertBtoST(value.count() - g_timeBeginTotal) << "\n";
		ss << "============================================================================================================================"
			  "=================================\n";
		ss << "Host              | Bytes all         | Bandwidth         |    Samples CH1    |    Samples CH2    |    Samples CH3    |    "
			  "Samples CH4    |      Lost        |\n";

		bool first = true;

		for (auto const &host : keys) {
			if (first) {
				first = false;
				ss << "--------------------------------------------------------------------------------------------------------------------"
					  "-----------------------------------------|\n";
			}
			auto bw = convertBtoSpeed(g_BytesCountTotal[host], value.count() - g_timeBeginTotal);
			ss << createStr(host, 18) << "|";
			ss << " " << createStr(convertBtoS(g_BytesCountTotal[host]), 18) << "|";
			ss << " " << createStr(bw, 18) << "|";
			ss << " " << createStr(std::to_string(g_packCounterTotal_ch1[host]), 18) << "|";
			ss << " " << createStr(std::to_string(g_packCounterTotal_ch2[host]), 18) << "|";
			ss << " " << createStr(std::to_string(g_packCounterTotal_ch3[host]), 18) << "|";
			ss << " " << createStr(std::to_string(g_packCounterTotal_ch4[host]), 18) << "|";
			ss << "                  |\n";
			ss << "                  "
				  "+...................+...................+...................+...................+...................+..................."
				  "+";
			ss << " " + createStr(std::to_string(g_lostRateTotal[host]), 17) << "|\n";
			ss << "                  |";
			ss << "Lost in file: " << createStr(std::to_string(g_lostFileRateTotal[host]), 105) << "|";
			ss << "                  |\n";
			ss << "                  "
				  "+...................+...................+...................+...................+...................+..................."
				  "+";
			ss << "                  |\n";
			if (g_brokenBuffer.size() > 0 && g_brokenBuffer[host] != -1) {
				ss << "                  |";
				ss << "Broken buffers: " << createStr(std::to_string(g_brokenBuffer[host]), 63) << "|";
				ss << "                                       |                  |\n";
			}
		}
		ss << "============================================================================================================================"
			  "=================================\n";
		aprintf(stdout, "%s", ss.str().c_str());
	}
}

auto testBuffer(uint8_t *buff_c1, uint8_t *buff_c2, uint8_t *buff_c3, uint8_t *buff_c4, size_t size_ch1, size_t size_ch2, size_t, size_t)
	-> bool
{
	if (size_ch1 > 0 && size_ch2 > 0 && size_ch1 != size_ch2)
		return false;

	auto size = size_ch1 < size_ch2 ? size_ch2 : size_ch1;

	if (size_ch1 > 0 && size_ch2 > 0 && buff_c1 && buff_c2) {
		if (buff_c1[0] != buff_c2[0]) {
			return false;
		}
	}

	uint8_t value = 0;

	if (size_ch1 > 0 && buff_c1) {
		value = buff_c1[0];
	}

	if (size_ch2 > 0 && buff_c2) {
		value = buff_c2[0];
	}

	for (size_t i = 1; i < size; i++) {
		value++;
		uint8_t v1 = buff_c1 ? buff_c1[i] : value;
		uint8_t v2 = buff_c2 ? buff_c2[i] : value;
		uint8_t v3 = buff_c3 ? buff_c3[i] : value;
		uint8_t v4 = buff_c4 ? buff_c4[i] : value;

		if (value != v1 && value != v2 && value != v3 && value != v4) {
			return false;
		}
	}
	return true;
}
