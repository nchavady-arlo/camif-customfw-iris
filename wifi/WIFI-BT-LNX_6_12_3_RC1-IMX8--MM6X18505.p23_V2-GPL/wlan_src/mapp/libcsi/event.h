/** @file  event.h
 *
 * @brief This handles CSI events
 *
 *
 * Copyright 2024 NXP
 *
 * NXP CONFIDENTIAL
 * The source code contained or described herein and all documents related to
 * the source code (Materials) are owned by NXP, its
 * suppliers and/or its licensors. Title to the Materials remains with NXP,
 * its suppliers and/or its licensors. The Materials contain
 * trade secrets and proprietary and confidential information of NXP, its
 * suppliers and/or its licensors. The Materials are protected by worldwide
 * copyright and trade secret laws and treaty provisions. No part of the
 * Materials may be used, copied, reproduced, modified, published, uploaded,
 * posted, transmitted, distributed, or disclosed in any way without NXP's prior
 * express written permission.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery
 * of the Materials, either expressly, by implication, inducement, estoppel or
 * otherwise. Any license under such intellectual property rights must be
 * express and approved by NXP in writing.
 *
 */

#ifndef _LIBCSI_EVENT_H_
#define _LIBCSI_EVENT_H_

#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include "wls_api.h"

/** Success */
#define MLAN_STATUS_SUCCESS (0)

#define CUS_EVT_TOD_TOA "EVENT=TOD-TOA"

#define CUS_EVT_MLAN_CSI "EVENT=MLAN_CSI"
#define CSI_DUMP_FILE_MAX 1200000 // 1.2MB

/** Unsigned long long integer */
typedef unsigned long long t_u64;

/** Structure for CSI config data*/
typedef struct wls_csi_cfg {
	/** Channel number for FTM session*/
	t_u8 channel;
	/**CSI processing config*/
	hal_wls_processing_input_params_t wls_processing_input;
} wls_csi_cfg_t;

void proc_csi_event(event_header *event, unsigned int *resArray);
void proc_csi_event_wls(event_header *event, unsigned int *resArray);

void send_csi_ack(unsigned int *resArray);

#endif /* _LIBCSI_EVENT_H */
