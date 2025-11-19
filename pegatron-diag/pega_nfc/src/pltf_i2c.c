/******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2020 STMicroelectronics</center></h2>
  *
  * Licensed under ST MYLIBERTY SOFTWARE LICENSE AGREEMENT (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/myliberty
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied,
  * AND SPECIFICALLY DISCLAIMING THE IMPLIED WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
******************************************************************************/

/*! \file pltf_i2c.c
 *
 *  \author
 *
 *  \brief Implementation for I2C communication.
 *
 */
 /*
 ******************************************************************************
 * INCLUDES
 ******************************************************************************
 */
#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>

#include "pltf_i2c.h"
#include "rfal_platform.h"


/*
 ******************************************************************************
 * DEFINES
 ******************************************************************************
 */
#ifndef I2C_DETAILED_LOG
#define I2C_DETAILED_LOG false
#endif

/*
 ******************************************************************************
 * STATIC VARIABLES
 ******************************************************************************
 */
#define ST25R3916_FIFO_DEPTH 512U /*!< Depth of FIFO   */
#define ST25R3916_CMD_LEN (1U)    /*!< ST25R3916 CMD length   */
#define ST25R3916_BUF_LEN \
    (ST25R3916_CMD_LEN + ST25R3916_FIFO_DEPTH)  /*!< ST25R3916 communication buffer: CMD + FIFO length    */
static uint8_t com_buf[ST25R3916_BUF_LEN];      /*!< ST25R3916 communication buffer            */
static uint16_t com_buf_len;                    /*!< ST25R3916 communication buffer iterator   */

#define I2CSLAVE 0x50

static int i2c_handle = -1;
static int nfc_slave_addr = I2CSLAVE;
static pthread_mutex_t i2c_lock;

/*
 ******************************************************************************
 * GLOBAL AND HELPER FUNCTIONS
 ******************************************************************************
 */
stError i2c_init()
{
    char dev_path[] = "/dev/i2c-3";

    int fd = open(dev_path, O_RDWR);
    if (fd < 0) {
        platformLog("Open i2c bus %s for nfc failed\n", dev_path);
        return ERR_IO;
    }
    platformLog("Open i2c bus %s for nfc successful\n", dev_path);

    if (ioctl(fd, I2C_SLAVE, I2CSLAVE) < 0) {
        platformLog("Set slave address of the sensor %s to (%#x) failed\n", dev_path, I2CSLAVE);
        close(fd);
        return ERR_IO;
    }

    i2c_handle = fd;
    platformLog("Set slave address of the sensor %s to (%#x) successful\n", dev_path, I2CSLAVE);

    if (pthread_mutex_init(&i2c_lock, NULL) != 0) {
        platformLog("Failed to initialize i2c lock\n");
        return ERR_INTERNAL;
    }

    return ERR_NONE;
}

void i2c_start(void)
{
    com_buf_len = 0;
    memset(com_buf, 0, ST25R3916_BUF_LEN);
}

static bool _i2c_read_wrapper(uint8_t* tx_buf, uint32_t tx_buf_len, uint8_t* rx_buf, uint32_t rx_buf_len)
{
    struct i2c_msg messages[2];
    struct i2c_rdwr_ioctl_data packets;

    messages[0].addr = nfc_slave_addr;
    messages[0].flags = 0;
    messages[0].len = tx_buf_len;
    messages[0].buf = tx_buf;

    messages[1].addr = nfc_slave_addr;
    messages[1].flags = I2C_M_RD;
    messages[1].len = rx_buf_len;
    messages[1].buf = rx_buf;

    packets.msgs = messages;
    packets.nmsgs = 2;

    if(ioctl(i2c_handle, I2C_RDWR, &packets) < 0) {
        return false;
    }

    return true;
}

static bool _i2c_write_wrapper(uint8_t* tx_buf, uint32_t tx_buf_len)
{
    struct i2c_msg messages[1];
    struct i2c_rdwr_ioctl_data packets;

    messages[0].addr = nfc_slave_addr;
    messages[0].flags = 0;
    messages[0].len = tx_buf_len;
    messages[0].buf = tx_buf;

    packets.msgs = messages;
    packets.nmsgs = 1;

    if(ioctl(i2c_handle, I2C_RDWR, &packets) < 0) {
        return false;
    }

    return true;
}

#if I2C_DETAILED_LOG
static char dump_buf[ST25R3916_BUF_LEN*3];

static void string_to_hex(const uint8_t* input, uint32_t bytes)
{
    memset(dump_buf, 0, ST25R3916_BUF_LEN*3);
    for (int i = 0; i < bytes; i++) {
        sprintf(dump_buf, "%s, %x", dump_buf, input[i]);
    }
}
#endif

stError i2c_tx(const uint8_t* tx_buf, uint16_t tx_buf_len, bool last, bool tx_only)
{
    uint16_t remain_len = ST25R3916_BUF_LEN - com_buf_len;
    uint16_t transfer_len = tx_buf_len < remain_len ? tx_buf_len : remain_len;
    memcpy(&com_buf[com_buf_len], tx_buf, transfer_len);
    com_buf_len += transfer_len;

    if (last && tx_only) {
#if I2C_DETAILED_LOG
        string_to_hex(com_buf, com_buf_len);
        platformLog("i2c_tx, value %s\n", dump_buf);
#endif

        if (!_i2c_write_wrapper(com_buf, com_buf_len)) {
            platformLog("i2c_tx failed\n");
            return ERR_IO;
        }
    }

    return ERR_NONE;
}

stError i2c_rx(const uint8_t* rx_buf, uint16_t rx_buf_len)
{
    if (!_i2c_read_wrapper(com_buf, com_buf_len, (uint8_t*)rx_buf, rx_buf_len)) {
        platformLog("i2c_rx, tx (part of rx) failed\n");
        return ERR_IO;
    }

#if I2C_DETAILED_LOG
    string_to_hex(com_buf, com_buf_len);
    platformLog("i2c_rx, reg_addr %s, ", dump_buf);
    string_to_hex(rx_buf, rx_buf_len);
    platformLog("value: %s\n", dump_buf);
#endif

    return ERR_NONE;
}

void pltf_protect_com(void)
{
    pthread_mutex_lock(&i2c_lock);
}

void pltf_unprotect_com(void)
{
    pthread_mutex_unlock(&i2c_lock);
}
