#ifdef GPL_FILE
/** @file   chip_simulator_main.c
 *
 *  @brief  This file contains the function prototypes of the Nxp specific
 *          Helper Protocol.
 *
 *  Copyright 2022 NXP
 *
 *  This software file (the File) is distributed by NXP
 *  under the terms of the GNU General Public License Version 2, June 1991
 *  (the License).  You may use, redistribute and/or modify the File in
 *  accordance with the terms and conditions of the License, a copy of which
 *  is available by writing to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA or on the
 *  worldwide web at http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt.
 *
 *  THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE
 *  ARE EXPRESSLY DISCLAIMED.  The License provides additional details about
 *  this warranty disclaimer.
 *
 */
#else
/** @file   chip_simulator_main.c
 *
 *  @brief  This file contains the function prototypes of the Nxp specific
 *          Helper Protocol.
 *
 *  Copyright 2022 NXP
 *
 *  NXP CONFIDENTIAL
 *  The source code contained or described herein and all documents related to
 *  the source code (Materials) are owned by NXP, its
 *  suppliers and/or its licensors. Title to the Materials remains with NXP,
 *  its suppliers and/or its licensors. The Materials contain
 *  trade secrets and proprietary and confidential information of NXP, its
 *  suppliers and/or its licensors. The Materials are protected by worldwide copyright
 *  and trade secret laws and treaty provisions. No part of the Materials may be
 *  used, copied, reproduced, modified, published, uploaded, posted,
 *  transmitted, distributed, or disclosed in any way without NXP's prior
 *  express written permission.
 *
 *  No license under any patent, copyright, trade secret or other intellectual
 *  property right is granted to or conferred upon you by disclosure or delivery
 *  of the Materials, either expressly, by implication, inducement, estoppel or
 *  otherwise. Any license under such intellectual property rights must be
 *  express and approved by NXP in writing.
 *
 */
#endif

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <signal.h>
#include <stdarg.h>
#include <time.h>
#include "fw_loader_frames.h"
#include "chip_simulator_spi.h"
#include "chip_simulator_gpio.h"


#define MY_SPI_SLAVE_DEV        "/dev/spidev1.0"      /* if it does not exist:
                                                       * kernel > 5.13: echo dh2228fv > /sys/class/spi_slave/spiX/slave
                                                       * kernel < 5.13: echo spidev > /sys/class/spi_slave/spiX/slave
                                                       * and use imx8mm-evk-ecspi-slave.dtb */
//#define MY_MY_SPI_SLAVE_MAX_FRANSFER_SIZE 0x800
#define MY_SPI_SLAVE_MAX_FRANSFER_SIZE 0x200          /* Max from the kernel driver spi-imx.c: MX53_MAX_TRANSFER_BYTES,
                                                         refer to Freescale errata ERR003775 */

/*
#define MY_GPIO_IN_DEV          "/dev/gpiochip5"
#define MY_GPIO_IN_LINE         12
#define MY_GPIO_IN_LABEL        "NXP_NCP_Fake_Loader_RST"
*/
#define MY_GPIO_OUT_DEV         "/dev/gpiochip5"
#define MY_GPIO_OUT_LINE        13
#define MY_GPIO_OUT_LABEL       "NXP_NCP_Fake_Loader_DATA"

#define STARTIND_TIMEOUT  -1
//#define STARTIND_TIMEOUT  1000
#define DATAREQ_TIMEOUT   1000
#define ACKNAK_TIMEOUT    1000
#define ERROR_TIMEOUT     5000
#define DATA_TIMEOUT      5000

static CTRL_FRAME txFrame;
static CTRL_FRAME rxFrame;
static u_int8_t   rxData[SPI_MAX_FRAME_SIZE];

static u_int8_t crc8_table[256];     /* 8-bit table */
static u_int8_t made_table = 0;
static u_int8_t running = 1;


static void sighandler(int sig)
{
    sig = 0;
    running = sig;
}


static void init_crc8(void)
{
#define DI            0x07
  int i,j;
  int crc;

  if (!made_table) {
    for (i = 0; i < 256; i++) {
      crc = i;
      for (j = 0; j < 8; j++)
        crc = (crc << 1) ^ ((crc & 0x80) ? DI : 0);
      crc8_table[i] = (u_int8_t) ((u_int8_t)crc & (u_int8_t)0xFF);
    }
    made_table = 1;
  }
}

static u_int8_t crc8 (u_int8_t *array, u_int16_t len)
{
  u_int8_t CRC = 0xff;

  for(;len >0 ; len--)
  {
    CRC = crc8_table[CRC ^ *array];
    array++;
  }
  return CRC;
}


void delayMs(u_int32_t ms)
{
    struct timespec ReqTime;
    time_t sec;

    // Initialize to 0
    ReqTime.tv_sec = 0;
    ReqTime.tv_nsec = 0;

    // Calculate the Delay
    sec = (time_t)(ms/1000);
    ms -= ((u_int32_t)sec * (u_int32_t)1000);
    ReqTime.tv_sec=sec;
    ReqTime.tv_nsec = (u_int32_t)ms*1000000L; // 1 ms = 1000000 ns

    // Sleep
    while (nanosleep(&ReqTime,&ReqTime)== -1)
    {
      continue;
    }
}

int printWithMs(char *format, ...)
{
  int ret;
  va_list args;

  va_start (args, format);

  ret = fprintf(stdout, "%s ", fw_upload_getMsString());
  ret += vfprintf(stdout, format, args);

  va_end (args);

  return ret;
}


static int recvAckNak(int fd, u_int8_t *buf, int len, int timeout)
{
  int ret;

  /* Clear buffer */
  memset(buf, 0xFF, len);

  ret = spi_slave_read(fd, buf, len, timeout);
  if(ret < 0)
      printWithMs("read ack/nak failed, ret %d: %s\n", ret, strerror(errno));
  else if(ret == 0)
      printWithMs("read ack/nak timeout\n");
  else
    fw_upload_dumpCtrlFrame(RX_DIR, (CTRL_FRAME *)buf, ret, "");

  return ret;
}

static int recvData(int fd, u_int8_t *buf, int len, int timeout)
{
  /* Cannot use select to manage timeout:
   * select will exit immediately with FD_ISSET(fd, &readFd) even if the is no data available */
  int ret;

  /* Clear buffer */
  memset(buf, 0xFF, len);

  ret = spi_slave_read(fd, buf, len, timeout);
  if(ret < 0)
      printWithMs("read data failed, ret %d: %s\n", ret, strerror(errno));
  else if(ret == 0)
      printWithMs("read data timeout\n");
  else
    fw_upload_dumpDataFrame(RX_DIR, buf, ret, "");


  return ret;
}

static int sendStartIndication(int fdSpi, int fdGpioOut, u_int16_t chipId, u_int8_t loaderVer)
{
  int ret;
  int frameLen;

  frameLen = sizeof(txFrame.header) + sizeof(txFrame.startIndV3);

  txFrame.header                 = V3_START_INDICATION;
  txFrame.startIndV3.uiChipId    = chipId;
  txFrame.startIndV3.uiLoaderVer = loaderVer;
  txFrame.startIndV3.uiCrc       = crc8((u_int8_t *)&txFrame, frameLen - 1);

#if defined(SPI_SLAVE_SEND_CTRL_FRAME_10_BYTES)
  if(frameLen <= 10) {
    for(int i = frameLen; i<10; i++) {
      txFrame.raw[i-sizeof(txFrame.header)] = 0xFF;
    }
    frameLen = 10;
  } else {
    printWithMs("FATAL ERROR, frame len %d >10, ABORT\n", frameLen);
    exit(EXIT_FAILURE);
  }
#endif

  fw_upload_dumpCtrlFrame(TX_DIR, &txFrame, frameLen, "");

  ret = spi_slave_write(fdSpi, fdGpioOut, (u_int8_t *)&txFrame, frameLen, STARTIND_TIMEOUT);
  if(ret == 0)
    printWithMs("=> Timeout\n");
  if(ret < frameLen) {
      printWithMs("=> Failed to send StartIndication (%d of %d, errno %d: %s)\n", ret, frameLen, errno, strerror(errno));
      return 0;
  }

  frameLen = sizeof(rxFrame.header) + sizeof(rxFrame.ackNakV3);
  ret = recvAckNak(fdSpi, (u_int8_t *)&rxFrame, frameLen, ACKNAK_TIMEOUT);

  if(ret == frameLen && fw_upload_isDumpFrame())
    printf("\n");

  return ret;

}

static int sendDataReq(int fdSpi, int fdGpioOut, u_int8_t *buf, int16 len, u_int32_t offset, u_int16_t error)
{
  int ret;
  int frameLen;

  memset(buf, 0xFF, len);

  frameLen = sizeof(txFrame.header) + sizeof(txFrame.dataReqV3);

  memset(&rxFrame, 0xFF, sizeof(rxFrame));

  txFrame.header             = V3_HEADER_DATA_REQ;
  txFrame.dataReqV3.uiLen    = len;
  txFrame.dataReqV3.ulOffset = offset;
  txFrame.dataReqV3.uiError  = error;

  txFrame.dataReqV3.uiCrc    = crc8((u_int8_t *)&txFrame, frameLen - 1);

#if defined(SPI_SLAVE_SEND_CTRL_FRAME_10_BYTES)
  if(frameLen <= 10) {
    for(int i = frameLen; i<10; i++) {
      txFrame.raw[i-sizeof(txFrame.header)] = 0xFF;
    }
    frameLen = 10;
  } else {
    printWithMs("FATAL ERROR, frame len %d >10, ABORT\n", frameLen);
    exit(EXIT_FAILURE);
  }
#endif

  fw_upload_dumpCtrlFrame(TX_DIR, &txFrame, frameLen, "");

  ret = spi_slave_write(fdSpi, fdGpioOut, (u_int8_t *)&txFrame, frameLen, DATAREQ_TIMEOUT);
  if(ret == 0) {
    printWithMs("=> Timeout\n");
    return ret;
  }
  if(ret < frameLen) {
    printWithMs("=> Failed to send DataReq (%d of %d)\n", ret, frameLen);
    return 0;
  }
  /* SPI Slave on IMX8 is not recommended to go faster that 1MHz, yes one! */
  /* Sometimes we see on SPI 0x00 instead of 0xa7, offset, error & crc are ok, but not detected here :( */
  /* Other times we see on SPI this transfer splitted on 2 transfers and the first one doesn't have a7 any more :( */

  frameLen = sizeof(rxFrame.header);
  frameLen += (error)?(sizeof(rxFrame.timeoutV3)):(sizeof(rxFrame.ackNakV3));

  /* Wait for Ack/Nak/timeout */
  ret = recvAckNak(fdSpi, (u_int8_t *)&rxFrame, frameLen, (error)?(ERROR_TIMEOUT):(ACKNAK_TIMEOUT));
  if(ret <= 0 || error)
    return ret;

  /* Wait for Data */
  if(txFrame.dataReqV3.uiLen) {
      ret = recvData(fdSpi, buf, len, DATA_TIMEOUT);

      if(ret == txFrame.dataReqV3.uiLen && fw_upload_isDumpFrame())
        printf("\n");
  }

  return ret;
}


u_int16_t getErrorCode(int expLen, int lastRet/* ... */)
{
  u_int16_t error = 0x0000;

  if(lastRet == 0) {
    /* Timeout */
      if(rxFrame.header == 0xFF)
        error |= (1<<2); /* Timeout Ack */
      if(rxFrame.header == V3_REQUEST_ACK)
        error |= (1<<4); /* Timeout Data */
  }
  else if(txFrame.dataReqV3.uiLen == expLen) {
    /* Post request */
      if(rxFrame.header == V3_REQUEST_ACK && expLen != lastRet) {
          /* Incomplete data */
        error |= (1<<1);
      }
  }


  /* ... */

  return error;
}

int main(int argc, char *argv[])
{
  int fdSpi     = -1;
  int fdGpioOut = -1;
  int ret       = -1;
  u_int16_t len = 0;
  u_int32_t offset = 0;

  signal(SIGINT, sighandler);

  init_crc8();

  fdGpioOut = gpio_open(MY_GPIO_OUT_DEV, MY_GPIO_OUT_LINE, MY_GPIO_OUT_LABEL, GPIO_OUTPUT);
  if(fdGpioOut < 0)
    goto err;

  fdSpi = spi_slave_open(MY_SPI_SLAVE_DEV);
  if(fdSpi < 0)
    goto err;

  /* Start */

  do {
    u_int16_t chipId = 0x7601;
    u_int8_t version = 0x00;
    HELPER_HEADER Helper;
    int dataXfered = 0;

    rxFrame.header = V3_REQUEST_ACK;

    printf("My ChipID is : %04x, my Version is : %d\n", chipId, version);

    ret = sendStartIndication(fdSpi, fdGpioOut, chipId, version);
    if(rxFrame.header != V3_REQUEST_ACK || !ret || !running) { printWithMs("Abort at StartInd (running: %d, ret %d, rxHeader %02x\n", running, ret, rxFrame.header); break;}
    if(!running || ret < 0) break;

    do {
      ret = sendDataReq(fdSpi, fdGpioOut, (u_int8_t *)&Helper, sizeof(Helper), 0x00000000, getErrorCode(sizeof(Helper), ret));
    } while(running && rxFrame.header != V3_REQUEST_ACK && ret < sizeof(Helper));
    if(!running || ret < 0) break;

    /* Loop downloading file using CMD1 (data payload), end detected by CMD4 (entry point) with len = 0 */
    for(offset = 0x00000010; running;) {

      len = sizeof(Helper);
      memset(&Helper, 0xFF, sizeof(Helper));
      do {
        ret = sendDataReq(fdSpi, fdGpioOut, (u_int8_t *)&Helper, len, offset, getErrorCode(len, ret));
      } while(running && rxFrame.header != V3_REQUEST_ACK && ret < len);
      if(!running || ret < 0) { running = 0; break; }

      dataXfered += len;
      offset += len;

      printf("File downloaded: %8d\r", dataXfered);

      /* End of download detection */
      if(Helper.ulCmd == 4 && Helper.ulLen == 0)
          break;

      if(Helper.ulCmd == 1) {
        if(Helper.ulLen <= MY_SPI_SLAVE_MAX_FRANSFER_SIZE) {
          len = Helper.ulLen;
        } else {
          printf("ERROR, TransferLen requested: %d, max: %d\n", Helper.ulLen, MY_SPI_SLAVE_MAX_FRANSFER_SIZE);
          sendDataReq(fdSpi, fdGpioOut, (u_int8_t *)&Helper, sizeof(Helper), 0x00000010, (1<<15) | (1<<1));
          printf("Please recompile you target with following patch:\n");
          printf("--- a/src/build/env/common.mak\n");
          printf("+++ b/src/build/env/common.mak\n");
          printf("@@ -809,7 +809,7 @@ ifeq (1,$(BUILT_SDIO))\n");
          printf("     BREAKSIZE = 1024\n");
          printf(" else\n");
          printf(" ifeq (1,$(BUILT_BT_UART))\n");
          printf("-    BREAKSIZE = 2048\n");
          printf("+    BREAKSIZE = 512\n");
          printf(" else\n");
          printf("     BREAKSIZE = 512\n");
          printf(" endif\n");
          running = 0;
          break;
        }
      } else {
        /* Invalid sequence */
          printf("ERROR, cmd%d is out of sequence\n", Helper.ulCmd);
          sendDataReq(fdSpi, fdGpioOut, (u_int8_t *)&Helper, sizeof(Helper), 0x00000010, (1<<15) | (1<<1));
          running = 0;
          break;
      }

      do {
        ret = sendDataReq(fdSpi, fdGpioOut, rxData, len, offset, getErrorCode(len , ret));
      } while(running && rxFrame.header != V3_REQUEST_ACK && ret < len);
      if(!running || ret < 0) { running = 0; break; }

      dataXfered += len;
      offset += len;

      /* Well, just do nothing of the received binary (rxData) */
    }

    if(!running)
        break;

    len = 0;
    memset(&Helper, 0xFF, sizeof(Helper));
    ret = sendDataReq(fdSpi, fdGpioOut, (u_int8_t *)&Helper, len, offset, getErrorCode(len, ret));
    if(!running || rxFrame.header != V3_REQUEST_ACK) break;

    printf("\nDownload complete, clear SPI_INT\n");
    gpio_set(fdGpioOut, 0);
    printf("Fake start firmware");
    for(int i=0; i<10;i++) {
      printf(".");
      delayMs(10);
    }
    printf("ok, set SPI_INT\n");
    gpio_set(fdGpioOut, 1);
  } while(0);

  if(!running) {
    printWithMs("\n");
    printWithMs("Status before leaving: \n");
    printWithMs("offset: %08x, len: %04x\n", offset, len);
    printWithMs("last header: %02x\n", rxFrame.header);
    printWithMs("ret: %d, running: %d\n", ret, running);
    printWithMs("\n");
  }

  /* Stop */

  gpio_set(fdGpioOut, 1);
  gpio_close(fdGpioOut);
  spi_slave_close(fdSpi);
  return 0;

err:
  if(fdGpioOut > 0)
    gpio_close(fdGpioOut);
  if(fdSpi > 0)
    gpio_close(fdSpi);
  return 1;
}
