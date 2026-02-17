/**
 * $Id: $
 *
 * @brief Red Pitaya Led System Module.
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>

#include <linux/ethtool.h>
#include <linux/mdio.h>
#include <linux/sockios.h>

#include <pthread.h>

#include "led_system.h"
#include "rp_log.h"

#define MAX_LINE_LENGTH 512

char mmc_Led_trigger[]="/sys/devices/platform/led-system/leds/led8/trigger";
char heartbeat_Led_trigger[]="/sys/devices/soc0/led-system-red/leds/led9/trigger";

static pthread_mutex_t phy_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
        char     ifnam[16];
        uint16_t phy_id;
        uint16_t reg;
} device_t;

const device_t eth = { { "eth0" }, 0x01 , 0x1b };

bool GetValueFromFile(char *file, char *value, size_t max_len){
    if (!file || !value) return false;
    FILE *fp = fopen(file, "r");
    if (!fp) {
        ERROR_LOG("Failed to open file: %s - %s", file, strerror(errno));
        return false;
    }

    bool r = fgets(value, max_len, fp) != NULL;
    fclose(fp);
    return r;
}

bool SetValueToFile(char *file,const char *value){
    if (!file || !value) return false;
    FILE *fp = fopen(file, "w");
    if (!fp) {
        ERROR_LOG("Failed to open file: %s - %s", file, strerror(errno));
        return false;
    }

    bool r = fputs(value, fp) >= 0;
    fclose(fp);
    return r;
}


int led_GetMMCState(bool *_enable){
    if (!_enable) {
        ERROR_LOG("NULL pointer provided");
        return RP_HW_EBIIC;
    }
    char value[MAX_LINE_LENGTH];
    if (GetValueFromFile(mmc_Led_trigger, value, sizeof(value))) {
        *_enable = strstr(value, "[mmc0]") != NULL;
        return RP_HW_OK;
    }

    ERROR_LOG("Failed to read MMC LED state from %s", mmc_Led_trigger);
    return RP_HW_EUF;
}

int led_SetMMCState(bool _enable){
    const char *value = _enable ? "mmc0" : "none";
    if (SetValueToFile(mmc_Led_trigger, value)) {
        TRACE("MMC LED set to %s", value);
        return RP_HW_OK;
    }

    ERROR_LOG("Failed to set MMC LED state to %s", value);
    return RP_HW_EUF;
}

int led_GetHeartBeatState(bool *_enable){
    if (!_enable) {
        ERROR_LOG("NULL pointer provided");
        return RP_HW_EBIIC;
    }
    char value[MAX_LINE_LENGTH];
    if (GetValueFromFile(heartbeat_Led_trigger,value,sizeof(value))){
        *_enable = strstr(value, "[heartbeat]") != NULL;
        return RP_HW_OK;
    }
    ERROR_LOG("Failed to read HB LED state from %s", heartbeat_Led_trigger);
    return RP_HW_EUF;
}

int led_SetHeartBeatState(bool _enable){
    const char *value = _enable ? "heartbeat" : "none";
    if (SetValueToFile(heartbeat_Led_trigger,value)){
        return RP_HW_OK;
    }
    ERROR_LOG("Failed to set HB LED state to %s", value);
    return RP_HW_EUF;
}

int __phy_op(const device_t *loc, uint16_t *val, int cmd)
{
    pthread_mutex_lock(&phy_mutex);
	int sd = -1;

	struct ifreq ifr;
	struct mii_ioctl_data* mii = (struct mii_ioctl_data *)(&ifr.ifr_data);
	int err;

	if (sd < 0)
		sd = socket(AF_INET, SOCK_DGRAM, 0);

	if (sd < 0){
        pthread_mutex_unlock(&phy_mutex);
        ERROR_LOG("Failed to create socket: %d (%s)", errno, strerror(errno));
        return -errno;
    }

	strncpy(ifr.ifr_name, loc->ifnam, sizeof(ifr.ifr_name));

	mii->phy_id  = loc->phy_id;
	mii->reg_num = loc->reg;
	mii->val_in  = *val;
	mii->val_out = 0;

	err = ioctl(sd, cmd, &ifr);
	if (err){
        if (sd >= 0) {
            close(sd);
            sd = -1;
        }
        pthread_mutex_unlock(&phy_mutex);
        return -errno;
    }

	*val = mii->val_out;
    if (sd >= 0) {
        close(sd);
        sd = -1;
    }
    pthread_mutex_unlock(&phy_mutex);
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
    int ret = phy_write(val);
    if (ret == 0) {
        TRACE("Eth state set to %s", _state ? "ON" : "OFF");
        return RP_HW_OK;
    } else {
        TRACE("Failed to set eth state: %d", ret);
        return RP_HW_EUF;
    }
}