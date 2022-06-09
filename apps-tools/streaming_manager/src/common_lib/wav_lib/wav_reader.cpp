#include <cassert>
#include <sstream>
#include <cstring>
#include "wav_reader.h"

using namespace std;

CWaveReader::CWaveReader(){
    std::memset(&m_header,0,sizeof (m_header));
}

auto CWaveReader::openFile(string fileName) -> bool{
    if (m_read_fs.is_open())
        m_read_fs.close();
    m_read_fs.open(fileName, ios::binary | std::ifstream::in );
    if (m_read_fs.fail()) {
        cout << "File " << fileName << " not exist" << std::endl;
        return false;
    }
    m_read_fs.read((char*)&m_header, sizeof(WavHeader_t));
    if (m_read_fs){
        return true;
    }else{
        m_read_fs.close();
        return false;
    }
}

auto CWaveReader::getHeader() -> WavHeader_t{
    return m_header;
}

auto CWaveReader::getBuffers(uint8_t **ch1,size_t *size_ch1, uint8_t **ch2,size_t *size_ch2) -> bool{
    *size_ch1 = 0;
    *size_ch2 = 0;
    if (strncmp((const char*)m_header.RIFF,"RIFF",4) == 0){
        if (m_header.AudioFormat != 1) return false;
        int channels = m_header.NumOfChan;
        int dataBitSize = m_header.bitsPerSample;
        if (dataBitSize != 16) return false;
        if (channels == 1 || channels == 2){
            *ch1 = new uint8_t[32 * 1024];
            memset(*ch1,0,32 * 1024);
        }
        if (channels == 2){
            *ch2 = new uint8_t[32 * 1024];
            memset(*ch2,0,32 * 1024);
        }
        int size = 16 * 1024;
        for(int i = 0 ; i < size; i++){
            uint16_t  value = 0;
            m_read_fs.read((char*)&value,sizeof(value));
            if (!m_read_fs){
                return true;
            }
            if (*ch1){
                ((uint16_t*)*ch1)[i] = value;
                *size_ch1 += 2;
            }

            if (channels == 2){
                value = 0;
                m_read_fs.read((char*)&value,sizeof(value));
                if (!m_read_fs){
                    return true;
                }
                if (*ch2){
                    ((uint16_t*)*ch2)[i] = value;
                    *size_ch2 += 2;
                }
            }
        }
        return true;
    }
    return false;
}

auto CWaveReader::getDataSize() -> uint64_t {
    return m_dataSize;
}