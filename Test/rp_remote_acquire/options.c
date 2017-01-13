/*
 * options.c
 *
 *  Created on: 17 Nov 2014
 *      Author: bkinman, nils
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2014 bkinman, Nils Roos
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include "options.h"

/******************************************************************************
 * static variables
 ******************************************************************************/
static struct option g_long_options[] =
{
	{"address",            required_argument, NULL, 'a'},
	{"port",               required_argument, NULL, 'p'},
	{"secondary_port",     required_argument, NULL, 'q'},
	{"mode",               required_argument, NULL, 'm'},
	{"udp",                no_argument,       NULL, 'u'},
	{"kbytes_to_transfer", required_argument, NULL, 'k'},
	{"fname",              required_argument, NULL, 'f'},
	{"secondary_fname",    required_argument, NULL, 'g'},
	{"report-rate",        no_argument,       NULL, 'r'},
	{"help",               no_argument,       NULL, 'h'},
	{"scope-channel",      required_argument, NULL, 'c'},
	{"scope-decimation",   required_argument, NULL, 'd'},
	{"scope-HV",           no_argument,       NULL, 1  },
	{"scope-no-equalizer", no_argument,       NULL, 'e'},
	{"scope-no-shaping",   no_argument,       NULL, 's'},
	{"use-rpad",           no_argument,       NULL, 'x'},
	{"load-bitstream",     optional_argument, NULL, 'l'},
	{NULL, 0, NULL, 0}
};

/******************************************************************************
 * non-static function definitions
 ******************************************************************************/
int handle_options(int argc, char *argv[], option_fields_t *options)
{
	int ch;
	int mode;

	if (argc <= 1)
		return -1;

	while ((ch = getopt_long(argc, argv, "a:p:q:m:uk:f:g:rhc:d:esxl::", g_long_options, NULL)) != -1)
	{
		// check to see if a single character or long option came through
		switch (ch)
		{
		case 'a': //Address
			strncpy(options->address, optarg ,sizeof(options->address));
			options->address[sizeof(options->address)-1]= '\0';
			break;
		case 'p': //Port
			options->port = atoi(optarg);
			break;
		case 'q': // secondary Port
			options->port2 = atoi(optarg);
			break;
		case 'm': //Mode
			mode = atoi(optarg);
			if (!strcmp("client", optarg) || mode == 1)
				options->mode = client;
			else if (!strcmp("server", optarg) || mode == 2)
				options->mode = server;
			else if (!strcmp("file", optarg) || mode == 3)
				options->mode = file;
			break;
		case 'u': //udp
			options->tcp = 0;
			break;
		case 'k': //number of kbytes to transfer
			options->kbytes_to_transfer = atol(optarg);
			break;
		case 'f': // file name
			options->fname = optarg;
			break;
		case 'g': // file name second channel
			options->fname2 = optarg;
			break;
		case 'r': // report-rate
			options->report_rate = 1;
			break;
		case 'c': // Channel
			if (!strcasecmp("a", optarg))
				options->scope_chn = 0;
			else if (!strcasecmp("b", optarg))
				options->scope_chn = 1;
			else if (!strcasecmp("ab", optarg))
				options->scope_chn = 2;
			else
				switch (atoi(optarg)) {
				case 1: options->scope_chn = 1; break;
				case 2: options->scope_chn = 2; break;
				default:options->scope_chn = 0; break;
				}
			break;
		case 'd': // Decimation
			options->scope_dec = atoi(optarg);
			break;
		case 1: // enable HV input settings
			options->scope_hv = 1;
			break;
		case 'e': // disable equalization filter
			options->scope_equalizer = 0;
			break;
		case 's': // disable shaping filter
			options->scope_shaping = 0;
			break;
		case 'x': // use rpad kernel module
			options->use_rpad = 1;
			break;
		case 'l': // load ddrdump bitstream
			if (optarg != NULL && strlen(optarg) > 0)
				options->bitstream = optarg;
			else
				options->bitstream = "./ddrdump.bit";
			break;
		case '?':
		case 'h':
		default:
			return 1;
		}
	}

	return check_options(options);
}

int check_options(option_fields_t *options)
{
	if (options->mode == client || options->mode == server) {
		if (options->port <1) {
			fprintf(stderr,"Port number invalid, or not selected (use -p)\n");
			return 1;
		}
	}

	if (options->mode == client) {
		if(0 == strncmp(options->address, "", sizeof(options->address)) )
		{
			fprintf(stderr,"No ip address provided (use -a)\n");
			return 1;
		}
	}
	else
	{
		strcpy(options->address,"127.0.0.1");
	}

	if (options->scope_chn == 2 && options->mode != client && options->mode != file) {
		fprintf(stderr,"Dual channel requires client mode or file mode\n");
		return 1;
	}

	return 0;
}

void usage(const char *name)
{
	printf("Usage: \033[1m%s [-mapqukfgrhcdes]\033[0m\n", name);
	printf("\033[1m-m  --mode <(1|client)|(2|server)|(3|file)>\033[0m\n"
	       "\toperating mode (default client)\n"
	       "\033[1m-a  --address <ip_address>\033[0m\n"
	       "\ttarget address in client mode (default empty)\n"
	       "\033[1m-p  --port <port_num>\033[0m\n"
	       "\tport number in client and server mode (default 14000)\n"
	       "\033[1m-q  --secondary_port <port_num>\033[0m\n"
	       "\tsecondary port number in dual channel client mode (default 14001)\n"
	       "\033[1m-u  --udp\033[0m\n"
	       "\tindicates tool should use udp transfer (default tcp)\n"
	       "\033[1m-k  --kbytes_to_transfer <num_kbytes>\033[0m\n"
	       "\tkilobytes to transfer (default 0 = unlimited)\n"
	       "\033[1m-f  --fname <target>\033[0m\n"
	       "\ttarget file name in file mode (default /tmp/out)\n"
	       "\033[1m-g  --secondary_fname <target>\033[0m\n"
	       "\ttarget file name for second channel in file mode (default /tmp/out2)\n"
	       "\033[1m-r  --report-rate\033[0m\n"
	       "\tturn on rate reporting (default off)\n"
	       "\033[1m-h  --help\033[0m\n"
	       "\tdisplay this usage information\n"
	       "\033[1m-c  --scope-channel <(0|A)|(1|B)(2|AB)>\033[0m\n"
	       "\tscope channel (default A)\n"
	       "\033[1m-d  --scope-decimation <decimation>\033[0m\n"
	       "\t0,1,2,4,..,65536 (default 32)\n"
	       "\033[1m-e  --scope-no-equalizer\033[0m\n"
	       "\tdisable equalization filter (default enabled)\n"
	       "\033[1m    --scope-HV\033[0m\n"
	       "\tenable HV equalizer settings (default LV)\n"
	       "\033[1m-s  --scope-no-shaping\033[0m\n"
	       "\tdisable shaping filter (default enabled)\n"
		   "\033[1m-x  --use-rpad\033[0m\n"
		   "\tuse rpad kernel module for scope access (default use /dev/mem and\n"
		   "\tassume that the top 32MB of RAM are reserved for the scope)\n"
		   "\033[1m-l  --load-bitstream [<bitstream>]\033[0m\n"
		   "\tload specified bitstream into the FPGA during startup (default do\n"
		   "\tnothing, default filename ./ddrdump.bit)\n"
	       "\n");
	printf("Examples:\n");
	printf("\033[1m%s -m 1 -a 192.168.1.1 -p 1234 -k 0 -c 0 -d 64\033[0m\n"
	       "\tconnects to a server on 192.168.1.1:1234 and streams samples from\n"
	       "\tchannel A for an unlimited time (until interrupted) at 1.953MSps\n", name);
	printf("\033[1m%s -m file -k 1024 -c 1 -d 4 -f /tmp/out.dat\033[0m\n"
	       "\twrites 1024kB from channel B to the file /tmp/out.dat at 31.25MSps\n", name);
}
