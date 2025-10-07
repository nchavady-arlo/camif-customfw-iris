#ifdef GPL_FILE
/** @file   chip_simulator_gpio.c
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
/** @file   chip_simulator_gpio.c
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
#include <linux/gpio.h>
#include "chip_simulator_gpio.h"


int gpio_open(char *dev, int line, char *label, GPIO_DIR direction)
{
  int fd, ret =-1;
  struct gpioevent_request reqEvent;
  struct gpiohandle_request reqHandle;

  fd = open(dev, O_RDWR);
  if(fd < 0) {
    fprintf(stderr, "%s(%s) open error: %s\n", __FUNCTION__, dev, strerror(errno));
    goto out;
  }

  switch(direction) {
  case GPIO_INPUT_FALLING:
    reqEvent.lineoffset   = line;
    reqEvent.handleflags  = GPIOHANDLE_REQUEST_INPUT;
    reqEvent.eventflags   = GPIOEVENT_REQUEST_FALLING_EDGE;
    snprintf(reqEvent.consumer_label, sizeof(reqEvent.consumer_label), "%s", label);
    if(ioctl(fd, GPIO_GET_LINEEVENT_IOCTL, &reqEvent) == -1) {
      fprintf(stderr, "%s(%s) config input falling error: %s\n", __FUNCTION__, dev, strerror(errno));
      goto out;
    }
    ret = reqEvent.fd;
    break;

  case GPIO_INPUT_RISING:
    reqEvent.lineoffset   = line;
    reqEvent.handleflags  = GPIOHANDLE_REQUEST_INPUT;
    reqEvent.eventflags   = GPIOEVENT_REQUEST_RISING_EDGE;
    snprintf(reqEvent.consumer_label, sizeof(reqEvent.consumer_label), "%s", label);
    if(ioctl(fd, GPIO_GET_LINEEVENT_IOCTL, &reqEvent) == -1) {
      fprintf(stderr, "%s(%s) config input rising error: %s\n", __FUNCTION__, dev, strerror(errno));
      goto out;
    }
    ret = reqEvent.fd;
    break;

  case GPIO_OUTPUT:
    reqHandle.flags             = GPIOHANDLE_REQUEST_OUTPUT;
    reqHandle.lines             = 1;
    reqHandle.lineoffsets[0]    = line;
    reqHandle.default_values[0] = 1;
    snprintf(reqHandle.consumer_label, sizeof(reqHandle.consumer_label), "%s", label);
    if(ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, &reqHandle) == -1) {
      fprintf(stderr, "%s(%s) config output error: %s\n", __FUNCTION__, dev, strerror(errno));
      goto out;
    }
    ret = reqHandle.fd;
    break;
  }

out:
  if(fd > 0)
    close(fd);

  return ret;
}

u_int8_t gpio_get(int fd)
{
  struct gpiohandle_data data = {0};

  if(ioctl(fd, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &data) == -1) {
    fprintf(stderr, "%s(%d) error: %s\n", __FUNCTION__, fd, strerror(errno));
  }

  return data.values[0];
}

void gpio_set(int fd, u_int8_t value)
{
  struct gpiohandle_data data;

  data.values[0] = value;
  if(ioctl(fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data) == -1) {
    fprintf(stderr, "%s(%d, %d) error: %s\n", __FUNCTION__, fd, value, strerror(errno));
  }
}

void gpio_close(int fd)
{
  close(fd);
}
