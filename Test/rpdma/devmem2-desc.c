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

int main(int argc, char **argv) {
    int fd;
    void *map_base;
    int32_t address;
    int   number;
	
    if(argc < 3) {
       fprintf(stderr, "\nUsage:\t%s { address } [ type [ data ] ]\n"
       	"\taddress : address of first descriptor\n"
       	"\tnumber  : number of descriptors to display\n\n",
       	argv[0]);
       exit(1);
    }
    address = strtoul(argv[1], 0, 0);
    number  = strtoul(argv[2], 0, 0);

    if((fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1) FATAL;
    printf("/dev/mem opened.\n"); 
    fflush(stdout);
    
    struct xilinx_dma_desc_hw {
        uint32_t next_desc;
        uint32_t next_desc_msb;
        uint32_t buf_addr;
        uint32_t buf_addr_msb;
        uint32_t pad1;
        uint32_t pad2;
        uint32_t control;
        uint32_t status;
        uint32_t app[5];
    };

    for (int i=0; i<number; i++) {
        /* Map one page */
        map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, address & ~MAP_MASK);
        if(map_base == (void *) -1) FATAL;

        // virtual address
        struct xilinx_dma_desc_hw *hw;
        hw = map_base + (address & MAP_MASK);
        // ptintout descriptor
        printf ("descriptor %d @p:0x%08X @v:0x%p\n", i, address, hw);
        printf ("descriptor next_desc: %08x %08x\n", hw->next_desc_msb, hw->next_desc);
        printf ("descriptor buf_addr : %08x %08x\n", hw->buf_addr_msb , hw->buf_addr );
        printf ("descriptor pad 1, 2 : %08x %08x\n", hw->pad1         , hw->pad2     );
        printf ("descriptor ctl, sts : %08x %08x\n", hw->control      , hw->status   );
        printf ("descriptor app      : %08x %08x %08x %08x %08x\n", hw->app[0], hw->app[1], hw->app[2], hw->app[3], hw->app[4]);
        fflush(stdout);

        address = hw->next_desc;
        munmap (map_base, MAP_SIZE);
    }
    fflush(stdout);
    close(fd);
    return 0;
}

