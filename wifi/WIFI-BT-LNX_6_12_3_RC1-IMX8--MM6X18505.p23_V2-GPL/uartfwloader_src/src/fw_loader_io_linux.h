/** @file   fw_loader_io_linux.h
 *
 *  @brief  This file contains the function prototypes of procedures that implement
 *          the Nxp specific Helper Protocol for Linux.
 *
 *  Copyright 2014-2022 NXP
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

/*===================== Include Files ============================================*/
#ifndef FW_LOADER_IO_LINUX_H
#define FW_LOADER_IO_LINUX_H

#include <unistd.h>
#include <time.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include "fw_loader_types.h"
#include "fw_loader_uart.h"
/*===================== Macros ===================================================*/

/*==================== Typedefs =================================================*/

/*===================== Global Vars ==============================================*/
extern SERIAL_INTERFACE sSerialInterfaceLinuxUart;
#ifdef LINUX_SPI
extern SERIAL_INTERFACE sSerialInterfaceLinuxSpi;
#endif

/*==================== Function Prototypes ======================================*/

extern void fw_upload_DelayInMs_Linux(uint32 uiMs);
extern uint64 fw_upload_GetTime_Linux(void);

#endif // FW_LOADER_IO_LINUX_H
