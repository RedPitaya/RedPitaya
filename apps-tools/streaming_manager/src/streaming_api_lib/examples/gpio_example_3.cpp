// GPIO example 3: the board records its GPIO input into a file of its own SD card (the file
// mode): a VCD of one million samples in /home/redpitaya/streaming_files/gpio. No data comes over
// the network; the client hears when the board ends the file.
//
// Usage: gpio_example_3 [IP]    (without IP the first board found on the network)

#include <iostream>

#include "callbacks.h"
#include "config_streaming.h"
#include "gpio_streaming.h"

class FileHandler : public GPIOCallback {
   public:
    void inputFileStopped(GPIOStreamClient*, std::string host, GPIOFileStop reason) override {
        static const char* names[] = {"the sample limit is written", "the SD card is full", "the file cannot be written", "empty", "missing"};
        std::cout << host << " ended its GPIO file: " << names[reason] << std::endl;
    }
};

int main(int argc, char* argv[]) {
    auto conf = std::make_shared<ConfigStreamClient>();
    if (!(argc > 1 ? conf->connect({argv[1]}) : conf->connect())) {
        std::cerr << "No board found" << std::endl;
        return 1;
    }
    const std::string host = conf->getHosts().front();

    // the file mode of the input: the format and the samples of the file
    conf->sendConfig(host, "gpio_decimation", "1");
    conf->sendConfig(host, "gpio_rle", "ON");
    conf->sendConfig(host, "gpio_in_pass_mode", "FILE");
    conf->sendConfig(host, "gpio_in_format_sd", "VCD");
    conf->sendConfig(host, "gpio_in_samples_limit_sd", "1000000");

    auto gpio = std::make_shared<GPIOStreamClient>(conf);
    gpio->setCallback(std::make_shared<FileHandler>());
    if (!gpio->startInput(host)) {
        std::cerr << "The GPIO input did not start" << std::endl;
        return 1;
    }
    std::cout << host << (gpio->inputToFile() ? " writes its GPIO input into a file" : " streams its GPIO input to the network") << std::endl;
    gpio->wait();  // until the board ends the file

    conf->sendConfig(host, "gpio_in_pass_mode", "NET");  // back to the network mode
    std::cout << "The file is in /home/redpitaya/streaming_files/gpio on the board" << std::endl;
    return 0;
}
