/**
 * $Id: $
 *
 * @brief Red Pitaya ptcc_control utility.
 *
 * Command line front end for librp-ptcc: verifies the USB link, reads the
 * monitor container and changes the setpoint without the web application.
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 */

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <unistd.h>

#include <iostream>

extern "C" {
#include "rp_ptcc.h"
}

/** Program name */
const char *g_argv0 = NULL;

/** Minimal number of command line arguments */
#define MINARGS 2

#define STR_BUFFER 1024

int get_mode(rp_ptcc_ctrl_t *mode, const char *str) {
    if (strncmp(str, "=Off", 4) == 0) {
        *mode = RP_PTCC_CTRL_OFF;
        return 0;
    }
    if (strncmp(str, "=On", 3) == 0) {
        *mode = RP_PTCC_CTRL_ON;
        return 0;
    }
    if (strncmp(str, "=Auto", 5) == 0) {
        *mode = RP_PTCC_CTRL_AUTO;
        return 0;
    }

    fprintf(stderr, "Unknown mode: %s\n", str);
    return -1;
}

int get_register(rp_ptcc_register_t *reg, const char *str) {
    if (strncmp(str, "=Set", 4) == 0) {
        *reg = RP_PTCC_REG_USER_SET;
        return 0;
    }
    if (strncmp(str, "=Default", 8) == 0) {
        *reg = RP_PTCC_REG_DEFAULT;
        return 0;
    }
    if (strncmp(str, "=Min", 4) == 0) {
        *reg = RP_PTCC_REG_USER_MIN;
        return 0;
    }
    if (strncmp(str, "=Max", 4) == 0) {
        *reg = RP_PTCC_REG_USER_MAX;
        return 0;
    }

    fprintf(stderr, "Unknown register: %s\n", str);
    return -1;
}

int get_float(float *value, const char *str) {
    if (str[0] != '=') {
        fprintf(stderr, "Expected =<value>, got: %s\n", str);
        return -1;
    }

    char *end = NULL;
    const double parsed = strtod(str + 1, &end);
    if (end == str + 1 || *end != '\0') {
        fprintf(stderr, "Not a number: %s\n", str + 1);
        return -1;
    }

    *value = static_cast<float>(parsed);
    return 0;
}

int get_long(long *value, const char *str) {
    char *end = NULL;
    const long parsed = strtol(str, &end, 10);
    if (end == str || *end != '\0') {
        fprintf(stderr, "Not an integer: %s\n", str);
        return -1;
    }

    *value = parsed;
    return 0;
}

int get_ulong(unsigned long *value, const char *str) {
    char *end = NULL;
    const unsigned long parsed = strtoul(str, &end, 10);
    if (end == str || *end != '\0') {
        fprintf(stderr, "Not a number: %s\n", str);
        return -1;
    }

    *value = parsed;
    return 0;
}

/** Prints an API error together with the underlying Python exception. */
void report(const char *what, rp_ptcc_error error) {
    const char *detail = rp_PtccGetLastPythonError();
    if (detail != NULL && detail[0] != '\0') {
        fprintf(stderr, "[Error] %s: %s (%s)\n", what, rp_PtccGetErrorText(error), detail);
    } else {
        fprintf(stderr, "[Error] %s: %s\n", what, rp_PtccGetErrorText(error));
    }
}

const char *module_name(rp_ptcc_module_t module) {
    switch (module) {
        case RP_PTCC_MODULE_NONE:
            return "NONE";
        case RP_PTCC_MODULE_NOMEM:
            return "NOMEM";
        case RP_PTCC_MODULE_MEM:
            return "MEM";
        case RP_PTCC_MODULE_LAB_M:
            return "LAB_M";
        default:
            return "UNKNOWN";
    }
}

const char *ctrl_name(rp_ptcc_ctrl_t mode) {
    switch (mode) {
        case RP_PTCC_CTRL_AUTO:
            return "Auto";
        case RP_PTCC_CTRL_OFF:
            return "Off";
        case RP_PTCC_CTRL_ON:
            return "On";
        default:
            return "Unknown";
    }
}

const char *register_name(rp_ptcc_register_t reg) {
    switch (reg) {
        case RP_PTCC_REG_DEFAULT:
            return "Default";
        case RP_PTCC_REG_USER_SET:
            return "Set";
        case RP_PTCC_REG_USER_MIN:
            return "Min";
        case RP_PTCC_REG_USER_MAX:
            return "Max";
        default:
            return "Unknown";
    }
}

/** Print usage information */
void usage() {
    const char *format =
            "\n"
            "Usage: %s [-d Device] [-b Baudrate] [-T Ms] [-O Ms]\n"
            "          [-l] [-i] [-e] [-m] [-q] [-w[=Period]] [-p[=Register]]\n"
            "          [-t[=Kelvin]] [-x[=Amperes]] [-c[=State]] [-f[=State]]\n"
            "          [-s] [-j] [-v]\n"
            "\n"
            "Connection:\n"
            "   -d    Serial device to use, for example /dev/ttyACM0.\n"
            "         Without this option all ttyACM and ttyUSB nodes are probed.\n"
            "   -b    Line rate. 57600 by default.\n"
            "   -T    Minimum interval between commands in milliseconds.\n"
            "         550 by default. Lower values make the firmware drop commands.\n"
            "   -O    Response timeout in milliseconds. 2000 by default.\n"
            "\n"
            "Reading:\n"
            "   -l    List candidate serial ports and exit.\n"
            "   -i    Print controller and module identification.\n"
            "   -e    Print link state and the number of rejected frames.\n"
            "   -m    Read the monitor container once and print it.\n"
            "   -q    Print the detector temperature only.\n"
            "   -w    Watch the monitor through the background poller. Period in\n"
            "         milliseconds, 1000 by default. Stop with Ctrl-C.\n"
            "   -p    Dump a parameter register, Set by default.\n"
            "   -s    Print the status code only. Useful in shell scripts.\n"
            "   -j    Print machine readable JSON instead of a table.\n"
            "   -v    Print the library version and protocol revision.\n"
            "\n"
            "Control:\n"
            "   -t    Without a value prints the temperature setpoint.\n"
            "         With =Kelvin writes a new setpoint, in whole Kelvins.\n"
            "         The accepted range depends on the module, see -i.\n"
            "   -x    TEC current limit in Amperes.\n"
            "   -c    Cooler control. Without a value prints the current mode.\n"
            "   -f    Fan control. Without a value prints the current mode.\n"
            "\n"
            "Optional parameters:\n"
            "    State    = [Off | On | Auto]\n"
            "    Register = [Set | Default | Min | Max]\n"
            "\n"
            "WARNING: -t and -x write to the module EEPROM. Do not call them in\n"
            "         a loop, the memory has a limited number of write cycles.\n"
            "\n"
            "Examples:\n"
            "    %s -i                    print identification\n"
            "    %s -m                    read the monitor once\n"
            "    %s -w=2000               watch every two seconds\n"
            "    %s -p=Max                show the upper parameter limits\n"
            "    %s -t                    print the current setpoint\n"
            "    %s -t=230                cool the detector to 230 K\n"
            "    %s -c=On -d /dev/ttyACM0 force the cooler on\n"
            "\n";

    fprintf(stderr, format, g_argv0, g_argv0, g_argv0, g_argv0, g_argv0, g_argv0, g_argv0, g_argv0);
}

void print_monitor(const rp_ptcc_monitor_t *monitor) {
    printf("Detector temperature = %8.3f K\n", static_cast<double>(monitor->t_det));
    printf("Internal temperature = %8.1f C\n", static_cast<double>(monitor->t_int));
    printf("TEC current          = %8.4f A\n", static_cast<double>(monitor->i_tec));
    printf("TEC voltage          = %8.3f V\n", static_cast<double>(monitor->u_tec));
    printf("Supply current +/-   = %8.4f / %8.4f A\n", static_cast<double>(monitor->i_sup_plus),
           static_cast<double>(monitor->i_sup_minus));
    printf("Supply voltage +/-   = %8.3f / %8.3f V\n", static_cast<double>(monitor->u_sup_plus),
           static_cast<double>(monitor->u_sup_minus));
    printf("Fan current          = %8.4f A\n", static_cast<double>(monitor->i_fan));
    printf("Thermistor           = %8.1f Ohm\n", static_cast<double>(monitor->th_resistance));
    printf("PWM                  = %8u\n", monitor->pwm);
    printf("Supply / fan         = %s / %s\n", monitor->supply_on ? "on" : "off",
           monitor->fan_on ? "on" : "off");
    bool is_error = false;
    rp_PtccIsErrorStatus(monitor->status, &is_error);
    printf("Status               = %u %s - %s\n", monitor->status, is_error ? "[ERROR]" : "",
           rp_PtccGetStatusText(monitor->status));
}

void print_monitor_json(const rp_ptcc_monitor_t *monitor) {
    printf("{");
    printf("\"t_det\":%.3f,", static_cast<double>(monitor->t_det));
    printf("\"t_int\":%.1f,", static_cast<double>(monitor->t_int));
    printf("\"i_tec\":%.4f,", static_cast<double>(monitor->i_tec));
    printf("\"u_tec\":%.3f,", static_cast<double>(monitor->u_tec));
    printf("\"i_sup_plus\":%.4f,", static_cast<double>(monitor->i_sup_plus));
    printf("\"i_sup_minus\":%.4f,", static_cast<double>(monitor->i_sup_minus));
    printf("\"u_sup_plus\":%.3f,", static_cast<double>(monitor->u_sup_plus));
    printf("\"u_sup_minus\":%.3f,", static_cast<double>(monitor->u_sup_minus));
    printf("\"i_fan\":%.4f,", static_cast<double>(monitor->i_fan));
    printf("\"th_resistance\":%.1f,", static_cast<double>(monitor->th_resistance));
    printf("\"pwm\":%u,", monitor->pwm);
    printf("\"supply_on\":%s,", monitor->supply_on ? "true" : "false");
    printf("\"fan_on\":%s,", monitor->fan_on ? "true" : "false");
    printf("\"status\":%u,", monitor->status);
    bool is_error = false;
    rp_PtccIsErrorStatus(monitor->status, &is_error);
    printf("\"is_error\":%s,", is_error ? "true" : "false");
    printf("\"status_text\":\"%s\",", rp_PtccGetStatusText(monitor->status));
    printf("\"valid\":%s,", monitor->valid ? "true" : "false");
    printf("\"timestamp\":%lld", static_cast<long long>(monitor->timestamp));
    printf("}\n");
    fflush(stdout);
}

int print_params(rp_ptcc_register_t reg, bool json) {
    rp_ptcc_params_t params;
    memset(&params, 0, sizeof(params));

    const rp_ptcc_error result = rp_PtccGetParams(reg, &params);
    if (result != RP_PTCC_OK) {
        report("Can't read parameters", result);
        return -1;
    }

    if (json) {
        printf("{");
        printf("\"register\":\"%s\",", register_name(reg));
        printf("\"setpoint\":%.3f,", static_cast<double>(params.setpoint));
        printf("\"i_tec_max\":%.4f,", static_cast<double>(params.i_tec_max));
        printf("\"u_sup_plus\":%.3f,", static_cast<double>(params.u_sup_plus));
        printf("\"u_sup_minus\":%.3f,", static_cast<double>(params.u_sup_minus));
        printf("\"pwm\":%u,", params.pwm);
        printf("\"supply_ctrl\":\"%s\",", ctrl_name(params.supply_ctrl));
        printf("\"fan_ctrl\":\"%s\",", ctrl_name(params.fan_ctrl));
        printf("\"tec_ctrl\":\"%s\"", ctrl_name(params.tec_ctrl));
        printf("}\n");
        return 0;
    }

    printf("Register             = %s\n", register_name(reg));
    printf("Setpoint             = %8.3f K\n", static_cast<double>(params.setpoint));
    printf("TEC current limit    = %8.4f A\n", static_cast<double>(params.i_tec_max));
    printf("Supply voltage +/-   = %8.3f / %8.3f V\n", static_cast<double>(params.u_sup_plus),
           static_cast<double>(params.u_sup_minus));
    printf("PWM                  = %8u\n", params.pwm);
    printf("Supply control       = %s\n", ctrl_name(params.supply_ctrl));
    printf("Fan control          = %s\n", ctrl_name(params.fan_ctrl));
    printf("Cooler control       = %s\n", ctrl_name(params.tec_ctrl));
    return 0;
}

int print_iden(bool json) {
    rp_ptcc_device_iden_t device;
    rp_ptcc_module_iden_t module;
    memset(&device, 0, sizeof(device));
    memset(&module, 0, sizeof(module));

    const rp_ptcc_error device_result = rp_PtccGetDeviceIden(&device);
    if (device_result != RP_PTCC_OK) {
        report("Can't read device identification", device_result);
        return -1;
    }

    const rp_ptcc_error module_result = rp_PtccGetModuleIden(&module);
    if (module_result != RP_PTCC_OK && module_result != RP_PTCC_ENOTSUP) {
        fprintf(stderr, "[Warning] Can't read module identification: %s\n",
                rp_PtccGetErrorText(module_result));
    }

    rp_ptcc_module_t module_type = RP_PTCC_MODULE_NONE;
    rp_PtccGetModuleType(&module_type);

    char path[STR_BUFFER] = {0};
    rp_PtccGetDevicePath(path, sizeof(path));

    if (json) {
        printf("{");
        printf("\"port\":\"%s\",", path);
        printf("\"module_type\":\"%s\",", module_name(module_type));
        printf("\"device\":{\"type\":\"%s\",\"name\":\"%s\",\"serial\":\"%s\",", device.type, device.name,
               device.serial);
        printf("\"firmware\":%u,\"hardware\":%u},", device.firmware_version, device.hardware_version);
        printf("\"module\":{\"type\":\"%s\",\"name\":\"%s\",\"serial\":\"%s\",", module.type, module.name,
               module.serial);
        printf("\"detector\":\"%s\",\"detector_serial\":\"%s\",\"cool_time\":%u}", module.detector_name,
               module.detector_serial, module.cool_time);
        printf("}\n");
        return 0;
    }

    printf("Port                 = %s\n", path);
    printf("Module type          = %s\n", module_name(module_type));

    rp_ptcc_limits_t limits;
    memset(&limits, 0, sizeof(limits));
    if (rp_PtccGetLimits(&limits) == RP_PTCC_OK && limits.valid) {
        printf("Setpoint range       = %.1f .. %.1f K\n", static_cast<double>(limits.setpoint_min),
               static_cast<double>(limits.setpoint_max));
        printf("Current limit range  = %.4f .. %.4f A\n", static_cast<double>(limits.i_tec_max_min),
               static_cast<double>(limits.i_tec_max_max));
    }

    printf("---- Controller ----\n");
    printf("Type                 = %s\n", device.type);
    printf("Name                 = %s\n", device.name);
    printf("Serial               = %s\n", device.serial);
    printf("Firmware version     = %u\n", device.firmware_version);
    printf("Hardware version     = %u\n", device.hardware_version);

    if (module.valid) {
        printf("---- Module ----\n");
        printf("Type                 = %s\n", module.type);
        printf("Name                 = %s\n", module.name);
        printf("Serial               = %s\n", module.serial);
        printf("Detector             = %s\n", module.detector_name);
        printf("Detector serial      = %s\n", module.detector_serial);
        printf("Cool down time       = %u s\n", module.cool_time);
    }

    return 0;
}

int print_link_state(bool json) {
    bool connected = false;
    uint64_t errors = 0;
    char path[STR_BUFFER] = {0};

    rp_PtccIsConnected(&connected);
    rp_PtccGetErrorCount(&errors);
    rp_PtccGetDevicePath(path, sizeof(path));

    if (json) {
        printf("{\"connected\":%s,\"port\":\"%s\",\"rejected_frames\":%llu}\n",
               connected ? "true" : "false", path, static_cast<unsigned long long>(errors));
        return 0;
    }

    printf("Connected            = %s\n", connected ? "yes" : "no");
    printf("Port                 = %s\n", path);
    printf("Rejected frames      = %llu\n", static_cast<unsigned long long>(errors));
    return 0;
}

int list_ports(bool json) {
    char buffer[STR_BUFFER] = {0};
    uint32_t count = 0;

    const rp_ptcc_error result = rp_PtccListPorts(buffer, sizeof(buffer), &count);
    if (result != RP_PTCC_OK) {
        fprintf(stderr, "[Error] Can't list ports: %s\n", rp_PtccGetErrorText(result));
        return -1;
    }

    if (json) {
        printf("{\"count\":%u,\"ports\":[", count);
        const char *cursor = buffer;
        bool first = true;
        while (*cursor != '\0') {
            const char *end = strchr(cursor, '\n');
            const size_t length = (end != NULL) ? static_cast<size_t>(end - cursor) : strlen(cursor);
            printf("%s\"%.*s\"", first ? "" : ",", static_cast<int>(length), cursor);
            first = false;
            if (end == NULL) {
                break;
            }
            cursor = end + 1;
        }
        printf("]}\n");
        return 0;
    }

    printf("Found %u candidate port(s)\n", count);
    if (count > 0) {
        printf("%s\n", buffer);
    }
    return 0;
}

/** Watches the monitor through the background poller. */
int watch_monitor(long period_ms, bool json) {
    const rp_ptcc_error start = rp_PtccStartMonitoring(static_cast<uint32_t>(period_ms));
    if (start != RP_PTCC_OK) {
        fprintf(stderr, "[Error] Can't start monitoring: %s\n", rp_PtccGetErrorText(start));
        return -1;
    }

    if (!json) {
        printf("%-14s %-12s %-12s %-10s %s\n", "T_det [K]", "I_TEC [A]", "U_TEC [V]", "PWM", "Status");
    }

    int64_t last_timestamp = -1;
    while (true) {
        rp_ptcc_monitor_t monitor;
        memset(&monitor, 0, sizeof(monitor));

        const rp_ptcc_error result = rp_PtccGetMonitor(&monitor);
        if (result != RP_PTCC_OK) {
            fprintf(stderr, "[Error] Can't read cached monitor: %s\n", rp_PtccGetErrorText(result));
            rp_PtccStopMonitoring();
            return -1;
        }

        if (monitor.valid && monitor.timestamp != last_timestamp) {
            last_timestamp = monitor.timestamp;
            if (json) {
                print_monitor_json(&monitor);
            } else {
                printf("%-14.3f %-12.4f %-12.3f %-10u %u %s\n", static_cast<double>(monitor.t_det),
                       static_cast<double>(monitor.i_tec), static_cast<double>(monitor.u_tec), monitor.pwm,
                       monitor.status, rp_PtccGetStatusText(monitor.status));
                fflush(stdout);
            }
        }

        usleep(50000);
    }

    rp_PtccStopMonitoring();
    return 0;
}

/** ptcc_control utility main */
int main(int argc, char *argv[]) {
    g_argv0 = argv[0];

    const char *device_path = NULL;
    unsigned long baudrate = RP_PTCC_DEFAULT_BAUDRATE;
    unsigned long throttle_ms = 0;
    unsigned long timeout_ms = 0;

    bool list_flag = false;
    bool iden_flag = false;
    bool link_flag = false;
    bool monitor_flag = false;
    bool quick_flag = false;
    bool status_flag = false;
    bool json_flag = false;
    bool version_flag = false;

    bool params_flag = false;
    rp_ptcc_register_t params_register = RP_PTCC_REG_USER_SET;

    bool watch_flag = false;
    long watch_period = 1000;

    bool temperature_flag = false;
    bool set_temperature = false;
    long temperature = 0;

    bool current_flag = false;
    bool set_current = false;
    float current = 0.0F;

    bool cooler_flag = false;
    bool set_cooler = false;
    rp_ptcc_ctrl_t cooler_mode = RP_PTCC_CTRL_AUTO;

    bool fan_flag = false;
    bool set_fan = false;
    rp_ptcc_ctrl_t fan_mode = RP_PTCC_CTRL_AUTO;

    if (argc < MINARGS) {
        usage();
        exit(EXIT_FAILURE);
    }

    const char *optstring = "d:b:T:O:limqew::p::t::c::f::x::sjvh";

    int ch = -1;
    while ((ch = getopt(argc, argv, optstring)) != -1) {
        switch (ch) {
            case 'd':
                device_path = optarg;
                break;

            case 'b':
                if (get_ulong(&baudrate, optarg) != 0) {
                    usage();
                    exit(EXIT_FAILURE);
                }
                break;

            case 'T':
                if (get_ulong(&throttle_ms, optarg) != 0) {
                    usage();
                    exit(EXIT_FAILURE);
                }
                break;

            case 'O':
                if (get_ulong(&timeout_ms, optarg) != 0) {
                    usage();
                    exit(EXIT_FAILURE);
                }
                break;

            case 'l':
                list_flag = true;
                break;

            case 'i':
                iden_flag = true;
                break;

            case 'e':
                link_flag = true;
                break;

            case 'm':
                monitor_flag = true;
                break;

            case 'q':
                quick_flag = true;
                break;

            case 'w':
                watch_flag = true;
                if (optarg) {
                    if (optarg[0] != '=') {
                        fprintf(stderr, "Expected -w=<period>, got: %s\n", optarg);
                        usage();
                        exit(EXIT_FAILURE);
                    }
                    watch_period = strtol(optarg + 1, NULL, 10);
                    if (watch_period < 100) {
                        fprintf(stderr, "Period must be at least 100 ms\n");
                        exit(EXIT_FAILURE);
                    }
                }
                break;

            case 'p':
                params_flag = true;
                if (optarg) {
                    if (get_register(&params_register, optarg) != 0) {
                        usage();
                        exit(EXIT_FAILURE);
                    }
                }
                break;

            case 't':
                temperature_flag = true;
                if (optarg) {
                    set_temperature = true;
                    if (optarg[0] != '=' || get_long(&temperature, optarg + 1) != 0) {
                        usage();
                        exit(EXIT_FAILURE);
                    }
                }
                break;

            case 'x':
                current_flag = true;
                if (optarg) {
                    set_current = true;
                    if (get_float(&current, optarg) != 0) {
                        usage();
                        exit(EXIT_FAILURE);
                    }
                }
                break;

            case 'c':
                cooler_flag = true;
                if (optarg) {
                    set_cooler = true;
                    if (get_mode(&cooler_mode, optarg) != 0) {
                        usage();
                        exit(EXIT_FAILURE);
                    }
                }
                break;

            case 'f':
                fan_flag = true;
                if (optarg) {
                    set_fan = true;
                    if (get_mode(&fan_mode, optarg) != 0) {
                        usage();
                        exit(EXIT_FAILURE);
                    }
                }
                break;

            case 's':
                status_flag = true;
                break;

            case 'j':
                json_flag = true;
                break;

            case 'v':
                version_flag = true;
                break;

            case 'h':
            default:
                usage();
                exit(EXIT_FAILURE);
        }
    }

    const bool needs_device = iden_flag || link_flag || monitor_flag || quick_flag || watch_flag ||
                              params_flag || temperature_flag || current_flag || cooler_flag || fan_flag ||
                              status_flag;

    if (version_flag) {
        printf("ptcc_control, library %s, protocol %s\n", rp_PtccGetVersion(),
               rp_PtccGetProtocolRevision());
        if (!needs_device && !list_flag) {
            return 0;
        }
    }

    /* listing ports needs no open connection */
    if (list_flag) {
        const int result = list_ports(json_flag);
        if (!needs_device) {
            return result;
        }
    }

    rp_ptcc_error init_result = RP_PTCC_OK;
    if (baudrate != RP_PTCC_DEFAULT_BAUDRATE) {
        init_result = rp_PtccInitEx(device_path, static_cast<uint32_t>(baudrate));
    } else if (device_path != NULL) {
        init_result = rp_PtccInitDevice(device_path);
    } else {
        init_result = rp_PtccInit();
    }

    if (init_result != RP_PTCC_OK) {
        report("Can't open PTCC controller", init_result);
        return -1;
    }

    if (throttle_ms > 0) {
        rp_PtccSetThrottle(static_cast<uint32_t>(throttle_ms));
    }
    if (timeout_ms > 0) {
        rp_PtccSetTimeout(static_cast<uint32_t>(timeout_ms));
    }

    int exit_code = 0;

    if (iden_flag) {
        if (print_iden(json_flag) != 0) {
            exit_code = -1;
        }
    }

    if (link_flag) {
        print_link_state(json_flag);
    }

    if (params_flag) {
        if (print_params(params_register, json_flag) != 0) {
            exit_code = -1;
        }
    }

    if (temperature_flag) {
        if (set_temperature) {
            const rp_ptcc_error result = rp_PtccSetSetpoint(static_cast<int32_t>(temperature));
            if (result != RP_PTCC_OK) {
                report("Can't set temperature", result);
                exit_code = -1;
            } else {
                printf("Setpoint = %ld K\n", temperature);
            }
        } else {
            float value = 0.0F;
            const rp_ptcc_error result = rp_PtccGetSetpoint(&value);
            if (result != RP_PTCC_OK) {
                report("Can't get temperature setpoint", result);
                exit_code = -1;
            } else {
                printf("Setpoint = %.3f K\n", static_cast<double>(value));
            }
        }
    }

    if (current_flag) {
        if (set_current) {
            const rp_ptcc_error result = rp_PtccSetMaxCurrent(current);
            if (result != RP_PTCC_OK) {
                report("Can't set current limit", result);
                exit_code = -1;
            } else {
                printf("TEC current limit = %.4f A\n", static_cast<double>(current));
            }
        } else {
            rp_ptcc_params_t params;
            memset(&params, 0, sizeof(params));
            const rp_ptcc_error result = rp_PtccGetParams(RP_PTCC_REG_USER_SET, &params);
            if (result != RP_PTCC_OK) {
                report("Can't read parameters", result);
                exit_code = -1;
            } else {
                printf("TEC current limit = %.4f A\n", static_cast<double>(params.i_tec_max));
            }
        }
    }

    if (cooler_flag) {
        if (set_cooler) {
            const rp_ptcc_error result = rp_PtccSetCooler(cooler_mode);
            if (result != RP_PTCC_OK) {
                report("Can't set cooler mode", result);
                exit_code = -1;
            } else {
                printf("Cooler = %s\n", ctrl_name(cooler_mode));
            }
        } else {
            rp_ptcc_params_t params;
            memset(&params, 0, sizeof(params));
            const rp_ptcc_error result = rp_PtccGetParams(RP_PTCC_REG_USER_SET, &params);
            if (result != RP_PTCC_OK) {
                report("Can't read parameters", result);
                exit_code = -1;
            } else {
                printf("Cooler = %s\n", ctrl_name(params.tec_ctrl));
            }
        }
    }

    if (fan_flag) {
        if (set_fan) {
            const rp_ptcc_error result = rp_PtccSetFan(fan_mode);
            if (result != RP_PTCC_OK) {
                report("Can't set fan mode", result);
                exit_code = -1;
            } else {
                printf("Fan = %s\n", ctrl_name(fan_mode));
            }
        } else {
            rp_ptcc_params_t params;
            memset(&params, 0, sizeof(params));
            const rp_ptcc_error result = rp_PtccGetParams(RP_PTCC_REG_USER_SET, &params);
            if (result != RP_PTCC_OK) {
                report("Can't read parameters", result);
                exit_code = -1;
            } else {
                printf("Fan = %s\n", ctrl_name(params.fan_ctrl));
            }
        }
    }

    if (quick_flag) {
        float value = 0.0F;
        const rp_ptcc_error result = rp_PtccGetTemperature(&value);
        if (result != RP_PTCC_OK) {
            report("Can't read temperature", result);
            exit_code = -1;
        } else {
            printf("%.3f\n", static_cast<double>(value));
        }
    }

    if (monitor_flag || status_flag) {
        rp_ptcc_monitor_t monitor;
        memset(&monitor, 0, sizeof(monitor));
        const rp_ptcc_error result = rp_PtccReadMonitor(&monitor);
        if (result != RP_PTCC_OK) {
            report("Can't read monitor", result);
            exit_code = -1;
        } else if (status_flag && !monitor_flag) {
            printf("%u\n", monitor.status);
        } else if (json_flag) {
            print_monitor_json(&monitor);
        } else {
            print_monitor(&monitor);
        }
    }

    if (watch_flag) {
        if (watch_monitor(watch_period, json_flag) != 0) {
            exit_code = -1;
        }
    }

    rp_PtccRelease();
    return exit_code;
}
