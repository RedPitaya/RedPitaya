#ifndef CONVERTER_LIB_CONVERTER_H
#define CONVERTER_LIB_CONVERTER_H

#include <atomic>
#include <memory>
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

    auto toStreamFile(const std::string& file, int32_t start_seg, int32_t end_seg, std::string prefix, CStreamSettings::DataFormat format, std::atomic_bool& stop) -> bool;
    auto gpioToText(bin_lib::FileReader& reader, const std::string& file, CStreamSettings::GPIOFormat format, const std::string& prefix) -> bool;
};

}  // namespace converter_lib

#endif
