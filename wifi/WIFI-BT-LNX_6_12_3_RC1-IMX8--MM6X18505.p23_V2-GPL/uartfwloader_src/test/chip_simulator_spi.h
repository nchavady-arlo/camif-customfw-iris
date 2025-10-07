#ifdef GPL_FILE
/** @file   chip_simulator_spi.h
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
/** @file   chip_simulator_spi.h
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

#ifndef CHIP_SIMULATOR_SPI_H
#define CHIP_SIMULATOR_SPI_H

#include <sys/types.h>

#ifndef MIN
#define MIN(a,b)                (a<b)?(a):(b)
#endif

#ifndef MAX
#define MAX(a,b)                (a>b)?(a):(b)
#endif

#define SPI_MAX_RX_FRAME_SIZE   2048 /* Observed so far */
#define SPI_MAX_TX_FRAME_SIZE   10   /* One frame is max 10 bytes */
#define SPI_MAX_FRAME_SIZE      MAX(SPI_MAX_RX_FRAME_SIZE,SPI_MAX_TX_FRAME_SIZE)


#ifdef __cplusplus
extern "C" {
#endif

  extern int spi_slave_open(char *dev);
  extern int spi_slave_read(int fd, u_int8_t *buf, int len, int timeout);
  extern int spi_slave_write(int fd, int fdGpioOut, u_int8_t *buf, int len, int timeout);
  extern void spi_slave_close(int fd);

#ifdef __cplusplus
}
#endif


#endif /* CHIP_SIMULATOR_SPI_H */
