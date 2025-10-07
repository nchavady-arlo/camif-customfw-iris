#ifdef GPL_FILE
/** @file   chip_simulator_spi.c
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
/** @file   chip_simulator_spi.c
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
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include "chip_simulator_spi.h"
#include "chip_simulator_timer.h"
#include "chip_simulator_gpio.h"

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#define SPI_USE_TRANSFER       0
#define SPI_USE_READ_WRITE     1
#define SPI_USE_SELECT         2

#define SPI_XFER         SPI_USE_TRANSFER
//#define SPI_XFER         SPI_USE_SELECT
//#define SPI_XFER         SPI_USE_READ_WRITE

//#define DEBUG_SPI

static char *hisDev = NULL;
static timer_t *myTimer =NULL;

#if (SPI_XFER == SPI_USE_TRANSFER)
static u_int8_t spiRxBuffer[SPI_MAX_FRAME_SIZE];
#endif


static void timeout_handler(union sigval sigv)
{
  /* to abort ongoing transfer, do open/close, it will trig in kernel - spidev_release():
    #ifdef CONFIG_SPI_SLAVE
        if (!dofree)
            spi_slave_abort(spidev->spi);
    #endif
    if will return errno EINTR (Interrupted system call)
   */
  close(open(hisDev, O_RDWR));
}



int spi_slave_open(char *dev)
{
  int fd;
  u_int8_t  mode, bitsPerWord;
  u_int32_t speed;

  fd = open(dev, O_RDWR | O_CLOEXEC | O_NONBLOCK);
  if(fd < 0) {
    fprintf(stderr, "%s(%s) open error: %s\n", __FUNCTION__, dev, strerror(errno));
    goto out;
  }

  if(ioctl(fd, SPI_IOC_RD_MODE, &mode)) {
    fprintf(stderr, "%s(%s) read mode error: %s\n", __FUNCTION__, dev, strerror(errno));
    goto err;
  }
  printf("%s(%s) current mode:        %d\n", __FUNCTION__, dev, mode);
  mode = SPI_MODE_0;
  if(ioctl(fd, SPI_IOC_WR_MODE, &mode)) {
    fprintf(stderr, "%s(%s) write mode error: %s\n", __FUNCTION__, dev, strerror(errno));
    goto err;
  }

  if(ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed)) {
    fprintf(stderr, "%s(%s) read speed error: %s\n", __FUNCTION__, dev, strerror(errno));
    goto err;
  }
  printf("%s(%s) current speed:       %d\n", __FUNCTION__, dev, speed);
  speed = 8000000;
  if(ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed)) {
    fprintf(stderr, "%s(%s) write speed error: %s\n", __FUNCTION__, dev, strerror(errno));
    goto err;
  }

  if(ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bitsPerWord)) {
    fprintf(stderr, "%s(%s) read bitsPerWord error: %s\n", __FUNCTION__, dev, strerror(errno));
    goto err;
  }
  printf("%s(%s) current bitsPerWord: %d\n", __FUNCTION__, dev, bitsPerWord);
  bitsPerWord = 8;
  if(ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bitsPerWord)) {
    fprintf(stderr, "%s(%s) write bitsPerWord error: %s\n", __FUNCTION__, dev, strerror(errno));
    goto err;
  }

  myTimer = timer_open(timeout_handler, NULL);
  if(!myTimer)
    goto err;

  hisDev   = dev;

  printf("%s(%s) return %d\n", __FUNCTION__, dev, fd);
  return fd;


err:
  close(fd);
out:
  return -1;
}

#if (SPI_XFER == SPI_USE_TRANSFER)
static int spi_slave_doTransfer(int fd, int fdGpioOut, u_int8_t *txBuf, int txLen, u_int8_t *rxBuf, int rxLen, int timeout)
{
  int ret;
  struct spi_ioc_transfer spiTransfer[1];
  int spiLen;

  memset(spiTransfer, 0, sizeof(spiTransfer));
  spiLen = MAX(txLen, rxLen);

#ifdef DEBUG_SPI
  printWithMs("SPI doTransfer tx %d, rx: %d => transferLen : %d\n", txLen, rxLen, spiLen);
#endif

  spiTransfer[0].tx_buf        = (__u64)txBuf;
  spiTransfer[0].rx_buf        = (__u64)rxBuf;
  spiTransfer[0].len           = spiLen;
  spiTransfer[0].speed_hz      = 8000000;
  spiTransfer[0].delay_usecs   = 0;
  spiTransfer[0].bits_per_word = 8;
  spiTransfer[0].cs_change     = FALSE;

#ifdef DEBUG_SPI
  if(txLen > 0 && txBuf)
    log_buffer(txBuf, spiLen, "SPI Tx %d", spiLen);
#endif

  timer_start(myTimer, timeout);
  if(fdGpioOut > 0)   gpio_set(fdGpioOut, 0);
  ret = ioctl(fd, SPI_IOC_MESSAGE(1), spiTransfer);
  if(fdGpioOut > 0)   gpio_set(fdGpioOut, 1);
  timer_stop(myTimer);
  if(ret < 0) {
    perror("Can't transfer Spi");
    return ret;
  }

#ifdef DEBUG_SPI
  log_buffer(rxBuf, spiLen, "SPI Rx %d", spiLen);
#endif

  return spiLen;
}
#endif

int spi_slave_read(int fd, u_int8_t *buf, int len, int timeout)
{
  int ret = -1;

#if (SPI_XFER == SPI_USE_TRANSFER)
  ret = spi_slave_doTransfer(fd, -1, NULL, 0, buf, len, timeout);
#elif (SPI_XFER == SPI_USE_SELECT)
#error SPI_XFER == SPI_USE_SELECT not working
  /* Cannot use select to manage timeout:
   * select will exit immediately with FD_ISSET(fd, &readFd) even if the is no data available */
#elif (SPI_XFER == SPI_USE_READ_WRITE)
  timer_start(myTimer, timeout);
  ret = read(fd, b, len);
  timer_stop(myTimer);
#endif

  if(ret == -1 && errno == EINTR)
    ret = 0; /* timeout */

  return ret;
}

int spi_slave_write(int fd, int fdGpioOut, u_int8_t *buf, int len, int timeout)
{
  int ret;

#if (SPI_XFER == SPI_USE_TRANSFER)
  ret = spi_slave_doTransfer(fd, fdGpioOut, buf, len, spiRxBuffer, 0, timeout);
#elif (SPI_XFER == SPI_USE_SELECT) || (SPI_XFER == SPI_USE_READ_WRITE)
  timer_start(myTimer, timeout);
  gpio_set(fdGpioOut, 0);
  ret = write(fd, buf, len);
  gpio_set(fdGpioOut, 1);
  timer_stop(myTimer);
#endif

  if(ret == -1 && errno == EINTR)
    ret = 0; /* timeout */

  return ret;
}

void spi_slave_close(int fd)
{
  int ret;
  timer_stop(myTimer);
  timer_close(myTimer);
  ret = close(fd);
  printf("%s(%d) return %d\n", __FUNCTION__, fd, ret);
}
