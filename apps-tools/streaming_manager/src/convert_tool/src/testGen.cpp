#include <math.h>
#include <vector>
#include "common_lib/logger_lib/file_logger.h"
#include "common_lib/wav_lib/wav_writer.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void createTestFiels() {
    auto func = [](size_t size, std::string suffix, std::iostream* s) {
        std::string file_name = "td_sin_" + std::to_string(size) + "_" + suffix;
        std::ofstream file(file_name, std::ios::out | std::ios::binary | std::ios::trunc);
        if (file.is_open()) {
            file << s->rdbuf();
            file.close();
        }
    };

    std::vector<size_t> size_list = {1024, 2048, 3072, 6144, 12288, 24576, 49152, 98304, 196608, 393216, 786432, 1572864, 3145728, 6291456, 12582912, 25165824};

    for (auto size : size_list) {
        std::vector<int8_t> data8;
        std::vector<int16_t> data16;
        data8.resize(size);
        data16.resize(size);

        for (int unsigned i = 0; i < size; i++) {
            data8[i] = (sin(2 * M_PI * (float)i / (float)size)) * std::numeric_limits<int8_t>::max();
            data16[i] = (sin(2 * M_PI * (float)i / (float)size)) * std::numeric_limits<int16_t>::max();
        }
        SBuffPass buff8, buff16;
        auto b8 = net_lib::createBuffer((const char*)data8.data(), data8.size());
        auto b16 = net_lib::createBuffer((const char*)data16.data(), data16.size() * 2);

        buff8.buffer = b8;
        buff8.bufferLen = data8.size();
        buff8.samplesCount = data8.size();
        buff8.adcSpeed = 44100;
        buff8.bitsBySample = 8;

        buff16.buffer = b16;
        buff16.bufferLen = data16.size();
        buff16.samplesCount = data16.size();
        buff16.adcSpeed = 44100;
        buff16.bitsBySample = 16;

        std::map<DataLib::EDataBuffersPackChannel, SBuffPass> pack_ch1_8;
        std::map<DataLib::EDataBuffersPackChannel, SBuffPass> pack_ch2_8;
        std::map<DataLib::EDataBuffersPackChannel, SBuffPass> pack_ch1_16;
        std::map<DataLib::EDataBuffersPackChannel, SBuffPass> pack_ch2_16;
        pack_ch1_8[DataLib::CH1] = buff8;
        pack_ch2_8[DataLib::CH1] = buff8;
        pack_ch2_8[DataLib::CH2] = buff8;
        pack_ch1_16[DataLib::CH1] = buff16;
        pack_ch2_16[DataLib::CH1] = buff16;
        pack_ch2_16[DataLib::CH2] = buff16;

        CWaveWriter* wav = new CWaveWriter();
        wav->resetHeaderInit();
        auto str_ch1_8 = wav->BuildWAVStream(pack_ch1_8);
        wav->resetHeaderInit();
        auto str_ch2_8 = wav->BuildWAVStream(pack_ch2_8);
        wav->resetHeaderInit();
        auto str_ch1_16 = wav->BuildWAVStream(pack_ch1_16);
        wav->resetHeaderInit();
        auto str_ch2_16 = wav->BuildWAVStream(pack_ch2_16);
        delete wav;

        func(size, "8bit_ch1.wav", str_ch1_8);
        func(size, "8bit_ch2.wav", str_ch2_8);
        func(size, "16bit_ch1.wav", str_ch1_16);
        func(size, "16bit_ch2.wav", str_ch2_16);

        str_ch1_8 = buildTDMSStream(pack_ch1_8);
        str_ch2_8 = buildTDMSStream(pack_ch2_8);
        str_ch1_16 = buildTDMSStream(pack_ch1_16);
        str_ch2_16 = buildTDMSStream(pack_ch2_16);

        func(size, "8bit_ch1.tdms", str_ch1_8);
        func(size, "8bit_ch2.tdms", str_ch2_8);
        func(size, "16bit_ch1.tdms", str_ch1_16);
        func(size, "16bit_ch2.tdms", str_ch2_16);
    }
}
