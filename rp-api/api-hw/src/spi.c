/**
 * $Id: $
 *
 * @brief Red Pitaya SPI Module.
 *
 * @Author Red Pitaya
 *
 * (c) Red Pitaya  http://www.redpitaya.com
 *
 * This part of code is written in C programming language.
 * Please visit http://en.wikipedia.org/wiki/C_(programming_language)
 * for more details on the language used herein.
 */

#include <stdint.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h> 

#include "spi.h"
#include "spi-helper.h"

spi_config_t g_settings;

/* File descriptor definition */
int spi_fd = -1;

int spi_Init(){
    return spi_InitDevice("/dev/spidev1.0");
}

int spi_InitDevice(char *_device){
    if(spi_fd != -1){
        if (spi_Release() != RP_HW_OK){
            return RP_HW_EIS;
        }
    }

    spi_fd = open(_device, O_RDONLY);

    if(spi_fd == -1){
        MSG("Failed to open SPI dev: %s.\n",_device);
        return RP_HW_EIS;
    }
    return read_spi_configuration(spi_fd,&g_settings);
}

int spi_SetDefaultSettings(){
    
    if(spi_fd == -1){
        MSG("Failed SPI not init\n");
        return RP_HW_EIS;
    }

    spi_config_t defConfig;
    defConfig.spi_mode = RP_SPI_MODE_LISL;
    defConfig.spi_ready = RP_SPI_STATE_NOT;
    defConfig.lsb_first = RP_SPI_ORDER_BIT_LSB;
    defConfig.bits_per_word = 8;
    defConfig.spi_speed = 50000000;
        
    int ret =  write_spi_configuration(spi_fd,&defConfig);
    if (ret != 0) {
        return ret;
    }
    return read_spi_configuration(spi_fd,&g_settings);
}

int spi_GetSettings(){
    if(spi_fd == -1){
        MSG("Failed SPI not init\n");
        return RP_HW_EIS;
    }
    return read_spi_configuration(spi_fd,&g_settings);
}

int spi_SetSettings(){    
    if(spi_fd == -1){
        MSG("Failed SPI not init\n");
        return RP_HW_EIS;
    }
    return write_spi_configuration(spi_fd,&g_settings);
}


int spi_Release(){
    if (spi_fd != -1){
        if (close(spi_fd) != 0){
            MSG("Failed to close SPI dev.\n");
            return RP_HW_EIS;
        }
        spi_fd = -1;
    }
    return RP_HW_OK;
}

int spi_GetMode(rp_spi_mode_t *mode){
    if(spi_fd == -1){
        MSG("Failed SPI not init\n");
        return RP_HW_EIS;
    }

    *mode = g_settings.spi_mode;
    return RP_HW_OK;
}

int spi_SetMode(rp_spi_mode_t mode){
    if(spi_fd == -1){
        MSG("Failed SPI not init\n");
        return RP_HW_EIS;
    }

    g_settings.spi_mode = mode;
    return RP_HW_OK;
}

int spi_GetState(rp_spi_state_t *state){
    if(spi_fd == -1){
        MSG("Failed SPI not init\n");
        return RP_HW_EIS;
    }

    *state = g_settings.spi_ready;
    return RP_HW_OK;
}

int spi_SetState(rp_spi_state_t state){
    if(spi_fd == -1){
        MSG("Failed SPI not init\n");
        return RP_HW_EIS;
    }

    g_settings.spi_ready = state;
    return RP_HW_OK;
}

int spi_GetOrderBit(rp_spi_order_bit_t *order){
    if(spi_fd == -1){
        MSG("Failed SPI not init\n");
        return RP_HW_EIS;
    }

    *order = g_settings.lsb_first;
    return RP_HW_OK;
}

int spi_SetOrderBit(rp_spi_order_bit_t order){
    if(spi_fd == -1){
        MSG("Failed SPI not init\n");
        return RP_HW_EIS;
    }

    g_settings.lsb_first = order;
    return RP_HW_OK;
}

int spi_GetSpeed(int *speed){
    if(spi_fd == -1){
        MSG("Failed SPI not init\n");
        return RP_HW_EIS;
    }

    *speed = g_settings.spi_speed;
    return RP_HW_OK;
}

int spi_SetSpeed(int speed){
    if(spi_fd == -1){
        MSG("Failed SPI not init\n");
        return RP_HW_EIS;
    }

    if (speed <= 0 && speed > 100000000){
        return RP_HW_EIPV;
    }

    g_settings.spi_speed = speed;
    return RP_HW_OK;
}

int spi_GetWordLen(int *len){
    if(spi_fd == -1){
        MSG("Failed SPI not init\n");
        return RP_HW_EIS;
    }

    *len = g_settings.bits_per_word;
    return RP_HW_OK;
}

int spi_SetWordLen(int len){
    if(spi_fd == -1){
        MSG("Failed SPI not init\n");
        return RP_HW_EIS;
    }

    if (len <= 6){
        return RP_HW_EIPV;
    }

    g_settings.bits_per_word = len;
    return RP_HW_OK;
}

int spi_ReadWrite(void *tx_buffer, void *rx_buffer, unsigned int length){
    if(spi_fd == -1){
        MSG("Failed SPI not init\n");
        return RP_HW_EIS;
    }

    return read_write_spi_buffers(spi_fd,tx_buffer,rx_buffer,length);
}