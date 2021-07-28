#include <iomanip>
#include <iostream>
#include <string>

#include "common/stream_settings.h"

int main(int argc, char* argv[])
{
    CStreamSettings settings;
    settings.setAttenuator(CStreamSettings::A_1_20);
    settings.setCalibration(false);
    settings.setChannels(CStreamSettings::CH2);
    settings.setDecimation(12);
    settings.setFormat(CStreamSettings::TDMS);
    settings.setType(CStreamSettings::VOLT);
    settings.setProtocol(CStreamSettings::TCP);
    settings.setResolution(CStreamSettings::BIT_16);
    settings.setSamples(-1);
    settings.setAC_DC(CStreamSettings::AC);
    auto ret = settings.writeToFile("./json.conf");
    std::cout << "Write state " << ret << "\n";
    ret = settings.readFromFile("./json.conf");
    std::cout << "Read state " << ret << "\n";
    std::cout << "DONE \n";

    return 0;
}


