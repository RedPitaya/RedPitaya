#include "spi.h"

#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string>

/*
 * The SPI master of the ADC and DAC sits in the housekeeping registers of the FPGA. They
 * are mapped through their generic-uio device: _path is /dev/uio/<node name>, as in the
 * configuration files (api@40000000). The device is looked up by its name in
 * /sys/class/uio and opened as /dev/uioN: at boot rp_power_on runs right after the
 * overlay is loaded, before udev may have made the /dev/uio/<name> link. fpga_address is
 * the physical address of the register block; it must lie in map0 of the device.
 */

#define UIO_PREFIX "/dev/uio/"
#define UIO_SYSFS "/sys/class/uio/"
#define REG_SPAN 0x10  // the address, data and read back registers

namespace {

struct Mapping {
    int fd = -1;
    void* base = MAP_FAILED;
    size_t size = 0;
    volatile uint32_t* regs = nullptr;
};

bool readSysfsLine(const std::string& path, std::string& out) {
    FILE* f = fopen(path.c_str(), "r");
    if (f == NULL) {
        return false;
    }
    char buf[128] = {0};
    bool ok = fgets(buf, sizeof(buf), f) != NULL;
    fclose(f);
    if (!ok) {
        return false;
    }
    out = buf;
    while (!out.empty() && (out.back() == '\n' || out.back() == '\r')) {
        out.pop_back();
    }
    return true;
}

bool readSysfsHex(const std::string& path, unsigned long long& value) {
    std::string s;
    if (!readSysfsLine(path, s)) {
        return false;
    }
    return sscanf(s.c_str(), "%llx", &value) == 1;
}

// /dev/uio/<name> -> uioN by the name in sysfs
bool findUio(const char* path, std::string& uio) {
    if (strncmp(path, UIO_PREFIX, strlen(UIO_PREFIX)) != 0) {
        fprintf(stderr, "[rp_spi] %s is not a uio device (" UIO_PREFIX "<name>)\n", path);
        return false;
    }
    std::string name = path + strlen(UIO_PREFIX);
    DIR* dir = opendir(UIO_SYSFS);
    if (dir == NULL) {
        fprintf(stderr, "[rp_spi] Error open " UIO_SYSFS "\n");
        return false;
    }
    bool found = false;
    while (struct dirent* e = readdir(dir)) {
        if (strncmp(e->d_name, "uio", 3) != 0) {
            continue;
        }
        std::string n;
        if (readSysfsLine(std::string(UIO_SYSFS) + e->d_name + "/name", n) && n == name) {
            uio = e->d_name;
            found = true;
            break;
        }
    }
    closedir(dir);
    if (!found) {
        fprintf(stderr, "[rp_spi] No uio device %s (is the bitstream loaded?)\n", name.c_str());
    }
    return found;
}

int mapRegs(const char* path, unsigned int fpga_address, unsigned short dev_address, Mapping& m) {
    std::string uio;
    if (!findUio(path, uio)) {
        return -3;
    }
    unsigned long long addr = 0, size = 0;
    std::string maps = std::string(UIO_SYSFS) + uio + "/maps/map0/";
    if (!readSysfsHex(maps + "addr", addr) || !readSysfsHex(maps + "size", size)) {
        fprintf(stderr, "[rp_spi] Error reading %s{addr,size}\n", maps.c_str());
        return -3;
    }
    unsigned long long first = (unsigned long long)fpga_address + dev_address;
    if (first < addr || first + REG_SPAN > addr + size) {
        fprintf(stderr, "[rp_spi] 0x%llX is outside %s (0x%llX + 0x%llX)\n", first, path, addr, size);
        return -2;
    }
    std::string dev = "/dev/" + uio;
    m.fd = open(dev.c_str(), O_RDWR | O_SYNC);
    if (m.fd == -1) {
        fprintf(stderr, "[rp_spi] Error open %s\n", dev.c_str());
        return -3;
    }
    m.size = size;
    m.base = mmap(NULL, m.size, PROT_READ | PROT_WRITE, MAP_SHARED, m.fd, 0);
    if (m.base == MAP_FAILED) {
        fprintf(stderr, "[rp_spi] Error mmap %s\n", dev.c_str());
        return -2;
    }
    m.regs = (volatile uint32_t*)((char*)m.base + (first - addr));
    return 0;
}

int unmapRegs(Mapping& m) {
    int retval = 0;
    if (m.base != MAP_FAILED) {
        if (munmap(m.base, m.size) == -1) {
            retval = -1;
        }
        m.base = MAP_FAILED;
    }
    if (m.fd != -1) {
        close(m.fd);
        m.fd = -1;
    }
    return retval;
}

}  // namespace

int write_to_fpga_spi(const char* _path, unsigned int fpga_address, unsigned short dev_address, char a_addr, uint8_t spi_val_to_write) {
    Mapping m;
    int retval = mapRegs(_path, fpga_address, dev_address, m);
    if (retval == 0) {
        m.regs[0] = (uint32_t)a_addr;
        usleep(1000);
        m.regs[1] = (uint32_t)spi_val_to_write;
    }
    int r = unmapRegs(m);
    return retval != 0 ? retval : r;
}

int read_from_fpga_spi(const char* _path, unsigned int fpga_address, unsigned short dev_address, char a_addr, uint8_t* value) {
    Mapping m;
    int retval = mapRegs(_path, fpga_address, dev_address, m);
    if (retval == 0) {
        m.regs[0] = (uint32_t)a_addr + 0x8000;
        m.regs[1] = 0x44;
        usleep(1000);
        *value = (uint8_t)m.regs[2];
    }
    int r = unmapRegs(m);
    return retval != 0 ? retval : r;
}
