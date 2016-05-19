/* Tester for the AC97 submodule of the FPGA, written by DF4IAH */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/mman.h>


int fpga_mmap_area(int* fd, void** mem, long base_addr, long base_size)
{
  const long page_size = sysconf(_SC_PAGESIZE);

  *fd = open("/dev/mem", O_RDWR | O_SYNC);
  if (*fd < 0) {
    fprintf(stderr, "ERROR - fpga_mmap_area - open(/dev/mem) failed: %s\n", strerror(errno));
    return -1;
  }
  long page_addr = base_addr & (~(page_size-1));
  long page_offs = base_addr - page_addr;

  void* page_ptr = mmap(NULL, page_offs + base_size, PROT_READ | PROT_WRITE,
			MAP_SHARED, *fd, page_addr);
  if (page_ptr == MAP_FAILED) {
    fprintf(stderr, "ERROR - fpga_mmap_area - mmap() failed: %s\n", strerror(errno));
    return -2;
  }

  *mem = page_ptr + page_offs;
  return 0;
}

int fpga_munmap_area(int* fd, void** mem, long base_addr, long base_size)
{
  if (*mem) {
    const long page_size = sysconf(_SC_PAGESIZE);

    long page_addr = base_addr & (~(page_size-1));
    long page_offs = base_addr - page_addr;

    if (munmap(*mem, page_offs + base_size) < 0) {
      fprintf(stderr, "ERROR - fpga_munmap_area - munmap() failed: %s\n", strerror(errno));
      return -1;
    }
    *mem = NULL;
  }

  if (*fd >= 0) {
    close(*fd);
    *fd = -1;
  }
  return 0;
}


int main() 
{
  void *mem = NULL;
  int fd = 0;
  const uint32_t base_addr = 0x40700000;
  const uint32_t base_size = 0x00010000;
  int i;
  void *addr;
  uint32_t data;

  printf("001\n");
  fpga_mmap_area(&fd, &mem, base_addr, base_size);
  printf("002: mem = 0x%p\n", mem);

  for (i = 0; i < 256; ++i) {
    addr = mem + ((i << 2) % 256);
    data = *(uint32_t *)addr;
    printf("READ addr = 0x%p --> data = 0x%08x \n", addr, data);
  }

  fpga_munmap_area(&fd, &mem, base_addr, base_size);
  return 0;
}
