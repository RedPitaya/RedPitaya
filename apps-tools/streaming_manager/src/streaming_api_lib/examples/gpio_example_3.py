#!/usr/bin/python3
# GPIO example 3: the board records its GPIO input into a file of its own SD card (the file
# mode): a VCD of one million samples in /home/redpitaya/streaming_files/gpio. No data comes over
# the network; the client hears when the board ends the file.
#
# Usage: gpio_example_3.py [IP]    (without IP the first board found on the network)

import sys

import streaming

REASONS = {streaming.GPIO_FILE_DONE: "the sample limit is written", streaming.GPIO_FILE_SD_FULL: "the SD card is full",
           streaming.GPIO_FILE_BROKEN: "the file cannot be written"}


class FileHandler(streaming.GPIOCallback):
    def inputFileStopped(self, client, host, reason):
        print(f"{host} ended its GPIO file: {REASONS.get(reason, reason)}")


def main():
    conf = streaming.ConfigStreamClient()
    if not (conf.connect([sys.argv[1]]) if len(sys.argv) > 1 else conf.connect()):
        print("No board found")
        return 1
    host = conf.getHosts()[0]

    # the file mode of the input: the format and the samples of the file
    for key, value in (("gpio_decimation", "1"), ("gpio_rle", "ON"), ("gpio_in_pass_mode", "FILE"),
                       ("gpio_in_format_sd", "VCD"), ("gpio_in_samples_limit_sd", "1000000")):
        conf.sendConfig(host, key, value)

    gpio = streaming.GPIOStreamClient(conf)
    handler = FileHandler()
    gpio.setCallback(handler)
    if not gpio.startInput(host):
        print("The GPIO input did not start")
        return 1
    print(host, "writes its GPIO input into a file" if gpio.inputToFile() else "streams its GPIO input to the network")
    gpio.wait()  # until the board ends the file

    conf.sendConfig(host, "gpio_in_pass_mode", "NET")  # back to the network mode
    gpio.removeCallback()
    print("The file is in /home/redpitaya/streaming_files/gpio on the board")
    return 0


if __name__ == "__main__":
    sys.exit(main())
