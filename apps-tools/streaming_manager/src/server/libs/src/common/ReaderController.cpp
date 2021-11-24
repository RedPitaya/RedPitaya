#include "ReaderController.h"

CReaderController::CReaderController(CStreamSettings::DataFormat _fileType, std::string _filePath,CStreamSettings::DACRepeat _repeat,uint32_t _rep_count):
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
    m_currentMetadataPtr()
{
    if (m_fileType == CStreamSettings::DataFormat::WAV){
        openWav();
    }

    if (m_fileType == CStreamSettings::DataFormat::TDMS){
        openTDMS();
    }
}

CReaderController::~CReaderController(){
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
            return true;
        }
    } catch (const std::bad_alloc& e) {
        std::cout << "[CReaderController]: Error Allocation failed: " << e.what() << '\n';
    }
    return false;
}

auto CReaderController::openTDMS() -> bool{
    try{
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
            m_currentSegment = 0;
            m_currentMetadata = -1;
            moveNextMetadata();
            return true;
        }
    } catch (const std::bad_alloc& e) {
        std::cout << "[CReaderController]: Error Allocation failed: " << e.what() << '\n';
    }
    return false;
}


CReaderController::Ptr CReaderController::Create(CStreamSettings::DataFormat _fileType, std::string _filePath,CStreamSettings::DACRepeat _repeat,uint32_t _rep_count){
    return std::make_shared<CReaderController>(_fileType, _filePath, _repeat,_rep_count);
}

auto CReaderController::getBuffer(uint8_t **ch1,size_t *size_ch1, uint8_t **ch2,size_t *size_ch2) -> bool{
    if (!m_wavReader) return false;
    if (!m_tdmsFile) return false;
    
    if (m_wavReader && m_fileType == CStreamSettings::DataFormat::WAV){
        if (m_wavReader->getBuffers(ch1,size_ch1,ch2,size_ch2)){
            if (*size_ch1 == 0 && *size_ch2 == 0){
                return false;
            }
            return true;
        }
    }
    
    if (m_tdmsFile && m_fileType == CStreamSettings::DataFormat::TDMS){
        if (!m_currentMetadataPtr) return false;
        // if (m_currentMetadataPtr->RawData.DataType == TDMS::TDMSType::Integer16 
        //     || m_currentMetadataPtr->RawData.DataType == TDMS::TDMSType::UnsignedInteger16) {
        //         // TODO надо сделать чтение
        // } else {
        //     return false;
        // }

        // if (m_currentMetadata >= m_currentMetadataPtr.size()) {

        // }

        // if (m_currentSegment >= m_tdmsSegments.size()) return;

        // auto segment = m_tdmsSegments[m_currentSegment];
        //  for(auto &seg : m2){
        // auto meta = outFile.GetMetadata(seg);
        // outFile.Print(meta,true,100);
        // getchar();
    
    }

    return false;
}

auto CReaderController::moveNextMetadata() -> bool{
    do{
        if (m_currentVecMetadataPtr.empty()){
            if (m_currentSegment >= m_tdmsSegments.size()) return false;
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
            m_currentVecMetadataPtr = vector<shared_ptr<TDMS::Metadata>>();
        }
    }
    while(true);
}
