#ifndef READER_CONTROLLER_H
#define READER_CONTROLLER_H

#include <string>
#include <iostream>
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


/**
 * Error of reading TMDS file if Metadata block has empty data.
 * Data is dublicated
 */

using namespace std;

class CReaderController
{
    public:

        enum OpenResult{
            OR_OK                    = 0,
            OR_MISSING_CHANNELS      = 1,
            OR_WRONG_DATA_TYPE       = 2,
            OR_DATA_NOT_EQUAL        = 3,
            OR_CLOSE                 = 4
        };

        enum BufferResult{
            BR_OK                    = 0,
            BR_ENDED                 = 1,
            BR_BROKEN                = 2,
            BR_EMPTY                 = 3
        };

        using Ptr = shared_ptr<CReaderController>;
        static Ptr Create(CStreamSettings::DataFormat _fileType, string _filePath,CStreamSettings::DACRepeat _repeat,int32_t _rep_count,uint64_t memoryCacheSize = 1024 * 1014);
        CReaderController(CStreamSettings::DataFormat _fileType, string _filePath,CStreamSettings::DACRepeat _repeat,int32_t _rep_count,uint64_t memoryCacheSize = 1024 * 1014);
        ~CReaderController();

        auto isOpen() -> CReaderController::OpenResult;
        auto checkFile() -> OpenResult;
        auto getBufferPrepared(uint8_t **ch1,size_t *size_ch1, uint8_t **ch2,size_t *size_ch2) -> BufferResult;

    private:

        struct TemperaryBuffer{
            uint8_t *buffer = nullptr;
            size_t   size   = 0;
            size_t   current_pos = 0;
            void deleteBuffer();
            bool isEnded() {return size == current_pos;}
            TemperaryBuffer(){};
            TemperaryBuffer(const TemperaryBuffer&) = delete;
            TemperaryBuffer(TemperaryBuffer&&) = delete;
            TemperaryBuffer& operator=(const TemperaryBuffer&) = delete;
            TemperaryBuffer& operator=(TemperaryBuffer&&) = delete;
        };

        CReaderController(CReaderController const&) = delete;
        CReaderController& operator=(CReaderController const&) = delete;

        auto checkTDMSFile() -> OpenResult;
        auto checkWavFile() -> OpenResult;
        auto getBufferFull(uint8_t **ch1,size_t *size_ch1, uint8_t **ch2,size_t *size_ch2) -> void;
        auto getBuffer(uint8_t **ch1,size_t *size_ch1, uint8_t **ch2,size_t *size_ch2) -> bool;
        auto getBufferWav(uint8_t **ch1,size_t *size_ch1, uint8_t **ch2,size_t *size_ch2) -> bool;
        auto getBufferTdms(uint8_t **ch1,size_t *size_ch1, uint8_t **ch2,size_t *size_ch2) -> bool;
        auto openWav() -> bool;
        auto openTDMS() -> bool;
        auto moveNextMetadata() -> bool;
        auto resetReadFromBuffer() -> bool;
        auto writeFromTemp(uint8_t **buff,size_t max_size,size_t *write_pos,CReaderController::TemperaryBuffer *temp_buf) -> void;

        CStreamSettings::DataFormat         m_fileType;
        string                              m_filePath;
        CStreamSettings::DACRepeat          m_repeat;
        int32_t                             m_rep_count;
        CWaveReader                        *m_wavReader;
        TDMS::File                         *m_tdmsFile;
        vector<shared_ptr<TDMS::Segment>>   m_tdmsSegments;
        uint32_t                            m_currentSegment;
        uint32_t                            m_currentMetadata;
        vector<shared_ptr<TDMS::Metadata>>  m_currentVecMetadataPtr;
        shared_ptr<TDMS::Metadata>          m_currentMetadataPtr;
        OpenResult                          m_result;
        bool                                m_channel1Present;
        bool                                m_channel2Present;
        TemperaryBuffer                     m_tempBuffer[2];
        bool                                m_checkEmptyFile;
        uint64_t                            m_memoryCacheSize;
        size_t                              m_channel1Size;
        size_t                              m_channel2Size;
        bool                                m_useMemoryCache;
};

#endif
