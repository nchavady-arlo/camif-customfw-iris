/** @file   fw_loader_frame.c
 *
 *  @brief  This file contains the functions that implement the Nxp specific
 *          Helper Protocol.
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
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <memory.h>
#include <setjmp.h>
#include <malloc.h>
#include <errno.h>
#include <stdarg.h>
#include <ctype.h>
#include "fw_loader_uart.h"
#include "fw_loader_frames.h"
#ifdef OS_TYPE_FW_LOADER_WIN
#include "fw_loader_io_win.h"
#endif
#ifdef OS_TYPE_FW_LOADER_LINUX
#include "fw_loader_io_linux.h"
#endif

/*===================== Macros ===================================================*/

#define PRINT(...)         printWithTimeStamp(__VA_ARGS__)

/*==================== Typedefs =================================================*/

/*===================== Global Vars ==============================================*/

static HELPER_HEADER lastCmd = { 0 };

uint64 fwLoaderStartTime = 0;
/*==================== Function Prototypes ======================================*/

/*==================== Coded Procedures =========================================*/
int32
fw_upload_getFrameLen(uint8 header)
{
	int len;

	switch (header) {
	case V1_HEADER_DATA_REQ:
		len = 1 + sizeof(V1_DATA_REQ);
		break;
	case V1_REQUEST_ACK:
		len = 1 + 0;
		break;
	case V1_START_INDICATION:
		len = 1 + sizeof(V1_START_IND);
		break;
	case V1_ERROR_ACK:
		len = 1 + 0;
		break;
	case V3_START_INDICATION:
		len = 1 + sizeof(V3_START_IND);
		break;
	case V3_HEADER_DATA_REQ:
		len = 1 + sizeof(V3_DATA_REQ);
		break;
	case V3_REQUEST_ACK:
		len = 1 + sizeof(V3_RESP_ACK_NAK);
		break;
	case V3_TIMEOUT_ACK:
		len = 1 + sizeof(V3_RESP_TIMEOUT);
		break;
	case V3_CRC_ERROR:
		len = 1 + sizeof(V3_RESP_ACK_NAK);
		break;
	default:
		len = 0;
		break;
	}
	return len;
}

int
fw_upload_isDumpFrame(void)
{
	char *dumpFrame = getenv("DUMP_FRAME");

	if (!dumpFrame)
		return 0;

	return atoi(dumpFrame);
}

char *
fw_upload_getMsString(void)
{
	uint64(*fw_upload_GetTime) (void);
#ifdef OS_TYPE_FW_LOADER_WIN
	fw_upload_GetTime = fw_upload_GetTime_Win;
#else
	fw_upload_GetTime = fw_upload_GetTime_Linux;
#endif // (OS_TYPE_FW_LOADER_WIN)

	static char buf[30];	/* Ugly!!!! */
	static uint64 start = 0;

	if (start == 0)
		start = fw_upload_GetTime();

	uint64 ms = fw_upload_GetTime() - start;
	uint64 sec = ms / 1000;
	uint64 min = sec / 60;
	uint64 hr = min / 60;

	snprintf(buf, 30, "[%03lld:%02lld:%02lld.%03lld]", hr % 24, min % 60,
		 sec % 60, ms % 1000);

	return buf;
}

void
fw_upload_dumpCtrlFrame(FRAME_DIR frameDir, CTRL_FRAME * pFrame, int32 len,
			char *pPostMsg)
{
	if (!fw_upload_isDumpFrame())
		return;

	PRINT("\r%s %s %-2d: <%02x> ",
	      fw_upload_getMsString(),
	      (frameDir == RX_DIR) ? ("Recv") : ("Send"), len, pFrame->header);

	switch (pFrame->header) {
	case V1_START_INDICATION:
		PRINT("startIndV1 [chipId %02x, chipRev %02x, compl %02x-%02x] ", pFrame->startIndV1.uiChipId, pFrame->startIndV1.uiRev, pFrame->startIndV1.uiChipIdComp, pFrame->startIndV1.uiRevComp);
		break;
	case V3_START_INDICATION:
		PRINT("startIndV3 [chipId %04x, loaderVer %02x, crc %02x] ",
		      pFrame->startIndV3.uiChipId,
		      pFrame->startIndV3.uiLoaderVer, pFrame->startIndV3.uiCrc);
		break;
	case V1_HEADER_DATA_REQ:
		PRINT("dataReqV1 [len: %04x, compl %04x] ",
		      pFrame->dataReqV1.uiLen, pFrame->dataReqV1.uiLenComp);
		break;
	case V3_HEADER_DATA_REQ:
		PRINT("dataReqV3 [len %04x, offset %08x, err: %04x [ %s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s], crc: %02x] ", pFrame->dataReqV3.uiLen, pFrame->dataReqV3.ulOffset, pFrame->dataReqV3.uiError, (pFrame->dataReqV3.uiError & (1 << 0)) ? ("Crc ") : (""), (pFrame->dataReqV3.uiError & (1 << 1)) ? ("Nak ") : (""), (pFrame->dataReqV3.uiError & (1 << 2)) ? ("ToAck ") : (""), (pFrame->dataReqV3.uiError & (1 << 3)) ? ("ToHead ") : (""), (pFrame->dataReqV3.uiError & (1 << 4)) ? ("ToDat ") : (""), (pFrame->dataReqV3.uiError & (1 << 5)) ? ("Help ") : (""), (pFrame->dataReqV3.uiError & (1 << 6)) ? ("Wifi ") : (""), (pFrame->dataReqV3.uiError & (1 << 7)) ? ("Bt ") : (""), (pFrame->dataReqV3.uiError & (1 << 8)) ? ("Ack ") : (""), (pFrame->dataReqV3.uiError & (1 << 9)) ? ("b9 ") : (""), (pFrame->dataReqV3.uiError & (1 << 10)) ? ("b10 ") : (""), (pFrame->dataReqV3.uiError & (1 << 11)) ? ("b11 ") : (""), (pFrame->dataReqV3.uiError & (1 << 12)) ? ("b12 ") : (""), (pFrame->dataReqV3.uiError & (1 << 13)) ? ("b13 ") : (""), (pFrame->dataReqV3.uiError & (1 << 14)) ? ("b14 ") : (""), (pFrame->dataReqV3.uiError & (1 << 15)) ? ("Loser ") : ("Winner "), pFrame->dataReqV3.uiCrc);
		break;
	case V1_REQUEST_ACK:
		PRINT("ackV1 [ ] ");	/* has no data */
		break;
	case V1_ERROR_ACK:
		PRINT("errorV1 [ ] ");	/* has no data */
		break;
	case V3_REQUEST_ACK:
		PRINT("ackV3 [crc %02x] ", pFrame->ackNakV3.uiCrc);
		break;
	case V3_TIMEOUT_ACK:
		PRINT("timeoutV3 [offset %08x, crc %02x] ",
		      pFrame->timeoutV3.ulOffset, pFrame->timeoutV3.uiCrc);
		break;
	case V3_CRC_ERROR:
		PRINT("nakV3 [crc %02x] ", pFrame->ackNakV3.uiCrc);
		break;
	default:
		PRINT("??? [ ");
		for (int i = 0; i < len - 1; i++) {
			PRINT("%02x ", pFrame->raw[i]);
		}
		PRINT("] ");
		break;
	}
	PRINT("%s\n", pPostMsg);
}

void
fw_upload_dumpDataFrame(FRAME_DIR frameDir, uint8 * pData, int32 len,
			char *pPostMsg)
{
	BOOLEAN bHeaderDetected = FALSE;
	HELPER_HEADER *pHelperHeader = (HELPER_HEADER *) pData;

	if (!fw_upload_isDumpFrame())
		return;

	/* How to detect an header? */

	/* len is a helper header AND CMD < 64 */
	if (len == sizeof(HELPER_HEADER) &&
	    pHelperHeader->ulCmd < 64 && pHelperHeader->ulCrc != 0)
		bHeaderDetected = TRUE;
	/* CMD1 with Data */
	if (pHelperHeader->ulCmd == 1 &&
	    pHelperHeader->ulLen > 0 && pHelperHeader->ulCrc != 0)
		bHeaderDetected = TRUE;
	/* CMD5 with Data */
	if (pHelperHeader->ulCmd == 5 &&
	    pHelperHeader->ulLen > 0 && pHelperHeader->ulCrc != 0)
		bHeaderDetected = TRUE;
	/* Is it good enough? */

	if (bHeaderDetected) {
		PRINT("%s %s %d: Helper [CMD%d, ADDR %08x, LEN %08x, CRC %08x] %s%s\n", fw_upload_getMsString(), (frameDir == RX_DIR) ? ("Recv") : ("Send"), len, pHelperHeader->ulCmd, pHelperHeader->ulAddr, pHelperHeader->ulLen, pHelperHeader->ulCrc, (len > sizeof(HELPER_HEADER)) ? ("... ") : (""), pPostMsg);
		memcpy(&lastCmd, pHelperHeader, sizeof(lastCmd));
	} else {
		PRINT("%s %s %d: Cmd%d-Data [ ",
		      fw_upload_getMsString(),
		      (frameDir == RX_DIR) ? ("Recv") : ("Send"),
		      len, lastCmd.ulCmd);
		if (len <= 16) {
			for (int i = 0; i < len; i++) {
				PRINT("%02x ", pData[i]);
			}
		} else {
			for (int i = 0; i < 8; i++) {
				PRINT("%02x ", pData[i]);
			}
			PRINT("...data... ");
			for (int i = len - 8; i < len; i++) {
				PRINT("%02x ", pData[i]);
			}

		}
		PRINT("] %s\n", pPostMsg);
		memset(&lastCmd, 0, sizeof(lastCmd));
	}
}

int
log_buffer(uint8 * buf, int count, const char *format, ...)
{
	int ret = 0;
	int i, j;
	va_list args;

	va_start(args, format);

	printf("\n");
	for (i = 0; i < count; i += 16) {
		printf("%s ", fw_upload_getMsString());
		ret += vfprintf(stdout, format, args);
		ret += printf(":  %p    ", buf + i);
		for (j = 0; j < 16; j++) {
			if (i + j < count)
				ret += printf("%02x ", buf[i + j]);
			else
				ret += printf("   ");
		}
		ret += printf("    ");
		for (j = 0; j < 16; j++) {
			if (i + j < count)
				ret += printf("%c",
					      (isprint(buf[i + j]))
					      ? (buf[i + j]) : ('.'));
			else
				ret += printf(" ");
		}
		ret += printf("\n");
		if (count > 32 && i == 0) {
			i = count - 16 - 16;
			printf("%s ", fw_upload_getMsString());
			printf("...\n");
		}
	}
	va_end(args);

	return ret;
}

/******************************************************************************
 *
 * Name: printWithTimeStamp
 *
 * Description:
 *   Prints a message to the console with timestamp.
 *
 * Conditions For Use:
 *   None.
 *
 * Arguments:
 *   It takes args like printf function.
 *
 * Return Value:
 *   None.
 *
 * Notes:
 *   None.
 *
 *****************************************************************************/

void
printWithTimeStamp(char *format, ...)
{
	uint64(*fw_upload_GetTime) (void);
#ifdef OS_TYPE_FW_LOADER_WIN
	fw_upload_GetTime = fw_upload_GetTime_Win;
#else
	fw_upload_GetTime = fw_upload_GetTime_Linux;
#endif // (OS_TYPE_FW_LOADER_WIN)
	printf("[%09.6f]\t",
	       (fw_upload_GetTime() - fwLoaderStartTime) / 1000.0);
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
}
