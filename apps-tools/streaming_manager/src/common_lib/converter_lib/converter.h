#ifndef CONVERTER_LIB_CONVERTER_H
#define CONVERTER_LIB_CONVERTER_H

#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include "settings_lib/stream_settings.h"
#include "writer_lib/file_helper.h"

namespace bin_lib {
class FileReader;
}

namespace converter_lib {

class CConverter {
   public:
    using Ptr = std::shared_ptr<CConverter>;

    static auto create() -> Ptr;

    CConverter();
    ~CConverter();

    bool convertToCSV(std::string _file_name, std::string _prefix, FH_CSVMode mode);
    bool convertToCSV(std::string _file_name, int32_t start_seg, int32_t end_seg, std::string _prefix, FH_CSVMode mode);

    bool convertToWAV(std::string _file_name, std::string _prefix);
    bool convertToWAV(std::string _file_name, int32_t start_seg, int32_t end_seg, std::string _prefix);
    bool convertToTDMS(std::string _file_name, std::string _prefix);
    bool convertToTDMS(std::string _file_name, int32_t start_seg, int32_t end_seg, std::string _prefix);

    // A GPIO recording (BIN) to the value change dump of its lines; convertToCSV() makes the CSV
    // of a GPIO recording (a row where the lines change)
    bool convertToVCD(std::string _file_name, std::string _prefix);

    // What the next conversions do (convert_tool, streaming_improvements 3.8): the output file
    // ("" - the input with the extension of the format), the inputs (bit k - IN(k+1); 0 - all),
    // an output file that is there overwritten (else the conversion fails and says so), a CSV
    // with a head row of the column names
    void setOutput(const std::string& path);
    void setChannels(uint32_t mask);
    void setOverwrite(bool overwrite);
    void setCSVHeader(bool header);
    // the last conversion: its output file, the channels and the samples per channel written,
    // why it failed ("" when it did not)
    auto lastOutput() const -> std::string;
    auto lastChannels() const -> uint32_t;
    auto lastSamples() const -> uint64_t;  // GPIO: the words
    auto lastWasGpio() const -> bool;
    auto lastError() const -> std::string;

    void stopWriteToCSV();
    void stopWriteToWAV();
    void stopWriteToTDMS();

   private:
    CConverter(const CConverter&) = delete;
    CConverter(CConverter&&) = delete;
    CConverter& operator=(const CConverter&) = delete;
    CConverter& operator=(const CConverter&&) = delete;

    std::atomic_bool m_stopWriteCSV;
    std::atomic_bool m_stopWriteWAV;
    std::atomic_bool m_stopWriteTDMS;
    std::mutex m_mtx;

    std::string m_output;
    uint32_t m_channels = 0;
    bool m_overwrite = true;
    bool m_csvHeader = false;
    std::string m_lastOutput;
    uint32_t m_lastChannels = 0;
    uint64_t m_lastSamples = 0;
    bool m_lastGpio = false;
    std::string m_lastError;

    // the output file of an input for a format extension ("csv"), "" when it may not be written
    auto outputFor(const std::string& input, const char* extension) -> std::string;
    auto fail(const std::string& prefix, const std::string& why) -> bool;
    auto wanted(uint8_t channelId) const -> bool;

    auto toStreamFile(const std::string& file, int32_t start_seg, int32_t end_seg, std::string prefix, CStreamSettings::DataFormat format, std::atomic_bool& stop) -> bool;
    auto gpioToText(bin_lib::FileReader& reader, const std::string& file, CStreamSettings::GPIOFormat format, const std::string& prefix) -> bool;
};

}  // namespace converter_lib

#endif
