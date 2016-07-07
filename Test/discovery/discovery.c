#include <cstdlib>
#include <cstdio>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <map>

#include <unistd.h>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <netdb.h>
#include <ifaddrs.h>

#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>

#include "redpitaya/rp.h"
#include "../../api/rpbase/src/common.h"


/* FPGA housekeeping structure - Bazaar relevant part */
typedef struct hk_fpga_reg_mem_s {
    /* configuration:
     * bit   [3:0] - hw revision
     * bits [31:4] - reserved
     */
    uint32_t rev;
    /* DNA low word */
    uint32_t dna_lo;
    /* DNA high word */
    uint32_t dna_hi;
} hk_fpga_reg_mem_t;


int get_xilinx_dna(unsigned long long *dna)
{
    void *page_ptr;
    long page_addr, page_size = sysconf(_SC_PAGESIZE);
    const long c_dna_fpga_base_addr = 0x40000000;
    const long c_dna_fpga_base_size = 0x20;
    int fd = -1;

    fd = open("/dev/mem", O_RDONLY | O_SYNC);
    if(fd < 0) {
        return -1;
    }

    page_addr = c_dna_fpga_base_addr & (~(page_size-1));

    page_ptr = mmap(NULL, c_dna_fpga_base_size, PROT_READ,
                          MAP_SHARED, fd, page_addr);

    if((void *)page_ptr == MAP_FAILED) {
        close(fd);
        return -1;
    }

    hk_fpga_reg_mem_t *hk =(hk_fpga_reg_mem_t *)page_ptr;
    *dna = hk->dna_hi & 0x01ffffff;

    *dna = *dna << 32 | hk->dna_lo;

    if(munmap(page_ptr, c_dna_fpga_base_size) < 0) {
        close(fd);
        return -1;
    }

    close (fd);

    return 0;
}

std::string GetOSVersion(const std::string path)
{
    std::ifstream infile(path);
    std::string line;
    int count = 0;
    std::string simbol;
    std::stringstream ss;

    // Get 3d string from file
    for(int i=0; i<3; i++)
    {
        std::getline(infile, line);
    }

    infile.close();

    for(size_t i=0; i<line.length(); i++)
    {
        simbol = line[i];
        if(count < 3)
        {
            if(simbol == "\"")
                count++;
            continue;
        }

        if(simbol != "-")
        {
            ss << simbol;
            continue;
        }
        else
            break;
    }

    return ss.str();
}

std::string GetUrl()
{
    std::string defaulturl("http://account.redpitaya.com/discovery.php");
    std::ifstream f("/opt/redpitaya/www/conf/testurl");
    if (f.good()) {
        std::string line;
        std::getline(f, line);
        f.close();
        return line;
    } else {
        f.close();

        return defaulturl;
    }
}

std::string GetOSBuild(const std::string path)
{
    std::ifstream infile(path);
    std::string line;
    bool riched = false;
    std::string simbol;
    std::stringstream ss;

    // Get 3d string from file
    for(int i=0; i<3; i++)
    {
        std::getline(infile, line);
    }

    infile.close();

    for(size_t i=0; i<line.length(); i++)
    {
        simbol = line[i];
        if(!riched)
        {
            if(simbol == "-")
                riched = true;
            continue;
        }

        if(simbol != "\"")
        {
            ss << simbol;
            continue;
        }
        else
            break;
    }

    return ss.str();
}

//Function returns MAC address
long GetMACAddressLinux(const char* nic, char *mac)
{
    FILE *fp;
    ssize_t read;
    const size_t len = 17;

    fp = fopen(nic, "r");
    if(fp == NULL) {
        return -1;
    }

    read = fread(mac, len, 1, fp);
    if(read != 1) {
        fclose(fp);
        return -2;
    }
    mac[len] = '\0';

    fclose(fp);

    return 0;
}

int main(int argc, char **argv)
{
    std::string vDNA;
    std::string vMAC_LAN;
    std::string vMAC_WIFI;
    std::string vIP_LAN;
    std::string vIP_WIFI;
    std::string vOS_VER;
    std::string vOS_BUILD;

    bool verbose = false;

    if (argc == 2)
    {
        std::string param(argv[1]);
        if (param == "-v" || param == "--verbose")
            verbose = true;
    }

    unsigned long long dna;
    char string_buffer[18];
    int res;

    // Get DNA
    get_xilinx_dna(&dna);
    sprintf(string_buffer, "%016llX", dna);
    vDNA = string_buffer;

    // Get MAC_LAN
    res = 0;
    sprintf(string_buffer, "00:00:00:00:00:00");
    res = GetMACAddressLinux("/sys/class/net/eth0/address", string_buffer);
    if(!res)
        vMAC_LAN = string_buffer;

    // Get MAC_WIFI
    res = 0;
    sprintf(string_buffer, "00:00:00:00:00:00");
    res = GetMACAddressLinux("/sys/class/net/wlan0/address", string_buffer);
    if(!res)
        vMAC_WIFI = string_buffer;


	// Get LAN IP
    struct ifaddrs* ifAddrStruct = 0;
    void* tmp_ptr = 0;      
    getifaddrs(&ifAddrStruct);

    std::stringstream whole_struct;

    for (ifaddrs* ifa = ifAddrStruct; ifa; ifa = ifa->ifa_next) {
        // if (ifa->ifa_addr->sa_family == AF_INET) {
            tmp_ptr = &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
            char addr_buf[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, tmp_ptr, addr_buf, INET_ADDRSTRLEN);

            whole_struct << ifa->ifa_name << ": " << addr_buf << std::endl;

            if (ifa->ifa_addr->sa_family == AF_INET) {
				if (!strcmp(ifa->ifa_name, "eth0") && addr_buf[1] != '6') // exclude 169...
					vIP_LAN = addr_buf;
				if(!strcmp(ifa->ifa_name, "wlan0wext"))
					vIP_WIFI = addr_buf;
			}
		// }
    }
    if (ifAddrStruct) 
        freeifaddrs(ifAddrStruct);

    vOS_VER = GetOSVersion("/opt/redpitaya/www/apps/info/info.json");
    vOS_BUILD = GetOSBuild("/opt/redpitaya/www/apps/info/info.json");
    /*
    std::cout << "DNA: " << vDNA << std::endl;
    std::cout << "LAN IP: " << vIP_LAN << std::endl;
    std::cout << "WIFI IP: " << vIP_WIFI << std::endl;
    std::cout << "LAN MAC: " << vMAC_LAN << std::endl;
    std::cout << "WIFI MAC: " << vMAC_WIFI << std::endl;
    std::cout << "OS VER: " << vOS_VER << std::endl;
    std::cout << "OS BUILD: " << vOS_BUILD << std::endl;*/

    // Build curl command
    std::stringstream cmd;
    cmd << "curl '" << GetUrl() << "?dna=" << vDNA << "&";
    cmd << "ip_lan=" << vIP_LAN << "&";
    cmd << "mac_lan=" << vMAC_LAN << "&";
    cmd << "os_ver=" << vOS_VER << "&";
    cmd << "os_build=" << vOS_BUILD;

    if(vIP_WIFI != "")
        cmd << "&ip_wifi=" << vIP_WIFI;
    if(vIP_WIFI != "")
        cmd << "&mac_wifi=" << vMAC_WIFI;

    cmd << "'";

    if (verbose) {
        std::cout << "Executing: " << cmd.str().c_str() << std::endl;
        std::cout << "getifaddrs() returned: " << std::endl << std::endl;
        std::cout << whole_struct.str();
    }

    system(cmd.str().c_str());

    return 0;
}

