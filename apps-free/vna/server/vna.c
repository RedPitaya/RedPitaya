#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

volatile uint16_t *rx_cntr;
volatile float *rx_data;

int sock_thread = -1;
uint32_t rate_thread = 5;
uint32_t size_thread = 6000;

void *read_handler(void *arg);

int main(int argc, char *argv[])
{
  int fd, sock_server, sock_client;
  struct sched_param param;
  pthread_attr_t attr;
  pthread_t thread;
  volatile void *cfg, *sts;
  volatile uint32_t *rx_freq, *rx_size, *tx_phase[2];
  volatile int16_t *tx_level[2];
  volatile uint8_t *rst, *gpio;
  struct sockaddr_in addr;
  uint32_t command, freq, rate, size, i;
  int32_t value, corr;
  int64_t start, stop;
  int yes = 1;

  if((fd = open("/dev/mem", O_RDWR)) < 0)
  {
    perror("open");
    return EXIT_FAILURE;
  }

  sts = mmap(NULL, sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0x40000000);
  cfg = mmap(NULL, sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0x40001000);
  rx_data = mmap(NULL, 2*sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0x40002000);
  rx_freq = mmap(NULL, 32*sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0x40020000);

  rx_cntr = ((uint16_t *)(sts + 12));

  rst = ((uint8_t *)(cfg + 0));
  gpio = ((uint8_t *)(cfg + 1));
  rx_size = ((uint32_t *)(cfg + 4));
  tx_phase[0] = ((uint32_t *)(cfg + 8));
  tx_phase[1] = ((uint32_t *)(cfg + 12));
  tx_level[0] = ((int16_t *)(cfg + 16));
  tx_level[1] = ((int16_t *)(cfg + 18));

  *tx_phase[0] = 0;
  *tx_phase[1] = 0;
  *tx_level[0] = 32766;
  *tx_level[1] = 0;
  *rx_size = 25000 - 1;

  start = 10000;
  stop = 60000000;
  size = 6000;
  rate = 5;
  corr = 0;

  *rst &= ~3;
  *rst |= 4;
  *gpio = 0;

  if((sock_server = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    perror("socket");
    return EXIT_FAILURE;
  }

  setsockopt(sock_server, SOL_SOCKET, SO_REUSEADDR, (void *)&yes , sizeof(yes));

  /* setup listening address */
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(1001);

  if(bind(sock_server, (struct sockaddr *)&addr, sizeof(addr)) < 0)
  {
    perror("bind");
    return EXIT_FAILURE;
  }

  listen(sock_server, 1024);

  pthread_attr_init(&attr);
  pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
  pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
  param.sched_priority = 99;
  pthread_attr_setschedparam(&attr, &param);

  while(1)
  {
    if((sock_client = accept(sock_server, NULL, NULL)) < 0)
    {
      perror("accept");
      return EXIT_FAILURE;
    }

    while(1)
    {
      if(recv(sock_client, (char *)&command, 4, MSG_WAITALL) <= 0) break;
      value = command & 0xfffffff;
      switch(command >> 28)
      {
        case 0:
          /* set start */
          if(value < 0 || value > 62500000) continue;
          start = value;
          break;
        case 1:
          /* set stop */
          if(value < 0 || value > 62500000) continue;
          stop = value;
          break;
        case 2:
          /* set size */
          if(value < 1 || value > 32768) continue;
          size = value;
          break;
        case 3:
          /* set rate */
          if(value < 5 || value > 50000) continue;
          rate = value;
          *rx_size = 2500 * (rate + 5) - 1;
          break;
        case 4:
          /* set correction */
          if(value < -100000 || value > 100000) continue;
          corr = value;
          break;
        case 5:
          /* set phase */
          if(value < 0 || value > 360) continue;
          *tx_phase[0] = (uint32_t)floor(value / 360.0 * (1<<30) + 0.5);
          break;
        case 6:
          /* set phase */
          if(value < 0 || value > 360) continue;
          *tx_phase[1] = (uint32_t)floor(value / 360.0 * (1<<30) + 0.5);
          break;
        case 7:
          /* set level */
          if(value < -32766 || value > 32766) continue;
          *tx_level[0] = value;
          break;
        case 8:
          /* set level */
          if(value < -32766 || value > 32766) continue;
          *tx_level[1] = value;
          break;
        case 9:
          /* set gpio */
          if(value < 0 || value > 255) continue;
          *gpio = value;
          break;
        case 10:
          /* sweep */
          *rst &= ~3;
          *rst |= 4;
          *rst &= ~4;
          *rst |= 2;
          rate_thread = rate;
          size_thread = size;
          sock_thread = sock_client;
          if(pthread_create(&thread, &attr, read_handler, NULL) < 0)
          {
            perror("pthread_create");
            return EXIT_FAILURE;
          }
          pthread_detach(thread);
          for(i = 0; i < size; ++i)
          {
            freq = start + (stop - start) * i / (size - 1);
            freq *= (1.0 + 1.0e-9 * corr);
            *rx_freq = (uint32_t)floor(freq / 125.0e6 * (1<<30) + 0.5);
          }
          *rst |= 1;
          break;
        case 11:
          /* cancel */
          *rst &= ~3;
          *rst |= 4;
          sock_thread = -1;
          break;
      }
    }

    *rst &= ~3;
    *rst |= 4;
    *gpio = 0;
    sock_thread = -1;
    close(sock_client);
  }

  close(sock_server);

  return EXIT_SUCCESS;
}

void *read_handler(void *arg)
{
  uint32_t i, j, cntr;
  uint32_t rate = rate_thread;
  uint32_t size = size_thread;
  float buffer[4];

  i = 0;
  cntr = 0;
  while(cntr < size * (rate + 5))
  {
    if(sock_thread < 0) break;

    if(*rx_cntr < 4)
    {
      usleep(1000);
      continue;
    }

    if(i < 5)
    {
      for(j = 0; j < 4; ++j)
      {
        buffer[j] = *rx_data;
      }
      memset(buffer, 0, 16);
    }
    else
    {
      for(j = 0; j < 4; ++j)
      {
        buffer[j] += *rx_data;
      }
    }

    ++i;
    ++cntr;

    if(i < rate + 5) continue;

    i = 0;

    for(j = 0; j < 4; ++j)
    {
      buffer[j] /= rate;
    }

    if(send(sock_thread, buffer, 16, MSG_NOSIGNAL) < 0) break;
  }

  return NULL;
}
