/**
 * $Id: $
 *
 * @brief Red Pitaya Led System Module.
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>

#include <linux/ethtool.h>
#include <linux/mdio.h>
#include <linux/sockios.h>

#include "led_system.h"
#include "rp_log.h"

const char mmc_Led[]="/sys/devices/soc0/led-system/leds/led8";
const char heartbeat_Led[]="/sys/devices/soc0/led-system/leds/led9";
const int  MAX_LINE_LENGTH = 512;

typedef struct {
        char     ifnam[16];
        uint16_t phy_id;
        uint16_t reg;
} device_t;

const device_t eth = { { "eth0" }, 0x01 , 0x1b };

bool GetValueFromFile(char *file, char *value){
    FILE *fp;
    fp = fopen (file, "r");
    if (fp == NULL) return false;
    bool r = fgets(value, MAX_LINE_LENGTH, fp) != NULL;
    fclose(fp);
    return r;
}

bool SetValueToFile(char *file, char *value){
    FILE *fp;
    fp = fopen (file, "w");
    if (fp == NULL) return false;
    bool r = fputs(value, fp) >=0;
    fclose(fp);
    return r;
}


int led_GetMMCState(bool *_enable){
    char path[255];
    char value[MAX_LINE_LENGTH];
    sprintf(path,"%s/%s",mmc_Led,"trigger");
    if (GetValueFromFile(path,value)){
        *_enable = strstr(value, "[mmc0]") != NULL;
        return RP_HW_OK;
    }
    return RP_HW_EUF;
}

int led_SetMMCState(bool _enable){
    char path[255];
    char value[10];
    sprintf(value,"%s",_enable ? "mmc0" : "none");
    sprintf(path,"%s/%s",mmc_Led,"trigger");
    if (SetValueToFile(path,value)){
        return RP_HW_OK;
    }
    return RP_HW_EUF;
}

int led_GetHeartBeatState(bool *_enable){
    char path[255];
    char value[MAX_LINE_LENGTH];
    sprintf(path,"%s/%s",heartbeat_Led,"trigger");
    if (GetValueFromFile(path,value)){
        *_enable = strstr(value, "[heartbeat]") != NULL;
        return RP_HW_OK;
    }
    return RP_HW_EUF;
}

int led_SetHeartBeatState(bool _enable){
    char path[255];
    char value[10];
    sprintf(value,"%s",_enable ? "heartbeat" : "none");
    sprintf(path,"%s/%s",heartbeat_Led,"trigger");
    if (SetValueToFile(path,value)){
        return RP_HW_OK;
    }
    return RP_HW_EUF;
}

int __phy_op(const device_t *loc, uint16_t *val, int cmd)
{
	static int sd = -1;

	struct ifreq ifr;
	struct mii_ioctl_data* mii = (struct mii_ioctl_data *)(&ifr.ifr_data);
	int err;

	if (sd < 0)
		sd = socket(AF_INET, SOCK_DGRAM, 0);

	if (sd < 0)
		return sd;

	strncpy(ifr.ifr_name, loc->ifnam, sizeof(ifr.ifr_name));

	mii->phy_id  = loc->phy_id;
	mii->reg_num = loc->reg;
	mii->val_in  = *val;
	mii->val_out = 0;

	err = ioctl(sd, cmd, &ifr);
	if (err)
		return -errno;

	*val = mii->val_out;
	return 0;
}

int phy_read(uint16_t *_val)
{
	int err = __phy_op(&eth, _val, SIOCGMIIREG);

	if (err) {
		ERROR_LOG("phy_read (%d)", err);
		return err;
	}
	return RP_HW_OK;
}

int phy_write(uint16_t val)
{
	int err = __phy_op(&eth, &val, SIOCSMIIREG);

	if (err)
		ERROR_LOG("phy_write (%d)", err);

	return err;
}

int led_GetEthState(bool *_state){
    uint16_t val = 0;
    if (phy_read(&val) == RP_HW_OK){
        *_state = val != 0;
        return RP_HW_OK;
    }
    return RP_HW_EUF;
}

int led_SetEthState(bool _state){
    uint16_t val = _state ? 0x0F00 : 0;
    if (phy_write(val) == 0){
        return RP_HW_OK;
    }
    return RP_HW_EUF;
}