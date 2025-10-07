/** @file   fw_loader_uart.h
 *
 *  @brief  This file contains the function prototypes of the Nxp specific
 *          Helper Protocol.
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

#ifndef FW_LOADER_H
#define FW_LOADER_H
/*===================== Include Files ============================================*/
#include "fw_loader_types.h"

/*===================== Macros ===================================================*/
#define FW_LOADER_WIN   1
#define FW_LOADER_LINUX 0

// Note: _WIN32 or _WIN64 are pre-defined macros
// for Windows applications.
#if defined(_WIN32) || defined(_WIN64)
#define OS_TYPE_FW_LOADER_WIN
#else // Linux
#define OS_TYPE_FW_LOADER_LINUX
#endif // defined (__WIN32) || defined (__WIN64)

typedef struct {
	uint32 iBaudRate;
	uint32 iUartDivisor;
	uint32 iClkDivisor;
} UART_BAUDRATE;

#define OPEN_FAILURE 1000001

typedef enum {
	UART_IF,
#ifdef LINUX_SPI
	SPI_IF,
#endif
	UNKNOWN_IF = -1
} SERIAL_TYPE;

/*==================== Typedefs =================================================*/

typedef struct {
	uint32 uiSpeed;		/* Speed in Bauds (uart) or Hz (spi) */
	uint8 ucByteSize;	/* # bits per data (5|6|7|8) */
	union {
		struct {
			uint8 ucParity;	/* 0: No Parity, 1: Even parity, 2: Odd parity */
			uint8 ucStopBits;	/* # of stop bits (1|2) */
			uint8 ucFlowCtrl;	/* 0: no Flow Control, 1: CTS-RTS handshake */
			uint32 uiFirstBaudrate;	/* Assume it does not use Flow Control */
			uint32 uiSecondBaudrate;	/* Assume it uses ucFlowCtrl setting (forced to 1 for uiProVer == Ver1) */
		} uart;
#ifdef LINUX_SPI
		struct {
			uint8 ucMode;	/* transfer mode (01|2|3) */
			char *pGpioIntDev;	/* Device name of the gpio used as "interrupt" */
			uint8 ucGpioIntLine;	/* Line number of the gpio used as "interrupt" */
		} spi;
#endif
	};
} SERIAL_CONFIG;

typedef struct {
	char *name;
	SERIAL_TYPE type;
	 int8(*Probe) (int8 * dev);
	 int32(*Open) (int8 * dev, SERIAL_CONFIG * pConfig);
	void (*Close)(int32 iPortID);
	 int8(*ComReadChar) (int32 iPortID);
	 int8(*ComWriteChar) (int32 iPortID, int8 iChar);
	 int8(*ComWriteChars) (int32 iPortID, int8 * pChBuffer, int32 uiLen);
	 int32(*ComReadChars) (int32 iPortID, int8 * pChBuffer, int32 uiCount);
	 int32(*ComGetCTS) (int32 iPortID);
	 int32(*ComGetBufferSize) (int32 iPortID);
	void (*Flush)(int32 iPortID);
} SERIAL_INTERFACE;

/*===================== Global Vars ==============================================*/

/*==================== Function Prototypes ======================================*/
#endif // FW_LOADER_H
