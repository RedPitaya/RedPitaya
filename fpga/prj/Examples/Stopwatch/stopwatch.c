#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
  int fd;
  float wait_time;
  uint32_t count;
  void *cfg;
  char *name = "/dev/mem";
  const int freq = 124998750; // Hz


  if (argc == 2) wait_time = atof(argv[1]);
  else wait_time = 1.0;

  if((fd = open(name, O_RDWR)) < 0)
  {
    perror("open");
    return 1;
  }

  cfg = mmap(NULL, sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0x42000000);

  *((uint32_t *)(cfg + 0)) = 1<<1;   // clear timer
  *((uint32_t *)(cfg + 0)) = 1<<0;   // start timer

  sleep(wait_time);   // wait [wait_time] seconds

  *((uint32_t *)(cfg + 0)) = 0<<0;   // stop timer


  count = *((uint32_t *)(cfg + 8));  // get binary counter output
  printf("Clock count: %5d, calculated time: %5f s\n", count, (double)count/freq);


  munmap(cfg, sysconf(_SC_PAGESIZE));
  return 0;
}
