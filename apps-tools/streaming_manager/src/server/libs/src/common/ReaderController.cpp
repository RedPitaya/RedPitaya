#include "ReaderController.h"

constexpr size_t g_max_buff = 32 * 1024;

CReaderController::CReaderController(CStreamSettings::DataFormat _fileType, std::string _filePath,CStreamSettings::DACRepeat _repeat,int32_t _rep_count,uint64_t memoryCacheSize):
    m_fileType(_fileType),
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
    m_memoryCacheSize(memoryCacheSize),
    m_channel1Size(0),
    m_channel2Size(0)
{
    m_useMemoryCache = false;
    if (m_repeat ==CStreamSettings::DACRepeat::DAC_REP_ON && m_rep_count == 0){
        m_rep_count = 1;
    }
    if (m_fileType == CStreamSettings::DataFormat::WAV){
        openWav();
    }

    if (m_fileType == CStreamSettings::DataFormat::TDMS){
        openTDMS();
    }
    m_result = checkFile();
    resetReadFromBuffer();
    m_useMemoryCache = memoryCacheSize >= m_channel1Size || memoryCacheSize >= m_channel2Size;
}

void CReaderController::TemperaryBuffer::deleteBuffer(){
    if (buffer) {
        delete[] buffer;
        buffer = nullptr;
    }
    size = 0;
    current_pos = 0;
}

CReaderController::~CReaderController(){
    m_tempBuffer[0].deleteBuffer();
    m_tempBuffer[1].deleteBuffer();
    if (m_wavReader) delete m_wavReader;
    if (m_tdmsFile) delete m_tdmsFile;
}

auto CReaderController::openWav() -> bool{
    try{
        if (m_fileType == CStreamSettings::DataFormat::WAV){
            if (m_wavReader) delete m_wavReader;
            m_wavReader = new CWaveReader();        
            if (!m_wavReader->openFile(m_filePath)){
                std::cerr << "[CReaderController]: Error open wav file("<< m_filePath << ") " << std::endl;
                delete m_wavReader;
                m_wavReader = nullptr;
                return false;
            }
            m_result = OpenResult::OR_OK;
            return true;
        }
    } catch (const std::bad_alloc& e) {
        std::cout << "[CReaderController]: Error Allocation failed: " << e.what() << '\n';
    }
    return false;
}

auto CReaderController::openTDMS() -> bool{
    try{
        m_channel1Present = false;
        m_channel2Present = false;
        if (m_fileType == CStreamSettings::DataFormat::TDMS){
            if (m_tdmsFile) delete m_tdmsFile;
            m_tdmsFile = new TDMS::File();
            m_tdmsSegments = m_tdmsFile->ReadFileWithoutClose(m_filePath);
            if (m_tdmsSegments.empty()){
                std::cerr << "[CReaderController]: Error open tdms file("<< m_filePath << ") " << std::endl;
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
        std::cout << "[CReaderController]: Error Allocation failed: " << e.what() << '\n';
    }
    return false;
}


auto CReaderController::resetReadFromBuffer() -> bool{
    if (m_useMemoryCache){
        m_tempBuffer[0].current_pos = 0;
        m_tempBuffer[1].current_pos = 0;
        if (m_fileType == CStreamSettings::DataFormat::TDMS){
            m_currentSegment = 0;
            m_currentMetadata = -1;
            if(m_tdmsFile) m_tdmsFile->clearPrevMetadata();
            moveNextMetadata();
        }
        return true;
    }
    else{
        m_tempBuffer[0].deleteBuffer();
        m_tempBuffer[1].deleteBuffer();
        m_checkEmptyFile = true;

        if (m_fileType == CStreamSettings::DataFormat::WAV){
            return openWav();
        }

        if (m_fileType == CStreamSettings::DataFormat::TDMS){
            m_currentSegment = 0;
            m_currentMetadata = -1;
            if(m_tdmsFile) m_tdmsFile->clearPrevMetadata();
            moveNextMetadata();
            return true;
        }
    }
    return false;
}



CReaderController::Ptr CReaderController::Create(CStreamSettings::DataFormat _fileType, std::string _filePath,CStreamSettings::DACRepeat _repeat,int32_t _rep_count,uint64_t memoryCacheSize){
    return std::make_shared<CReaderController>(_fileType, _filePath, _repeat,_rep_count,memoryCacheSize);
}

auto CReaderController::writeFromTemp(uint8_t **buff,size_t max_size,size_t *write_pos,CReaderController::TemperaryBuffer *temp_buf) -> void{
    size_t sizeForWrite = temp_buf->size - temp_buf->current_pos;
    size_t aviableSize  = max_size - *write_pos;
    size_t sizeWrite = (sizeForWrite > aviableSize ? aviableSize : sizeForWrite);
    memcpy_neon((&(**buff) + *write_pos), (&(*temp_buf->buffer) + temp_buf->current_pos) , sizeWrite);
    *write_pos += sizeWrite;
    temp_buf->current_pos += sizeWrite;
}


auto CReaderController::getBufferPrepared(uint8_t **ch1,size_t *size_ch1, uint8_t **ch2,size_t *size_ch2) -> BufferResult{
    auto fillZero = [](uint8_t **ch,size_t *size){
        if (*ch){
            if (0 != *size){
                memset((&(**ch) + *size),0,g_max_buff - *size);
                *size = g_max_buff;
            }else{
                delete [] *ch;
                *ch = nullptr;
                *size = 0;
            }
        }
    };
    *size_ch1 = 0;
    *size_ch2 = 0;
    if (m_channel1Present){
        *ch1 = new uint8_t[g_max_buff];
    }
    if (m_channel2Present){
        *ch2 = new uint8_t[g_max_buff];
    }
    while(1) {
        if (*ch1){
            if (m_tempBuffer[0].size > 0 && !m_tempBuffer[0].isEnded()){
                writeFromTemp(ch1,g_max_buff,size_ch1,&m_tempBuffer[0]);
            }
        }

        if (*ch2){
            if (m_tempBuffer[1].size > 0 && !m_tempBuffer[1].isEnded()){
                writeFromTemp(ch2,g_max_buff,size_ch2,&m_tempBuffer[1]);
            }
        }

        if (*ch1 && *ch2){
            if (*size_ch1 == g_max_buff && *size_ch2 == g_max_buff){
                return BR_OK;
            }else{
                if (m_fileType == CStreamSettings::WAV){
                    if (*size_ch1 != *size_ch2){
                        return BR_BROKEN;
                    }
                }
            }
        }else if(*ch1){
            if (*size_ch1 == g_max_buff)
                return BR_OK;
        }else if(*ch2){
            if (*size_ch2 == g_max_buff)
                return BR_OK;
        }

        if (m_tempBuffer[0].isEnded() || m_tempBuffer[1].isEnded()){
            uint8_t *ch1_read = nullptr,*ch2_read = nullptr;
            size_t   size1_read = 0, size2_read = 0;
            auto res = false;
            if(m_useMemoryCache){
                if ((m_tempBuffer[0].size == 0 && m_channel1Present) || (m_tempBuffer[1].size == 0 && m_channel2Present)){
                    getBufferFull(&ch1_read,&size1_read,&ch2_read,&size2_read);
                    m_tempBuffer[0].buffer = ch1_read;
                    m_tempBuffer[0].size = size1_read;
                    m_tempBuffer[1].buffer = ch2_read;
                    m_tempBuffer[1].size = size2_read;

                    if (m_checkEmptyFile){
                        if (size1_read || size2_read){
                            m_checkEmptyFile = false;
                        }else{
                            return BR_EMPTY;
                        }
                    }

                    continue;
                }
                res = false;
            }else{
                res = getBuffer(&ch1_read,&size1_read,&ch2_read,&size2_read);
            }

            if (m_checkEmptyFile){
                if (size1_read || size2_read){
                    m_checkEmptyFile = false;
                }else{
                    return BR_EMPTY;
                }
            }

            if (res){ // if don't use memory cache
                if (ch1_read){
                    m_tempBuffer[0].deleteBuffer();
                    m_tempBuffer[0].buffer = ch1_read;
                    m_tempBuffer[0].size = size1_read;
                }

                if (ch2_read){
                    m_tempBuffer[1].deleteBuffer();
                    m_tempBuffer[1].buffer = ch2_read;
                    m_tempBuffer[1].size = size2_read;
                }
            }else{
                if (m_repeat != CStreamSettings::DACRepeat::DAC_REP_OFF){
                    if (resetReadFromBuffer()){
                        if (m_repeat != CStreamSettings::DACRepeat::DAC_REP_INF){                           
                            m_rep_count--;
                            if (m_rep_count <= 0){
                                fillZero(ch1,size_ch1);
                                fillZero(ch2,size_ch2);
                                return BR_ENDED;
                            }
                        }
                    }else{
                        return BR_BROKEN;
                    }
                }else{
                    fillZero(ch1,size_ch1);
                    fillZero(ch2,size_ch2);
                    return BR_ENDED;
                }
            }
        }
    }
}

auto CReaderController::getBuffer(uint8_t **ch1,size_t *size_ch1, uint8_t **ch2,size_t *size_ch2) -> bool{
    if (m_fileType == CStreamSettings::DataFormat::WAV){
        return getBufferWav(ch1,size_ch1,ch2,size_ch2);
    }
    if (m_fileType == CStreamSettings::DataFormat::TDMS){
        return getBufferTdms(ch1,size_ch1,ch2,size_ch2);
    }
    return false;
}


auto CReaderController::getBufferWav(uint8_t **ch1,size_t *size_ch1, uint8_t **ch2,size_t *size_ch2) -> bool{
    try{
        if (m_wavReader && m_fileType == CStreamSettings::DataFormat::WAV){
            if (m_wavReader->getBuffers(ch1,size_ch1,ch2,size_ch2)){
                if (*size_ch1 == 0 && *size_ch2 == 0){
                    return false;
                }
                return true;
            }
        }
    }catch (const std::bad_alloc& e) {
        if (*ch1) delete[] *ch1;
        *ch1 = nullptr;
        if (*ch2) delete[] *ch2;
        *ch2 = nullptr;
        *size_ch1 = 0;
        *size_ch2 = 0;
        std::cout << "[CReaderController]: Error Allocation failed: " << e.what() << '\n';
    }
    return false;
}

auto CReaderController::getBufferFull(uint8_t **ch1,size_t *size_ch1, uint8_t **ch2,size_t *size_ch2) -> void{
    try{
        if (m_channel1Present){
            *ch1 = new uint8_t[m_channel1Size];
            *size_ch1 = m_channel1Size;
        }
        if (m_channel2Present){
            *ch2 = new uint8_t[m_channel2Size];
            *size_ch2 = m_channel2Size;
        }
        uint8_t *ch1_read = nullptr;
        uint8_t *ch2_read = nullptr;
        size_t size1_read = 0;
        size_t size2_read = 0;
        size_t lastPos1 = 0;
        size_t lastPos2 = 0;

        while(getBuffer(&ch1_read,&size1_read,&ch2_read,&size2_read)){
            if (ch1_read && size1_read > 0){
                if (lastPos1 + size1_read > m_channel1Size){
                    size1_read = m_channel1Size - lastPos1;
                }
                memcpy_neon((&(**ch1) + lastPos1), ch1_read , size1_read);
                lastPos1 += size1_read;
            }
            if (ch2_read && size2_read > 0){
                if (lastPos2 + size2_read > m_channel2Size){
                    size2_read = m_channel2Size - lastPos2;
                }
                memcpy_neon((&(**ch2) + lastPos2), ch2_read , size2_read);
                lastPos2 += size2_read;
            }
            if (ch1_read){
                delete[] ch1_read;
                ch1_read = nullptr;
                size1_read = 0;
            }
            if (ch2_read){
                delete[] ch2_read;
                ch2_read = nullptr;
                size2_read = 0;
            }
        }
    } catch (const std::bad_alloc& e) {
        if (*ch1) delete[] *ch1;
        *ch1 = nullptr;
        if (*ch2) delete[] *ch2;
        *ch2 = nullptr;
        *size_ch1 = 0;
        *size_ch2 = 0;
        std::cout << "[CReaderController]: Error Allocation failed: " << e.what() << '\n';
    }
}


auto CReaderController::getBufferTdms(uint8_t **ch1,size_t *size_ch1, uint8_t **ch2,size_t *size_ch2) -> bool{
    if (m_tdmsFile && m_fileType == CStreamSettings::DataFormat::TDMS){

        if (m_channel1Present == false && m_channel2Present == false){
            return false;
        }

        size_t size1 = 0;
        size_t size2 = 0;

        if (!m_currentMetadataPtr) {
            *size_ch1 = size1;
            *size_ch2 = size2;
            return false;
        }

        if (m_currentMetadataPtr->RawData.Size > 0) {
            if (m_currentMetadataPtr->PathStr ==  "/'Group'/'ch1'"){
                size_t sizeCh1 = 0;
                for (auto &m : m_currentMetadataPtr->RawData.DataType.GetRawVector()){
                    sizeCh1 += m->size;
                }
                *ch1 = new uint8_t[sizeCh1];
                *size_ch1 = sizeCh1;
                *size_ch2 = 0;
                uint8_t *buffer = *ch1;
                size_t pos = 0;
                for (auto &m : m_currentMetadataPtr->RawData.DataType.GetRawVector()){
                    memcpy_neon((&(*buffer) + pos), m->data, m->size);
                    pos += m->size;
                }
                moveNextMetadata();
                return true;
            }

            if (m_currentMetadataPtr->PathStr ==  "/'Group'/'ch2'"){
                size_t sizeCh2 = 0;
                for (auto &m : m_currentMetadataPtr->RawData.DataType.GetRawVector()){
                    sizeCh2 += m->size;
                }
                *ch2 = new uint8_t[sizeCh2];
                *size_ch1 = 0;
                *size_ch2 = sizeCh2;
                uint8_t *buffer = *ch2;
                size_t pos = 0;
                for (auto &m : m_currentMetadataPtr->RawData.DataType.GetRawVector()){
                    memcpy_neon((&(*buffer) + pos), m->data, m->size);
                    pos += m->size;
                }
                moveNextMetadata();
                return true;
            }

            moveNextMetadata();
            *size_ch1 = size1;
            *size_ch2 = size2;
            return true;
        }else{
            moveNextMetadata();
        }
    }
    return false;
}

auto CReaderController::moveNextMetadata() -> bool{
    do{
        if (m_currentVecMetadataPtr.empty()){
            if (m_currentSegment >= m_tdmsSegments.size()) {
                return false;
            }
            m_currentVecMetadataPtr = m_tdmsFile->GetMetadata(m_tdmsSegments[m_currentSegment]);
        }
        m_currentMetadata++;
        if (m_currentMetadata < m_currentVecMetadataPtr.size()){
            m_currentMetadataPtr = m_currentVecMetadataPtr[m_currentMetadata];
            if (m_currentMetadataPtr->RawData.Size > 0) 
                return true;
        }else{
            m_currentMetadata = 0;
            m_currentSegment++;
            m_currentMetadataPtr = shared_ptr<TDMS::Metadata>();
            m_currentVecMetadataPtr = vector<shared_ptr<TDMS::Metadata>>();
        }
    }
    while(true);
}

auto CReaderController::isOpen() -> OpenResult {
    return m_result;
}

auto CReaderController::checkFile() -> OpenResult{
    m_channel1Present = false;
    m_channel2Present = false;
    m_channel1Size = 0;
    m_channel2Size = 0;
    if (m_fileType == CStreamSettings::DataFormat::TDMS){
        return checkTDMSFile();
    }
    if (m_fileType == CStreamSettings::DataFormat::WAV){
        return checkWavFile();
    }
    return OpenResult::OR_CLOSE;
}

auto CReaderController::checkWavFile() -> OpenResult{
    if (m_wavReader){
        auto header = m_wavReader->getHeader();

        int channels = header.NumOfChan;
        int dataBitSize = header.bitsPerSample;
        if (dataBitSize != 16) return OpenResult::OR_WRONG_DATA_TYPE;
        if (channels == 1 || channels == 2){
            m_channel1Present = true;
        }
        if (channels == 2){
            m_channel2Present = true;
        }

        auto dataSize = m_wavReader->getDataSize();
        if (m_channel1Present && m_channel2Present){
            if (dataSize % 4){
                return OpenResult::OR_DATA_NOT_EQUAL;
            }
            m_channel1Size = dataSize / 2;
            m_channel2Size = dataSize / 2;
        }else{
            if (dataSize % 2){
                return OpenResult::OR_WRONG_DATA_TYPE;
            }
            if (m_channel1Present) {
                m_channel1Size = dataSize;
            }
            if (m_channel2Present) {
                m_channel2Size = dataSize;
            }
        }

        if (m_channel1Present || m_channel2Present){
            return OpenResult::OR_OK;
        }else{
            return OpenResult::OR_MISSING_CHANNELS;
        }
    }
    return OpenResult::OR_CLOSE;
}

auto CReaderController::checkTDMSFile() -> OpenResult{
    bool channel1 = false;
    bool channel2 = false;
    size_t channel1DataSize = 0;
    size_t channel2DataSize = 0;
    bool channel1WrongType = false;
    bool channel2WrongType = false;


    if (m_tdmsFile){
        for(auto &seg : m_tdmsSegments){
            auto meta = m_tdmsFile->GetMetadata(seg);
            for(auto &m: meta){
                if (m->PathStr == "/'Group'/'ch1'"){
                    channel1 = true;
                    channel1DataSize += m->RawData.Size;
                    auto type = m->RawData.DataType.GetDataType();
                    switch (type) {
                        case TDMS::TDMSType::Integer16:
                        case TDMS::TDMSType::UnsignedInteger16:
                        break;
                    default:
                        channel1WrongType = true;
                    }
                }

                if (m->PathStr == "/'Group'/'ch2'"){
                    channel2 = true;
                    channel2DataSize += m->RawData.Size;
                    auto type = m->RawData.DataType.GetDataType();
                    switch (type) {
                        case TDMS::TDMSType::Integer16:
                        case TDMS::TDMSType::UnsignedInteger16:
                        break;
                    default:
                        channel1WrongType = true;
                    }
                }
            }
        }
        if (channel1WrongType || channel2WrongType) return OpenResult::OR_WRONG_DATA_TYPE;
        if (channel1 || channel2){
            if (channel1DataSize != 0 && channel2DataSize != 0  && channel1DataSize != channel2DataSize){
                return OpenResult::OR_DATA_NOT_EQUAL;
            }
            m_channel1Present = channel1;
            m_channel2Present = channel2;
            m_channel1Size = channel1DataSize;
            m_channel2Size = channel2DataSize;
            return OpenResult::OR_OK;
        }else{
            return OpenResult::OR_MISSING_CHANNELS;
        }
    }
    return OpenResult::OR_CLOSE;
}
