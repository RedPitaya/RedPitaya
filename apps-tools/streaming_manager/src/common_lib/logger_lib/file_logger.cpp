#include "file_logger.h"
#include <cstdarg>
#include "data_lib/network_header.h"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <time.h>
#include <vector>

#ifdef ENABLE_DEBUG_TRACE
#include <cxxabi.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <iomanip>
#include <sstream>
#endif

static std::mutex g_mtx;

// One aligned entry of the lost file; the widths keep the columns of the lines under each other
static auto writeLostEntry(std::ostream& os, const std::string& name, uint64_t pos, uint64_t samples, uint64_t lost, uint64_t wait) -> void {
    os << std::left << std::setw(10) << (name + ":") << std::right << " Pos " << std::setw(14) << pos << "  Get: " << std::setw(8) << samples << std::setw(13)
       << ("(" + std::to_string(lost) + ")") << "  Wait: " << std::setw(12) << wait;
}

static constexpr const char* kLostSeparator = "   ";

#define ANSI_COLOR_RED "\033[1;91m"
#define ANSI_COLOR_GREEN "\033[1;92m"
#define ANSI_COLOR_YELLOW "\033[1;93m"
#define ANSI_COLOR_BLUE "\033[1;94m"
#define ANSI_COLOR_MAGENTA "\033[1;95m"
#define ANSI_COLOR_CYAN "\033[1;97m"
#define ANSI_COLOR_RESET "\x1b[0m"

void aprintf(FILE* stream, const char* format, ...) {
    const std::lock_guard lock(g_mtx);
    va_list args;
    va_start(args, format);
    vfprintf(stream, format, args);
    va_end(args);
}

void acprintf(FILE* stream, __attribute__((unused)) PColor color, const char* format, ...) {
    const std::lock_guard lock(g_mtx);
    va_list args;
    va_start(args, format);
    std::string s = format;
#ifndef _WIN32
    switch (color) {
        case WHITE:
            vfprintf(stream, format, args);
            break;
        case RED:
            s = ANSI_COLOR_RED + s + ANSI_COLOR_RESET;
            vfprintf(stream, s.c_str(), args);
            break;
        case GREEN:
            s = ANSI_COLOR_GREEN + s + ANSI_COLOR_RESET;
            vfprintf(stream, s.c_str(), args);
            break;
        case YELLOW:
            s = ANSI_COLOR_YELLOW + s + ANSI_COLOR_RESET;
            vfprintf(stream, s.c_str(), args);
            break;
        case BLUE:
            s = ANSI_COLOR_BLUE + s + ANSI_COLOR_RESET;
            vfprintf(stream, s.c_str(), args);
            break;
        case MAGENTA:
            s = ANSI_COLOR_MAGENTA + s + ANSI_COLOR_RESET;
            vfprintf(stream, s.c_str(), args);
            break;
        case CYAN:
            s = ANSI_COLOR_CYAN + s + ANSI_COLOR_RESET;
            vfprintf(stream, s.c_str(), args);
            break;
        default:
            vfprintf(stream, format, args);
            break;
    }
#else
    vfprintf(stream, format, args);
#endif
    va_end(args);
}

auto CFileLogger::create(std::string _filePath, bool testMode) -> CFileLogger::Ptr {
    return std::make_shared<CFileLogger>(_filePath, testMode);
}

CFileLogger::CFileLogger(std::string _filePath, bool testMode)
    : m_filePath(_filePath + ".txt"),
      m_filePathLost(_filePath + ".lost.txt"),
      m_file_open(false),
      m_oscRate(0),
      m_fileSystemLostRate(0),
      m_reciveData(0),
      m_out_of_memory(0),
      m_testMode(testMode),
      m_channels(),
      m_current_sample(),
      m_packs(0),
      m_decimation(0),
      m_flags(0),
      m_triggerCountFirst(0),
      m_triggerCountLast(0),
      m_gpioProperties(),
      m_gpioPropertiesSet(false),
      m_gpio() {
    resetCounters();
    m_file_open = true;
    if (!m_testMode) {
        m_fileLost.open(m_filePathLost, std::ios_base::app | std::ios_base::out);
        if (m_fileLost.is_open() && m_fileLost.tellp() == 0) {
            m_fileLost << "# One line per buffer. Pos - where the buffer starts on the time line of the stream, its gaps come first:"
                          " (n) - samples lost to an overflow, Wait - samples not acquired while waiting for a trigger;"
                          " then Get - samples in the buffer. Trig - the FPGA trigger counter after the buffer (trigger mode only)."
                          " GPIO lines count samples of the GPIO clock, Words - the stream words of the buffer (an RLE word stands for several samples).\n";
        }
    }
}

CFileLogger::~CFileLogger() {}

void CFileLogger::resetCounters() {
    m_out_of_memory = 0;
    m_fileSystemLostRate = 0;
    m_reciveData = 0;
    m_oscRate = 0;
    m_packs = 0;
    m_decimation = 0;
    m_flags = 0;
    m_triggerCountFirst = 0;
    m_triggerCountLast = 0;
    m_gpioProperties = GpioProperties();
    m_gpioPropertiesSet = false;
    m_gpio = GpioStat();
    for (auto i = (int)DataLib::CH1; i <= (int)DataLib::CH4; i++) {
        DataLib::EDataBuffersPackChannel ch = (DataLib::EDataBuffersPackChannel)i;
        m_current_sample[ch] = 0;
    }
    m_channels.clear();
}

auto CFileLogger::addMetric(CFileLogger::EMetric _metric, uint64_t _value) -> void {
    const std::lock_guard<std::mutex> lock(m_mtx);
    switch (_metric) {
        case EMetric::OSC_RATE:
            m_oscRate = _value;
            break;

        case EMetric::OUT_OF_MEMORY:
            m_out_of_memory += _value;
            break;

        case EMetric::FILESYSTEM_RATE:
            m_fileSystemLostRate += _value;
            break;

        case EMetric::RECIVE_DATE:
            m_reciveData += _value;
            break;
        default:
            break;
    }
}

auto CFileLogger::getFileLost() -> uint64_t {
    const std::lock_guard<std::mutex> lock(m_mtx);
    return m_fileSystemLostRate;
}

auto CFileLogger::addMetric(DataLib::CDataBuffersPackDMA::Ptr pack) -> void {
    const std::lock_guard<std::mutex> lock(m_mtx);
    DataLib::CDataBufferDMA::Ptr first;
    for (auto i = (int)DataLib::CH1; i <= (int)DataLib::CH4 && !first; i++) {
        first = pack->getBuffer((DataLib::EDataBuffersPackChannel)i);
    }
    if (!first)
        return;
    if (m_packs == 0) {
        m_triggerCountFirst = first->getTriggerCount();
    }
    m_packs++;
    m_decimation = first->getDecimation();
    m_flags = first->getADCFlags();
    m_triggerCountLast = first->getTriggerCount();

    if (m_file_open && m_fileLost.is_open()) {
        for (auto i = (int)DataLib::CH1; i <= (int)DataLib::CH4; i++) {
            DataLib::EDataBuffersPackChannel ch = (DataLib::EDataBuffersPackChannel)i;
            auto buff = pack->getBuffer(ch);
            if (buff) {
                if (buff != first)
                    m_fileLost << kLostSeparator;
                writeLostEntry(m_fileLost, getName(ch), m_current_sample[ch], buff->getSamplesCount(), buff->getLostSamplesAll(), buff->getWaitSamples());
                m_current_sample[ch] += buff->getSamplesWithGaps();
            }
        }
        if (m_flags & DataLib::ADC_FLAG_TRIGGER_MODE) {
            m_fileLost << kLostSeparator << "Trig: " << m_triggerCountLast;
        }
        m_fileLost << "\n";
    }
}

auto CFileLogger::addMetric(DataLib::EDataBuffersPackChannel channel, uint64_t _value, uint64_t _lostFPGA, uint64_t _wait, uint64_t _samples) -> void {
    const std::lock_guard<std::mutex> lock(m_mtx);
    auto& stat = m_channels[channel];  // value-initialized on the first call
    stat.pass += _value;
    stat.lostFpga += _lostFPGA;
    stat.wait += _wait;
    stat.samples += _samples;
}

auto CFileLogger::setGpioProperties(const GpioProperties& properties) -> void {
    const std::lock_guard<std::mutex> lock(m_mtx);
    m_gpioProperties = properties;
    m_gpioPropertiesSet = true;
}

auto CFileLogger::addGpioMetric(uint64_t bytes, uint64_t words, uint64_t samples, uint64_t lost, uint64_t wait, uint32_t triggerCount) -> void {
    const std::lock_guard<std::mutex> lock(m_mtx);
    if (m_gpio.buffers == 0) {
        m_gpio.triggerCountFirst = triggerCount;
    }
    m_gpio.buffers++;
    m_gpio.bytes += bytes;
    m_gpio.words += words;
    m_gpio.samples += samples;
    m_gpio.lost += lost;
    m_gpio.wait += wait;
    m_gpio.triggerCountLast = triggerCount;
    if (m_file_open && m_fileLost.is_open()) {
        writeLostEntry(m_fileLost, "GPIO", m_gpio.pos, samples, lost, wait);
        m_fileLost << kLostSeparator << "Words: " << std::setw(8) << words;
        if (m_gpioProperties.triggerMode) {
            m_fileLost << kLostSeparator << "Trig: " << triggerCount;
        }
        m_fileLost << "\n";
    }
    m_gpio.pos += lost + wait + samples;
}

void CFileLogger::dumpToFile() {
    const std::lock_guard<std::mutex> lock(m_mtx);
    try {
        if (!m_file_open)
            return;
        m_file_open = false;
        if (m_fileLost.is_open()) {
            m_fileLost.close();
        }

        std::ofstream log(m_filePath, std::ios_base::app | std::ios_base::out);

        char buff[20];
        struct tm* sTm;
        time_t now = time(0);
        sTm = gmtime(&now);
        strftime(buff, sizeof(buff), "%Y-%m-%d %H:%M:%S", sTm);
        log << "======================================================================\n";
        log << "====   Data transfer report  " << buff << " UTC =================\n";
        log << "======================================================================\n";
        log << "\n\n";
        if (m_packs || !m_gpio.buffers) {  // a GPIO only recording has no ADC part
            // The rate of the buffers is the sample rate after the decimation
            log << "Current ADC speed:\t" << m_oscRate << "\n";
            if (m_decimation) {
                log << "Decimation:\t" << m_decimation << "\n";
                log << "Sample rate:\t" << m_oscRate << "\n";
                log << "ADC rate:\t" << m_oscRate * m_decimation << "\n";
                log << "Trigger mode:\t" << ((m_flags & DataLib::ADC_FLAG_TRIGGER_MODE) ? "on" : "off") << "\n";
                if (m_flags & DataLib::ADC_FLAG_TRIGGER_MODE) {
                    log << "FPGA trigger counter after the first and the last buffer:\t" << m_triggerCountFirst << " / " << m_triggerCountLast << "\n";
                }
                log << "The FPGA counts the trigger wait apart from the loss:\t" << ((m_flags & DataLib::ADC_FLAG_WAIT_COUNTER) ? "yes" : "no") << "\n";
            } else {
                log << "Decimation, trigger mode:\tnot reported\n";
            }
            log << "Buffers:\t" << m_packs << "\n";
            log << "\n\n";
        }
        log << "Lost data due to file write buffer overflow:\t" << m_fileSystemLostRate << " buffers\n";
        log << "Loss of data due to lack of memory:\t" << m_out_of_memory << " buffers\n";
        log << "\n";
        log << "Total amount of data transferred:\n";
        log << "\t-" << m_reciveData << "b \n";
        log << "\t-" << m_reciveData / 1024 << "kb \n";
        log << "\t-" << m_reciveData / (1024 * 1024) << "Mb \n";
        log << "\n";
        for (const auto& kv : m_channels) {
            log << "The total amount of data transmitted on: " << getName(kv.first) << "\n";
            log << "\t-" << kv.second.samples << " Samples\n";
            log << "\t-" << kv.second.pass << "b \n";
            log << "\t-" << kv.second.pass / 1024 << "kb \n";
            log << "\t-" << kv.second.pass / (1024 * 1024) << "Mb \n";
            log << "\tLost data on: " << getName(kv.first) << "\n";
            log << "\t- FPGA:\t" << kv.second.lostFpga << " Samples\n";
            log << "\tWait for a trigger on: " << getName(kv.first) << "\n";
            log << "\t- " << kv.second.wait << " Samples\n";
            log << "\n";
        }
        if (m_gpio.buffers || m_gpioPropertiesSet) {
            const auto& g = m_gpioProperties;
            log << "GPIO input:\n";
            log << "\tBase rate:\t" << g.baseRate << "\n";
            log << "\tDecimation:\t" << g.decimation << "\n";
            if (g.decimation) {
                char rate[32];
                snprintf(rate, sizeof(rate), "%.3f", (double)g.baseRate / g.decimation);
                log << "\tSample rate:\t" << rate << "\n";
            }
            log << "\tRLE:\t" << (g.rle ? "on" : "off") << "\n";
            log << "\tTrigger mode:\t" << (g.triggerMode ? "on" : "off") << "\n";
            if (g.triggerMode) {
                log << "\tFPGA trigger counter after the first and the last buffer:\t" << m_gpio.triggerCountFirst << " / " << m_gpio.triggerCountLast << "\n";
            }
            log << "\tThe FPGA counts the trigger wait apart from the loss:\t" << (g.waitCounter ? "yes" : "no") << "\n";
            log << "\tBuffers:\t" << m_gpio.buffers << "\n";
            log << "The total amount of data transmitted on: GPIO\n";
            log << "\t-" << m_gpio.samples << " Samples\n";
            log << "\t-" << m_gpio.words << " Words\n";
            log << "\t-" << m_gpio.bytes << "b \n";
            log << "\t-" << m_gpio.bytes / 1024 << "kb \n";
            log << "\t-" << m_gpio.bytes / (1024 * 1024) << "Mb \n";
            // not "FPGA:" - the regress reads that pattern once per ADC channel
            log << "\tLost data on: GPIO\n";
            log << "\t- FPGA overflow:\t" << m_gpio.lost << " Samples\n";
            log << "\tWait for a trigger on: GPIO\n";
            log << "\t- " << m_gpio.wait << " Samples\n";
            log << "\n";
        }

    } catch (std::exception& e) {
        fprintf(stderr, "Error: CFileLogger::DumpToFile() %s\n", e.what());
    }
}

auto CFileLogger::getName(DataLib::EDataBuffersPackChannel channel) -> std::string {
    switch (channel) {
        case DataLib::EDataBuffersPackChannel::CH1:
            return "Channel 1";
        case DataLib::EDataBuffersPackChannel::CH2:
            return "Channel 2";
        case DataLib::EDataBuffersPackChannel::CH3:
            return "Channel 3";
        case DataLib::EDataBuffersPackChannel::CH4:
            return "Channel 4";
        default:
            return "UNDEFINED";
    }
}

#ifdef ENABLE_DEBUG_TRACE

std::string demangle(const char *symbol)
{
	int status = 0;
	char *demangled = abi::__cxa_demangle(symbol, nullptr, nullptr, &status);
	if (status == 0 && demangled) {
		std::string result(demangled);
		free(demangled);
		return result;
	}
	return symbol;
}

struct FunctionInfo
{
	std::string fullName;	// namespace::Class::function(int, char*)
	std::string shortName;	// function
	std::string withParams; // function(int, char*)
	void *address;
};

FunctionInfo resolveFunction(void *addr)
{
	FunctionInfo info;
	info.address = addr;

	Dl_info dlinfo;
	if (dladdr(addr, &dlinfo) && dlinfo.dli_sname) {
		info.fullName = demangle(dlinfo.dli_sname);

		if (dlinfo.dli_saddr) {
			uintptr_t offset = (uintptr_t) addr - (uintptr_t) dlinfo.dli_saddr;
			if (offset > 0) {
				info.fullName += "+0x" + std::to_string(offset);
			}
		}

		info.withParams = demangle(dlinfo.dli_sname);

		size_t paren = info.withParams.find('(');
		if (paren != std::string::npos) {
			info.shortName = info.withParams.substr(0, paren);
		} else {
			info.shortName = info.withParams;
		}

		size_t lastColon = info.shortName.rfind("::");
		if (lastColon != std::string::npos) {
			info.shortName = info.shortName.substr(lastColon + 2);
		} else {
			size_t lastSpace = info.shortName.rfind(' ');
			if (lastSpace != std::string::npos) {
				info.shortName = info.shortName.substr(lastSpace + 1);
			}
		}
	} else {
		std::ostringstream oss;
		oss << "[" << addr << "]";
		info.fullName = oss.str();
		info.shortName = info.fullName;
		info.withParams = info.fullName;
	}

	return info;
}

std::string getStackTrace(int skipFrames, int maxFrames, StackTraceFormat format)
{
	std::vector<void *> array(maxFrames + skipFrames);
	int size = backtrace(array.data(), maxFrames + skipFrames);

	std::vector<FunctionInfo> functions;
	functions.reserve(size);
	for (int i = 0; i < size; i++) {
		functions.push_back(resolveFunction(array[i]));
	}

	std::ostringstream oss;

	switch (format) {
		case StackTraceFormat::SHORT: {
			int count = 0;
			for (int i = skipFrames; i < size && count < maxFrames; i++) {
				if (count > 0) {
					oss << " <- ";
				}
				oss << functions[i].shortName;
				count++;
			}
			break;
		}

		case StackTraceFormat::WITH_PARAMS: {
			int count = 0;
			for (int i = skipFrames; i < size && count < maxFrames; i++) {
				if (count > 0) {
					oss << " <- ";
				}
				oss << functions[i].withParams;
				count++;
			}
			break;
		}

		case StackTraceFormat::FULL: {
			for (int i = 0; i < size; i++) {
				oss << "  #" << std::setw(2) << std::setfill('0') << i << " ";
				oss << functions[i].fullName;
				oss << " [" << functions[i].address << "]\n";
			}
			break;
		}
	}

	return oss.str();
}

#endif // ENABLE_DEBUG_TRACE
