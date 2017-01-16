/*
 * scope.c
 *
 *  Created on: 26 Oct 2014
 *      Author: nils
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

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
//#include <rp_scope.h>
#define RPAD_SCOPE_CHA_BUF	0x00000000
#define RPAD_SCOPE_CHB_BUF	0x00200000

#include "options.h"
#include "scope.h"

#define RAM_A_ADDRESS           0x1e000000UL
#define RAM_A_SIZE              0x01000000UL
#define RAM_B_ADDRESS           0x1f000000UL
#define RAM_B_SIZE              0x01000000UL

static int load_bitstream(char *name);
static int set_axi_64bit_mode();
static int restore_axi_settings();
static void start_scope(struct scope_parameter *param);
static void reset_dumping(struct scope_parameter *param);
static void start_dumping(struct scope_parameter *param);

static unsigned long axi_hp0_clock;
static unsigned long axi_hp0_read_mode;
static unsigned long axi_hp0_write_mode;

int scope_init(struct scope_parameter *param, option_fields_t *options)
{
	off_t buf_a_addr, buf_b_addr;
	size_t buf_a_size, buf_b_size;
	int decimation = options->scope_dec;

	if (load_bitstream(options->bitstream)) {
		fprintf(stderr, "loading ddrdump.bit into FPGA failed, %d\n", errno);
		return -1;
	}

	if (set_axi_64bit_mode()) {
		fprintf(stderr, "setting AXI HP0 mode failed, %d\n", errno);
		return -1;
	}

	memset(param, 0, sizeof(*param));

	param->channel = options->scope_chn;
	if (options->use_rpad) {
		/* TODO get phys addr and size from sysfs */
		buf_a_addr = RPAD_SCOPE_CHA_BUF;
		buf_a_size = 0x00200000;
		buf_b_addr = RPAD_SCOPE_CHB_BUF;
		buf_b_size = 0x00200000;
		param->scope_fd = open("/dev/rpad_scope0", O_RDWR);
	} else {
		buf_a_addr = RAM_A_ADDRESS;
		buf_a_size = RAM_A_SIZE;
		buf_b_addr = RAM_B_ADDRESS;
		buf_b_size = RAM_B_SIZE;
		param->scope_fd = open("/dev/mem", O_RDWR);
	}
	if (param->scope_fd < 0) {
		fprintf(stderr, "open scope failed, %d\n", errno);
		return -1;
	}

	param->mapped_io = mmap(NULL, 0x00100000UL, PROT_WRITE | PROT_READ,
	                        MAP_SHARED, param->scope_fd, 0x40100000UL);
	if (param->mapped_io == MAP_FAILED) {
		fprintf(stderr, "mmap scope io failed (non-fatal), %d\n",
		        errno);
		param->mapped_io = NULL;
	}

	if (param->channel == 0 || param->channel == 2) {
		param->buf_a_size = buf_a_size;
		param->mapped_buf_a = mmap(NULL, param->buf_a_size, PROT_READ,
		                           MAP_SHARED, param->scope_fd,
		                           buf_a_addr);
		if (param->mapped_buf_a == MAP_FAILED) {
			fprintf(stderr,
			        "mmap scope ddr a failed (non-fatal), %d\n",
			        errno);
			param->mapped_buf_a = NULL;
		}
	}
	if (param->channel == 1 || param->channel == 2) {
		param->buf_b_size = buf_b_size;
		param->mapped_buf_b = mmap(NULL, param->buf_b_size, PROT_READ,
		                           MAP_SHARED, param->scope_fd,
		                           buf_b_addr);
		if (param->mapped_buf_b == MAP_FAILED) {
			fprintf(stderr,
			        "mmap scope ddr b failed (non-fatal), %d\n",
			        errno);
			param->mapped_buf_b = NULL;
		}
	}

	for (param->decimation = 1; decimation; decimation >>= 1)
		param->decimation <<= 1;
	param->decimation >>= 1;

	if (!param->mapped_io)
		goto out;

	/* set up scope decimation */
	*(volatile unsigned long *)(param->mapped_io + 0x0014) = param->decimation;
	if (param->decimation)
		*(volatile unsigned long *)(param->mapped_io + 0x0028) = 1;

	/* set up filters
	 * SCOPE_a_filt_aa		0x00000030UL
	 * SCOPE_a_filt_bb		0x00000034UL
	 * SCOPE_a_filt_kk		0x00000038UL
	 * SCOPE_a_filt_pp		0x0000003cUL
	 * SCOPE_b_filt_aa		0x00000040UL
	 * SCOPE_b_filt_bb		0x00000044UL
	 * SCOPE_b_filt_kk		0x00000048UL
	 * SCOPE_b_filt_pp		0x0000004cUL
	 */
	/* Equalization filter */
	if (options->scope_equalizer) {
		if (options->scope_hv) {
			/* Low gain = HV */
			*(volatile unsigned long *)(param->mapped_io + 0x0030) = 0x4C5F;
			*(volatile unsigned long *)(param->mapped_io + 0x0034) = 0x2F38B;
			*(volatile unsigned long *)(param->mapped_io + 0x0040) = 0x4C5F;
			*(volatile unsigned long *)(param->mapped_io + 0x0044) = 0x2F38B;
		} else {
			/* High gain = LV */
			*(volatile unsigned long *)(param->mapped_io + 0x0030) = 0x7D93;
			*(volatile unsigned long *)(param->mapped_io + 0x0034) = 0x437C7;
			*(volatile unsigned long *)(param->mapped_io + 0x0040) = 0x7D93;
			*(volatile unsigned long *)(param->mapped_io + 0x0044) = 0x437C7;
		}
	} else {
		*(volatile unsigned long *)(param->mapped_io + 0x0030) = 0;
		*(volatile unsigned long *)(param->mapped_io + 0x0034) = 0;
		*(volatile unsigned long *)(param->mapped_io + 0x0040) = 0;
		*(volatile unsigned long *)(param->mapped_io + 0x0044) = 0;
	}

	/* Shaping filter */
	if (options->scope_shaping) {
		*(volatile unsigned long *)(param->mapped_io + 0x0038) = 0xd9999a;
		*(volatile unsigned long *)(param->mapped_io + 0x003c) = 0x2666;
		*(volatile unsigned long *)(param->mapped_io + 0x0048) = 0xd9999a;
		*(volatile unsigned long *)(param->mapped_io + 0x004c) = 0x2666;
	} else {
		*(volatile unsigned long *)(param->mapped_io + 0x0038) = 0xffffff;
		*(volatile unsigned long *)(param->mapped_io + 0x003c) = 0;
		*(volatile unsigned long *)(param->mapped_io + 0x0048) = 0xffffff;
		*(volatile unsigned long *)(param->mapped_io + 0x004c) = 0;
	}

	if (!options->use_rpad)
		start_scope(param);

out:
	return 0;
}

void scope_cleanup(struct scope_parameter *param, option_fields_t *options)
{
	if (param->mapped_io) {
		if (!options->use_rpad)
			reset_dumping(param);
		munmap(param->mapped_io, 0x00100000UL);
	}

	if (param->mapped_buf_a)
		munmap(param->mapped_buf_a, param->buf_a_size);

	if (param->mapped_buf_b)
		munmap(param->mapped_buf_b, param->buf_b_size);

	close(param->scope_fd);

	if (restore_axi_settings()) {
		fprintf(stderr, "restoring AXI HP0 settings failed, %d\n", errno);
	}
}

static int load_bitstream(char *name)
{
	int fd_i = -1, fd_o = -1;
	void *buffer = NULL;
	const int buffer_size = 65536;
	ssize_t bytes_read, pos, bytes_written;
	int rc = -1;

	if (name == NULL)
		return 0;

	fd_o = open("/dev/xdevcfg", O_WRONLY);
	if (fd_o < 0)
		goto out;

	fd_i = open(name, O_RDONLY);
	if (fd_i < 0)
		goto out;

	buffer = malloc(buffer_size);
	if (!buffer)
		goto out;

	do {
		bytes_read = read(fd_i, buffer, buffer_size);
		if (bytes_read < 0)
			goto out;

		pos = 0;
		while (pos < bytes_read) {
			bytes_written = write(fd_o, buffer + pos, bytes_read - pos);
			if (bytes_written < 0)
				goto out;
			pos += bytes_written;
		}
	} while (bytes_read > 0);

	rc = 0;

out:
	if (buffer)
		free(buffer);
	if (fd_i >= 0)
		close(fd_i);
	if (fd_o >= 0)
		close(fd_o);
	return rc;
}

static int set_axi_64bit_mode()
{
	int fd;
	void *zynq_config;

	fd = open("/dev/mem", O_RDWR);
	if (fd < 0) {
		fprintf(stderr, "open /dev/mem failed, %d\n", errno);
		return -1;
	}

	zynq_config = mmap(NULL, 0x00010000UL, PROT_WRITE | PROT_READ, MAP_SHARED, fd,
	                   0xf8000000UL);
	if (zynq_config == MAP_FAILED) {
		fprintf(stderr, "mmap ZYNQ configuration failed, %d\n", errno);
		return -1;
	}

	/* unlock SLCR */
	*(volatile unsigned long *)(zynq_config + 0x0008) = 0x0000df0d;

	/* remember AXI_HP0 clock setting */
	axi_hp0_clock      = *(volatile unsigned long *)(zynq_config + 0x0170);

	/* remember AXI_HP0 modes */
	axi_hp0_read_mode  = *(volatile unsigned long *)(zynq_config + 0x8000);
	axi_hp0_write_mode = *(volatile unsigned long *)(zynq_config + 0x8014);

	/* program 125MHz AXI_HP0 clock 64bit read and write channel */
	*(volatile unsigned long *)(zynq_config + 0x0170) =   0x00100800;
	*(volatile unsigned long *)(zynq_config + 0x8000) &= ~0x00000001;
	*(volatile unsigned long *)(zynq_config + 0x8014) &= ~0x00000001;

	munmap(zynq_config, 0x00010000UL);
	close(fd);
	return 0;
}

static int restore_axi_settings()
{
	int fd;
	void *zynq_config;

	fd = open("/dev/mem", O_RDWR);
	if (fd < 0) {
		fprintf(stderr, "open /dev/mem failed, %d\n", errno);
		return -1;
	}

	zynq_config = mmap(NULL, 0x00010000UL, PROT_WRITE | PROT_READ, MAP_SHARED, fd,
	                   0xf8000000UL);
	if (zynq_config == MAP_FAILED) {
		fprintf(stderr, "mmap ZYNQ configuration failed, %d\n", errno);
		return -1;
	}

	/* unlock SLCR */
	*(volatile unsigned long *)(zynq_config + 0x0008) = 0x0000df0d;

	/* restore settings for AXI_HP0 clock, read and write channel */
	*(volatile unsigned long *)(zynq_config + 0x0170) = axi_hp0_clock;
	*(volatile unsigned long *)(zynq_config + 0x8000) = axi_hp0_read_mode;
	*(volatile unsigned long *)(zynq_config + 0x8014) = axi_hp0_write_mode;

	munmap(zynq_config, 0x00010000UL);
	close(fd);

	return 0;
}

static void start_scope(struct scope_parameter *param)
{
	/* load buffer addresses */
	*(volatile unsigned long *)(param->mapped_io + 0x0104) = RAM_A_ADDRESS;
	*(volatile unsigned long *)(param->mapped_io + 0x0108)
	        = RAM_A_ADDRESS + param->buf_a_size;
	*(volatile unsigned long *)(param->mapped_io + 0x010c) = RAM_B_ADDRESS;
	*(volatile unsigned long *)(param->mapped_io + 0x0110)
	        = RAM_B_ADDRESS + param->buf_b_size;

	reset_dumping(param);
	start_dumping(param);
}

/*
 * Stops the scope and resets the write pointers to the start of the buffers.
 */
static void reset_dumping(struct scope_parameter *param)
{
	/* stop scope */
	*(volatile unsigned long *)(param->mapped_io + 0x0000) = 0x00000002;
	/* activate address injection A/B */
	*(volatile unsigned long *)(param->mapped_io + 0x0100) = 0x0000000c;
	/* injection takes a few ADC cycles */
	usleep(5);
}

/*
 * Starts recording.
 */
static void start_dumping(struct scope_parameter *param)
{
	/* enable dumping on A/B */
	*(volatile unsigned long *)(param->mapped_io + 0x0100) = 0x00000003;
	/* arm scope */
	*(volatile unsigned long *)(param->mapped_io + 0x0000) = 0x00000001;
}
