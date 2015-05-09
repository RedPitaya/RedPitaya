/**
 * $Id: system.c 1061 2014-02-11 15:50:28Z ales.bardorfer $
 *
 * @brief Red Pitaya system utility library.
 *
 * @Author Ales Bardorfer <ales.bardorfer@redpitaya.com>
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <net/if.h>
#include <arpa/inet.h>

#include "redpitaya/system.h"


/* Debug channel */
#ifdef DEBUG
#  define PDEBUG(args) fprintf(stderr, args)
#else
#  define PDEBUG(args...) {}
#endif


/*----------------------------------------------------------------------------*/
/**
 * @brief Get MAC address of a specific NIC via sysfs.
 *
 * Function reads the MAC address of a specific network interface via sysfs.
 *
 * @param[in]    nic   Path to NIC address sysfs node.
 * @param[out]   mac   Mac address string in lowercase hexadecimal format,
 *                     separated with colons (:).
 *
 * @retval   0 Success
 * @retval < 0 Failure
 */
int get_mac(const char* nic, char *mac)
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


/*----------------------------------------------------------------------------*/
/**
 * @brief Get IP address of a specific NIC.
 *
 * Function reads the IP address of a specific network interface.
 *
 * @param[in]    nic   NIC string (e.g. "eth0").
 * @param[out]   ip    IP address in network byte order.
 *
 * @retval   0 Success
 * @retval < 0 Failure
 */
int get_ip(const char *nic, struct in_addr *ip)
{
    int fd;
    struct ifreq ifr;
    int ret = 0;

    ip->s_addr = 0;

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        PDEBUG("%s: Cannot open socket: %s.\n",
               __FUNCTION__, strerror(errno));
        return fd;
    }

    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, nic, IFNAMSIZ-1);

    ret = ioctl(fd, SIOCGIFADDR, &ifr);
    if (ret < 0) {
        close(fd);
        //if (errno != ENODEV) {
        //    PDEBUG("ioctl(): %s.\n", strerror(errno));
        //}
        return ret;
    }

    close(fd);

    memcpy(ip, &((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr, sizeof(*ip));

    return ret;
}


/*----------------------------------------------------------------------------*/
/**
 * @brief Get Xilinx DNA number.
 *
 * Function reads the Xilinx DNA number of Red Pitaya.
 *
 * @param[out]   dna    64-bit unsigned DNA number.
 *
 * @retval   0 Success
 * @retval < 0 Failure
 */
int get_xilinx_dna(unsigned long long *dna)
{
    void *page_ptr;
    long page_addr, page_size = sysconf(_SC_PAGESIZE);
    const long c_dna_fpga_base_addr = 0x40000000;
    const long c_dna_fpga_base_size = 0x20;
    int fd = -1;

    fd = open("/dev/mem", O_RDONLY | O_SYNC);
    if(fd < 0) {
        PDEBUG("%s: open(/dev/mem) failed: %s\n",
               __FUNCTION__, strerror(errno));
        return -1;
    }

    page_addr = c_dna_fpga_base_addr & (~(page_size-1));

    page_ptr = mmap(NULL, c_dna_fpga_base_size, PROT_READ,
                          MAP_SHARED, fd, page_addr);

    if((void *)page_ptr == MAP_FAILED) {
        PDEBUG("%s: mmap() failed: %s\n",
               __FUNCTION__, strerror(errno));
        close(fd);
        return -1;
    }

    hk_fpga_reg_mem_t *hk = page_ptr;
    *dna = hk->dna_hi & 0x00ffffff;
    *dna = *dna << 32 | hk->dna_lo;

    if(munmap(page_ptr, c_dna_fpga_base_size) < 0) {
        PDEBUG("%s: munmap() failed: %s\n", 
               __FUNCTION__, strerror(errno));
        close(fd);
        return -1;
    }

    close (fd);

    return 0;
}
