#include <iomanip>
#include <iostream>
#include <string>

#include "common/stream_settings.h"

int main(int argc, char* argv[])
{
    CStreamSettings settings;
    settings.setHost("127.0.0.1");
    settings.setPort("1234");
    settings.setAttenuator(CStreamSettings::A_1_1);
    settings.setCalibration(false);
    settings.setChannels(CStreamSettings::CH2);
    settings.setDecimation(12);
    settings.setFormat(CStreamSettings::WAV);
    settings.setType(CStreamSettings::RAW);
    settings.setProtocol(CStreamSettings::TCP);
    settings.setResolution(CStreamSettings::BIT_16);
    settings.setSamples(-1);
    auto ret = settings.writeToFile("/home/swat/json.conf");
    std::cout << "Write state " << ret << "\n";
    settings.setHost("127.0.0.2");
    std::cout << "Host " << settings.getHost().c_str() << "\n";
    ret = settings.readFromFile("/home/swat/json.conf");
    std::cout << "Read state " << ret << "\n";
    std::cout << "Host " << settings.getHost().c_str() << "\n";
    std::cout << "DONE \n";

    return 0;
}


