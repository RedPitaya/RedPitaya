#ifndef CONVERTER_LIB_CONVERTER_H
#define CONVERTER_LIB_CONVERTER_H

#include <memory>

#include "settings_lib/stream_settings.h"
#include "logger_lib/file_logger.h"
#include "writer_lib/file_helper.h"
#include "writer_lib/file_queue_manager.h"
#include "wav_lib/wav_writer.h"
#include "net_lib/asio_common.h"
#include "data_lib/signal.hpp"


namespace converter_lib {

class CConverter
{
public:

    static auto makeEmptyDir(const std::string &_filePath) -> void;
  
    using Ptr = std::shared_ptr<CConverter>;


    static auto create() -> Ptr;

    CConverter();
    ~CConverter();

    bool convertToCSV(std::string _file_name, std::string _prefix);
    bool convertToCSV(std::string _file_name, int32_t start_seg, int32_t end_seg,std::string _prefix);
    void stopWriteToCSV();

private:


    CConverter(const CConverter &) = delete;
    CConverter(CConverter &&) = delete;
    CConverter& operator=(const CConverter&) =delete;
    CConverter& operator=(const CConverter&&) =delete;

    std::atomic_bool m_stopWriteCSV;

    std::mutex  m_mtx;
};

}

#endif
