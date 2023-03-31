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
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include "spi.h"
#include "spi-helper.h"

static spi_config_t g_settings;

/* File descriptor definition */
int spi_fd = -1;
spi_data_t *g_spi_data = NULL;
pthread_mutex_t spi_mutex = PTHREAD_MUTEX_INITIALIZER;

int spi_Init(){
    return spi_InitDevice("/dev/spidev1.0");
}

int spi_InitDevice(const char *_device){
    if(spi_fd != -1){
        if (spi_Release() != RP_HW_OK){
            return RP_HW_EIS;
        }
    }

    spi_DestoryMessage();

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
    defConfig.raw_value = 0;
    defConfig.cs_mode = RP_SPI_CS_NORMAL;
    defConfig.spi_mode = RP_SPI_MODE_LISL;
    defConfig.spi_ready = RP_SPI_STATE_NOT;
    defConfig.lsb_first = RP_SPI_ORDER_BIT_MSB;
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
    spi_DestoryMessage();
    return RP_HW_OK;
}

int spi_CreateMessage(size_t len){
    spi_DestoryMessage();
   	pthread_mutex_lock(&spi_mutex);
    g_spi_data = malloc(sizeof(spi_data_t));

    if (!g_spi_data){
		MSG("[Error] Can't allocate memory for spi_data_t\n");
		return RP_HW_EAL;
	}

    g_spi_data->messages = calloc(len,sizeof(spi_message_t));

    if (!g_spi_data){
		MSG("[Error] Can't allocate memory for spi_message_t\n");
        free(g_spi_data);
        g_spi_data = NULL;
		return RP_HW_EAL;
	}

    g_spi_data->size = len;
  	pthread_mutex_unlock(&spi_mutex);
    return RP_HW_OK;
}

int spi_DestoryMessage(){
    pthread_mutex_lock(&spi_mutex);
    if (g_spi_data){
        if(g_spi_data->messages){
            for (size_t i = 0; i < g_spi_data->size; i++){
                free(g_spi_data->messages[i].rx_buffer);
                free(g_spi_data->messages[i].tx_buffer);
            }
        }
        free(g_spi_data->messages);
        g_spi_data = NULL;
       	pthread_mutex_unlock(&spi_mutex);
        return RP_HW_OK;
    }
	pthread_mutex_unlock(&spi_mutex);
    return RP_HW_ESMI;
}

int spi_GetMessageLen(size_t *len){
    pthread_mutex_lock(&spi_mutex);
    if (g_spi_data){
        *len = g_spi_data->size;
    	pthread_mutex_unlock(&spi_mutex);
        return RP_HW_OK;
    }
   	pthread_mutex_unlock(&spi_mutex);
    return RP_HW_ESMI;
}

int spi_GetRxBuffer(size_t msg,const uint8_t **buffer,size_t *len){
    pthread_mutex_lock(&spi_mutex);
    if (g_spi_data){
        if (g_spi_data->size <= msg){
            pthread_mutex_unlock(&spi_mutex);
            return RP_HW_ESMO;
        }
        *buffer = g_spi_data->messages[msg].rx_buffer;
        *len   = g_spi_data->messages[msg].size;
    	pthread_mutex_unlock(&spi_mutex);
        return RP_HW_OK;
    }
   	pthread_mutex_unlock(&spi_mutex);
    return RP_HW_ESMI;
}

int spi_GetTxBuffer(size_t msg,const uint8_t **buffer,size_t *len){
    pthread_mutex_lock(&spi_mutex);
    if (g_spi_data){
        if (g_spi_data->size <= msg){
            pthread_mutex_unlock(&spi_mutex);
            return RP_HW_ESMO;
        }
        *buffer = g_spi_data->messages[msg].tx_buffer;
        *len   = g_spi_data->messages[msg].size;
    	pthread_mutex_unlock(&spi_mutex);
        return RP_HW_OK;
    }
   	pthread_mutex_unlock(&spi_mutex);
    return RP_HW_ESMI;
}

int spi_GetCSChangeState(size_t msg,bool *cs_change){
    pthread_mutex_lock(&spi_mutex);
    if (g_spi_data){
        if (g_spi_data->size <= msg){
            pthread_mutex_unlock(&spi_mutex);
            return RP_HW_ESMO;
        }
        *cs_change = g_spi_data->messages[msg].cs_change;
    	pthread_mutex_unlock(&spi_mutex);
        return RP_HW_OK;
    }
   	pthread_mutex_unlock(&spi_mutex);
    return RP_HW_ESMI;
}

int spi_SetBufferForMessage(size_t msg,const uint8_t *tx_buffer,bool init_rx_buffer,size_t len, bool cs_change){
    pthread_mutex_lock(&spi_mutex);
    if (g_spi_data){
        if (g_spi_data->size <= msg){
            pthread_mutex_unlock(&spi_mutex);
            return RP_HW_ESMO;
        }
        if (g_spi_data->messages[msg].rx_buffer){
            free(g_spi_data->messages[msg].rx_buffer);
        }
        g_spi_data->messages[msg].rx_buffer = 0;
        if (init_rx_buffer) {
            g_spi_data->messages[msg].rx_buffer = malloc(len);
            if (!g_spi_data->messages[msg].rx_buffer){
		        MSG("[Error] Can't allocate memory for rx_buffer\n");
                pthread_mutex_unlock(&spi_mutex);
	        	return RP_HW_EAL;
    	    }
            memset(g_spi_data->messages[msg].rx_buffer,0,len);
        }
        if (g_spi_data->messages[msg].tx_buffer){
            free(g_spi_data->messages[msg].tx_buffer);
        }
        g_spi_data->messages[msg].tx_buffer = 0;
        if (tx_buffer){
            g_spi_data->messages[msg].tx_buffer = malloc(len);
            if (!g_spi_data->messages[msg].tx_buffer){
		        MSG("[Error] Can't allocate memory for tx_buffer\n");
                pthread_mutex_unlock(&spi_mutex);
	        	return RP_HW_EAL;
    	    }
            memcpy(g_spi_data->messages[msg].tx_buffer,tx_buffer,len);
        }
        g_spi_data->messages[msg].size = len;
        g_spi_data->messages[msg].cs_change = cs_change;
    	pthread_mutex_unlock(&spi_mutex);
        return RP_HW_OK;
    }
   	pthread_mutex_unlock(&spi_mutex);
    return RP_HW_ESMI;
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

int spi_GetCSMode(rp_spi_cs_mode_t  *mode){
    if(spi_fd == -1){
        MSG("Failed SPI not init\n");
        return RP_HW_EIS;
    }

    *mode = g_settings.cs_mode;
    return RP_HW_OK;
}

int spi_SetCSMode(rp_spi_cs_mode_t mode){
    if(spi_fd == -1){
        MSG("Failed SPI not init\n");
        return RP_HW_EIS;
    }

    g_settings.cs_mode = mode;
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

int spi_ReadWrite(){
    if(spi_fd == -1){
        MSG("Failed SPI not init\n");
        return RP_HW_EIS;
    }
   	pthread_mutex_lock(&spi_mutex);
    int res = read_write_spi_buffers(spi_fd,g_spi_data);
  	pthread_mutex_unlock(&spi_mutex);
    return res;
}
