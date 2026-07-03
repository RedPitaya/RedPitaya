#include <stdint.h>
#include <stdio.h>

#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <i2c/smbus.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "i2c-helper.h"
#include "rp_log.h"

pthread_mutex_t i2c_mutex = PTHREAD_MUTEX_INITIALIZER;

int openDevice(const char* i2c_dev_node_path, uint8_t i2c_dev_address, bool force, int *i2c_dev_node){
    if (!i2c_dev_node_path || !i2c_dev_node) {
        ERROR_LOG("Invalid parameters: path=%p, dev_node=%p", (void*)i2c_dev_node_path, (void*)i2c_dev_node);
        return RP_HW_EBIIC;
    }

    if (strlen(i2c_dev_node_path) == 0) {
        ERROR_LOG("Device path is empty");
        return RP_HW_EBIIC;
    }

    if (i2c_dev_address < 0x03 || i2c_dev_address > 0x77) {
        ERROR_LOG("Device address 0x%02X out of range [0x03-0x77]", i2c_dev_address);
        return RP_HW_EISAIIC;
    }

    int ret_val = 0;
    *i2c_dev_node = open(i2c_dev_node_path, O_RDWR);
    if (*i2c_dev_node < 0) {
        if (errno == EBUSY) {
            TRACE("I2C device busy: %s", i2c_dev_node_path);
            return RP_HW_EBUSYIIC;
        }
        TRACE("Unable to open device node: %s, ERROR: %d: %s", i2c_dev_node_path, errno, strerror(errno));
        return RP_HW_EIIIC;
    }

    unsigned long flag = I2C_SLAVE;
    if (force) {
        flag = I2C_SLAVE_FORCE;
        TRACE_SHORT("Using force mode for device: %s", i2c_dev_node_path);
    }

    ret_val = ioctl(*i2c_dev_node, flag, i2c_dev_address);
    if (ret_val < 0) {
        if (errno == EIO) {
            TRACE("I2C device not responding at address 0x%02X", i2c_dev_address);
            close(*i2c_dev_node);
            return RP_HW_ENRSPIIC;
        }
        if (errno == ENOTTY) {
            TRACE("Invalid I2C slave address 0x%02X", i2c_dev_address);
            close(*i2c_dev_node);
            return RP_HW_EISAIIC;
        }
        TRACE("Could not set I2C_SLAVE address 0x%02X on %s. Errno: %d: %s",
                  i2c_dev_address, i2c_dev_node_path, errno, strerror(errno));
        close(*i2c_dev_node);
        return RP_HW_ESIIC;
    }

    TRACE_SHORT("Device opened successfully: %s, addr=0x%02X, force=%d", i2c_dev_node_path, i2c_dev_address, force);
    return RP_HW_OK;
}

static int lock_mutex(void) {
    int ret = pthread_mutex_lock(&i2c_mutex);
    if (ret != 0) {
        ERROR_LOG("Failed to lock mutex: %d: %s", ret, strerror(ret));
        return RP_HW_EMLKIIC;
    }
    return RP_HW_OK;
}

static int unlock_mutex(void) {
    int ret = pthread_mutex_unlock(&i2c_mutex);
    if (ret != 0) {
        ERROR_LOG("Failed to unlock mutex: %d: %s", ret, strerror(ret));
        return RP_HW_EMUKIIC;
    }
    return RP_HW_OK;
}

int i2c_SMBUS_write_byte(const char* i2c_dev_node_path, uint8_t i2c_dev_address, uint8_t i2c_dev_reg_addr,
                          uint8_t i2c_val_to_write, bool force){
    if (!i2c_dev_node_path) {
        ERROR_LOG("Device path is NULL");
        return RP_HW_EBIIC;
    }

    int ret = lock_mutex();
    if (ret != RP_HW_OK) return ret;

    int i2c_dev_node = 0;
    int ret_val = 0;

    ret_val = openDevice(i2c_dev_node_path, i2c_dev_address, force, &i2c_dev_node);

    if (ret_val != RP_HW_OK){
        unlock_mutex();
        return ret_val;
    }

    ret_val = i2c_smbus_write_byte_data(i2c_dev_node, i2c_dev_reg_addr, i2c_val_to_write);

    if (ret_val < 0) {
        if (errno == EREMOTEIO) {
            TRACE("I2C NACK received. Addr: 0x%02X, Reg: 0x%02X", i2c_dev_address, i2c_dev_reg_addr);
            close(i2c_dev_node);
            unlock_mutex();
            return RP_HW_ENACKIIC;
        }
        if (errno == ETIMEDOUT) {
            TRACE("I2C timeout. Addr: 0x%02X, Reg: 0x%02X", i2c_dev_address, i2c_dev_reg_addr);
            close(i2c_dev_node);
            unlock_mutex();
            return RP_HW_ETOIIC;
        }
        if (errno == ENXIO) {
            TRACE("I2C device not responding. Addr: 0x%02X", i2c_dev_address);
            close(i2c_dev_node);
            unlock_mutex();
            return RP_HW_ENRSPIIC;
        }
        TRACE("I2C Write Operation failed. Addr: 0x%02X, Reg: 0x%02X, Val: 0x%02X, Errno: %d (%s)",
                  i2c_dev_address, i2c_dev_reg_addr, i2c_val_to_write, errno, strerror(errno));
        close(i2c_dev_node);
        unlock_mutex();
        return RP_HW_EWIIC;
    }

    TRACE_SHORT("I2C Write success: addr=0x%02X, reg=0x%02X, val=0x%02X",
                i2c_dev_address, i2c_dev_reg_addr, i2c_val_to_write);
    close(i2c_dev_node);
    unlock_mutex();
    return RP_HW_OK;
}

int i2c_SMBUS_write_word(const char* i2c_dev_node_path, uint8_t i2c_dev_address, uint8_t i2c_dev_reg_addr,
                          uint16_t i2c_val_to_write, bool force){
    if (!i2c_dev_node_path) {
        ERROR_LOG("Device path is NULL");
        return RP_HW_EBIIC;
    }

    int ret = lock_mutex();
    if (ret != RP_HW_OK) return ret;

    int i2c_dev_node = 0;
    int ret_val = 0;

    ret_val = openDevice(i2c_dev_node_path, i2c_dev_address, force, &i2c_dev_node);

    if (ret_val != RP_HW_OK){
        unlock_mutex();
        return ret_val;
    }

    ret_val = i2c_smbus_write_word_data(i2c_dev_node, i2c_dev_reg_addr, i2c_val_to_write);

    if (ret_val < 0) {
        if (errno == EREMOTEIO) {
            TRACE("I2C NACK received. Addr: 0x%02X, Reg: 0x%02X", i2c_dev_address, i2c_dev_reg_addr);
            close(i2c_dev_node);
            unlock_mutex();
            return RP_HW_ENACKIIC;
        }
        if (errno == ETIMEDOUT) {
            TRACE("I2C timeout. Addr: 0x%02X, Reg: 0x%02X", i2c_dev_address, i2c_dev_reg_addr);
            close(i2c_dev_node);
            unlock_mutex();
            return RP_HW_ETOIIC;
        }
        TRACE("I2C Write Word failed. Addr: 0x%02X, Reg: 0x%02X, Val: 0x%04X, Errno: %d (%s)",
                  i2c_dev_address, i2c_dev_reg_addr, i2c_val_to_write, errno, strerror(errno));
        close(i2c_dev_node);
        unlock_mutex();
        return RP_HW_EWIIC;
    }

    TRACE_SHORT("I2C Write Word success: addr=0x%02X, reg=0x%02X, val=0x%04X",
                i2c_dev_address, i2c_dev_reg_addr, i2c_val_to_write);
    close(i2c_dev_node);
    unlock_mutex();
    return RP_HW_OK;
}

int i2c_SMBUS_write_command(const char* i2c_dev_node_path, uint8_t i2c_dev_address, uint8_t i2c_dev_command, bool force){
    if (!i2c_dev_node_path) {
        ERROR_LOG("Device path is NULL");
        return RP_HW_EBIIC;
    }

    int ret = lock_mutex();
    if (ret != RP_HW_OK) return ret;

    int i2c_dev_node = 0;
    int ret_val = 0;

    ret_val = openDevice(i2c_dev_node_path, i2c_dev_address, force, &i2c_dev_node);

    if (ret_val != RP_HW_OK){
        unlock_mutex();
        return ret_val;
    }

    ret_val = i2c_smbus_write_byte(i2c_dev_node, i2c_dev_command);

    if (ret_val < 0) {
        if (errno == EREMOTEIO) {
            TRACE("I2C NACK received. Addr: 0x%02X, Cmd: 0x%02X", i2c_dev_address, i2c_dev_command);
            close(i2c_dev_node);
            unlock_mutex();
            return RP_HW_ENACKIIC;
        }
        TRACE("I2C Write Command failed. Addr: 0x%02X, Cmd: 0x%02X, Errno: %d (%s)",
                  i2c_dev_address, i2c_dev_command, errno, strerror(errno));
        close(i2c_dev_node);
        unlock_mutex();
        return RP_HW_EWIIC;
    }

    TRACE_SHORT("I2C Write Command success: addr=0x%02X, cmd=0x%02X", i2c_dev_address, i2c_dev_command);
    close(i2c_dev_node);
    unlock_mutex();
    return RP_HW_OK;
}

int i2c_SMBUS_write_buffer(const char* i2c_dev_node_path, uint8_t i2c_dev_address, uint8_t i2c_dev_reg_addr,
                            const uint8_t *buffer, int len, bool force){
    if (!buffer) {
        ERROR_LOG("Buffer is NULL");
        return RP_HW_EBIIC;
    }
    if (len <= 0 || len > I2C_SMBUS_BLOCK_MAX) {
        ERROR_LOG("Invalid buffer length: %d. Must be between 1 and %d", len, I2C_SMBUS_BLOCK_MAX);
        return RP_HW_EIBLIIC;
    }
    if (!i2c_dev_node_path) {
        ERROR_LOG("Device path is NULL");
        return RP_HW_EBIIC;
    }

    int ret = lock_mutex();
    if (ret != RP_HW_OK) return ret;

    int i2c_dev_node = 0;
    int ret_val = 0;

    ret_val = openDevice(i2c_dev_node_path, i2c_dev_address, force, &i2c_dev_node);

    if (ret_val != RP_HW_OK){
        unlock_mutex();
        return ret_val;
    }

    ret_val = i2c_smbus_write_block_data(i2c_dev_node, i2c_dev_reg_addr, len, buffer);

    if (ret_val < 0) {
        if (errno == EREMOTEIO) {
            TRACE("I2C NACK received. Addr: 0x%02X, Reg: 0x%02X", i2c_dev_address, i2c_dev_reg_addr);
            close(i2c_dev_node);
            unlock_mutex();
            return RP_HW_ENACKIIC;
        }
        if (errno == ENOBUFS) {
            TRACE("I2C buffer too large. Addr: 0x%02X, Reg: 0x%02X, Len: %d",
                      i2c_dev_address, i2c_dev_reg_addr, len);
            close(i2c_dev_node);
            unlock_mutex();
            return RP_HW_EIBLIIC;
        }
        TRACE("I2C Write Buffer failed. Addr: 0x%02X, Reg: 0x%02X, Len: %d, Errno: %d (%s)",
                  i2c_dev_address, i2c_dev_reg_addr, len, errno, strerror(errno));
        close(i2c_dev_node);
        unlock_mutex();
        return RP_HW_EWIIC;
    }

    TRACE_SHORT("I2C Write Buffer success: addr=0x%02X, reg=0x%02X, len=%d",
                i2c_dev_address, i2c_dev_reg_addr, len);
    close(i2c_dev_node);
    unlock_mutex();
    return RP_HW_OK;
}

int i2c_SMBUS_read_byte(const char* i2c_dev_node_path, uint8_t i2c_dev_address, uint8_t i2c_dev_reg_addr,
                         uint8_t *value, bool force){
    if (!value) {
        ERROR_LOG("Output value pointer is NULL");
        return RP_HW_EBIIC;
    }
    if (!i2c_dev_node_path) {
        ERROR_LOG("Device path is NULL");
        return RP_HW_EBIIC;
    }

    int ret = lock_mutex();
    if (ret != RP_HW_OK) return ret;

    int i2c_dev_node = 0;
    int ret_val = 0;
    __s32 read_value = 0;

    ret_val = openDevice(i2c_dev_node_path, i2c_dev_address, force, &i2c_dev_node);

    if (ret_val != RP_HW_OK){
        unlock_mutex();
        return ret_val;
    }

    read_value = i2c_smbus_read_byte_data(i2c_dev_node, i2c_dev_reg_addr);

    if (read_value < 0) {
        if (errno == EREMOTEIO) {
            TRACE("I2C NACK received. Addr: 0x%02X, Reg: 0x%02X", i2c_dev_address, i2c_dev_reg_addr);
            close(i2c_dev_node);
            unlock_mutex();
            return RP_HW_ENACKIIC;
        }
        if (errno == ETIMEDOUT) {
            TRACE("I2C timeout. Addr: 0x%02X, Reg: 0x%02X", i2c_dev_address, i2c_dev_reg_addr);
            close(i2c_dev_node);
            unlock_mutex();
            return RP_HW_ETOIIC;
        }
        TRACE("I2C Read Byte failed. Addr: 0x%02X, Reg: 0x%02X, Errno: %d (%s)",
                  i2c_dev_address, i2c_dev_reg_addr, errno, strerror(errno));
        close(i2c_dev_node);
        unlock_mutex();
        return RP_HW_ERIIC;
    }

    close(i2c_dev_node);
    *value = (uint8_t)read_value;
    TRACE_SHORT("I2C Read Byte success: addr=0x%02X, reg=0x%02X, val=0x%02X",
                i2c_dev_address, i2c_dev_reg_addr, *value);
    unlock_mutex();
    return RP_HW_OK;
}

int i2c_SMBUS_read_word(const char* i2c_dev_node_path, uint8_t i2c_dev_address, uint8_t i2c_dev_reg_addr,
                         uint16_t *value, bool force){
    if (!value) {
        ERROR_LOG("Output value pointer is NULL");
        return RP_HW_EBIIC;
    }
    if (!i2c_dev_node_path) {
        ERROR_LOG("Device path is NULL");
        return RP_HW_EBIIC;
    }

    int ret = lock_mutex();
    if (ret != RP_HW_OK) return ret;

    int i2c_dev_node = 0;
    int ret_val = 0;
    __s32 read_value = 0;

    ret_val = openDevice(i2c_dev_node_path, i2c_dev_address, force, &i2c_dev_node);

    if (ret_val != RP_HW_OK){
        unlock_mutex();
        return ret_val;
    }

    read_value = i2c_smbus_read_word_data(i2c_dev_node, i2c_dev_reg_addr);

    if (read_value < 0) {
        if (errno == EREMOTEIO) {
            TRACE("I2C NACK received. Addr: 0x%02X, Reg: 0x%02X", i2c_dev_address, i2c_dev_reg_addr);
            close(i2c_dev_node);
            unlock_mutex();
            return RP_HW_ENACKIIC;
        }
        TRACE("I2C Read Word failed. Addr: 0x%02X, Reg: 0x%02X, Errno: %d (%s)",
                  i2c_dev_address, i2c_dev_reg_addr, errno, strerror(errno));
        close(i2c_dev_node);
        unlock_mutex();
        return RP_HW_ERIIC;
    }

    close(i2c_dev_node);
    *value = (uint16_t)read_value;
    TRACE_SHORT("I2C Read Word success: addr=0x%02X, reg=0x%02X, val=0x%04X",
                i2c_dev_address, i2c_dev_reg_addr, *value);
    unlock_mutex();
    return RP_HW_OK;
}

int i2c_SMBUS_read_command(const char* i2c_dev_node_path, uint8_t i2c_dev_address, uint8_t *value, bool force){
    if (!value) {
        ERROR_LOG("Output value pointer is NULL");
        return RP_HW_EBIIC;
    }
    if (!i2c_dev_node_path) {
        ERROR_LOG("Device path is NULL");
        return RP_HW_EBIIC;
    }

    int ret = lock_mutex();
    if (ret != RP_HW_OK) return ret;

    int i2c_dev_node = 0;
    int ret_val = 0;
    __s32 read_value = 0;

    ret_val = openDevice(i2c_dev_node_path, i2c_dev_address, force, &i2c_dev_node);

    if (ret_val != RP_HW_OK){
        unlock_mutex();
        return ret_val;
    }

    read_value = i2c_smbus_read_byte(i2c_dev_node);

    if (read_value < 0) {
        if (errno == EREMOTEIO) {
            TRACE("I2C NACK received. Addr: 0x%02X", i2c_dev_address);
            close(i2c_dev_node);
            unlock_mutex();
            return RP_HW_ENACKIIC;
        }
        TRACE("I2C Read Command failed. Addr: 0x%02X, Errno: %d (%s)",
                  i2c_dev_address, errno, strerror(errno));
        close(i2c_dev_node);
        unlock_mutex();
        return RP_HW_ERIIC;
    }

    close(i2c_dev_node);
    *value = (uint8_t)read_value;
    TRACE_SHORT("I2C Read Command success: addr=0x%02X, val=0x%02X", i2c_dev_address, *value);
    unlock_mutex();
    return RP_HW_OK;
}

int i2c_SMBUS_read_buffer(const char* i2c_dev_node_path, uint8_t i2c_dev_address, uint8_t i2c_dev_reg_addr,
                           uint8_t *buffer, int *len, bool force){
    if (!buffer || !len) {
        ERROR_LOG("Buffer or length pointer is NULL: buffer=%p, len=%p", (void*)buffer, (void*)len);
        return RP_HW_EBIIC;
    }

    if (*len <= 0) {
        ERROR_LOG("Invalid buffer length: %d. Must be > 0", *len);
        return RP_HW_EIBLIIC;
    }

    if (*len > I2C_SMBUS_BLOCK_MAX) {
        WARNING("The buffer length (%d) is very large, limited to %d", *len, I2C_SMBUS_BLOCK_MAX);
        *len = I2C_SMBUS_BLOCK_MAX;
    }

    if (!i2c_dev_node_path) {
        ERROR_LOG("Device path is NULL");
        return RP_HW_EBIIC;
    }

    int ret = lock_mutex();
    if (ret != RP_HW_OK) return ret;

    int i2c_dev_node = 0;
    int ret_val = 0;
    __s32 read_value = 0;

    ret_val = openDevice(i2c_dev_node_path, i2c_dev_address, force, &i2c_dev_node);

    if (ret_val != RP_HW_OK){
        unlock_mutex();
        return ret_val;
    }

    read_value = i2c_smbus_read_i2c_block_data(i2c_dev_node, i2c_dev_reg_addr, *len, buffer);

    if (read_value < 0) {
        if (errno == EREMOTEIO) {
            TRACE("I2C NACK received. Addr: 0x%02X, Reg: 0x%02X", i2c_dev_address, i2c_dev_reg_addr);
            close(i2c_dev_node);
            unlock_mutex();
            return RP_HW_ENACKIIC;
        }
        if (errno == ETIMEDOUT) {
            TRACE("I2C timeout. Addr: 0x%02X, Reg: 0x%02X", i2c_dev_address, i2c_dev_reg_addr);
            close(i2c_dev_node);
            unlock_mutex();
            return RP_HW_ETOIIC;
        }
        TRACE("I2C Read Buffer failed. Addr: 0x%02X, Reg: 0x%02X, Req Len: %d, Errno: %d (%s)",
                  i2c_dev_address, i2c_dev_reg_addr, *len, errno, strerror(errno));
        close(i2c_dev_node);
        unlock_mutex();
        return RP_HW_ERIIC;
    }

    close(i2c_dev_node);
    *len = read_value;
    TRACE_SHORT("I2C Read Buffer success: addr=0x%02X, reg=0x%02X, read=%d bytes",
                i2c_dev_address, i2c_dev_reg_addr, *len);
    unlock_mutex();
    return RP_HW_OK;
}

int i2c_IOCTL_read_buffer(const char* i2c_dev_node_path, uint8_t i2c_dev_address, uint8_t *buffer, int len, bool force){
    if (!buffer) {
        ERROR_LOG("Buffer is NULL");
        return RP_HW_EBIIC;
    }
    if (len <= 0) {
        ERROR_LOG("Invalid buffer length: %d. Must be > 0", len);
        return RP_HW_EIBLIIC;
    }
    if (!i2c_dev_node_path) {
        ERROR_LOG("Device path is NULL");
        return RP_HW_EBIIC;
    }

    int ret = lock_mutex();
    if (ret != RP_HW_OK) return ret;

    int i2c_dev_node = 0;
    int ret_val = 0;

    ret_val = openDevice(i2c_dev_node_path, i2c_dev_address, force, &i2c_dev_node);

    if (ret_val != RP_HW_OK){
        unlock_mutex();
        return ret_val;
    }

    struct i2c_rdwr_ioctl_data data;
    struct i2c_msg message;

    message.addr = i2c_dev_address;
    message.flags = 1;  // Read
    message.len = len;
    message.buf = (unsigned char*)buffer;

    data.msgs = &message;
    data.nmsgs = 1;

    if (ioctl(i2c_dev_node, I2C_RDWR, &data) < 0){
        if (errno == EREMOTEIO) {
            TRACE("I2C NACK received. Addr: 0x%02X", i2c_dev_address);
            close(i2c_dev_node);
            unlock_mutex();
            return RP_HW_ENACKIIC;
        }
        if (errno == ETIMEDOUT) {
            TRACE("I2C timeout. Addr: 0x%02X", i2c_dev_address);
            close(i2c_dev_node);
            unlock_mutex();
            return RP_HW_ETOIIC;
        }
        if (errno == EAGAIN) {
            TRACE("I2C bus busy. Addr: 0x%02X", i2c_dev_address);
            close(i2c_dev_node);
            unlock_mutex();
            return RP_HW_EBUSYIIC;
        }
        TRACE("I2C IOCTL Read failed. Addr: 0x%02X, Len: %d, Errno: %d (%s)",
                  i2c_dev_address, len, errno, strerror(errno));
        close(i2c_dev_node);
        unlock_mutex();
        return RP_HW_ERIIC;
    }

    TRACE_SHORT("I2C IOCTL Read success: addr=0x%02X, read=%d bytes", i2c_dev_address, len);
    close(i2c_dev_node);
    unlock_mutex();
    return RP_HW_OK;
}

int i2c_IOCTL_write_buffer(const char* i2c_dev_node_path, uint8_t i2c_dev_address, uint8_t *buffer, int len, bool force){
    if (!buffer) {
        ERROR_LOG("Buffer is NULL");
        return RP_HW_EBIIC;
    }
    if (len <= 0) {
        ERROR_LOG("Invalid buffer length: %d. Must be > 0", len);
        return RP_HW_EIBLIIC;
    }
    if (!i2c_dev_node_path) {
        ERROR_LOG("Device path is NULL");
        return RP_HW_EBIIC;
    }

    int ret = lock_mutex();
    if (ret != RP_HW_OK) return ret;

    int i2c_dev_node = 0;
    int ret_val = 0;

    ret_val = openDevice(i2c_dev_node_path, i2c_dev_address, force, &i2c_dev_node);

    if (ret_val != RP_HW_OK){
        unlock_mutex();
        return ret_val;
    }

    struct i2c_rdwr_ioctl_data data;
    struct i2c_msg message;

    message.addr = i2c_dev_address;
    message.flags = 0;  // Write
    message.len = len;
    message.buf = (unsigned char*)buffer;

    data.msgs = &message;
    data.nmsgs = 1;

    if (ioctl(i2c_dev_node, I2C_RDWR, &data) < 0){
        if (errno == EREMOTEIO) {
            TRACE("I2C NACK received. Addr: 0x%02X", i2c_dev_address);
            close(i2c_dev_node);
            unlock_mutex();
            return RP_HW_ENACKIIC;
        }
        if (errno == ETIMEDOUT) {
            TRACE("I2C timeout. Addr: 0x%02X", i2c_dev_address);
            close(i2c_dev_node);
            unlock_mutex();
            return RP_HW_ETOIIC;
        }
        if (errno == ENXIO) {
            TRACE("I2C device not responding. Addr: 0x%02X", i2c_dev_address);
            close(i2c_dev_node);
            unlock_mutex();
            return RP_HW_ENRSPIIC;
        }
        TRACE("I2C IOCTL Write failed. Addr: 0x%02X, Len: %d, Errno: %d (%s)",
                  i2c_dev_address, len, errno, strerror(errno));
        close(i2c_dev_node);
        unlock_mutex();
        return RP_HW_EWIIC;
    }

    TRACE_SHORT("I2C IOCTL Write success: addr=0x%02X, wrote=%d bytes", i2c_dev_address, len);
    close(i2c_dev_node);
    unlock_mutex();
    return RP_HW_OK;
}