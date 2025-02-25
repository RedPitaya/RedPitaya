#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <linux/ioctl.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <syslog.h>
#include <unistd.h>
#include <atomic>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <mutex>
#include <vector>

#include "rp_hw-profiles.h"

using namespace std;

#define SYS(X, ...) syslog(X, __VA_ARGS__);
#define MINARGS 2
#define printWithLog(X, Y, ...) \
    fprintf(Y, __VA_ARGS__);    \
    SYS(X, __VA_ARGS__);

#define OVERLAY_PATH "/sys/kernel/config/device-tree/overlays/Led"
#define I2C_SLAVE_FORCE 0x0706
#define EXPANDER_ADDR 0x10
#define VALUE_MAX 40
#define RP_GPIO_IN 0
#define RP_GPIO_OUT 1
#define YELLOW_LED8 906

std::atomic_bool g_run(true);

static void termSignalHandler(int) {
    printWithLog(LOG_INFO, stdout, "Received terminate signal. Exiting...\n");
    g_run = false;
}

static void handleCloseChildEvents() {
    struct sigaction sigchld_action;
    sigchld_action.sa_handler = SIG_DFL, sigchld_action.sa_flags = SA_NOCLDWAIT;
    sigaction(SIGCHLD, &sigchld_action, NULL);
}

static void installTermSignalHandler() {
    signal(SIGINT, termSignalHandler);
    signal(SIGTERM, termSignalHandler);
}

bool isOverlayLoaded() {
    struct stat sb;
    if (((stat(OVERLAY_PATH, &sb) == 0) && S_ISDIR(sb.st_mode))) {
        return true;
    }
    return false;
}

void removeOverlay() {
    char command[2048];
    if (isOverlayLoaded()) {
        snprintf(command, sizeof(command), "rmdir %s", OVERLAY_PATH);
        system(command);
    }
}

std::string getPathToOverlay() {
    std::string path = "/opt/redpitaya/fpga/";
    std::string path_suff = "/barebones/dts_rp/led-system.dtbo";
    char* modelFPGA = NULL;
    auto ret = rp_HPGetFPGAVersion(&modelFPGA);
    if (ret == RP_HP_OK) {
        path = path + modelFPGA + path_suff;
    } else {
        return "";
    }
    return path;
}

int loadOverlay() {
    char command[2048];
    if (!isOverlayLoaded()) {
        snprintf(command, sizeof(command), "mkdir -p %s", OVERLAY_PATH);
        system(command);
    }
    auto overlay_file = getPathToOverlay();
    snprintf(command, sizeof(command), "cat %s > %s/dtbo", overlay_file.c_str(), OVERLAY_PATH);
    return system(command);
}

bool checkExtensionModuleConnection(bool _muteWarnings) {
    int status;
    int fd;
    char buf[2];

    // Open the device.
    fd = open("/dev/i2c-0", O_RDWR);
    if (fd < 0) {
        if (!_muteWarnings)
            printWithLog(LOG_ERR, stderr, "Cannot open the I2C device: %d\n", fd);
        return false;
    }

    // set slave address
    status = ioctl(fd, I2C_SLAVE_FORCE, EXPANDER_ADDR);
    if (status < 0) {
        close(fd);
        if (!_muteWarnings)
            printWithLog(LOG_ERR, stderr, "Unable to set the I2C address: %d\n", status);
        return false;
    }

    int retVal = read(fd, buf, 1);
    close(fd);
    if (retVal < 0) {
        return false;
    }
    return true;
}

int gpio_write(int pin, int value) {
    char path[VALUE_MAX];
    int fd;

    snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
    // get pin value file descrptor
    fd = open(path, O_WRONLY);
    if (-1 == fd) {
        printWithLog(LOG_ERR, stderr, "[gpio_write] Unable to to open sysfs pins value file %s for writing\n", path);
        return -1;
    }
    if (value == 0) {
        //write low
        if (1 != write(fd, "0", 1)) {
            printWithLog(LOG_ERR, stderr, "[gpio_write] Unable to write value\n");
            return -1;
        }
    } else if (value == 1) {
        //write high
        if (1 != write(fd, "1", 1)) {
            printWithLog(LOG_ERR, stderr, "[gpio_write] Unable to write value\n");
            return -1;
        }
    } else
        printWithLog(LOG_ERR, stderr, "[gpio_write] Nonvalid pin value requested\n");

    //close file
    close(fd);
    return 0;
}

int gpio_read(int pin) {

    char path[VALUE_MAX];
    char value_str[3];
    int fd;

    snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);

    // get pin file descriptor for reading its state
    fd = open(path, O_RDONLY);
    if (-1 == fd) {
        printWithLog(LOG_ERR, stderr, "[gpio_read] Unable to open gpio sysfs pin value file %s for reading\n", path);
        return -1;
    }

    // read value
    if (-1 == read(fd, value_str, 3)) {
        printWithLog(LOG_ERR, stderr, "[gpio_read] Unable to read value\n");
        close(fd);
        return -1;
    }

    // close file
    close(fd);

    // return integar value
    return atoi(value_str);
}

int gpio_pin_direction(int pin, int value) {
    char path[VALUE_MAX];
    int fd;

    snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/direction", pin);
    // get pin value file descrptor
    fd = open(path, O_WRONLY);
    if (-1 == fd) {
        printWithLog(LOG_ERR, stderr, "[gpio_pin_direction] Unable to to open sysfs pins value file %s for writing direction\n", path);
        return -1;
    }

    switch (value) {
        case RP_GPIO_IN:
            if (2 != write(fd, "in", 2)) {
                printWithLog(LOG_ERR, stderr, "[gpio_pin_direction] Unable to write direction value\n");
                close(fd);
                return -1;
            }
            break;
        case RP_GPIO_OUT:
            if (3 != write(fd, "out", 3)) {
                printWithLog(LOG_ERR, stderr, "[gpio_pin_direction] Unable to write direction value\n");
                close(fd);
                return -1;
            }
            break;
        default:
            printWithLog(LOG_ERR, stderr, "[gpio_pin_direction] Nonvalid pin direction requested\n");
            break;
    }

    //close file
    close(fd);
    return 0;
}

int gpio_is_export(int pin, bool* value) {
    char path[VALUE_MAX];
    int fd;
    snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/direction", pin);
    // get pin value file descrptor
    fd = open(path, O_WRONLY);
    if (-1 == fd) {
        *value = false;
        return 0;
    }
    close(fd);
    *value = true;
    return 0;
}

int gpio_export(int pin) {
    char path[VALUE_MAX];
    int fd;
    char buffer[10];

    snprintf(path, VALUE_MAX, "/sys/class/gpio/export");
    // get pin value file descrptor
    fd = open(path, O_WRONLY);
    if (-1 == fd) {
        printWithLog(LOG_ERR, stderr, "[gpio_export] Unable to to open sysfs pins value file %s for export\n", path);
        return -1;
    }
    sprintf(buffer, "%d", pin);
    int str_len = strlen(buffer);
    //write in
    if (str_len != write(fd, buffer, str_len)) {
        printWithLog(LOG_ERR, stderr, "[gpio_export] Unable to write export pin %s\n", buffer);
        close(fd);
        return -1;
    }

    //close file
    close(fd);
    return 0;
}

int gpio_unexport(int pin) {
    char path[VALUE_MAX];
    int fd;
    char buffer[10];

    snprintf(path, VALUE_MAX, "/sys/class/gpio/unexport");
    // get pin value file descrptor
    fd = open(path, O_WRONLY);
    if (-1 == fd) {
        printWithLog(LOG_ERR, stderr, "[gpio_unexport] Unable to to open sysfs pins value file %s for unexport\n", path);
        return -1;
    }
    sprintf(buffer, "%d", pin);
    int str_len = strlen(buffer);
    //write in
    if (str_len != write(fd, buffer, str_len)) {
        printWithLog(LOG_ERR, stderr, "[gpio_unexport] Unable to write unexport pin\n");
        close(fd);
        return -1;
    }

    //close file
    close(fd);
    return 0;
}

int get_state_gpio(int* state, const char* str) {
    if (strncmp(str, "=IN", 3) == 0) {
        *state = RP_GPIO_IN;
        return 0;
    }
    if (strncmp(str, "=OUT", 4) == 0) {
        *state = RP_GPIO_OUT;
        return 0;
    }

    fprintf(stderr, "Unknown state: %s\n", str);
    return -1;
}

int get_value_gpio(int* state, const char* str) {
    if (strncmp(str, "=0", 2) == 0) {
        *state = 0;
        return 0;
    }
    if (strncmp(str, "=1", 2) == 0) {
        *state = 1;
        return 0;
    }

    fprintf(stderr, "Unknown state: %s\n", str);
    return -1;
}

void usage(const char* args) {
    const char* format =
        "Usage: %s [-b] [-r] [-l] [-d] [-g=IN|OUT] [-v[=Value]]]\n"
        "\t\t-b    Runs as a service. Other keys are ignored.\n"
        "\t\t-r    Remove system LEDs overlay.\n"
        "\t\t-l    Load system LEDs overlay.\n"
        "\t\t-d    Checks for the presence of an expansion board on the i2c bus at the address: 0x%X.\n"
        "\t\t-g    Sets the mode for the GPIO. Required value is IN or OUT.\n"
        "\t\t-v    Reads or sets the value depending on the GPIO mode.\n"
        "Optional parameter:\n"
        "    Value = [0 | 1]  Sets the value on the GPIO in OUT mode\n";

    fprintf(stderr, format, args, EXPANDER_ADDR);
}

int main(int argc, char** argv) {

    bool is_background = false;
    bool need_remove = false;
    bool need_load = false;
    bool need_detect = false;
    bool is_set_gpio = false;
    int set_gpio_dir = 0;
    bool is_get_gpio_value = false;
    bool is_set_gpio_value = false;
    int set_gpio_value = 0;

    if (argc < MINARGS) {
        usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    const char* optstring = "brldg:v::";

    int ch = -1;
    while ((ch = getopt(argc, argv, optstring)) != -1) {
        switch (ch) {
            case 'b':
                is_background = true;
                break;
            case 'r':
                need_remove = true;
                break;
            case 'l':
                need_load = true;
                break;
            case 'd':
                need_detect = true;
                break;
            case 'g':
                is_set_gpio = true;

                if (get_state_gpio(&set_gpio_dir, optarg) != 0) {
                    usage(argv[0]);
                    exit(EXIT_FAILURE);
                }
                break;
            case 'v':
                is_get_gpio_value = true;
                if (optarg) {
                    is_set_gpio_value = true;
                    if (get_value_gpio(&set_gpio_value, optarg) != 0) {
                        usage(argv[0]);
                        exit(EXIT_FAILURE);
                    }
                }
                break;
        }
    }

    if (need_remove) {
        removeOverlay();
        exit(EXIT_SUCCESS);
    }

    if (need_load) {
        if (isOverlayLoaded()) {
            printWithLog(LOG_ERR, stderr, "[ERROR] Overlay loaded.\n");
            exit(EXIT_FAILURE);
        }
        bool is_export = false;
        gpio_is_export(YELLOW_LED8, &is_export);
        int ret = 0;
        if (!is_export) {
            ret = gpio_export(YELLOW_LED8);
            if (ret == -1)
                exit(ret);
        }
        ret = gpio_pin_direction(YELLOW_LED8, RP_GPIO_OUT);
        if (ret == -1)
            exit(ret);
        gpio_unexport(YELLOW_LED8);
        if (ret == -1)
            exit(ret);
        exit(loadOverlay());
    }

    if (need_detect) {
        auto b = checkExtensionModuleConnection(false);
        printf("%d", b);
        exit(EXIT_SUCCESS);
    }

    if (is_set_gpio) {
        if (isOverlayLoaded()) {
            printWithLog(LOG_ERR, stderr, "[ERROR] Overlay loaded. GPIO control is not possible\n");
            exit(EXIT_FAILURE);
        } else {
            bool is_export = false;
            gpio_is_export(YELLOW_LED8, &is_export);
            if (!is_export) {
                int ret = gpio_export(YELLOW_LED8);
                if (ret != 0) {
                    return ret;
                }
            }
            exit(gpio_pin_direction(YELLOW_LED8, set_gpio_dir));
        }
    }

    if (is_get_gpio_value) {
        if (isOverlayLoaded()) {
            printWithLog(LOG_ERR, stderr, "[ERROR] Overlay loaded. GPIO control is not possible\n");
            exit(EXIT_FAILURE);
        } else {
            bool is_export = false;
            gpio_is_export(YELLOW_LED8, &is_export);
            if (is_export) {
                if (is_set_gpio_value) {
                    exit(gpio_write(YELLOW_LED8, set_gpio_value));
                } else {
                    int ret = gpio_read(YELLOW_LED8);
                    if (ret == -1) {
                        return ret;
                    }
                    printf("%d", ret);
                    exit(EXIT_SUCCESS);
                }
            } else {
                printWithLog(LOG_ERR, stderr, "[ERROR] GPIO Direction not set\n");
                exit(EXIT_FAILURE);
            }
        }
    }

    setlogmask(LOG_UPTO(LOG_INFO));

    openlog("e3_led_controller", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
    if (rp_HPGetIsE3PresentOrDefault()) {
        if (is_background) {
            pid_t process_id = fork();
            if (process_id < 0) {
                fprintf(stderr, "Fork failed!\n");
                exit(1);
            } else if (process_id > 0) {
                exit(0);
            }

            umask(0);
            if (setsid() < 0) {
                exit(1);
            }

            close(STDIN_FILENO);
            close(STDOUT_FILENO);
            close(STDERR_FILENO);

            installTermSignalHandler();
            handleCloseChildEvents();

            while (g_run) {
                sleep(1);
                bool is_ext = checkExtensionModuleConnection(true);
                if (is_ext) {
                    if (isOverlayLoaded()) {
                        removeOverlay();
                        printWithLog(LOG_INFO, stdout, "[INFO] Remove overlay.\n");
                    }
                    bool is_export = false;
                    gpio_is_export(YELLOW_LED8, &is_export);
                    if (!is_export) {
                        int ret = gpio_export(YELLOW_LED8);
                        if (ret != 0) {
                            printWithLog(LOG_ERR, stderr, "[ERROR] Can't export GPIO %d\n", YELLOW_LED8);
                            continue;
                        }
                        ret = gpio_pin_direction(YELLOW_LED8, RP_GPIO_IN);
                        if (ret != 0) {
                            printWithLog(LOG_ERR, stderr, "[ERROR] Can't set IN mode for GPIO %d\n", YELLOW_LED8);
                            continue;
                        }
                    }
                    int ret = gpio_read(YELLOW_LED8);
                    if (ret == -1) {
                        printWithLog(LOG_ERR, stderr, "[ERROR] Can't read value for GPIO %d\n", YELLOW_LED8);
                        continue;
                    }
                    if (ret == 1) {
                        printWithLog(LOG_INFO, stdout, "[INFO] Run poweroff command\n");
                        ret = system("poweroff");
                        if (ret != 0) {
                            printWithLog(LOG_ERR, stderr, "[ERROR] Error executing poweroff command\n");
                            continue;
                        }
                    }
                } else {
                    if (!isOverlayLoaded()) {
                        bool is_export = false;
                        gpio_is_export(YELLOW_LED8, &is_export);
                        int ret = 0;
                        if (!is_export) {
                            ret = gpio_export(YELLOW_LED8);
                            if (ret == -1) {
                                printWithLog(LOG_ERR, stderr, "[ERROR] Can't export GPIO %d\n", YELLOW_LED8);
                                continue;
                            }
                        }
                        ret = gpio_pin_direction(YELLOW_LED8, RP_GPIO_IN);
                        if (ret != 0) {
                            printWithLog(LOG_ERR, stderr, "[ERROR] Can't set OUT mode for GPIO %d\n", YELLOW_LED8);
                            continue;
                        }
                        ret = gpio_unexport(YELLOW_LED8);
                        if (ret == -1) {
                            printWithLog(LOG_ERR, stderr, "[ERROR] Can't unexport GPIO %d\n", YELLOW_LED8);
                            continue;
                        }
                        loadOverlay();
                        printWithLog(LOG_INFO, stdout, "[INFO] Loaded overlay.\n");
                    }
                }
            }
        }
    } else {
        if (!isOverlayLoaded()) {
            bool is_export = false;
            gpio_is_export(YELLOW_LED8, &is_export);
            int ret = 0;
            if (!is_export) {
                ret = gpio_export(YELLOW_LED8);
                if (ret == -1) {
                    printWithLog(LOG_ERR, stderr, "[ERROR] Can't export GPIO %d\n", YELLOW_LED8);
                }
            }
            ret = gpio_pin_direction(YELLOW_LED8, RP_GPIO_IN);
            if (ret != 0) {
                printWithLog(LOG_ERR, stderr, "[ERROR] Can't set OUT mode for GPIO %d\n", YELLOW_LED8);
            }
            ret = gpio_unexport(YELLOW_LED8);
            if (ret == -1) {
                printWithLog(LOG_ERR, stderr, "[ERROR] Can't unexport GPIO %d\n", YELLOW_LED8);
            }
            loadOverlay();
            printWithLog(LOG_INFO, stdout, "[INFO] Loaded overlay.\n");
        }
    }
    printWithLog(LOG_INFO, stdout, "[INFO] Exit.\n");
    closelog();
    return (EXIT_SUCCESS);
}
