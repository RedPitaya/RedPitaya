#include "wav_reader.h"
#include <cstring>
#include <iostream>

using namespace std;

CWaveReader::CWaveReader() {
    m_dataSize = 0;
    std::memset(&m_header, 0, sizeof(m_header));
}

CWaveReader::~CWaveReader() {
    if (m_read_fs.is_open())
        m_read_fs.close();
}

auto CWaveReader::openFile(string fileName) -> bool {
    if (m_read_fs.is_open())
        m_read_fs.close();
    m_read_fs.open(fileName, ios::binary | std::ifstream::in);
    if (m_read_fs.fail()) {
        cout << "File " << fileName << " not exist" << std::endl;
        return false;
    }
    m_read_fs.seekg(0, m_read_fs.end);
    uint64_t length = m_read_fs.tellg();
    m_read_fs.seekg(0, m_read_fs.beg);
    m_dataSize = length - sizeof(WavHeader_t);
    m_read_fs.read((char*)&m_header, sizeof(WavHeader_t));
    if (m_read_fs) {
        return true;
    } else {
        m_read_fs.close();
        return false;
    }
}

auto CWaveReader::getHeader() -> WavHeader_t {
    return m_header;
}

auto CWaveReader::getBuffers(uint8_t** ch1, size_t* size_ch1, uint8_t** ch2, size_t* size_ch2, uint8_t* bits) -> bool {
    auto delFunc = [=]() {
        if (*size_ch1 == 0) {
            if (*ch1)
                delete[] *ch1;
            *ch1 = nullptr;
        }
        if (*size_ch2 == 0) {
            if (*ch2)
                delete[] *ch2;
            *ch2 = nullptr;
        }
    };
    *size_ch1 = 0;
    *size_ch2 = 0;
    if (strncmp((const char*)m_header.RIFF, "RIFF", 4) == 0) {
        if (m_header.AudioFormat != 1)
            return false;
        int channels = m_header.NumOfChan;
        int dataBitSize = m_header.bitsPerSample;
        if (!(dataBitSize == 16 || dataBitSize == 8))
            return false;
        *bits = dataBitSize;
        if (channels == 1 || channels == 2) {
            *ch1 = new uint8_t[32 * 1024];
        }
        if (channels == 2) {
            *ch2 = new uint8_t[32 * 1024];
        }
        int size = 0;
        if (dataBitSize == 16)
            size = 16 * 1024;
        if (dataBitSize == 8)
            size = 32 * 1024;

        for (int i = 0; i < size; i++) {
            uint16_t value = 0;
            m_read_fs.read((char*)&value, dataBitSize / 8);
            if (!m_read_fs) {
                delFunc();
                return true;
            }
            if (*ch1) {
                if (dataBitSize == 8) {
                    ((uint8_t*)*ch1)[i] = (uint8_t)value;
                    *size_ch1 += 1;
                }

                if (dataBitSize == 16) {
                    ((uint16_t*)*ch1)[i] = value;
                    *size_ch1 += 2;
                }
            }

            if (channels == 2) {
                value = 0;
                m_read_fs.read((char*)&value, dataBitSize / 8);
                if (!m_read_fs) {
                    delFunc();
                    return true;
                }
                if (*ch2) {
                    if (dataBitSize == 8) {
                        ((uint8_t*)*ch2)[i] = (uint8_t)value;
                        *size_ch2 += 1;
                    }

                    if (dataBitSize == 16) {
                        ((uint16_t*)*ch2)[i] = value;
                        *size_ch2 += 2;
                    }
                }
            }
        }
        delFunc();
        return true;
    }
    return false;
}

auto CWaveReader::getDataSize() -> uint64_t {
    return m_dataSize;
}
