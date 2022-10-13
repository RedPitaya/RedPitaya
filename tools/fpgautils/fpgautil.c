/******************************************************************************
 *
 * Copyright (C) 2019-2020 Xilinx, Inc.  All rights reserved.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 ******************************************************************************/
/*****************************************************************************/
/**
 * @file: fpgautil.c
 * Simple command line tool to load fpga via overlay or through sysfs interface
 * and read fpga configuration using Xilinx Zynq/ZynqMP fpga manager
 * Author: Appana Durga Kedareswara Rao <appanad@xilinx.com>
 * Author: Nava kishore Manne <navam@xilinx.com>
 */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <getopt.h>
#include <poll.h>
#include <ctype.h>
#include <libgen.h>
#include <time.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/stat.h>

#define OVERLAY      1
#define FPGA_SYSFS   2
#define READBACK     3
#define ENCRYPTION_USERKEY_EN		(0x20U)

int fpga_getplatform()
{
        char fpstr[100];
        FILE *fptr;
        char *zynqmpstr = "Xilinx ZynqMP FPGA Manager";

        if ((fptr = fopen("/sys/class/fpga_manager/fpga0/name", "r")) == NULL)
        {
                printf("Error! opening file");
                // Program exits if file pointer returns NULL.
                exit(1);
         }

        // reads text until newline
        fscanf(fptr,"%[^\n]", fpstr);
        fclose(fptr);

        if (!strcmp(zynqmpstr, fpstr))
                return 1;
        else
                return 0;

}

void print_usage(char *prg)
{
	int iszynqmp = fpga_getplatform();

	fprintf(stderr, "\n%s: FPGA Utility for Loading/reading PL Configuration\n\n", prg);
	fprintf(stderr, "Usage:	%s -b <bin file path> -o <dtbo file path>\n\r", prg);
	fprintf(stderr, "\n");
	fprintf(stderr, "Options: -b <binfile>		(Bin file path)\n");
	fprintf(stderr, "         -o <dtbofile>		(DTBO file path)\n");
	fprintf(stderr, "         -f <flags>		Optional: <Bitstream type flags>\n");
	fprintf(stderr, "				   f := <Full | Partial > \n");
	fprintf(stderr, "         -n <Fpga region info>  FPGA Regions represent FPGA's\n");
	fprintf(stderr, "                                and partial reconfiguration\n");
	fprintf(stderr, "                                regions of FPGA's in the\n");
	fprintf(stderr, "                                Device Tree\n");
	if (iszynqmp)
	{
		fprintf(stderr, "				Default: <Full>\n");
		fprintf(stderr, "	  -s <secure flags>	Optional: <Secure flags>\n");
		fprintf(stderr, "				   s := <AuthDDR | AuthOCM | EnUsrKey | EnDevKey | AuthEnUsrKeyDDR | AuthEnUsrKeyOCM | AuthEnDevKeyDDR | AuthEnDevKeyOCM>\n");
		fprintf(stderr, "	  -k <AesKey>		Optional: <AES User Key>\n");
		fprintf(stderr, "	  -r <Readback> 	Optional: <file name>\n");
		fprintf(stderr, "				Default: By default Read back contents will be stored in readback.bin file\n");
		fprintf(stderr, "	  -t			Optional: <Readback Type>\n");
		fprintf(stderr, "				   0 - Configuration Register readback\n");
		fprintf(stderr, "				   1 - Configuration Data Frames readback\n");
		fprintf(stderr, "				Default: 0 (Configuration register readback)\n");
		fprintf(stderr, "	  -R 			Optional: Remove overlay from a live tree\n");
	}

	fprintf(stderr, " \n");
	fprintf(stderr, "Examples:\n");
	fprintf(stderr, "(Load Full bitstream using Overlay)\n");
	fprintf(stderr, "%s -b top.bit.bin -o can.dtbo -f Full -n Full \n", prg);
	fprintf(stderr, "(Load Partial bitstream using Overlay)\n");
	fprintf(stderr, "%s -b rm0.bit.bin -o rm0.dtbo -f Partial -n PR0\n", prg);
	fprintf(stderr, "(Load Full bitstream using sysfs interface)\n");
	fprintf(stderr, "%s -b top.bit.bin -f Full\n", prg);
	fprintf(stderr, "(Load Partial bitstream using sysfs interface)\n");
	fprintf(stderr, "%s -b rm0.bit.bin -f Partial\n", prg);
	if (iszynqmp)
	{
		fprintf(stderr, "(Load Authenticated bitstream through the sysfs interface)\n");
		fprintf(stderr, "%s -b top.bit.bin -f Full -s AuthDDR \n", prg);
		fprintf(stderr, "(Load Parital Encrypted Userkey bitstream using Overlay)\n");
		fprintf(stderr, "%s -b top.bit.bin -o pl.dtbo -f Partial -s EnUsrKey -k <32byte key value>\n", prg);
		fprintf(stderr, "(Read PL Configuration Registers)\n");
		fprintf(stderr, "%s -b top.bit.bin -r\n", prg);
	}
	fprintf(stderr, " \n");
}

int gettime(struct timeval  t0, struct timeval t1)
{
        return ((t1.tv_sec - t0.tv_sec) * 1000.0f + (t1.tv_usec -t0.tv_usec) / 1000.0f);
}

int fpga_state()
{
	FILE *fptr;
	char buf[10];
	char *state_operating = "operating";
	char *state_unknown = "unknown";

	system("cat /sys/class/fpga_manager/fpga0/state >> /tmp/fpga_util_state_bit.txt");
	fptr = fopen("/tmp/fpga_util_state_bit.txt", "r");
	if (fptr) {
		fgets(buf, 10, fptr);
		fclose(fptr);
		system("rm /tmp/fpga_util_state_bit.txt");
		if ((strncmp(buf, state_operating, 9) == 0) || (strncmp(buf, state_unknown, 7) == 0))
			return 0;
		else
			return 1;
	}

	return 1;
}

static int fpga_overlay_check(char *cmd, char *state)
{
	char buf[512];
	FILE *fptr;
	int len;

	system(cmd);
	len = strlen(state) + 1;
	fptr = fopen("/tmp/fpga_util_state.txt", "r");
	if (fptr) {
		fgets(buf, len, fptr);
		fclose(fptr);
		system("rm /tmp/fpga_util_state.txt");
		if (!strcmp(buf, state))
			return 0;
		else
			return 1;
	}

	return 1;
}

struct fpgaflag {
        char *flag;
        unsigned int value;
};

static struct fpgaflag flagdump[] = {
	{.flag = "Full",                .value = 0x0},
	{.flag = "Partial",             .value = 0x1},
	{.flag = "AuthDDR",             .value = 0x40},
	{.flag = "AuthOCM",             .value = 0x80},
	{.flag = "EnUsrKey",            .value = 0x20},
	{.flag = "EnDevKey",            .value = 0x4},
	{.flag = "AuthEnUsrKeyDDR",     .value = 0x60},
	{.flag = "AuthEnUsrKeyOCM",     .value = 0xA0},
	{.flag = "AuthEnDevKeyDDR",     .value = 0x44},
	{.flag = "AuthEnDevKeyOCM",     .value = 0x84},
	{}
};

static int cmd_flags(int argc, const char *name)
{
	int valid_flag = 0;
	int flag = 0;
	struct fpgaflag *p = flagdump;

	while (p->flag) {
		if (!strcmp(name, p->flag)) {
			flag = p->value;
			break;
		}
		p++;
	}

	return flag;
}

static int isvalid_flags(int argc, const char *name, bool is_secure)
{
	int valid_flag = 0;
	int count = 0;
	struct fpgaflag *p;

	if (!is_secure)
		p = flagdump;
	else
		p = &flagdump[2];

	while (p->flag) {
		if (!strcmp(name, p->flag))
			return 0;
		else if ((!is_secure) && (++count == 2))
			return 1;
		p++;
	}

	return 1;
}

int main(int argc, char **argv)
{
	int ret;
	int iszynqmp = fpga_getplatform();
	char *binfile = NULL, *overlay = NULL, *AesKey = NULL, *flag = NULL, *partial_overlay = NULL;
	char *region = NULL, *Module[100] = {0};
	int opt, flags = 0, flow = 0,rm_overlay = 0, readback_type = 0, sflags = 0;
	char command[2048], folder[512], *token, *tmp, *tmp1, *tmp2 , *tmp3;
	const char *filename = "readback", *name;
	struct stat sb;
	double time;
        struct timeval t1, t0;

	if (argc == 1) {
		print_usage(basename(argv[0]));
		return 1;
	}

	while ((opt = getopt(argc, argv, "o:b:n:f:s:p:k:rt::Rh?:")) != -1) {
		switch (opt) {
		case 'o':
			overlay = optarg;
			flow = OVERLAY;
			break;
		case 'b':
			binfile = optarg;
			if (!(flow == OVERLAY))
				flow = FPGA_SYSFS;
			break;
		case 'n':
                        region = optarg;
                        break;
		case 'f':
			if (flow == OVERLAY) {
				name = argv[6];
				flags = cmd_flags(argc, name);
			} else if (flow == FPGA_SYSFS) {
				name = argv[4];
				flags = cmd_flags(argc, name);
			}

			ret = isvalid_flags(argc, name, false);
			if (ret) {
				printf("Error: Invalid arugments :%s\n", strerror(1));
				print_usage(basename(argv[0]));
				return -EINVAL;
                        }

			flags += sflags;
			break;
		case 's':
			if (flow == OVERLAY) {
				name = argv[8];
				sflags = cmd_flags(argc, name);
			} else if (flow == FPGA_SYSFS) {
				name = argv[6];
				sflags = cmd_flags(argc, name);
			}

			ret = isvalid_flags(argc, name, true);
			if (ret) {
				printf("Error: Invalid arugments :%s\n", strerror(1));
				print_usage(basename(argv[0]));
				return -EINVAL;
			}

			flags += sflags;
			break;
		case 'p':
			partial_overlay = optarg;
			break;
		case 'k':
			AesKey = optarg;
			break;
		case 't':
			if (optarg == NULL && argv[4] != NULL)
				readback_type = atoi(argv[4]);
			break;
		case 'r':
			if (optarg == NULL && argv[2] != NULL)
				filename = argv[2];
			flow = READBACK;
			break;
		case 'R':
			rm_overlay = 1;
			break;
		case '?':
		case 'h':
		default:
			print_usage(basename(argv[0]));
			return 1;
			break;
		}
	}

	if(region != NULL)
		snprintf(folder, sizeof(folder), "/configfs/device-tree/overlays/%s", region);
	else if (!(flags & 1))
		snprintf(folder, sizeof(folder), "/configfs/device-tree/overlays/full");
	else if (overlay != NULL) {
		printf("Error: Provide valid Overlay region info\n\r");
		return 1;
	}
	system("mkdir -p /lib/firmware");
	if (rm_overlay) {
		if (((stat(folder, &sb) == 0) && S_ISDIR(sb.st_mode))) {
			snprintf(command, sizeof(command), "rmdir %s", folder);
			system(command);
		}
		return 0;
	}

	if (flow == OVERLAY) {
		if (((stat(folder, &sb) == 0) && S_ISDIR(sb.st_mode))) {
			printf("Error: Overlay already exists in the live tree\n\r");
			return 1;
		}

		if (((stat("/configfs/device-tree/", &sb) == 0) && S_ISDIR(sb.st_mode))) {
		} else {
			system("mkdir /configfs");
			system("mount -t configfs configfs /configfs");
		}

		if (binfile != NULL) {
			snprintf(command, sizeof(command), "cp %s /lib/firmware", binfile);
			system(command);
		}

		snprintf(command, sizeof(command), "cp %s /lib/firmware", overlay);
		system(command);
		tmp = strdup(overlay);
		while((token = strsep(&tmp, "/"))) {
			tmp1 = token;
		}

		if (binfile != NULL) {
			snprintf(command, sizeof(command), "echo %x > /sys/class/fpga_manager/fpga0/flags", flags);
			system(command);
			if (ENCRYPTION_USERKEY_EN & flags) {
				snprintf(command, sizeof(command), "echo %s > /sys/class/fpga_manager/fpga0/key", AesKey);
				system(command);
			}
		}

		snprintf(command, sizeof(command), "mkdir %s", folder);
		system(command);
		snprintf(command, sizeof(command), "echo -n %s > %s/path", tmp1, folder);
		gettimeofday(&t0, NULL);
		system(command);
		gettimeofday(&t1, NULL);
		time = gettime(t0, t1);

		snprintf(command, sizeof(command), "cat %s/path >> /tmp/fpga_util_state.txt", folder);
		ret = fpga_overlay_check(command, tmp1);
		if (ret) {
			printf("Failed to apply Overlay\n\r");
		}

		/* Delete Bin file and DTBO file*/
		snprintf(command, sizeof(command), "rm /lib/firmware/%s", tmp1);
		system(command);
		if (binfile != NULL) {
			tmp = strdup(binfile);
			while((token = strsep(&tmp, "/"))) {
				tmp1 = token;
			}
			snprintf(command, sizeof(command), "rm /lib/firmware/%s", tmp1);
			system(command);
		}

		/* FPGA state check */
		if (binfile != NULL) {
			if (!fpga_state()) {
				printf("Time taken to load BIN is %f Milli Seconds\n\r", time);
				printf("BIN FILE loaded through FPGA manager successfully\n\r");
			} else {
				printf("BIN FILE loading through FPGA manager failed\n\r");
			}
		}
	} else if (flow == FPGA_SYSFS) {
		if (argc < 3) {
			printf("%s: For more information run %s -h\n", strerror(22), basename(argv[0]));
			return 1;
		}
		snprintf(command, sizeof(command), "cp %s /lib/firmware", binfile);
		system(command);
		snprintf(command, sizeof(command), "echo %x > /sys/class/fpga_manager/fpga0/flags", flags);
		system(command);
		if (ENCRYPTION_USERKEY_EN & flags) {
			snprintf(command, sizeof(command), "echo %s > /sys/class/fpga_manager/fpga0/key", AesKey);
			system(command);
		}
		tmp = strdup(binfile);
		while((token = strsep(&tmp, "/"))) {
			tmp1 = token;
		}
		snprintf(command, sizeof(command), "echo %s > /sys/class/fpga_manager/fpga0/firmware", tmp1);
		gettimeofday(&t0, NULL);
		system(command);
		gettimeofday(&t1, NULL);
		time = gettime(t0, t1);

		/* Delete Bin file and DTBO file*/
		snprintf(command, sizeof(command), "rm /lib/firmware/%s", tmp1);
		system(command);

		/* FPGA state check */
		if (!fpga_state()) {
			printf("Time taken to load BIN is %f Milli Seconds\n\r", time);
			printf("BIN FILE loaded through FPGA manager successfully\n\r");
		} else {
			printf("BIN FILE loading through FPGA manager failed\n\r");
		}
	} else if (flow == READBACK) {
		if (readback_type > 1) {
			printf("Invalid arugments :%s\n", strerror(1));
			printf("For more information run %s -h\n", basename(argv[0]));
			return -EINVAL;
		}
		snprintf(command, sizeof(command), "echo %x > /sys/module/zynqmp_fpga/parameters/readback_type", readback_type);
		system(command);
		snprintf(command, sizeof(command), "cat /sys/kernel/debug/fpga/fpga0/image >> %s.bin", filename);
		system(command);
		printf("Readback contents are stored in the file %s.bin\n\r", filename);
	}

	return 0;
}
