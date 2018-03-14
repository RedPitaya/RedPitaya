/*
command to compile:
gcc -shared -Wall -fPIC -Os -s main.c -o controllerhf.so
*/

#include <stdio.h>
#include <stdlib.h>

typedef struct rp_app_params_s
{
  char *name;
  float value;
  int fpga_update;
  int read_only;
  float min_val;
  float max_val;
}
rp_app_params_t;

const char *rp_app_desc()
{
  return "Vector Network Analyzer.\n";
}

int rp_app_init()
{
  fprintf(stderr, "Starting VNA server.\n");
  system("/opt/redpitaya/www/apps/vna/start.sh");
  return 0;
}

int rp_app_exit(void)
{
  fprintf(stderr, "Stopping VNA server.\n");
  system("/opt/redpitaya/www/apps/vna/stop.sh");
  return 0;
}

int rp_set_params(rp_app_params_t *p, int len)
{
  return 0;
}

int rp_get_params(rp_app_params_t **p)
{
  return 0;
}

int rp_get_signals(float ***s, int *sig_num, int *sig_len)
{
  return 0;
}
