#include "reader_controller.h"
#include "data_lib/neon_asm.h"
#include "logger_lib/file_logger.h"

// constexpr size_t g_max_buff = 32 * 1024;

CReaderController::Ptr CReaderController::Create(CStreamSettings::DataFormat _fileType, std::string _filePath, CStreamSettings::DACRepeat _repeat,
                                                 uint32_t _rep_count, uint32_t blockSize) {
    return std::make_shared<CReaderController>(_fileType, _filePath, _repeat, _rep_count, blockSize);
}

CReaderController::Ptr CReaderController::Create(DataIn* dataIn, CStreamSettings::DACRepeat _repeat, uint32_t _rep_count, uint32_t blockSize) {
    return std::make_shared<CReaderController>(dataIn, _repeat, _rep_count, blockSize);
}

CReaderController::CReaderController(CStreamSettings::DataFormat _fileType, std::string _filePath, CStreamSettings::DACRepeat _repeat, uint32_t _rep_count,
                                     uint32_t blockSize)
    : m_fileType(_fileType),
      m_filePath(_filePath),
      m_repeat(_repeat),
      m_rep_count(_rep_count),
      m_wavReader(nullptr),
      m_tdmsFile(nullptr),
      m_tdmsSegments(),
      m_currentSegment(0),
      m_currentMetadata(0),
      m_currentVecMetadataPtr(),
      m_currentMetadataPtr(),
      m_result(OpenResult::OR_CLOSE),
      m_channel1Present(false),
      m_channel2Present(false),
      m_checkEmptyFile(false),
      m_channel1Size(0),
      m_channel2Size(0),
      m_blockSize(blockSize),
      m_genData(nullptr) {
    m_dataBuffers[0] = new uint8_t[blockSize];
    m_dataBuffers[1] = new uint8_t[blockSize];
    m_tempBuffer[0].memoryMode = false;
    m_tempBuffer[1].memoryMode = false;

    if (m_repeat.value == CStreamSettings::DACRepeat::DAC_REP_ON && m_rep_count == 0) {
        m_rep_count = 1;
    }

    if (m_fileType.value == CStreamSettings::DataFormat::WAV) {
        openWav();
    }

    if (m_fileType.value == CStreamSettings::DataFormat::TDMS) {
        openTDMS();
    }
    m_result = checkFile();
    resetReadFromBuffer();
}

CReaderController::CReaderController(DataIn* dataIn, CStreamSettings::DACRepeat _repeat, uint32_t _rep_count, uint32_t blockSize)
    : m_fileType(CStreamSettings::DataFormat::BIN),
      m_filePath(""),
      m_repeat(_repeat),
      m_rep_count(_rep_count),
      m_wavReader(nullptr),
      m_tdmsFile(nullptr),
      m_tdmsSegments(),
      m_currentSegment(0),
      m_currentMetadata(0),
      m_currentVecMetadataPtr(),
      m_currentMetadataPtr(),
      m_result(OpenResult::OR_CLOSE),
      m_channel1Present(false),
      m_channel2Present(false),
      m_checkEmptyFile(false),
      m_channel1Size(0),
      m_channel2Size(0),
      m_blockSize(blockSize),
      m_genData(dataIn) {
    m_dataBuffers[0] = new uint8_t[blockSize];
    m_dataBuffers[1] = new uint8_t[blockSize];
    m_tempBuffer[0].memoryMode = true;
    m_tempBuffer[1].memoryMode = true;

    if (m_repeat.value == CStreamSettings::DACRepeat::DAC_REP_ON && m_rep_count == 0) {
        m_rep_count = 1;
    }
    if (dataIn) {
        if (dataIn->ch[0] && dataIn->ch[1] && dataIn->size[0] != dataIn->size[1])
            FATAL("The buffers of the first and second channels must be equal.")
        if (dataIn->size[0] & 0x1 || dataIn->size[1] & 0x1)
            FATAL("Data buffers must be a multiple of 2")
    } else {
        FATAL("Memory not initialized")
    }
    m_result = checkFile();
    resetReadFromBuffer();
}

void CReaderController::TemperaryBuffer::deleteBuffer() {
    if (!memoryMode)
        delete[] buffer;
    buffer = nullptr;
    size = 0;
    current_pos = 0;
}

CReaderController::~CReaderController() {
    m_tempBuffer[0].deleteBuffer();
    m_tempBuffer[1].deleteBuffer();
    delete m_wavReader;
    delete m_tdmsFile;
    delete[] m_dataBuffers[0];
    delete[] m_dataBuffers[1];
    delete m_genData;
}

auto CReaderController::getChannels(bool* ch1Active, bool* ch2Active) -> void {
    *ch1Active = m_channel1Present;
    *ch2Active = m_channel2Present;
}

auto CReaderController::getChannelsSize(size_t* ch1Size, size_t* ch2Size) -> void {
    *ch1Size = m_channel1Size;
    *ch2Size = m_channel2Size;
}

auto CReaderController::disableRepeatMode() -> void {
    m_repeat = CStreamSettings::DACRepeat::DAC_REP_OFF;
}

auto CReaderController::openWav() -> bool {
    try {
        if (m_fileType.value == CStreamSettings::DataFormat::WAV) {
            if (m_wavReader)
                delete m_wavReader;
            m_wavReader = new CWaveReader();
            if (!m_wavReader->openFile(m_filePath)) {
                std::cerr << "[CReaderController]: Error open wav file(" << m_filePath << ") " << std::endl;
                delete m_wavReader;
                m_wavReader = nullptr;
                return false;
            }
            m_result = OpenResult::OR_OK;
            return true;
        }
    } catch (const std::bad_alloc& e) {
        ERROR_LOG("Error Allocation failed: %s ", e.what());
    }
    return false;
}

auto CReaderController::openTDMS() -> bool {
    try {
        m_channel1Present = false;
        m_channel2Present = false;
        if (m_fileType.value == CStreamSettings::DataFormat::TDMS) {
            if (m_tdmsFile)
                delete m_tdmsFile;
            m_tdmsFile = new TDMS::File();
            m_tdmsSegments = m_tdmsFile->ReadFileWithoutClose(m_filePath);
            if (m_tdmsSegments.empty()) {
                std::cerr << "[CReaderController]: Error open tdms file(" << m_filePath << ") " << std::endl;
                m_tdmsFile->Close();
                delete m_tdmsFile;
                m_tdmsFile = nullptr;
                return false;
            }
            resetReadFromBuffer();
            m_result = OpenResult::OR_OK;
            return true;
        }
    } catch (const std::bad_alloc& e) {
        ERROR_LOG("Error Allocation failed: %s ", e.what());
    }
    return false;
}

auto CReaderController::resetReadFromBuffer() -> bool {
    m_tempBuffer[0].deleteBuffer();
    m_tempBuffer[1].deleteBuffer();
    if (m_genData) {
        m_genData->readPosition = 0;
        m_result = OpenResult::OR_OK;
        return true;
    } else {
        m_checkEmptyFile = true;

        if (m_fileType.value == CStreamSettings::DataFormat::WAV) {
            return openWav();
        }

        if (m_fileType.value == CStreamSettings::DataFormat::TDMS) {
            m_currentSegment = 0;
            m_currentMetadata = -1;
            if (m_tdmsFile)
                m_tdmsFile->clearPrevMetadata();
            moveNextMetadata();
            return true;
        }
        return false;
    }
}

auto CReaderController::writeFromTemp(uint8_t** buff, size_t max_size, size_t* write_pos, CReaderController::TemperaryBuffer* temp_buf,
                                      size_t* realSize) -> void {
    size_t sizeForWrite = temp_buf->size - temp_buf->current_pos;
    size_t aviableSize = max_size - *write_pos;
    size_t sizeWrite = (sizeForWrite > aviableSize ? aviableSize : sizeForWrite);
    memcpy((&(**buff) + *write_pos), (&(*temp_buf->buffer) + temp_buf->current_pos), sizeWrite);
    *write_pos += sizeWrite;
    temp_buf->current_pos += sizeWrite;
    *realSize += sizeWrite;
}

auto CReaderController::getBufferPrepared(Data& data) -> BufferResult {
    auto fillZero = [&](uint8_t** ch, size_t* size) {
        if (*ch) {
            if (0 != *size) {
                memset((&(**ch) + *size), 0, m_blockSize - *size);
                *size = m_blockSize;
            }
        }
    };
    data.size[0] = data.size[1] = 0;
    if (m_channel1Present) {
        data.ch[0] = m_dataBuffers[0];
    } else {
        data.ch[0] = nullptr;
    }
    if (m_channel2Present) {
        data.ch[1] = m_dataBuffers[1];
    } else {
        data.ch[1] = nullptr;
    }

    while (1) {
        if (data.ch[0]) {
            if (m_tempBuffer[0].size > 0 && !m_tempBuffer[0].isEnded()) {
                writeFromTemp(&data.ch[0], m_blockSize, &data.size[0], &m_tempBuffer[0], &data.real_size[0]);
                data.bits = m_tempBuffer[0].bits;
            }
        }

        if (data.ch[1]) {
            if (m_tempBuffer[1].size > 0 && !m_tempBuffer[1].isEnded()) {
                writeFromTemp(&data.ch[1], m_blockSize, &data.size[1], &m_tempBuffer[1], &data.real_size[1]);
                data.bits = m_tempBuffer[0].bits;
            }
        }

        if (data.ch[0] && data.ch[1]) {
            if (data.size[0] == m_blockSize && data.size[1] == m_blockSize) {
                return BR_OK;
            } else {
                if (m_fileType.value == CStreamSettings::DataFormat::WAV) {
                    if (data.size[0] != data.size[1]) {
                        return BR_BROKEN;
                    }
                }
            }
        } else if (data.ch[0]) {
            if (data.size[0] == m_blockSize)
                return BR_OK;
        } else if (data.ch[1]) {
            if (data.size[1] == m_blockSize)
                return BR_OK;
        }

        if (m_tempBuffer[0].isEnded() || m_tempBuffer[1].isEnded()) {
            Data data_read;
            auto res = getBuffer(data_read);
            if (m_checkEmptyFile) {
                if (data_read.size[0] || data_read.size[1]) {
                    m_checkEmptyFile = false;
                } else {
                    if (m_genData == nullptr) {
                        delete[] data_read.ch[0];
                        delete[] data_read.ch[1];
                    }
                    return BR_EMPTY;
                }
            }

            if (res) {  // if don't use memory cache
                data.bits = data_read.bits;
                if (data_read.ch[0]) {
                    m_tempBuffer[0].deleteBuffer();
                    m_tempBuffer[0].buffer = data_read.ch[0];
                    m_tempBuffer[0].size = data_read.size[0];
                    m_tempBuffer[0].bits = data_read.bits;
                }

                if (data_read.ch[1]) {
                    m_tempBuffer[1].deleteBuffer();
                    m_tempBuffer[1].buffer = data_read.ch[1];
                    m_tempBuffer[1].size = data_read.size[1];
                    m_tempBuffer[1].bits = data_read.bits;
                }
            } else {
                if (m_repeat.value != CStreamSettings::DACRepeat::DAC_REP_OFF) {
                    if (resetReadFromBuffer()) {
                        if (m_repeat.value != CStreamSettings::DACRepeat::DAC_REP_INF) {
                            m_rep_count--;
                            if (m_rep_count <= 0) {
                                fillZero(&data.ch[0], &data.size[0]);
                                fillZero(&data.ch[1], &data.size[1]);
                                return BR_ENDED;
                            }
                        }
                    } else {
                        return BR_BROKEN;
                    }
                } else {
                    fillZero(&data.ch[0], &data.size[0]);
                    fillZero(&data.ch[1], &data.size[1]);
                    return BR_ENDED;
                }
            }
        }
    }
}

auto CReaderController::getBuffer(Data& data) -> bool {
    if (m_genData) {
        data.bits = m_genData->bits;
        for (int i = 0; i < 2; i++) {
            if (m_genData->ch[i]) {
                data.ch[i] = m_genData->ch[i] + m_genData->readPosition;
                data.size[i] = std::min(m_genData->size[i] > m_genData->readPosition ? m_genData->size[i] - m_genData->readPosition : 0, (size_t)m_blockSize);
            } else {
                data.size[i] = 0;
            }
        }
        m_genData->readPosition += std::max(data.size[0], data.size[1]);
        return data.size[0] > 0 || data.size[1] > 0;
    } else if (m_fileType.value == CStreamSettings::DataFormat::WAV) {
        return getBufferWav(data);
    } else if (m_fileType.value == CStreamSettings::DataFormat::TDMS) {
        return getBufferTdms(data);
    }
    return false;
}

auto CReaderController::getBufferWav(Data& data) -> bool {
    try {
        if (m_wavReader && m_fileType.value == CStreamSettings::DataFormat::WAV) {
            if (m_wavReader->getBuffers(&data.ch[0], &data.size[0], &data.ch[1], &data.size[1], &data.bits)) {
                if (data.size[0] == 0 && data.size[1] == 0) {
                    return false;
                }
                return true;
            }
        }
    } catch (const std::bad_alloc& e) {
        for (int i = 0; i < 2; i++) {
            delete[] data.ch[i];
            data.ch[i] = nullptr;
            data.size[i] = 0;
        }
        std::cout << "[CReaderController]: Error Allocation failed: " << e.what() << '\n';
    }
    return false;
}

auto CReaderController::getBufferTdms(Data& data) -> bool {
    if (m_tdmsFile && m_fileType.value == CStreamSettings::DataFormat::TDMS) {
        if (m_channel1Present == false && m_channel2Present == false) {
            return false;
        }

        size_t size1 = 0;
        size_t size2 = 0;

        if (!m_currentMetadataPtr) {
            data.size[0] = size1;
            data.size[1] = size2;
            return false;
        }

        if (m_currentMetadataPtr->RawData.Size > 0) {
            if (m_currentMetadataPtr->PathStr == "/'Group'/'ch1'") {
                size_t sizeCh1 = 0;
                for (auto& m : m_currentMetadataPtr->RawData.DataType.GetRawVector()) {
                    sizeCh1 += m->size;
                    if (m->dataType == TDMS::TDMSType::Integer8) {
                        data.bits = 8;
                    }
                    if (m->dataType == TDMS::TDMSType::Integer16) {
                        data.bits = 16;
                    }
                }
                data.ch[0] = new uint8_t[sizeCh1];
                data.size[0] = sizeCh1;
                data.size[1] = 0;
                uint8_t* buffer = data.ch[0];
                size_t pos = 0;
                for (auto& m : m_currentMetadataPtr->RawData.DataType.GetRawVector()) {
                    memcpy_neon((&(*buffer) + pos), m->data.get(), m->size);
                    pos += m->size;
                }
                moveNextMetadata();
                return true;
            }

            if (m_currentMetadataPtr->PathStr == "/'Group'/'ch2'") {
                size_t sizeCh2 = 0;
                for (auto& m : m_currentMetadataPtr->RawData.DataType.GetRawVector()) {
                    sizeCh2 += m->size;
                    if (m->dataType == TDMS::TDMSType::Integer8) {
                        data.bits = 8;
                    }
                    if (m->dataType == TDMS::TDMSType::Integer16) {
                        data.bits = 16;
                    }
                }
                data.ch[1] = new uint8_t[sizeCh2];
                data.size[0] = 0;
                data.size[1] = sizeCh2;
                uint8_t* buffer = data.ch[1];
                size_t pos = 0;
                for (auto& m : m_currentMetadataPtr->RawData.DataType.GetRawVector()) {
                    memcpy_neon((&(*buffer) + pos), m->data.get(), m->size);
                    pos += m->size;
                }
                moveNextMetadata();
                return true;
            }

            moveNextMetadata();
            data.size[0] = size1;
            data.size[1] = size2;
            return true;
        } else {
            moveNextMetadata();
        }
    }
    return false;
}

auto CReaderController::moveNextMetadata() -> bool {
    do {
        if (m_currentVecMetadataPtr.empty()) {
            if (m_currentSegment >= m_tdmsSegments.size()) {
                return false;
            }
            m_currentVecMetadataPtr = m_tdmsFile->GetMetadata(m_tdmsSegments[m_currentSegment]);
        }
        m_currentMetadata++;
        if (m_currentMetadata < m_currentVecMetadataPtr.size()) {
            m_currentMetadataPtr = m_currentVecMetadataPtr[m_currentMetadata];
            if (m_currentMetadataPtr->RawData.Size > 0)
                return true;
        } else {
            m_currentMetadata = 0;
            m_currentSegment++;
            m_currentMetadataPtr = shared_ptr<TDMS::Metadata>();
            m_currentVecMetadataPtr = vector<shared_ptr<TDMS::Metadata>>();
        }
    } while (true);
}

auto CReaderController::isOpen() -> OpenResult {
    return m_result;
}

auto CReaderController::checkFile() -> OpenResult {
    m_channel1Present = false;
    m_channel2Present = false;
    m_channel1Size = 0;
    m_channel2Size = 0;
    if (m_genData) {
        return checkMemory();
    }
    if (m_fileType.value == CStreamSettings::DataFormat::TDMS) {
        return checkTDMSFile();
    }
    if (m_fileType.value == CStreamSettings::DataFormat::WAV) {
        return checkWavFile();
    }
    return OpenResult::OR_CLOSE;
}

auto CReaderController::checkMemory() -> OpenResult {
    m_channel1Present = m_genData->ch[0] != nullptr;
    m_channel2Present = m_genData->ch[1] != nullptr;
    m_channel1Size = m_genData->size[0];
    m_channel2Size = m_genData->size[0];
    if (m_channel1Present || m_channel2Present) {
        return OpenResult::OR_OK;
    }
    return OpenResult::OR_MISSING_CHANNELS;
}

auto CReaderController::checkWavFile() -> OpenResult {
    if (m_wavReader) {
        auto header = m_wavReader->getHeader();

        int channels = header.NumOfChan;
        int dataBitSize = header.bitsPerSample;
        if (!(dataBitSize == 16 || dataBitSize == 8))
            return OpenResult::OR_WRONG_DATA_TYPE;
        if (channels == 1 || channels == 2) {
            m_channel1Present = true;
        }
        if (channels == 2) {
            m_channel2Present = true;
        }

        auto dataSize = m_wavReader->getDataSize();
        if (m_channel1Present && m_channel2Present) {
            if (dataSize % 4) {
                return OpenResult::OR_DATA_NOT_EQUAL;
            }
            m_channel1Size = dataSize / 2;
            m_channel2Size = dataSize / 2;
        } else {
            if (dataSize % 2) {
                return OpenResult::OR_WRONG_DATA_TYPE;
            }
            if (m_channel1Present) {
                m_channel1Size = dataSize;
            }
            if (m_channel2Present) {
                m_channel2Size = dataSize;
            }
        }

        if (m_channel1Present || m_channel2Present) {
            return OpenResult::OR_OK;
        } else {
            return OpenResult::OR_MISSING_CHANNELS;
        }
    }
    return OpenResult::OR_CLOSE;
}

auto CReaderController::checkTDMSFile() -> OpenResult {
    bool channel1 = false;
    bool channel2 = false;
    size_t channel1DataSize = 0;
    size_t channel2DataSize = 0;
    bool channel1WrongType = false;
    bool channel2WrongType = false;

    if (m_tdmsFile) {
        for (auto& seg : m_tdmsSegments) {
            auto meta = m_tdmsFile->GetMetadata(seg);
            for (auto& m : meta) {
                if (m->PathStr == "/'Group'/'ch1'") {
                    channel1 = true;
                    channel1DataSize += m->RawData.Size;
                    auto type = m->RawData.DataType.GetDataType();
                    switch (type) {
                        case TDMS::TDMSType::Integer8:
                        case TDMS::TDMSType::UnsignedInteger8:
                        case TDMS::TDMSType::Integer16:
                        case TDMS::TDMSType::UnsignedInteger16:
                            break;
                        default:
                            channel1WrongType = true;
                    }
                }

                if (m->PathStr == "/'Group'/'ch2'") {
                    channel2 = true;
                    channel2DataSize += m->RawData.Size;
                    auto type = m->RawData.DataType.GetDataType();
                    switch (type) {
                        case TDMS::TDMSType::Integer8:
                        case TDMS::TDMSType::UnsignedInteger8:
                        case TDMS::TDMSType::Integer16:
                        case TDMS::TDMSType::UnsignedInteger16:
                            break;
                        default:
                            channel1WrongType = true;
                    }
                }
            }
        }
        if (channel1WrongType || channel2WrongType)
            return OpenResult::OR_WRONG_DATA_TYPE;
        if (channel1 || channel2) {
            if (channel1DataSize != 0 && channel2DataSize != 0 && channel1DataSize != channel2DataSize) {
                return OpenResult::OR_DATA_NOT_EQUAL;
            }
            m_channel1Present = channel1;
            m_channel2Present = channel2;
            m_channel1Size = channel1DataSize;
            m_channel2Size = channel2DataSize;
            return OpenResult::OR_OK;
        } else {
            return OpenResult::OR_MISSING_CHANNELS;
        }
    }
    return OpenResult::OR_CLOSE;
}
