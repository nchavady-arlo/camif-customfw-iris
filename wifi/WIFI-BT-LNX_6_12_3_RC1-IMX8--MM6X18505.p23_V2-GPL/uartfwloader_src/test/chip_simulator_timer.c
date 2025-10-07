#ifdef GPL_FILE
/** @file   chip_simulator_timer.c
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
/** @file   chip_simulator_timer.c
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
//#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
//#include <sys/stat.h>
//#include <fcntl.h>
//#include <sys/ioctl.h>
//#include <linux/gpio.h>
#include "chip_simulator_timer.h"


timer_t *timer_open(void (*handler) (__sigval_t), void *info)
{
  int ret;
  timer_t *timer;
  struct sigevent sev = {0};

  timer = (timer_t *)malloc(sizeof(timer_t));
  if(!timer) {
    fprintf(stderr, "%s() malloc error: %s\n", __FUNCTION__, strerror(errno));
    return NULL;
  }
  memset(timer, 0, sizeof(timer_t));

  sev.sigev_notify            = SIGEV_THREAD;
  sev.sigev_value.sival_ptr   = timer;
  sev.sigev_notify_function   = handler;
  sev.sigev_notify_attributes = info;

  ret = timer_create(CLOCK_REALTIME, &sev, timer);
  if(ret == -1) {
    fprintf(stderr, "%s() create error: %s\n", __FUNCTION__, strerror(errno));
    free(timer);
    timer = NULL;
  }

  printf("%s(%p, %p) return %p\n", __FUNCTION__, handler, info, timer);

  return timer;
}

int timer_start(timer_t *timer, int timeout)
{
  int ret = 0;
  struct itimerspec trigger = {0};

  if(timeout == -1)
    return 0;

  if(timeout > 0) {
    /* arm timer */
    trigger.it_value.tv_sec  = timeout/1000;
    trigger.it_value.tv_nsec = (timeout%1000)*1000*1000;
  } else {
      /* disarm timer */
    trigger.it_value.tv_sec  = 0;
    trigger.it_value.tv_nsec = 0;
  }

  ret = timer_settime(*timer, 0, &trigger, NULL);
  if(ret == -1)
    fprintf(stderr, "%s() settime error: %s\n", __FUNCTION__, strerror(errno));

//  printf("%s(%p, %d) return %d\n", __FUNCTION__, timer, timeout, ret);

  return ret;
}

void timer_close(timer_t *timer)
{
  printf("%s(%p)\n", __FUNCTION__, timer);

  if(timer) {
    timer_start(timer, 0);
    timer_delete(*timer);
    free(timer);
  }
}

