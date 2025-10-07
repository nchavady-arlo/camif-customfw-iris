/** @file   fw_loader_io_linux.c
 *
 *  @brief  This file contains the functions that implement the Nxp specific
 *          Helper Protocol for Linux.
 *
 *  Copyright 2014-2024 NXP
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
#include <stdio.h>
#include <stdlib.h>
#include "fw_loader_io_linux.h"

/*===================== Macros ===================================================*/
#define TIMEOUT_SEC             6
#define TIMEOUT_FOR_READ        2000
#define USE_SELECT
#define FAILURE 1000001
/*==================== Typedefs =================================================*/

/*===================== Global Vars ==============================================*/

/*==================== Function Prototypes ======================================*/

/*==================== Coded Procedures =========================================*/
/******************************************************************************
 *
 * Name: fw_upload_DelayInMs_Linux
 *
 * Description:
 *   This function delays the execution of the program for the time
 *   specified in uiMs.
 *
 * Conditions For Use:
 *   None.
 *
 * Arguments:
 *   uiMs - Delay in Milliseconds.
 *
 * Return Value:
 *   None.
 *
 * Notes:
 *   None.
 *
 *****************************************************************************/
void
fw_upload_DelayInMs_Linux(uint32 uiMs)
{
	struct timespec ReqTime;
	time_t sec;

	// Initialize to 0
	ReqTime.tv_sec = 0;
	ReqTime.tv_nsec = 0;

	// Calculate the Delay
	sec = (time_t) (uiMs / 1000);
	uiMs = (uiMs - ((uint32) sec * (uint32) 1000));
	ReqTime.tv_sec = sec;
	ReqTime.tv_nsec = (int32) uiMs *1000000L;	// 1 ms = 1000000 ns  

	// Sleep 
	while (nanosleep(&ReqTime, &ReqTime) == -1) {
		continue;
	}
}

/******************************************************************************
 *
 * Name: fw_upload_GetTime_Linux
 *
 * Description:
 *   Get the current time
 *
 * Conditions For Use:
 *   None.
 *
 * Arguments:
 *
 * Return Value: 
 *   return the current time 
 *
 * Notes:
 *   None.
 *
 *****************************************************************************/

uint64
fw_upload_GetTime_Linux(void)
{
	struct timespec time;
	clockid_t clk_id;
	uint64 millsectime = -1;

	clk_id = CLOCK_MONOTONIC;
	if (!clock_gettime(clk_id, &time)) {
		millsectime =
			(((uint64) time.tv_sec * 1000 * 1000 * 1000) +
			 time.tv_nsec) / 1000000;
	} else {
		perror("clock gettime");
		exit(-1);
	}
	return millsectime;
}
