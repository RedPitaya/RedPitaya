#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <linux/ioctl.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <mutex>
#include <vector>

#include "rp_hw-profiles.h"

using namespace std;

enum RP_PWR_state { PWR_OFF, PWR_UP, PWR_ON, PWR_DWN, PWR_DWN_RST };

#define MINARGS 2

#define EXPANDER_ADDR 0x10

#define printWithLog(X, Y, ...) fprintf(Y, __VA_ARGS__);

std::vector<uint8_t> readData() {
    int status;
    int fd;
    std::vector<uint8_t> buf;
    buf.resize(4);

    // Open the device.
    fd = open("/dev/i2c-0", O_RDWR);
    if (fd < 0) {
        return {};
    }

    // set slave address
    status = ioctl(fd, I2C_SLAVE_FORCE, EXPANDER_ADDR);
    if (status < 0) {
        close(fd);
        return {};
    }

    int retVal = read(fd, buf.data(), buf.size());
    close(fd);
    if (retVal < 0) {
        return {};
    }
    return buf;
}

bool writeData(uint8_t hw_id, uint8_t code) {
    int status;
    int fd;
    std::vector<uint8_t> buf = {0xE3, 0xAD, hw_id, code};

    // Open the device.
    fd = open("/dev/i2c-0", O_RDWR);
    if (fd < 0) {
        return false;
    }

    // set slave address
    status = ioctl(fd, I2C_SLAVE_FORCE, EXPANDER_ADDR);
    if (status < 0) {
        close(fd);
        return false;
    }

    int retVal = write(fd, buf.data(), buf.size());
    close(fd);
    if (retVal < 0) {
        return false;
    }
    return true;
}

void usage(const char* args) {
    const char* format =
        "Usage: %s -w hw_id Value\n"
        "Usage: %s -r [-v]\n"

        "\t\t-w    Writes a value to the device at the address: 0x%X.\n"
        "\t\t-r    Reads a value from a device.\n"
        "\t\t-v    Decodes the received values.\n"
        "Parameters:\n"
        "    hw_id = Expansion board version. The value must be in HEX format. (For example 0x01)\n"
        "    Value = Value in HEX format or in string format. (For example 0x01 or PWR_OFF)\n";

    fprintf(stderr, format, args, args, EXPANDER_ADDR);

    const char* format_2 =
        "\nValue:\n"
        "\t\tPWR_OFF = 0x%02X.\n"
        "\t\tPWR_UP = 0x%02X.\n"
        "\t\tPWR_ON = 0x%02X.\n"
        "\t\tPWR_DWN = 0x%02X.\n"
        "\t\tPWR_DWN_RST = 0x%02X.\n"
        "\t\tVERB = 0xFF.\n";

    fprintf(stderr, format_2, PWR_OFF, PWR_UP, PWR_ON, PWR_DWN, PWR_DWN_RST);

    const char* format_3 =
        "\nExamples:\n"
        "\t\te3_i2c_controller -w 0x02 PWR_UP\n"
        "\t\te3_i2c_controller -w 0x1 0x2\n"
        "\t\te3_i2c_controller -r -v\n"
        "\t\te3_i2c_controller -r\n";

    fprintf(stderr, format_3, "");
}

int parseEnum(std::string param) {
    if (param == "PWR_OFF")
        return PWR_OFF;
    if (param == "PWR_UP")
        return PWR_UP;
    if (param == "PWR_ON")
        return PWR_ON;
    if (param == "PWR_DWN")
        return PWR_DWN;
    if (param == "PWR_DWN_RST")
        return PWR_DWN_RST;
    if (param == "VERB")
        return 0xFF;
    return -1;
}

std::string parseRetValue(RP_PWR_state param) {
    if (param == PWR_OFF)
        return "PWR_OFF";
    if (param == PWR_UP)
        return "PWR_UP";
    if (param == PWR_ON)
        return "PWR_ON";
    if (param == PWR_DWN)
        return "PWR_DWN";
    if (param == PWR_DWN_RST)
        return "PWR_DWN_RST";
    return "";
}

int main(int argc, char** argv) {

    uint8_t hw_id = 0;
    uint8_t code = 0;
    bool verb = false;

    if (argc < MINARGS) {
        usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    if (strncmp(argv[1], "-w", 2) == 0) {
        if (argc < 4) {
            usage(argv[0]);
            exit(EXIT_FAILURE);
        }
        auto result = sscanf(argv[2], "%hhx", &hw_id);
        if (result != 1) {
            usage(argv[0]);
            exit(EXIT_FAILURE);
        }

        auto codeEnum = parseEnum(argv[3]);
        if (codeEnum == -1) {
            auto result = sscanf(argv[3], "%hhx", &code);
            if (result != 1) {
                usage(argv[0]);
                exit(EXIT_FAILURE);
            }
        } else {
            code = codeEnum;
        }
        if (writeData(hw_id, code)) {
            exit(EXIT_SUCCESS);
        }
        exit(EXIT_FAILURE);
    }

    if (strncmp(argv[1], "-r", 2) == 0) {
        if (argc >= 3) {
            if (strncmp(argv[2], "-v", 2) == 0) {
                verb = true;
            }
        }
        auto data = readData();
        if (verb) {

            if (data.size() == 0) {
                fprintf(stderr, "Error get data from i2c device\n");
                exit(EXIT_FAILURE);
            }

            if (data[0] != 0xE3) {
                fprintf(stderr, "Error. Wrong data header 0x%2X. Must be 0xE3\n", data[0]);
                exit(EXIT_FAILURE);
            }

            if (data[1] == 0xAD) {
                fprintf(stderr, "HW ID = 0x%02x\n", data[2]);
                fprintf(stderr, "Data = 0x%02x - %s\n", data[3], parseRetValue((RP_PWR_state)data[3]).c_str());
            } else if (data[1] == 0x33) {
                fprintf(stderr, "HW ID = 0x%02X\n", data[2]);
                fprintf(stderr, "Error code = 0x%02X\n", data[3]);
            } else {
                fprintf(stderr, "Error. Wrong data type 0x%02X.\n", data[1]);
                exit(EXIT_FAILURE);
            }

        } else {
            for (size_t i = 0; i < data.size(); i++) {
                printf("%02X", data[i]);
            }
        }

        exit(EXIT_SUCCESS);
    }

    exit(EXIT_FAILURE);
}
