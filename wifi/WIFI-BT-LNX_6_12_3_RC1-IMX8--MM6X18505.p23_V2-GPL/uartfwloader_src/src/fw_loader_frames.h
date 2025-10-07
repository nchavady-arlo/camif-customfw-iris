/** @file   fw_loader_frames.h
 *
 *  @brief  This file contains the Nxp specific frame definitions of
 *          standatd ANSI-C data types.
 *
 *  Copyright 2022-2024 NXP
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
#ifndef FW_LOADER_FRAMES_H
#define FW_LOADER_FRAMES_H

#include "fw_loader_types.h"

/*===================== Macros ===================================================*/

#define V1_HEADER_DATA_REQ      0xa5
#define V1_REQUEST_ACK          0x5a
#define V1_START_INDICATION     0xaa
#define V1_ERROR_ACK            0xbf

#define V3_START_INDICATION     0xab
#define V3_HEADER_DATA_REQ      0xa7
#define V3_REQUEST_ACK          0x7a
#define V3_TIMEOUT_ACK          0x7b
#define V3_CRC_ERROR            0x7c

#define SPI_SLAVE_SEND_CTRL_FRAME_10_BYTES	/* header + V3_DATA_REQ */
//#define SPI_SLAVE_SEND_CTRL_FRAME_2XFER_1_THEN_REMAINING

#define MAX_CTRL_FRAME_DATA 16	/* Should fit */

/*==================== Typedefs =================================================*/

#if defined(__CWCC__) || defined(_WIN32) || defined(_WIN64)
#pragma pack(push, 1)
#else
#pragma pack(1)
#endif

typedef struct {
	uint8 uiChipId;
	uint8 uiRev;
	uint8 uiChipIdComp;
	uint8 uiRevComp;
} V1_START_IND;			/* 4 bytes */
typedef struct {
	uint16 uiLen;
	uint16 uiLenComp;
} V1_DATA_REQ;			/* 4 bytes */
typedef struct {
	uint16 uiChipId;
	uint8 uiLoaderVer;
	uint8 uiCrc;
} V3_START_IND;			/* 4 bytes */
typedef struct {
	uint16 uiLen;
	uint32 ulOffset;
	uint16 uiError;
	uint8 uiCrc;
} V3_DATA_REQ;			/* 9 bytes */
typedef struct {
	uint8 uiCrc;
} V3_RESP_ACK_NAK;		/* 1 bytes */
typedef struct {
	uint32 ulOffset;
	uint8 uiCrc;
} V3_RESP_TIMEOUT;		/* 5 bytes */

typedef struct {
	uint8 header;
	union {
		/* Request: 4~9 bytes */
		V1_START_IND startIndV1;
		V1_DATA_REQ dataReqV1;
		V3_START_IND startIndV3;
		V3_DATA_REQ dataReqV3;
		/* Response: 1~5 bytes */
		V3_RESP_ACK_NAK ackNakV3;
		V3_RESP_TIMEOUT timeoutV3;
		/* Raw */
		uint8 raw[MAX_CTRL_FRAME_DATA];
	};
} CTRL_FRAME;

typedef struct {
	uint32 ulCmd;
	uint32 ulAddr;
	uint32 ulLen;
	uint32 ulCrc;
} HELPER_HEADER;

#if defined(__CWCC__) || defined(_WIN32) || defined(_WIN64)
#pragma pack(pop)
#else
#pragma pack()
#endif

typedef enum { RX_DIR, TX_DIR } FRAME_DIR;

/*===================== Global Vars ==============================================*/

extern uint64 fwLoaderStartTime;
/*==================== Function Prototypes ======================================*/

extern int32 fw_upload_getFrameLen(uint8 header);

extern int fw_upload_isDumpFrame(void);
extern char *fw_upload_getMsString(void);
extern void fw_upload_dumpCtrlFrame(FRAME_DIR frameDir, CTRL_FRAME * pFrame,
				    int32 len, char *pPostMsg);
extern void fw_upload_dumpDataFrame(FRAME_DIR frameDir, uint8 * pData,
				    int32 len, char *pPostMsg);

extern int log_buffer(uint8 * buf, int count, const char *format, ...);

void printWithTimeStamp(char *format, ...);

#endif // FW_LOADER_FRAMES_H
