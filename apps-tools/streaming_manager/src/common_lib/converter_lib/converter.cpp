#include <iostream>
#include <fstream>

#include "converter.h"
#include "logger_lib/file_logger.h"
#include "writer_lib/file_helper.h"

using namespace converter_lib;

auto CConverter::create() -> CConverter::Ptr
{
	return std::make_shared<CConverter>();
}

CConverter::CConverter()
{
	m_stopWriteCSV = false;
}

CConverter::~CConverter()
{
	stopWriteToCSV();
	std::lock_guard lock(m_mtx); // wait while file converting
}

auto CConverter::stopWriteToCSV() -> void
{
	m_stopWriteCSV = true;
}

bool CConverter::convertToCSV(std::string _file_name, std::string _prefix)
{
	return convertToCSV(_file_name, -2, -2, _prefix);
}

bool CConverter::convertToCSV(std::string _file_name, int32_t start_seg, int32_t end_seg, std::string _prefix)
{
	std::lock_guard lock(m_mtx);
	bool ret = true;
	m_stopWriteCSV = false;
	try {
		if (m_stopWriteCSV)
			return false;
		if (_prefix != "") {
			_prefix = "[" + _prefix + "] ";
		}
		aprintf(stdout, "%s Started converting to CSV\n", _prefix.c_str());
		std::string csv_file = _file_name.substr(0, _file_name.size() - 3) + "csv";

		aprintf(stdout, "%s %s\n", _prefix.c_str(), csv_file.c_str());
		std::fstream fs;
		std::fstream fs_out;
		fs.open(_file_name, std::ios::binary | std::ofstream::in | std::ofstream::out);
		fs_out.open(csv_file, std::ofstream::in | std::ofstream::trunc | std::ofstream::out);
		if (fs.fail() || fs_out.fail()) {
			aprintf(stderr, "Error open files\n");
			ret = false;
		} else {
			fs.seekg(0, std::ios::end);
			int64_t Length = fs.tellg();
			int64_t position = 0;
			int32_t curSegment = 0;
			uint64_t samplePos = 0;
			int channels = 0;
			start_seg = std::max(start_seg, 1);
			while (position >= 0) {
				auto freeSize = getFreeSpaceDisk(csv_file);
				if (freeSize <= USING_FREE_SPACE) {
					aprintf(stdout, "%s Disk is full\n", _prefix.c_str());
					ret = false;
					break;
				}
				if (m_stopWriteCSV) {
					aprintf(stdout, "%s Abort writing to CSV file\n", _prefix.c_str());
					ret = false;
					break;
				}
				curSegment++;
				bool notSkip = (start_seg <= curSegment) && ((end_seg != -2 && end_seg >= curSegment) || end_seg == -2);
				auto csv_seg = readCSV(&fs, &position, &channels, &samplePos, !notSkip);
				if (end_seg == -2) {
					if (position >= 0) {
						aprintf(stdout, "\r%s PROGRESS: %d %", _prefix.c_str(), (position * 100) / Length);
					} else {
						if (position == -2) {
							aprintf(stdout, "\r%s PROGRESS: 100 %", _prefix.c_str());
						}
					}
				} else {
					if (curSegment - start_seg >= 0 && (end_seg - 1) > start_seg) {
						aprintf(stdout,
								"\r%s PROGRESS: %d %",
								_prefix.c_str(),
								((curSegment - start_seg - 1) * 100) / (end_seg - start_seg));
					}
				}

				if (notSkip && csv_seg) {
					csv_seg->seekg(0, csv_seg->beg);
					fs_out << csv_seg->rdbuf();
					fs_out.flush();
					fs_out.sync();
				}
				delete csv_seg;

				if (end_seg != -2 && end_seg < curSegment) {
					break;
				}

				if (fs.fail() || fs_out.fail()) {
					aprintf(stdout, "\n%s Error write to CSV file\n", _prefix.c_str());
					if (fs.fail())
						aprintf(stdout, "\n%s FS is fail\n", _prefix.c_str());
					if (fs_out.fail())
						aprintf(stdout, "\n%s FS out is fail\n", _prefix.c_str());
					ret = false;
					break;
				}
			}
		}
		aprintf(stdout, "\n%s Ended converting\n", _prefix.c_str());
	} catch (std::exception &e) {
		aprintf(stderr, "%s Error: convertToCSV() : %s\n", _prefix.c_str(), e.what());
		ret = false;
	}
	return ret;
}
