/*
 * devmem2.c: Simple program to read/write from/to any location in memory.
 *
 *  Copyright (C) 2000, Jan-Derk Bakker (jdb@lartmaker.nl)
 *
 *
 * This software has been developed for the LART computing board
 * (http://www.lart.tudelft.nl/). The development has been sponsored by
 * the Mobile MultiMedia Communications (http://www.mmc.tudelft.nl/)
 * and Ubiquitous Communications (http://www.ubicom.tudelft.nl/)
 * projects.
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <ctype.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <stdint.h>
  
#define FATAL do { fprintf(stderr, "Error at line %d, file %s (%d) [%s]\n", \
  __LINE__, __FILE__, errno, strerror(errno)); exit(1); } while(0)
 
#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)

int main() {
    int fd;
    void *map_base;
    int address = 0xE000A000;
    volatile int *regset;
	
    if((fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1) FATAL;
    printf("/dev/mem opened.\n"); 
    fflush(stdout);
    
    /* Map one page */
    map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, address & ~MAP_MASK);
    if(map_base == (void *) -1) FATAL;

    regset = (int *) map_base + (address & MAP_MASK);

    printf ("MASK_DATA_0_LSW @0x%04x = 0x%08x 32 mixed x          Maskable Output Data         (GPIO Bank0, MIO, Lower 16bits) \n", 0x0000, regset[0x0000>>2]);
    printf ("MASK_DATA_0_MSW @0x%04x = 0x%08x 32 mixed x          Maskable Output Data         (GPIO Bank0, MIO, Upper 16bits) \n", 0x0004, regset[0x0004>>2]);
    printf ("MASK_DATA_1_LSW @0x%04x = 0x%08x 32 mixed x          Maskable Output Data         (GPIO Bank1, MIO, Lower 16bits) \n", 0x0008, regset[0x0008>>2]);
    printf ("MASK_DATA_1_MSW @0x%04x = 0x%08x 22 mixed x          Maskable Output Data         (GPIO Bank1, MIO, Upper 6bits)  \n", 0x000C, regset[0x000C>>2]);
    printf ("MASK_DATA_2_LSW @0x%04x = 0x%08x 32 mixed 0x00000000 Maskable Output Data         (GPIO Bank2, EMIO, Lower 16bits)\n", 0x0010, regset[0x0010>>2]);
    printf ("MASK_DATA_2_MSW @0x%04x = 0x%08x 32 mixed 0x00000000 Maskable Output Data         (GPIO Bank2, EMIO, Upper 16bits)\n", 0x0014, regset[0x0014>>2]);
    printf ("MASK_DATA_3_LSW @0x%04x = 0x%08x 32 mixed 0x00000000 Maskable Output Data         (GPIO Bank3, EMIO, Lower 16bits)\n", 0x0018, regset[0x0018>>2]);
    printf ("MASK_DATA_3_MSW @0x%04x = 0x%08x 32 mixed 0x00000000 Maskable Output Data         (GPIO Bank3, EMIO, Upper 16bits)\n", 0x001C, regset[0x001C>>2]);
    printf ("DATA_0          @0x%04x = 0x%08x 32 rw    x          Output Data                  (GPIO Bank0, MIO)               \n", 0x0040, regset[0x0040>>2]);
    printf ("DATA_1          @0x%04x = 0x%08x 22 rw    x          Output Data                  (GPIO Bank1, MIO)               \n", 0x0044, regset[0x0044>>2]);
    printf ("DATA_2          @0x%04x = 0x%08x 32 rw    0x00000000 Output Data                  (GPIO Bank2, EMIO)              \n", 0x0048, regset[0x0048>>2]);
    printf ("DATA_3          @0x%04x = 0x%08x 32 rw    0x00000000 Output Data                  (GPIO Bank3, EMIO)              \n", 0x004C, regset[0x004C>>2]);
    printf ("DATA_0_RO       @0x%04x = 0x%08x 32 ro    x          Input Data                   (GPIO Bank0, MIO)               \n", 0x0060, regset[0x0060>>2]);
    printf ("DATA_1_RO       @0x%04x = 0x%08x 22 ro    x          Input Data                   (GPIO Bank1, MIO)               \n", 0x0064, regset[0x0064>>2]);
    printf ("DATA_2_RO       @0x%04x = 0x%08x 32 ro    0x00000000 Input Data                   (GPIO Bank2, EMIO)              \n", 0x0068, regset[0x0068>>2]);
    printf ("DATA_3_RO       @0x%04x = 0x%08x 32 ro    0x00000000 Input Data                   (GPIO Bank3, EMIO)              \n", 0x006C, regset[0x006C>>2]);
    printf ("DIRM_0          @0x%04x = 0x%08x 32 rw    0x00000000 Direction mode               (GPIO Bank0, MIO)               \n", 0x0204, regset[0x0204>>2]);
    printf ("OEN_0           @0x%04x = 0x%08x 32 rw    0x00000000 Output enable                (GPIO Bank0, MIO)               \n", 0x0208, regset[0x0208>>2]);
    printf ("INT_MASK_0      @0x%04x = 0x%08x 32 ro    0x00000000 Interrupt Mask Status        (GPIO Bank0, MIO)               \n", 0x020C, regset[0x020C>>2]);
    printf ("INT_EN_0        @0x%04x = 0x%08x 32 wo    0x00000000 Interrupt Enable/Unmask      (GPIO Bank0, MIO)               \n", 0x0210, regset[0x0210>>2]);
    printf ("INT_DIS_0       @0x%04x = 0x%08x 32 wo    0x00000000 Interrupt Disable/Mask       (GPIO Bank0, MIO)               \n", 0x0214, regset[0x0214>>2]);
    printf ("INT_STAT_0      @0x%04x = 0x%08x 32 wtc   0x00000000 Interrupt Status             (GPIO Bank0, MIO)               \n", 0x0218, regset[0x0218>>2]);
    printf ("INT_TYPE_0      @0x%04x = 0x%08x 32 rw    0xFFFFFFFF Interrupt Type               (GPIO Bank0, MIO)               \n", 0x021C, regset[0x021C>>2]);
    printf ("INT_POLARITY_0  @0x%04x = 0x%08x 32 rw    0x00000000 Interrupt Polarity           (GPIO Bank0, MIO)               \n", 0x0220, regset[0x0220>>2]);
    printf ("INT_ANY_0       @0x%04x = 0x%08x 32 rw    0x00000000 Interrupt Any Edge Sensitive (GPIO Bank0, MIO)               \n", 0x0224, regset[0x0224>>2]);
    printf ("DIRM_1          @0x%04x = 0x%08x 22 rw    0x00000000 Direction mode               (GPIO Bank1, MIO)               \n", 0x0244, regset[0x0244>>2]);
    printf ("OEN_1           @0x%04x = 0x%08x 22 rw    0x00000000 Output enable                (GPIO Bank1, MIO)               \n", 0x0248, regset[0x0248>>2]);
    printf ("INT_MASK_1      @0x%04x = 0x%08x 22 ro    0x00000000 Interrupt Mask Status        (GPIO Bank1, MIO)               \n", 0x024C, regset[0x024C>>2]);
    printf ("INT_EN_1        @0x%04x = 0x%08x 22 wo    0x00000000 Interrupt Enable/Unmask      (GPIO Bank1, MIO)               \n", 0x0250, regset[0x0250>>2]);
    printf ("INT_DIS_1       @0x%04x = 0x%08x 22 wo    0x00000000 Interrupt Disable/Mask       (GPIO Bank1, MIO)               \n", 0x0254, regset[0x0254>>2]);
    printf ("INT_STAT_1      @0x%04x = 0x%08x 22 wtc   0x00000000 Interrupt Status             (GPIO Bank1, MIO)               \n", 0x0258, regset[0x0258>>2]);
    printf ("INT_TYPE_1      @0x%04x = 0x%08x 22 rw    0x003FFFFF Interrupt Type               (GPIO Bank1, MIO)               \n", 0x025C, regset[0x025C>>2]);
    printf ("INT_POLARITY_1  @0x%04x = 0x%08x 22 rw    0x00000000 Interrupt Polarity           (GPIO Bank1, MIO)               \n", 0x0260, regset[0x0260>>2]);
    printf ("INT_ANY_1       @0x%04x = 0x%08x 22 rw    0x00000000 Interrupt Any Edge Sensitive (GPIO Bank1, MIO)               \n", 0x0264, regset[0x0264>>2]);
    printf ("DIRM_2          @0x%04x = 0x%08x 32 rw    0x00000000 Direction mode               (GPIO Bank2, EMIO)              \n", 0x0284, regset[0x0284>>2]);
    printf ("OEN_2           @0x%04x = 0x%08x 32 rw    0x00000000 Output enable                (GPIO Bank2, EMIO)              \n", 0x0288, regset[0x0288>>2]);
    printf ("INT_MASK_2      @0x%04x = 0x%08x 32 ro    0x00000000 Interrupt Mask Status        (GPIO Bank2, EMIO)              \n", 0x028C, regset[0x028C>>2]);
    printf ("INT_EN_2        @0x%04x = 0x%08x 32 wo    0x00000000 Interrupt Enable/Unmask      (GPIO Bank2, EMIO)              \n", 0x0290, regset[0x0290>>2]);
    printf ("INT_DIS_2       @0x%04x = 0x%08x 32 wo    0x00000000 Interrupt Disable/Mask       (GPIO Bank2, EMIO)              \n", 0x0294, regset[0x0294>>2]);
    printf ("INT_STAT_2      @0x%04x = 0x%08x 32 wtc   0x00000000 Interrupt Status             (GPIO Bank2, EMIO)              \n", 0x0298, regset[0x0298>>2]);
    printf ("INT_TYPE_2      @0x%04x = 0x%08x 32 rw    0xFFFFFFFF Interrupt Type               (GPIO Bank2, EMIO)              \n", 0x029C, regset[0x029C>>2]);
    printf ("INT_POLARITY_2  @0x%04x = 0x%08x 32 rw    0x00000000 Interrupt Polarity           (GPIO Bank2, EMIO)              \n", 0x02A0, regset[0x02A0>>2]);
    printf ("INT_ANY_2       @0x%04x = 0x%08x 32 rw    0x00000000 Interrupt Any Edge Sensitive (GPIO Bank2, EMIO)              \n", 0x02A4, regset[0x02A4>>2]);
    printf ("DIRM_3          @0x%04x = 0x%08x 32 rw    0x00000000 Direction mode               (GPIO Bank3, EMIO)              \n", 0x02C4, regset[0x02C4>>2]);
    printf ("OEN_3           @0x%04x = 0x%08x 32 rw    0x00000000 Output enable                (GPIO Bank3, EMIO)              \n", 0x02C8, regset[0x02C8>>2]);
    printf ("INT_MASK_3      @0x%04x = 0x%08x 32 ro    0x00000000 Interrupt Mask Status        (GPIO Bank3, EMIO)              \n", 0x02CC, regset[0x02CC>>2]);
    printf ("INT_EN_3        @0x%04x = 0x%08x 32 wo    0x00000000 Interrupt Enable/Unmask      (GPIO Bank3, EMIO)              \n", 0x02D0, regset[0x02D0>>2]);
    printf ("INT_DIS_3       @0x%04x = 0x%08x 32 wo    0x00000000 Interrupt Disable/Mask       (GPIO Bank3, EMIO)              \n", 0x02D4, regset[0x02D4>>2]);
    printf ("INT_STAT_3      @0x%04x = 0x%08x 32 wtc   0x00000000 Interrupt Status             (GPIO Bank3, EMIO)              \n", 0x02D8, regset[0x02D8>>2]);
    printf ("INT_TYPE_3      @0x%04x = 0x%08x 32 rw    0xFFFFFFFF Interrupt Type               (GPIO Bank3, EMIO)              \n", 0x02DC, regset[0x02DC>>2]);
    printf ("INT_POLARITY_3  @0x%04x = 0x%08x 32 rw    0x00000000 Interrupt Polarity           (GPIO Bank3, EMIO)              \n", 0x02E0, regset[0x02E0>>2]);
    printf ("INT_ANY_3       @0x%04x = 0x%08x 32 rw    0x00000000 Interrupt Any Edge Sensitive (GPIO Bank3, EMIO)              \n", 0x02E4, regset[0x02E4>>2]);
    fflush(stdout);

    munmap (map_base, MAP_SIZE);
    fflush(stdout);
    close(fd);
    return 0;
}

