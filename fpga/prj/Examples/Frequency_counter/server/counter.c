#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
  int fd;
  int log2_Ncycles;
  uint32_t phase_inc;
  double phase_in, freq_in;
  uint32_t count;
  void *cfg;
  char *name = "/dev/mem";
  const int freq = 125000000; // Hz
  int Ncycles; 

  if (argc == 3) 
  {
	log2_Ncycles = atoi(argv[1]);
	freq_in = atof(argv[2]);
  }
  else 
  {
	log2_Ncycles = 1;
	freq_in = 1.;
  }
  phase_inc = (uint32_t)(2.147482*freq_in);
  Ncycles = 1<<log2_Ncycles;

  if((fd = open(name, O_RDWR)) < 0)
  {
    perror("open");
    return 1;
  }

  cfg = mmap(NULL, sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0x42000000);
 

  *((uint32_t *)(cfg + 8)) = (0x1f & log2_Ncycles) + (phase_inc << 5);   // set log2_Ncycles and phase_inc

  count = *((uint32_t *)(cfg + 0));
  printf("Counts: %5d, cycles: %5d, frequency: %6.5f Hz\n", count, Ncycles, (double)Ncycles/count*freq);


  munmap(cfg, sysconf(_SC_PAGESIZE));

  return 0;
}
