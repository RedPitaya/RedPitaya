#include "rpsa/common/core/wavWriter.h"


CWaveWriter::CWaveWriter(){
    resetHeaderInit();
    m_endianness = CWaveWriter::Endianness::LittleEndian;
}

// void CWaveWriter::writeStringToFileData (std::vector<uint8_t>& fileData, std::string s)
// {
//     for (int i = 0; i < s.length();i++)
//         fileData.push_back ((uint8_t) s[i]);
// }

void CWaveWriter::resetHeaderInit(){
    m_headerInit = true;
}

std::iostream *CWaveWriter::BuildWAVStream(uint8_t* buffer_ch1,size_t size_ch1,uint8_t* buffer_ch2,size_t size_ch2,unsigned short resolution){

    // IF resolution = 32bit this FLOAT type data
    m_bitDepth = resolution;
    assert(((m_bitDepth % 8 == 0 ) && (m_bitDepth / 8 > 0))  && "Wav bit resolution is invalid");

    if (size_ch1!=0 && size_ch2 != 0)
        assert(size_ch1 == size_ch2);

    // Init variables
    m_numChannels = 1; 
    if (size_ch1 > 0 && size_ch2 > 0){
        m_numChannels = 2;
    }else
    if (size_ch1 == 0 && size_ch2 == 0)
    {
        m_numChannels = 0;
    }

    if (size_ch1!=0)
        m_samplesPerChannel = size_ch1 / (m_bitDepth / 8);
    else
        m_samplesPerChannel = size_ch2 / (m_bitDepth / 8);
    //////////////////

    std::stringstream *memory = new std::stringstream(std::ios_base::in | std::ios_base::out | std::ios_base::binary);
    if (m_headerInit)
    {
        BuildHeader(memory);
        m_headerInit = false;
    }

    

    if (m_bitDepth == 8)
    {
        if ((size_ch1 + size_ch2) > 0){
        uint8_t* cross_buff = new uint8_t[size_ch1 + size_ch2];

        if (size_ch2 > 0 && size_ch1 > 0){
            for (int i = 0; i < m_samplesPerChannel; i++)
            {
                cross_buff[i*2] = buffer_ch1[i];
            }

            for (int i = 0; i < m_samplesPerChannel; i++)
            {
                cross_buff[i*2 + 1] = buffer_ch2[i];                            
            }
        }
        else {
            if (size_ch1 > 0){
                for (int i = 0; i < m_samplesPerChannel; i++) {
                    cross_buff[i] = buffer_ch1[i];
                }
            }

            if (size_ch2 > 0) {
                for (int i = 0; i < m_samplesPerChannel; i++) {
                    cross_buff[i] = buffer_ch2[i];
                }
            }
        }
        memory->write((const char*)cross_buff, size_ch1 + size_ch2);
        delete [] cross_buff;
        }
    }

    if (m_bitDepth == 16)
    {
        int Bufflen = (size_ch1 > 0 ? m_samplesPerChannel : 0) + (size_ch2 > 0 ? m_samplesPerChannel : 0);
        if (Bufflen > 0){
            uint16_t* cross_buff = new uint16_t[Bufflen];


            if (size_ch2 > 0 && size_ch1 > 0){
                for (int i = 0; i < m_samplesPerChannel; i++)
                {
                    cross_buff[i*2] = ((uint16_t*)buffer_ch1)[i];
                }

                for (int i = 0; i < m_samplesPerChannel; i++)
                {
                    cross_buff[i*2 + 1] = ((uint16_t*)buffer_ch2)[i];                            
                }
            }
            else {
                if (size_ch1 > 0){
                    for (int i = 0; i < m_samplesPerChannel; i++)
                    {
                        cross_buff[i] = ((uint16_t *) buffer_ch1)[i];
                    }
                }

                if (size_ch2 > 0) {
                    for (int i = 0; i < m_samplesPerChannel; i++) {
                        cross_buff[i] = ((uint16_t *) buffer_ch2)[i];
                    }
                }
            }
        memory->write((const char*)cross_buff, sizeof(uint16_t)* (Bufflen));
        delete [] cross_buff;
        }
    }

    if (m_bitDepth == 32)
    {
        int Bufflen = (size_ch1 > 0 ? m_samplesPerChannel : 0) + (size_ch2 > 0 ? m_samplesPerChannel : 0);
        if (Bufflen > 0){
            float* cross_buff = new float[Bufflen];


            if (size_ch2 > 0 && size_ch1 > 0){
                for (int i = 0; i < m_samplesPerChannel; i++)
                {
                    cross_buff[i*2] = ((float*)buffer_ch1)[i];
                }

                for (int i = 0; i < m_samplesPerChannel; i++)
                {
                    cross_buff[i*2 + 1] = ((float*)buffer_ch2)[i];                            
                }
            }
            else {
                if (size_ch1 > 0){
                    for (int i = 0; i < m_samplesPerChannel; i++)
                    {
                        cross_buff[i] = ((float *) buffer_ch1)[i];
                    }
                }

                if (size_ch2 > 0) {
                    for (int i = 0; i < m_samplesPerChannel; i++) {
                        cross_buff[i] = ((float *) buffer_ch2)[i];
                    }
                }
            }
        memory->write((const char*)cross_buff, sizeof(float)* (Bufflen));
        delete [] cross_buff;
        }
    }

    return memory;
}

void CWaveWriter::BuildHeader(std::stringstream *memory){

    int sampleRate = 44100;
    int32_t dataChunkSize = m_samplesPerChannel * m_numChannels * (m_bitDepth / 8);
    int16_t data_format = m_bitDepth == 32 ? 0x0003: 0x0001; 
    addStringToFileData(memory,"RIFF");
    
    int32_t fileSizeInBytes = 4 + 24 + 8 + dataChunkSize;
    addInt32ToFileData (memory, fileSizeInBytes);
    addStringToFileData(memory,"WAVE");
    

    // -----------------------------------------------------------
    // FORMAT CHUNK
    addStringToFileData(memory,"fmt ");
    addInt32ToFileData (memory, 16); // format chunk size (16 for PCM)
    addInt16ToFileData (memory, data_format); // audio format = 1
    addInt16ToFileData (memory, (int16_t)m_numChannels); // num channels
    addInt32ToFileData (memory, (int32_t)m_samplesPerChannel); // sample rate
    
    int32_t numBytesPerSecond = (int32_t) ((m_numChannels * sampleRate * m_bitDepth) / 8);
    addInt32ToFileData (memory, numBytesPerSecond);
    
    int16_t numBytesPerBlock = m_numChannels * (m_bitDepth / 8);
    addInt16ToFileData (memory, numBytesPerBlock);
    
    addInt16ToFileData (memory, (int16_t)m_bitDepth);
    
    // -----------------------------------------------------------
    memory->write("data",4);
    addInt32ToFileData (memory, dataChunkSize);
//    std::cout << "BuildHeader: dataChunkSize " << dataChunkSize << "\n";
}


void CWaveWriter::addStringToFileData (std::stringstream *memory, std::string s)
{
    memory->write(s.data(),s.size());
}


void CWaveWriter::addInt32ToFileData (std::stringstream *memory, int32_t i)
{
    char bytes[4];
    
    if (m_endianness == Endianness::LittleEndian)
    {
        bytes[3] = (i >> 24) & 0xFF;
        bytes[2] = (i >> 16) & 0xFF;
        bytes[1] = (i >> 8) & 0xFF;
        bytes[0] = i & 0xFF;
    }
    else
    {
        bytes[0] = (i >> 24) & 0xFF;
        bytes[1] = (i >> 16) & 0xFF;
        bytes[2] = (i >> 8) & 0xFF;
        bytes[3] = i & 0xFF;
    }
    memory->write(bytes,4);
    
}

void CWaveWriter::addInt16ToFileData (std::stringstream *memory, int16_t i)
{
    char bytes[2];
    
    if (m_endianness == Endianness::LittleEndian)
    {
        bytes[1] = (i >> 8) & 0xFF;
        bytes[0] = i & 0xFF;
    }
    else
    {
        bytes[0] = (i >> 8) & 0xFF;
        bytes[1] = i & 0xFF;
    }
    
    memory->write(bytes,2);
}

