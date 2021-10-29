#ifndef READER_CONTROLLER_H
#define READER_CONTROLLER_H

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <vector>
#include <string>
#include <asio.hpp>
#include "neon_asm.h"
#include "stream_settings.h"
#include "wavReader.h"
#include "common/TDMS/File.h"

using namespace std;

class CReaderController
{
    public:

        using Ptr = shared_ptr<CReaderController>;
        static Ptr Create(CStreamSettings::DataFormat _fileType, string _filePath,CStreamSettings::DACRepeat _repeat,uint32_t _rep_count);
        CReaderController(CStreamSettings::DataFormat _fileType, string _filePath,CStreamSettings::DACRepeat _repeat,uint32_t _rep_count);
        ~CReaderController();

    private:
        CReaderController(CReaderController const&) = delete;
        CReaderController& operator=(CReaderController const&) = delete;
        
        // Return 16kB * 2 for each channels
        auto getBuffer(uint8_t **ch1,size_t *size_ch1, uint8_t **ch2,size_t *size_ch2) -> bool;
        auto openWav() -> bool;
        auto openTDMS() -> bool;
        auto moveNextMetadata() -> bool;

        CStreamSettings::DataFormat         m_fileType;
        string                              m_filePath;
        CStreamSettings::DACRepeat          m_repeat;
        uint32_t                            m_rep_count;
        CWaveReader                        *m_wavReader;
        TDMS::File                         *m_tdmsFile;
        vector<shared_ptr<TDMS::Segment>>   m_tdmsSegments;
        uint32_t                            m_currentSegment;
        uint32_t                            m_currentMetadata;
        vector<shared_ptr<TDMS::Metadata>>  m_currentVecMetadataPtr;
        shared_ptr<TDMS::Metadata>          m_currentMetadataPtr;
};

#endif