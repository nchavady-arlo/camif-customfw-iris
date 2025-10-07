/** @file   fw_loader_uart.c
 *
 *  @brief  This file contains the functions that implement the Nxp specific
 *          Helper Protocol.
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
#include <stddef.h>
#include <memory.h>
#include <setjmp.h>
#include <malloc.h>
#include <errno.h>
#include <fcntl.h>
#include "fw_loader_uart.h"
#ifdef OS_TYPE_FW_LOADER_WIN
#include "fw_loader_io_win.h"
#endif
#ifdef OS_TYPE_FW_LOADER_LINUX
#include <termios.h>
#include <unistd.h>
#include "fw_loader_io_linux.h"
#endif
#include "fw_loader_frames.h"

/*===================== Macros ===================================================*/
#define VERSION_NUMBER      "357"
#ifdef BUILD_PALLADIUM
#define VERSION            "M" VERSION_NUMBER "-Palladium"
#else
#define VERSION            "M" VERSION_NUMBER
#endif
#define MAX_LENGTH         0xFFFF	//Maximum 2 byte value
#define END_SIG_TIMEOUT    2500
#define MAX_CTS_TIMEOUT    500	//500ms
#ifdef BUILD_PALLADIUM
#define TIMEOUT_VAL_MILLISEC  60000	// For Palladium, it is much slower, baudrate at 9600 or spi at 1kHz and the bootCode takes about 49s to re-send a DataReq in case of no Ack from the Host
#else
#define TIMEOUT_VAL_MILLISEC  4000	// Timeout for getting 0xa5 or 0xaa or 0xa7, 2 times of helper timeout
#endif
#define RETRY_TIMEOUT      2000	// To wake up BT CPU again, 2 seconds
#define STRING_SIZE        6
#define HDR_LEN            16
#define CMD4               0x4
#define CMD6               0x6
#define CMD7               0x7

#define PRINT(...)         printWithTimeStamp(__VA_ARGS__)

#define DOWNLOAD_SUCCESS                 0x0
#define OPEN_SERIAL_PORT_OR_FILE_ERROR   0x1
#define FEEK_SEEK_ERROR                  0x2
#define FILESIZE_IS_ZERO                 0x3
#define HEADER_SIGNATURE_TIMEOUT         0x4
#define READ_FILE_FAIL                   0x5
#define CHANGE_BAUDRATE_FAIL             0x6
#define CHANGE_TIMEOUT_VALUE_FAIL    0x7
#define OPEN_FILE_FAIL                   0x8
#define FILE_MODE_CANNOT_CHANGE          0X9
#define UNEXPECTED_BEHAVIOUR_IN_SETJMP   0xA
#define MALLOC_RETURNED_NULL             0xB

#define REQ_HEADER_LEN          1
#define A6REQ_PAYLOAD_LEN       8
#define AbREQ_PAYLOAD_LEN       3

#define END_SIG       0x005043

#define GP            0x107	/* x^8 + x^2 + x + 1 */
#define DI            0x07

#define CRC_ERR_BIT            1 << 0
#define NAK_REC_BIT            1 << 1
#define TIMEOUT_REC_ACK_BIT    1 << 2
#define TIMEOUT_REC_HEAD_BIT   1 << 3
#define TIMEOUT_REC_DATA_BIT   1 << 4
#define INVALID_CMD_REC_BIT    1 << 5
#define WIFI_MIC_FAIL_BIT      1 << 6
#define BT_MIC_FAIL_BIT        1 << 7

#define SWAPL(x) ((((x) >> 24) & 0xff) \
                 | (((x) >> 8) & 0xff00) \
                 | (((x) << 8) & 0xff0000L) \
                 | (((x) << 24) & 0xff000000L))

#define POLYNOMIAL 0x04c11db7L

#define CLKDIVAddr       0x7f00008f
#define UARTDIVAddr      0x7f000090
#define UARTMCRAddr      0x7f000091
#define UARTREINITAddr   0x7f000092
#define UARTICRAddr      0x7f000093
#define UARTFCRAddr      0x7f000094

#define MCR   0x00000022
#define INIT  0x00000001
#define ICR   0x000000c7
#define FCR   0x000000c7

static unsigned char crc8_table[256];	/* 8-bit table */
static int made_table = 0;

static unsigned long crc_table[256];
static BOOLEAN cmd7_Req = FALSE;
static BOOLEAN EntryPoint_Req = FALSE;
static uint32 change_baudrata_buffer_len = 0;
static uint32 cmd7_change_timeout_len = 0;
static BOOLEAN send_poke = TRUE;
static int8 m_Buffer_Poke[2] = { 0xdc, 0xe9 };

//CMD5 Header to change bootload baud rate 
static int8 m_Buffer_CMD5_Header[16] =
	{ 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2c, 0x00, 0x00,
0x00, 0x77, 0xdb, 0xfd, 0xe0 };
static int8 m_Buffer_CMD7_ChangeTimeoutValue[16] =
	{ 0x07, 0x00, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x5b, 0x88, 0xf8, 0xba };
static const UART_BAUDRATE UartCfgTbl[] = {
	{115200, 16, 0x0075F6FD},
	{3000000, 1, 0x00C00000},
	{4000000, 1, 0x01000000},
};

#define OWL_A0_a  0x8000
#define OWL_A0_c 0x8001
static uint16 ChipId;
//#define DEBUG_PRINT
#define DEBUG_FRAME		/* Enabled by env: DUMP_FRAME=1 */

 /*==================== Typedefs =================================================*/

/*===================== Global Vars ==============================================*/

static SERIAL_INTERFACE *pSerialIf = NULL;
static SERIAL_CONFIG SerialCfg = { 0 };

// Maximum Length that could be asked by the Helper = 2 bytes 
static uint8 ucByteBuffer[MAX_LENGTH];

//Handler of File
static FILE *pFile;

// The uart port
static int8 *gPortName = NULL;

// ID of the current com port
static int32 iPortID = OPEN_FAILURE;

// Size of the File to be downloaded
static long ulTotalFileSize = 0;

// Current size of the Download
static uint32 ulCurrFileSize = 0;
static uint32 ulLastOffsetToSend = 0xFFFF;
static BOOLEAN uiErrCase = FALSE;
static BOOLEAN uiReDownload = FALSE;
// Received Frame
static CTRL_FRAME sRxFrame;

// Transmitted Frame
static CTRL_FRAME sTxFrame;

static uint8 ucString[STRING_SIZE];

static BOOLEAN b16BytesData = FALSE;

static uint16 uiNewLen;
static uint32 ulNewOffset;
static uint16 uiNewError;

static uint8 uiProVer;
static BOOLEAN bVerChecked = FALSE;

typedef enum {
	Ver1,
	Ver2,
	Ver3,
} Version;

#ifdef DEBUG_PRINT
static uint8 uiErrCnt[16] = { 0 };
#endif

/*==================== Function Prototypes ======================================*/
static uint8 fw_upload_ComReadChar(int32 iPortID);
static int8 fw_upload_ComWriteChar(int32 iPortID, int8 iChar);
static int8 fw_upload_ComWriteChars(int32 iPortID, int8 * pChBuffer,
				    uint32 uiLen);
static int32 fw_upload_ComReadChars(int32 iPortID, int8 * pChBuffer,
				    int32 uiCount);
static void fw_upload_Send_Ack(uint8 uiAck);
static void (*fw_upload_DelayInMs)(uint32 uiMs);
static uint64(*fw_upload_GetTime) (void);

static void fw_upload_gen_crc_table(void);
static unsigned long fw_upload_update_crc(unsigned long crc_accum,
					  char *data_blk_ptr,
					  int data_blk_size);
static void fw_upload_io_func_init(void);
static BOOLEAN fw_upload_lenValid(uint16 * uiLenToSend, uint8 * ucArray);
static void closeFileorDescriptor(int fileDescriptor);
static void init_crc8(void);

/*==================== Coded Procedures =========================================*/

/******************************************************************************

 *
 * Name: gen_crc_table
 *
 * Description:
 *   Genrate crc table    
 *
 * Conditions For Use:
 *   None.
 *
 * Arguments:
 *   None.
 *
 * Return Value:
 *   None.
 *
 * Notes:
 *   None.
 *
 *****************************************************************************/
void
fw_upload_gen_crc_table()
{
	int i, j;
	unsigned long crc_accum;

	for (i = 0; i < 256; i++) {
		crc_accum = ((unsigned long)i << 24);
		for (j = 0; j < 8; j++) {
			if (crc_accum & 0x80000000L) {
				crc_accum = (crc_accum << 1) ^ POLYNOMIAL;
			} else {
				crc_accum = (crc_accum << 1);
			}
		}
		crc_table[i] = crc_accum;
	}

	return;
}

/******************************************************************************

 *
 * Name: update_crc
 *
 * Description:
 *   update the CRC on the data block one byte at a time    
 *
 * Conditions For Use:
 *   None.
 *
 * Arguments:
 *   ata_blk_ptr:   the buffer pointer for updating crc.
 *   data_blk_size: the size of buffer
 *
 * Return Value:
 *   CRC value.
 *
 * Notes:
 *   None.
 *
 *****************************************************************************/
unsigned long
fw_upload_update_crc(unsigned long crc_accum, char *data_blk_ptr,
		     int data_blk_size)
{
	unsigned long i, j;

	for (j = 0; j < data_blk_size; j++) {
		i = ((unsigned long)(crc_accum >> 24) ^ *data_blk_ptr++) & 0xff;
		crc_accum = (crc_accum << 8) ^ crc_table[i];
	}
	return crc_accum;
}

/******************************************************************************
 *
 * Name: fw_upload_io_func_init
 *
 * Description:
 *   This function initializes the IO function pointers.    
 *
 * Conditions For Use:
 *   None.
 *
 * Arguments:
 *   None.
 *
 * Return Value:
 *   None.
 *
 * Notes:
 *   None.
 *
 *****************************************************************************/
void
fw_upload_io_func_init()
{
// Initialize the function pointers depending
// on the OS type.
#ifdef OS_TYPE_FW_LOADER_WIN
	fw_upload_DelayInMs = fw_upload_DelayInMs_Win;
	fw_upload_GetTime = fw_upload_GetTime_Win;
#else
	fw_upload_DelayInMs = fw_upload_DelayInMs_Linux;
	fw_upload_GetTime = fw_upload_GetTime_Linux;
#endif // (OS_TYPE_FW_LOADER_WIN)
}

static uint8
fw_upload_ComReadChar(int32 iPortID)
{
	uint8 iChar = 0xFF;	/* Failure value of fw_upload_ComReadChar_Xxx, also an invalid value for RxFrame.header */
	iChar = pSerialIf->ComReadChar(iPortID);
#ifdef DEBUG_FRAME
	if (iChar != 0xFF && fw_upload_isDumpFrame()) {
		PRINT("%s Recv1: %02x \n", fw_upload_getMsString(), iChar);
	}
#endif
	return iChar;
}

static int32
fw_upload_ComReadChars(int32 iPortID, int8 * pChBuffer, int32 uiCount)
{
	int32 iRead;
	iRead = pSerialIf->ComReadChars(iPortID, pChBuffer, uiCount);
#ifdef DEBUG_FRAME
	if (fw_upload_isDumpFrame()) {
		PRINT("\r%s Recv%d: %02x... ", fw_upload_getMsString(), iRead,
		      pChBuffer[0]);
	}
#endif
	return iRead;
}

static int8
fw_upload_ComWriteChar(int32 iPortID, int8 iChar)
{
	int8 iRet;
	iRet = pSerialIf->ComWriteChar(iPortID, iChar);
	return iRet;
}

static int8
fw_upload_ComWriteChars(int32 iPortID, int8 * pChBuffer, uint32 uiLen)
{
	int8 iRet;
	iRet = pSerialIf->ComWriteChars(iPortID, pChBuffer, uiLen);
	return iRet;
}

/******************************************************************************
 *
 * Name: init_crc8
 *
 * Description:
 *   This function init crc.    
 *
 * Conditions For Use:
 *   None.
 *
 * Arguments:
 *   None.
 *
 * Return Value:
 *   None.
 *
 * Notes:
 *   None.
 *
 *****************************************************************************/
static void
init_crc8()
{
	int i, j;
	// unsigned char crc;
	int crc;
	if (!made_table) {
		for (i = 0; i < 256; i++) {
			crc = i;
			for (j = 0; j < 8; j++)
				crc = (crc << 1) ^ ((crc & 0x80) ? DI : 0);
			crc8_table[i] =
				(unsigned char)((unsigned char)crc &
						(unsigned char)0xFF);
			/* printf("table[%d] = %d (0x%X)\n", i, crc, crc); */
		}
		made_table = 1;
	}
}

/******************************************************************************
 *
 * Name: crc8
 *
 * Description:
 *   This function calculate crc.    
 *
 * Conditions For Use:
 *   None.
 *
 * Arguments:
 *   array: array to be calculated.
 *   len :  len of array.
 *
 * Return Value:
 *   None.
 *
 * Notes:
 *   None.
 *
 *****************************************************************************/
static unsigned char
crc8(unsigned char *array, unsigned char len)
{
	unsigned char CRC = 0xff;
	for (; len > 0; len--) {
		CRC = crc8_table[CRC ^ *array];
		array++;
	}
	return CRC;
}

/******************************************************************************
 *
 * Name: fw_toggle_rts()
 *
 * Description:
 *   This function toggles RTS by open and close UART port
 *
 * Conditions For Use:
 *   None.
 *
 * Arguments:
 *   None.
 *
 * Return Value:
 *   TRUE:   Toggle RTS sucessfully.
 *   FALSE:  Toggle RTS unsuccessfully.
 *
 * Notes:
 *   None.
 *
 *****************************************************************************/

static BOOLEAN
fw_toggle_rts(void)
{
	// UART Init
	if (pSerialIf->type == UART_IF) {
		uint8 flowCtrlBackUp = SerialCfg.uart.ucFlowCtrl;
		// Enable flow control and close the port to send a pulse on CTS line
		// to resume from Boot Sleep Patch
		SerialCfg.uart.ucFlowCtrl = 1;
		iPortID = pSerialIf->Open(gPortName, &SerialCfg);

		if (iPortID == OPEN_FAILURE) {
			PRINT("Cannot open serial port\n");
			SerialCfg.uart.ucFlowCtrl = flowCtrlBackUp;
			return FALSE;
		}
		PRINT("Sent Pulse on CTS Line\n");
		pSerialIf->Close(iPortID);
		SerialCfg.uart.ucFlowCtrl = flowCtrlBackUp;
		return TRUE;
	}
	PRINT("It is not a UART interface, cannot toggle RTS\n");
	return FALSE;
}

/******************************************************************************
 *
 * Name: fw_upload_v1_xor_fail
 *
 * Description:
 *   Process v1 packet with NAK on XOR check fail
 *
 * Conditions For Use:
 *   Check XOR bytes.
 *
 * Arguments:
 *
 * Return Value:
 *
 *****************************************************************************/
static void
fw_upload_v1_xor_fail(void)
{
	int8 bWriteStatus;
#ifdef DEBUG_PRINT
	PRINT("    NAK case: bootloader LEN = 0x%x bytes \n",
	      sRxFrame.dataReqV1.uiLen);
	PRINT("    NAK case: bootloader LENComp = 0x%x bytes \n",
	      sRxFrame.dataReqV1.uiLenComp);
#endif
	// Failure due to mismatch.
	sTxFrame.header = V1_ERROR_ACK;
	bWriteStatus = fw_upload_ComWriteChar(iPortID, sTxFrame.header);
#ifdef DEBUG_FRAME
	fw_upload_dumpCtrlFrame(TX_DIR, &sTxFrame, sizeof(sTxFrame.header),
				(bWriteStatus) ? ("(write ok)")
				: ("(write error)"));
#endif
}

/******************************************************************************
 *
 * Name: fw_upload_v1_xor_pass
 *
 * Description:
 *   Process v1 packet with on XOR check pass
 *
 * Conditions For Use:
 *   Check XOR bytes.
 *
 * Arguments:
 *
 * Return Value:
 *
 *****************************************************************************/
static void
fw_upload_v1_xor_pass(void)
{
	int8 bWriteStatus;
	PRINT("CRC sent\n");
	sTxFrame.header = V1_REQUEST_ACK;
	bWriteStatus = fw_upload_ComWriteChar(iPortID, sTxFrame.header);
#ifdef DEBUG_FRAME
	fw_upload_dumpCtrlFrame(TX_DIR, &sTxFrame, sizeof(sTxFrame.header),
				(bWriteStatus) ? ("(write ok)")
				: ("(write error)"));
#endif
}

/******************************************************************************
*
* Name: fw_upload_read_char
*
* Description:
*   Read pkt_size in sRxFrame.raw
*
* Conditions For Use:
*
* Arguments:
*   pkt_size: Size of packet to read
*   timeout:  the expired time, 0 is considered infinte timeout
*   startTime: start time for timeout.
* Return Value:
*   TRUE:   pkt_size read successfully.
*   FALSE:  pkt_size read failed
*****************************************************************************/
static BOOLEAN
fw_upload_read_char(uint32 pkt_size, uint32 timeout, uint64 startTime)
{
	uint32 ulRxCnt = 0;
	int8 *pBuf = (int8 *) & sRxFrame.raw;
	while ((pkt_size != ulRxCnt)) {
		ulRxCnt +=
			fw_upload_ComReadChars(iPortID, pBuf + ulRxCnt,
					       pkt_size - ulRxCnt);
		if (timeout) {
			if ((fw_upload_GetTime() - startTime) > timeout) {
				PRINT("Failed to read payload in %u ms\n",
				      timeout);
				return FALSE;
			}
		}
	}
	return TRUE;
}

/******************************************************************************
 *
 * Name: fw_upload_read_header_payload
 *
 * Description:
 *   Process payload of the packet using the header byte recevied from bootcode
 *
 * Conditions For Use:
 *
 * Arguments:
 *
 * Return Value:
 *   TRUE:   Packet processed successfully.
 *   FALSE:  Packet processed failed
 *****************************************************************************/
static BOOLEAN
fw_upload_read_header_payload(uint32 timeout, uint64 startTime)
{
	BOOLEAN bCrcMatch;
	BOOLEAN ret = FALSE;
	uint16 uiXorOfLen = 0xFFFF;

	switch (sRxFrame.header) {
	case V1_HEADER_DATA_REQ:
		if (fw_upload_read_char
		    (sizeof(sRxFrame.dataReqV1), timeout, startTime) == FALSE) {
			return ret;
		}
#ifdef DEBUG_FRAME
		fw_upload_dumpCtrlFrame(RX_DIR, &sRxFrame,
					sizeof(sRxFrame.header) +
					sizeof(sRxFrame.dataReqV1), "");
#endif
		if ((sRxFrame.dataReqV1.uiLen ^ sRxFrame.dataReqV1.uiLenComp) ==
		    uiXorOfLen) {
			ret = TRUE;
			uiNewLen = sRxFrame.dataReqV1.uiLen;
		} else {
			fw_upload_v1_xor_fail();
		}
		break;
	case V1_START_INDICATION:
		if (fw_upload_read_char
		    (sizeof(sRxFrame.startIndV1), timeout,
		     startTime) == FALSE) {
			return ret;
		}
#ifdef DEBUG_FRAME
		fw_upload_dumpCtrlFrame(RX_DIR, &sRxFrame,
					sizeof(sRxFrame.header) +
					sizeof(sRxFrame.startIndV1), "");
#endif
		if (((sRxFrame.startIndV1.uiChipId ^ sRxFrame.startIndV1.
		      uiChipIdComp) == uiXorOfLen) &&
		    ((sRxFrame.startIndV1.uiRev ^ sRxFrame.startIndV1.
		      uiRevComp) == 0xFF)) {
			fw_upload_v1_xor_pass();
			ret = TRUE;
		} else {
			fw_upload_v1_xor_fail();
		}
		break;
	case V3_START_INDICATION:
		if (fw_upload_read_char
		    (sizeof(sRxFrame.startIndV3), timeout,
		     startTime) == FALSE) {
			return ret;
		}
		bCrcMatch =
			sRxFrame.startIndV3.uiCrc == crc8((uint8 *) & sRxFrame,
							  sizeof(sRxFrame.
								 header) +
							  sizeof(sRxFrame.
								 startIndV3) -
							  1);
#ifdef DEBUG_FRAME
		fw_upload_dumpCtrlFrame(RX_DIR, &sRxFrame,
					sizeof(sRxFrame.startIndV3) +
					sizeof(sRxFrame.header),
					(bCrcMatch) ? ("(crc ok)")
					: ("(crc error)"));
#endif
		if (bCrcMatch) {
			ret = TRUE;
			fw_upload_Send_Ack(V3_REQUEST_ACK);
			ChipId = sRxFrame.startIndV3.uiChipId;
#ifdef DEBUG_FRAME
			if (!fw_upload_isDumpFrame())
#endif
			{
				PRINT("ChipID is : 0x%x, Version is : 0x%x\n",
				      sRxFrame.startIndV3.uiChipId,
				      sRxFrame.startIndV3.uiLoaderVer);
			}
		} else {
			fw_upload_Send_Ack(V3_CRC_ERROR);
		}

		break;
	case V3_HEADER_DATA_REQ:
		if (fw_upload_read_char
		    (sizeof(sRxFrame.dataReqV3), timeout, startTime) == FALSE) {
			return ret;
		}
#ifdef DEBUG_PRINT
		PRINT(" <=== REQ = 0xA7, Len = 0x%x,Off = 0x%x,Err = 0x%x,CRC = 0x%x\n", sRxFrame.dataReqV3.uiLen, sRxFrame.dataReqV3.ulOffset, sRxFrame.dataReqV3.uiError, sRxFrame.dataReqV3.uiCrc);
#endif
		bCrcMatch =
			sRxFrame.dataReqV3.uiCrc == crc8((uint8 *) & sRxFrame,
							 sizeof(sRxFrame.
								header) +
							 sizeof(sRxFrame.
								dataReqV3) - 1);
#ifdef DEBUG_FRAME
		fw_upload_dumpCtrlFrame(RX_DIR, &sRxFrame,
					sizeof(sRxFrame.startIndV3) +
					sizeof(sRxFrame.header),
					(bCrcMatch) ? ("(crc ok)")
					: ("(crc error)"));
#endif
		if (bCrcMatch) {
			ret = TRUE;
			uiNewLen = sRxFrame.dataReqV3.uiLen;
			ulNewOffset = sRxFrame.dataReqV3.ulOffset;
			uiNewError = sRxFrame.dataReqV3.uiError;
		} else {
			fw_upload_Send_Ack(V3_CRC_ERROR);
		}
		break;
	}
	return ret;
}

/******************************************************************************
 *
 * Name: fw_upload_WaitForHeaderSignature
 *
 * Description:
 *   This function basically waits for reception
 *   of character 0xa5 on UART Rx. If no 0xa5 is 
 *   received, it will kind of busy wait checking for
 *   0xa5.
 *
 * Conditions For Use:
 *   None.
 *
 * Arguments:
 *   uiMs:   the expired time.
 *
 * Return Value:
 *   TRUE:   0xa5 or 0xab is received.
 *   FALSE:  0xa5 or 0xab is not received.
 *
 * Notes:
 *   None.
 *
 *****************************************************************************/
static BOOLEAN
fw_upload_WaitForHeaderSignature(uint32 uiMs)
{
	uint8 ucDone = 0;	// signature not Received Yet.
	uint64 startTime = 0;
	uint64 currTime = 0;
	BOOLEAN bResult = TRUE;
	startTime = fw_upload_GetTime();
	BOOLEAN firstWake = TRUE;

	while (!ucDone) {
		sRxFrame.header = fw_upload_ComReadChar(iPortID);
		if ((sRxFrame.header == V1_HEADER_DATA_REQ) ||
		    (sRxFrame.header == V1_START_INDICATION) ||
		    (sRxFrame.header == V3_START_INDICATION) ||
		    (sRxFrame.header == V3_HEADER_DATA_REQ)) {

			if (fw_upload_read_header_payload(uiMs, startTime)) {
				if ((sRxFrame.header == V1_HEADER_DATA_REQ) ||
				    (sRxFrame.header == V3_HEADER_DATA_REQ)) {
					ucDone = 1;
				}
			}
#ifdef DEBUG_PRINT
			PRINT("Received 0x%x\n", sRxFrame.header);
#endif
			if (!bVerChecked && (ucDone == 1)) {
				if ((sRxFrame.header == V1_HEADER_DATA_REQ) ||
				    (sRxFrame.header == V1_START_INDICATION)) {
					uiProVer = (uint8) Ver1;
				} else {
					uiProVer = (uint8) Ver3;
				}
				bVerChecked = TRUE;
			}
		} else {
#ifdef DEBUG_FRAME
			if (sRxFrame.header != 0xFF && fw_upload_isDumpFrame()) {
				PRINT("=> Unknown header(%02x)\n",
				      sRxFrame.header);
			}
#endif

			currTime = fw_upload_GetTime();
			if (firstWake &&
			    (currTime - startTime >
			     RETRY_TIMEOUT /*2 seconds */ )) {
				PRINT("No signature received more than 2 seconds, wake up BT again\n");
				pSerialIf->Close(iPortID);

				fw_toggle_rts();

				iPortID =
					pSerialIf->Open(gPortName, &SerialCfg);
				firstWake = FALSE;
			}

			if (uiMs) {
				if (currTime - startTime > uiMs) {
					bResult = FALSE;
					break;
				}
			}
			if (send_poke == TRUE) {
				if (fw_upload_ComWriteChars
				    (iPortID, m_Buffer_Poke, 2) == 1) {
					PRINT("Poke Sent\n");
					send_poke = FALSE;
				}
			}
			fw_upload_DelayInMs(1);
		}
	}
	send_poke = FALSE;
	return bResult;
}

/******************************************************************************
 *
 * Name: fw_upload_Send_Ack
 *
 * Description:
 *   This function sends ack to per req.    
 *
 * Conditions For Use:
 *   None.
 *
 * Arguments:
 *   uiAck: the ack type.
 *
 * Return Value:
 *   None.
 *
 * Notes:
 *   None.
 *
 *****************************************************************************/
static void
fw_upload_Send_Ack(uint8 uiAck)
{
	int8 bWriteStatus;
	static uint64 timeoutStart = 0;
	uint64 currTime = 0;

	if ((uiAck == V3_REQUEST_ACK) || (uiAck == V3_CRC_ERROR)) {
		// prepare data & crc for 0x7A or 0x7C
		sTxFrame.header = uiAck;
		sTxFrame.ackNakV3.uiCrc =
			crc8((uint8 *) & sTxFrame,
			     sizeof(sTxFrame.header) +
			     sizeof(sTxFrame.ackNakV3) - 1);
		bWriteStatus =
			fw_upload_ComWriteChars(iPortID, (int8 *) & sTxFrame,
						sizeof(sTxFrame.header) +
						sizeof(sTxFrame.ackNakV3));
#ifdef DEBUG_FRAME
		fw_upload_dumpCtrlFrame(TX_DIR, &sTxFrame,
					sizeof(sTxFrame.header) +
					sizeof(sTxFrame.ackNakV3),
					(bWriteStatus) ? ("(write ok)")
					: ("(write error)"));
#endif
#ifdef DEBUG_PRINT
		PRINT(" ===> ACK = 0x%x, CRC = 0x%x \n", uiAck,
		      sTxFrame.ackNakV3.uiCrc);
#endif
	} else if (uiAck == V3_TIMEOUT_ACK) {
		// prepare data & crc for 0x7B
		sTxFrame.header = uiAck;
		sTxFrame.timeoutV3.ulOffset = ulNewOffset;
		sTxFrame.timeoutV3.uiCrc =
			crc8((uint8 *) & sTxFrame,
			     sizeof(sTxFrame.header) +
			     sizeof(sTxFrame.timeoutV3) - 1);
		bWriteStatus =
			fw_upload_ComWriteChars(iPortID, (int8 *) & sTxFrame,
						sizeof(sTxFrame.header) +
						sizeof(sTxFrame.timeoutV3));
#ifdef DEBUG_FRAME
		fw_upload_dumpCtrlFrame(TX_DIR, &sTxFrame,
					sizeof(sTxFrame.header) +
					sizeof(sTxFrame.timeoutV3),
					(bWriteStatus) ? ("(write ok)")
					: ("(write error)"));
#endif
#ifdef DEBUG_PRINT
		PRINT(" ===> ACK = 0x%x, CRC = 0x%x \n", uiAck,
		      sTxFrame.timeoutV3.uiCrc);
#endif
	} else {
		PRINT("Non-empty else statement\n");
	}

	switch (uiAck) {
	case V3_TIMEOUT_ACK:
	case V3_CRC_ERROR:
		currTime = fw_upload_GetTime();
		if (timeoutStart != 0) {
			if ((currTime - timeoutStart) > TIMEOUT_VAL_MILLISEC) {
				PRINT("Recovery timeout: %llu\n",
				      currTime - timeoutStart);
				exit(EIO);
			}
		} else {
			timeoutStart = currTime;
		}
		break;
	case V3_REQUEST_ACK:
		timeoutStart = 0;
		break;
	}
}

/******************************************************************************
 *
 * Name: fw_upload_GetCmd
 *
 * Description:
 *   This function gets CMD value in the header.    
 *
 * Conditions For Use:
 *   None.
 *
 * Arguments:
 *   *buf: buffer that stores header and following data.
 *
 * Return Value:
 *   CMD value part in the buffer.
 *
 * Notes:
 *   None.
 *
 *****************************************************************************/
static uint32
fw_upload_GetCmd(uint8 * buf)
{
	return (buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[32] << 24));
}

/******************************************************************************
 *
 * Name: fw_upload_GetDataLen
 *
 * Description:
 *   This function gets buf data length.    
 *
 * Conditions For Use:
 *   None.
 *
 * Arguments:
 *   *buf: buffer that stores header and following data.
 *
 * Return Value:
 *   length of data part in the buffer.
 *
 * Notes:
 *   None.
 *
 *****************************************************************************/
static uint16
fw_upload_GetDataLen(uint8 * buf)
{
	return (buf[8] | (buf[9] << 8));
}

/******************************************************************************
 *
 * Name: fw_upload_lenValid
 *
 * Description:
 *   This function validates the length from 5 bytes request.    
 *
 * Conditions For Use:
 *   None.
 *
 * Arguments:
 *   ucArray: store the 5 bytes request.
 *
 * Return Value:
 *   uiLenToSend: if the length is valid, get value from ucArray.
 *
 * Notes:
 *   None.
 *
 
*****************************************************************************/
BOOLEAN
fw_upload_lenValid(uint16 * uiLenToSend, uint8 * ucArray)
{
	uint16 uiLen, uiLenComp;
	uint16 uiXorOfLen = 0xFFFF;
	uiLen = (uint16) ((ucArray[1] & 0xFF) | ((ucArray[2] << 8) & 0xFF00));
	uiLenComp =
		(uint16) ((ucArray[3] & 0xFF) | ((ucArray[4] << 8) & 0xFF00));
	// LEN valid if len & complement match
	if ((uiLen ^ uiLenComp) == uiXorOfLen)	// All 1's
	{
		*uiLenToSend = uiLen;
		return TRUE;
	} else {
		return FALSE;
	}
}

/******************************************************************************
 *
 * Name: fw_upload_GetHeaderStartBytes
 *
 * Description:
 *   This function gets 0xa5 and it's following 4 bytes length.    
 *
 * Conditions For Use:
 *   None.
 *
 * Arguments:
 *   None.
 *
 * Return Value:
 *   None.
 *
 * Notes:
 *   None.
 *
 
*****************************************************************************/
static void
fw_upload_GetHeaderStartBytes(uint8 * ucStr)
{
	BOOLEAN ucDone = FALSE, ucStringCnt = 0;
	while (!ucDone) {
		sRxFrame.header = fw_upload_ComReadChar(iPortID);

		if (sRxFrame.header == V1_HEADER_DATA_REQ) {
			ucStr[ucStringCnt++] = sRxFrame.header;
			ucDone = TRUE;
#ifdef DEBUG_PRINT
			PRINT("Received 0x%x\n ", sRxFrame.header);
#endif
		} else {
#ifdef DEBUG_FRAME
			if (sRxFrame.header != 0xFF && fw_upload_isDumpFrame()) {
				PRINT("=> Unknown header(%02x)\n",
				      sRxFrame.header);
			}
#endif
			fw_upload_DelayInMs(1);
		}
	}
	while (pSerialIf->ComGetBufferSize(iPortID) < 4) ;
	if (fw_upload_ComReadChars(iPortID, (int8 *) & ucStr[ucStringCnt], 4) <=
	    0) {
		PRINT("Read Error\n");
	}
}

/******************************************************************************
 *
 * Name: fw_upload_GetLast5Bytes
 *
 * Description:
 *   This function gets last valid request.    
 *
 * Conditions For Use:
 *   None.
 *
 * Arguments:
 *   *buf: buffer that stores header and following data.
 *
 * Return Value:
 *   None.
 *
 * Notes:
 *   None.
 *
 
*****************************************************************************/
static void
fw_upload_GetLast5Bytes(uint8 * buf)
{
	uint8 a5cnt, i;
	uint8 ucTemp[STRING_SIZE];
	uint16 uiTempLen = 0;
	uint16 len = 0;
	int32 fifosize;
	BOOLEAN alla5times = FALSE;

	// initialise 
	memset(ucString, 0x00, STRING_SIZE);

	fifosize = pSerialIf->ComGetBufferSize(iPortID);

	fw_upload_GetHeaderStartBytes(ucString);

	if (fw_upload_lenValid(&uiTempLen, ucString) == TRUE) {
		//Valid length recieved 
#ifdef DEBUG_PRINT
		PRINT(" Valid length = %d \n", uiTempLen);
#endif
	}

	len = fw_upload_GetDataLen(buf);
	if ((fifosize < 6) && ((uiTempLen == HDR_LEN) || (uiTempLen == len))) {
#ifdef DEBUG_PRINT
		PRINT("=========>success case\n");
#endif
		uiErrCase = FALSE;
	} else			// start to get last valid 5 bytes
	{
#ifdef DEBUG_PRINT
		PRINT("=========>fail case\n");
#endif
		while (fw_upload_lenValid(&uiTempLen, ucString) == FALSE) {
			fw_upload_GetHeaderStartBytes(ucString);
			fifosize -= 5;
		}
#ifdef DEBUG_PRINT
		PRINT("Error cases 1, 2, 3, 4, 5...\n");
#endif
		if (fifosize > 5) {
			fifosize -= 5;
			do {
				do {
					a5cnt = 0;
					do {
						fw_upload_GetHeaderStartBytes
							(ucTemp);
						fifosize -= 5;
					} while ((fw_upload_lenValid
						  (&uiTempLen, ucTemp) == TRUE)
						 && (!alla5times) &&
						 (fifosize > 5));
					//if 5bytes are all 0xa5, continue to clear 0xa5
					for (i = 0; i < 5; i++) {
						if (ucTemp[i] ==
						    V1_HEADER_DATA_REQ) {
							a5cnt++;
						}
					}
					alla5times = TRUE;
				} while (a5cnt == 5);
#ifdef DEBUG_PRINT
				PRINT("a5 count in last 5 bytes: %d\n", a5cnt);
#endif
				if (fw_upload_lenValid(&uiTempLen, ucTemp) ==
				    FALSE) {
					fw_upload_ComReadChars(iPortID,
							       (int8 *) &
							       ucTemp[a5cnt],
							       5 - a5cnt);
					if (a5cnt > 0) {
						memcpy(ucString,
						       &ucTemp[a5cnt - 1],
						       (5 -
							a5cnt) * sizeof(uint8));
					}
				} else {
					memcpy(ucString, ucTemp,
					       5 * sizeof(uint8));
				}
			} while (fw_upload_lenValid(&uiTempLen, ucTemp) ==
				 FALSE);
		}
		uiErrCase = TRUE;
	}
}

/******************************************************************************
 *
 * Name: fw_upload_SendBuffer
 *
 * Description:
 *   This function sends buffer with header and following data.
 *
 * Conditions For Use:
 *   None.
 *
 * Arguments:
 *      uiLenToSend: len of header request.
 *            ucBuf: the buf to be sent.
 *   uiHighBaudrate: send the buffer for high baud rate change.
 * Return Value:
 *   Returns the len of next header request.
 *
 * Notes:
 *   None.
 *
 
*****************************************************************************/
static uint16
fw_upload_SendBuffer(uint16 uiLenToSend, uint8 * ucBuf, BOOLEAN uiHighBaudrate)
{
	uint16 uiBytesToSend = HDR_LEN, uiFirstChunkSent = 0;
	uint16 uiDataLen = 0;
	uint8 ucSentDone = 0;
	BOOLEAN uiValidLen = FALSE;
	uint8 bWriteStatus;

	// Get data len
	uiDataLen = fw_upload_GetDataLen(ucBuf);
	// Send buffer
	while (!ucSentDone) {
		if (uiBytesToSend == uiLenToSend) {
			// All good
			if ((uiBytesToSend == HDR_LEN) && (!b16BytesData)) {
				if ((uiFirstChunkSent == 0) ||
				    ((uiFirstChunkSent == 1) &&
				     (uiErrCase == TRUE))) {
					// Write first 16 bytes of buffer
#ifdef DEBUG_PRINT
					PRINT("====>  Sending first chunk...\n");
					PRINT("====>  Sending %d bytes...\n",
					      uiBytesToSend);
#endif
					bWriteStatus =
						fw_upload_ComWriteChars(iPortID,
									(int8 *)
									ucBuf,
									uiBytesToSend);
#ifdef DEBUG_FRAME
					fw_upload_dumpDataFrame(TX_DIR,
								(uint8 *) ucBuf,
								uiBytesToSend,
								(bWriteStatus)
								? ("(write ok)")
								:
								("(write error)"));
#endif
					if (cmd7_Req == TRUE ||
					    EntryPoint_Req == TRUE) {
						uiBytesToSend = HDR_LEN;
						uiFirstChunkSent = 1;
					} else {
						uiBytesToSend = uiDataLen;
						uiFirstChunkSent = 0;
						if (uiBytesToSend == HDR_LEN) {
							b16BytesData = TRUE;
						}
					}
				} else {
					// Done with buffer
					ucSentDone = 1;
					break;
				}
			} else {
				// Write remaining bytes
#ifdef DEBUG_PRINT
				PRINT("====>  Sending %d bytes...\n",
				      uiBytesToSend);
#endif
				if (uiBytesToSend != 0) {
					bWriteStatus =
						fw_upload_ComWriteChars(iPortID,
									(int8 *)
									&
									ucBuf
									[HDR_LEN],
									uiBytesToSend);
#ifdef DEBUG_FRAME
					fw_upload_dumpDataFrame(TX_DIR,
								(uint8 *) &
								ucBuf[HDR_LEN],
								uiBytesToSend,
								(bWriteStatus)
								? ("(write ok)")
								:
								("(write error)"));
#endif
					uiFirstChunkSent = 1;
					// We should expect 16, then next block will start
					uiBytesToSend = HDR_LEN;
					b16BytesData = FALSE;
					if (uiHighBaudrate) {
						return 0;
					}
				} else	//end of bin download
				{
#ifdef DEBUG_PRINT
					PRINT(" ========== Download Complete =========\n");
#endif
					return 0;
				}
			}
		} else {
			// Something not good
			if ((uiLenToSend & 0x01) == 0x01) {
				// some kind of error
				if (uiLenToSend == (HDR_LEN + 1)) {
					// Send first chunk again
#ifdef DEBUG_PRINT
					PRINT("1. Resending first chunk...\n");
#endif
					bWriteStatus =
						fw_upload_ComWriteChars(iPortID,
									(int8 *)
									ucBuf,
									(uiLenToSend
									 - 1));
#ifdef DEBUG_FRAME
					fw_upload_dumpDataFrame(TX_DIR, ucBuf,
								uiLenToSend - 1,
								(bWriteStatus)
								? ("(write ok)")
								:
								("(write error)"));
#endif
					uiBytesToSend = uiDataLen;
					uiFirstChunkSent = 0;
				} else if (uiLenToSend == (uiDataLen + 1)) {
					// Send second chunk again
#ifdef DEBUG_PRINT
					PRINT("2. Resending second chunk...\n");
#endif
					bWriteStatus =
						fw_upload_ComWriteChars(iPortID,
									(int8 *)
									&
									ucBuf
									[HDR_LEN],
									(uiLenToSend
									 - 1));
#ifdef DEBUG_FRAME
					fw_upload_dumpDataFrame(TX_DIR, ucBuf,
								uiLenToSend - 1,
								(bWriteStatus)
								? ("(write ok)")
								:
								("(write error)"));
#endif
					uiBytesToSend = HDR_LEN;
					uiFirstChunkSent = 1;
				} else {
					PRINT("Non-empty terminating else statement\n");
				}
			} else if (uiLenToSend == HDR_LEN) {
				// Out of sync. Restart sending buffer
#ifdef DEBUG_PRINT
				PRINT("3.  Restart sending the buffer...\n");
#endif
				bWriteStatus =
					fw_upload_ComWriteChars(iPortID,
								(int8 *) ucBuf,
								uiLenToSend);
#ifdef DEBUG_FRAME
				fw_upload_dumpDataFrame(TX_DIR, ucBuf,
							uiLenToSend,
							(bWriteStatus)
							? ("(write ok)")
							: ("(write error)"));
#endif
				uiBytesToSend = uiDataLen;
				uiFirstChunkSent = 0;
			} else {
				PRINT("Non-empty else statement\n");
			}
		}
		// Get last 5 bytes now
		fw_upload_GetLast5Bytes(ucBuf);
		// Get next length
		uiValidLen = FALSE;
		do {
			if (fw_upload_lenValid(&uiLenToSend, ucString) == TRUE) {
				// Valid length received
				uiValidLen = TRUE;
#ifdef DEBUG_PRINT
				PRINT(" Valid length = %d \n", uiLenToSend);
#endif
				// ACK the bootloader
				sTxFrame.header = V1_REQUEST_ACK;
				bWriteStatus =
					fw_upload_ComWriteChar(iPortID,
							       sTxFrame.header);
#ifdef DEBUG_FRAME
				fw_upload_dumpCtrlFrame(TX_DIR, &sTxFrame,
							sizeof(sTxFrame.header),
							(bWriteStatus)
							? ("(write ok)")
							: ("(write error)"));
#endif
#ifdef DEBUG_PRINT
				PRINT("  BOOT_HEADER_ACK 0x5a sent \n");
#endif
			}
		} while (!uiValidLen);
	}
#ifdef DEBUG_PRINT
	PRINT(" ========== Buffer is successfully sent =========\n");
#endif
	return uiLenToSend;
}

/******************************************************************************
 *
 * Name: fw_upload_V1SendLenBytes
 *
 * Description:
 *   This function sends Len bytes(header+data) to the boot code.    
 *
 * Conditions For Use:
 *   None.
 *
 * Arguments:
 *   pFileBuffer: bin file buffer being sent.
 *   uiLenTosend: the length will be sent.
 *
 * Return Value:
 *   the 'len' of next header request.
 *
 * Notes:
 *   None.
 *
 *****************************************************************************/
static uint16
fw_upload_V1SendLenBytes(uint8 * pFileBuffer, uint16 uiLenToSend)
{
	uint16 ucDataLen, uiLen;
	uint32 ulCmd;
#ifdef DEBUG_PRINT
	uint16 i;
#endif
	memset(ucByteBuffer, 0, sizeof(ucByteBuffer));

	cmd7_Req = FALSE;
	EntryPoint_Req = FALSE;
	// fread(void *buffer, size_t size, size_t count, FILE *stream)

	if (ulCurrFileSize + uiLenToSend > ulTotalFileSize)
		uiLenToSend = (uint16) (ulTotalFileSize - ulCurrFileSize);

	memcpy(&ucByteBuffer[uiLenToSend] - uiLenToSend,
	       pFileBuffer + ulCurrFileSize, uiLenToSend);
	ulCurrFileSize += uiLenToSend;
	ulCmd = fw_upload_GetCmd(ucByteBuffer);
	if (ulCmd == CMD7) {
		cmd7_Req = TRUE;
		ucDataLen = 0;
	} else {
		ucDataLen = fw_upload_GetDataLen(ucByteBuffer);
		memcpy(&ucByteBuffer[uiLenToSend], pFileBuffer + ulCurrFileSize,
		       ucDataLen);
		ulCurrFileSize += ucDataLen;
		if ((ulCurrFileSize < ulTotalFileSize) &&
		    (ulCmd == CMD6 || ulCmd == CMD4)) {
			EntryPoint_Req = TRUE;
		}
	}

#ifdef DEBUG_PRINT
	PRINT("The buffer is to be sent: %d\n", uiLenToSend + ucDataLen);
	for (i = 0; i < (uiLenToSend + ucDataLen); i++) {
		if (i % 16 == 0) {
			PRINT("\n");
		}
		PRINT(" %02x ", ucByteBuffer[i]);
	}
#endif
	//start to send Temp buffer
	uiLen = fw_upload_SendBuffer(uiLenToSend, ucByteBuffer, FALSE);
	PRINT("File downloaded: %8u:%8ld\r", ulCurrFileSize, ulTotalFileSize);

	return uiLen;
}

/******************************************************************************
 *
 * Name: fw_upload_V3SendLenBytes
 *
 * Description:
 *   This function sends Len bytes to the Helper.    
 *
 * Conditions For Use:
 *   None.
 *
 * Arguments:
 *   pFileBuffer: bin file buffer being sent.
 *   uiLenTosend: the length will be sent.
 *   ulOffset: the offset of current sending.
 *
 * Return Value:
 *   None.
 *
 * Notes:
 *   None.
 *
 *****************************************************************************/
static void
fw_upload_V3SendLenBytes(uint8 * pFileBuffer, uint16 uiLenToSend,
			 uint32 ulOffset)
{
	int8 bWriteStatus;

	// Retransmittion of previous block
	if (ulOffset == ulLastOffsetToSend) {
#ifdef DEBUG_PRINT
		PRINT("Resend offset %d...\n", ulOffset);
#endif
		bWriteStatus =
			fw_upload_ComWriteChars(iPortID, (int8 *) ucByteBuffer,
						uiLenToSend);
#ifdef DEBUG_FRAME
		fw_upload_dumpDataFrame(TX_DIR, ucByteBuffer, uiLenToSend,
					(bWriteStatus) ? ("(write ok)")
					: ("(write error)"));
#endif
	} else {

#ifdef DEBUG_PRINT
		PRINT("%s : pFileBuffer %p ulOffset %d change_baudrata_buffer_len %d \
           cmd7_change_timeout_len %d\n", __func__, pFileBuffer, ulOffset,
		      change_baudrata_buffer_len, cmd7_change_timeout_len);
#endif
		if (ulOffset <
		    (change_baudrata_buffer_len + cmd7_change_timeout_len)) {
			PRINT("%s: Something wrong during FW downloading, \
             please power cycling and execute fw_loader again\n", __func__);
			/* TODO: Power cycling implementation */
			exit(EIO);
		}

		// The length requested by the Helper is equal to the Block
		// sizes used while creating the FW.bin. The usual
		// block sizes are 128, 256, 512.
		// uiLenToSend % 16 == 0. This means the previous packet
		// was error free (CRC ok) or this is the first packet received.
		//  We can clear the ucByteBuffer and populate fresh data. 
		memset(ucByteBuffer, 0, MAX_LENGTH * sizeof(uint8));
		memcpy(ucByteBuffer,
		       pFileBuffer + ulOffset - change_baudrata_buffer_len -
		       cmd7_change_timeout_len, uiLenToSend);
		ulCurrFileSize =
			ulOffset - change_baudrata_buffer_len -
			cmd7_change_timeout_len + uiLenToSend;

		bWriteStatus =
			fw_upload_ComWriteChars(iPortID, (int8 *) ucByteBuffer,
						uiLenToSend);
#ifdef DEBUG_FRAME
		fw_upload_dumpDataFrame(TX_DIR, ucByteBuffer, uiLenToSend,
					(bWriteStatus) ? ("(write ok)")
					: ("(write error)"));
#endif

		ulLastOffsetToSend = ulOffset;
	}
}

/******************************************************************************
 *
 * Name: fw_Change_Baudrate
 *
 * Description:
 *   This function changes the baud rate of bootrom.
 *
 * Conditions For Use:
 *   None.
 *
 * Arguments:
 *   pPortName:                  Serial port value.
 *   bFirstWaitHeaderSignature:  TBD.
 *
 * Return Value:
 *   TRUE:            Change baud rate successfully
 *   FALSE:           Change baud rate unsuccessfully
 *
 * Notes:
 *   None.
 *
 *****************************************************************************/
static int32
fw_Change_Baudrate(int8 * pPortName, BOOLEAN bFirstWaitHeaderSignature)
{
	uint8 uartConfig[60];
	uint8 ucBuffer[80];
	uint32 j;
	uint32 uartClk = 0x00C00000;
	uint32 uartDiv = 0x1;
	uint32 uiCrc = 0;
	uint32 uiLen = 0;
	BOOLEAN bRetVal = FALSE;
	int32 ucResult = -1;
	uint8 ucLoadPayload = 0;
	uint32 waitHeaderSigTime = 0;
	BOOLEAN uiReUsedInitBaudrate = FALSE;
	uint32 payload_len = 0;
	BOOLEAN flag = TRUE;

	uint32 mcr = MCR;
	uint32 init = INIT;
	uint32 icr = ICR;
	uint32 fcr = FCR;
	uint32 brAddr = CLKDIVAddr;
	uint32 divAddr = UARTDIVAddr;
	uint32 mcrAddr = UARTMCRAddr;
	uint32 reInitAddr = UARTREINITAddr;
	uint32 icrAddr = UARTICRAddr;
	uint32 fcrAddr = UARTFCRAddr;
	int8 bWriteStatus;
	//WSW-25304: Special handling for OWL A0 only due to boot code side change. CChange is reverted back fro BB/Nighthawk
	if (ChipId == OWL_A0_a || ChipId == OWL_A0_c) {
		brAddr -= 1;
		divAddr -= 1;
		mcrAddr -= 1;
		reInitAddr -= 1;
		icrAddr -= 1;
		fcrAddr -= 1;
	}

	for (j = 0; j < sizeof(UartCfgTbl) / sizeof(UART_BAUDRATE); j++) {
		if (SerialCfg.uart.uiSecondBaudrate == UartCfgTbl[j].iBaudRate) {
			uartDiv = UartCfgTbl[j].iUartDivisor;
			uartClk = UartCfgTbl[j].iClkDivisor;
			ucResult = 0;
			break;
		}
	}

	if (ucResult != 0) {
		return ucResult;
	}

	if (iPortID == OPEN_FAILURE) {
		return RW_FAILURE;
	}

	// Generate CRC value for CMD5 payload
	memcpy(uartConfig + uiLen, &brAddr, 4);
	uiLen += 4;
	memcpy(uartConfig + uiLen, &uartClk, 4);
	uiLen += 4;
	memcpy(uartConfig + uiLen, &divAddr, 4);
	uiLen += 4;
	memcpy(uartConfig + uiLen, &uartDiv, 4);
	uiLen += 4;
	memcpy(uartConfig + uiLen, &mcrAddr, 4);
	uiLen += 4;
	memcpy(uartConfig + uiLen, &mcr, 4);
	uiLen += 4;
	memcpy(uartConfig + uiLen, &reInitAddr, 4);
	uiLen += 4;
	memcpy(uartConfig + uiLen, &init, 4);
	uiLen += 4;
	memcpy(uartConfig + uiLen, &icrAddr, 4);
	uiLen += 4;
	memcpy(uartConfig + uiLen, &icr, 4);
	uiLen += 4;
	memcpy(uartConfig + uiLen, &fcrAddr, 4);
	uiLen += 4;
	memcpy(uartConfig + uiLen, &fcr, 4);
	uiLen += 4;
	payload_len = uiLen + 4;

	fw_upload_gen_crc_table();
	memcpy(m_Buffer_CMD5_Header + 8, &payload_len, 4);
	uiCrc = (uint32) fw_upload_update_crc((unsigned long)0,
					      (char *)m_Buffer_CMD5_Header,
					      (int)12);
	uiCrc = (uint32) SWAPL(uiCrc);
	memcpy(m_Buffer_CMD5_Header + 12, &uiCrc, 4);
	uiCrc = (uint32) fw_upload_update_crc((unsigned long)0,
					      (char *)uartConfig, (int)uiLen);
	uiCrc = (uint32) SWAPL(uiCrc);
	memcpy(uartConfig + uiLen, &uiCrc, 4);
	uiLen += 4;

	while (!bRetVal) {
		if (uiProVer == Ver1) {
			if (uiNewLen == HDR_LEN) {
				// Download CMD5 header and Payload packet.
				memcpy(ucBuffer, m_Buffer_CMD5_Header, HDR_LEN);
				memcpy(ucBuffer + HDR_LEN, uartConfig, uiLen);
				fw_upload_v1_xor_pass();
				fw_upload_SendBuffer(uiNewLen, ucBuffer, TRUE);
				pSerialIf->Close(iPortID);
				if (pSerialIf->type == UART_IF) {
					SerialCfg.uiSpeed =
						SerialCfg.uart.uiSecondBaudrate;
					SerialCfg.uart.ucFlowCtrl = 1;
				}
				iPortID =
					pSerialIf->Open(pPortName, &SerialCfg);
				if (iPortID == OPEN_FAILURE) {
					return RW_FAILURE;
				}
				ucLoadPayload = 1;
			} else if (uiNewLen == uiLen) {
				// Download CMD5 header and Payload packet
				bWriteStatus =
					fw_upload_ComWriteChars(iPortID,
								(int8 *)
								uartConfig,
								uiLen);
#ifdef DEBUG_FRAME
				fw_upload_dumpDataFrame(TX_DIR, uartConfig,
							uiLen,
							(bWriteStatus)
							? ("uartCfg (write ok)")
							:
							("uartCfg (write error)"));
#endif
				pSerialIf->Close(iPortID);
				if (pSerialIf->type == UART_IF) {
					SerialCfg.uiSpeed =
						SerialCfg.uart.uiSecondBaudrate;
					SerialCfg.uart.ucFlowCtrl = 1;
				}
				iPortID =
					pSerialIf->Open(pPortName, &SerialCfg);
				ucLoadPayload = 1;
			} else {
				PRINT("Non-empty terminating else statement\n");
			}
		} else if (uiProVer == Ver3) {
			if (uiNewLen != 0 &&
			    sRxFrame.header == V3_HEADER_DATA_REQ) {
				if (uiNewError == 0) {
					fw_upload_Send_Ack(V3_REQUEST_ACK);
					bFirstWaitHeaderSignature = TRUE;

					if (uiNewLen == HDR_LEN) {
						bWriteStatus =
							fw_upload_ComWriteChars
							(iPortID,
							 m_Buffer_CMD5_Header,
							 uiNewLen);
#ifdef DEBUG_FRAME
						fw_upload_dumpDataFrame(TX_DIR,
									(uint8
									 *)
									m_Buffer_CMD5_Header,
									uiNewLen,
									(bWriteStatus)
									?
									("(write ok)")
									:
									("(write error)"));
#endif
					} else {
						bWriteStatus =
							fw_upload_ComWriteChars
							(iPortID,
							 (int8 *) uartConfig,
							 uiNewLen);
#ifdef DEBUG_FRAME
						fw_upload_dumpDataFrame(TX_DIR,
									uartConfig,
									uiNewLen,
									(bWriteStatus)
									?
									("uartCfg (write ok)")
									:
									("uartCfg (write error)"));
#endif
						//Reopen Uart by using the second baudrate after downloading the payload.
						pSerialIf->Close(iPortID);
						if (pSerialIf->type == UART_IF) {
							SerialCfg.uiSpeed =
								SerialCfg.uart.
								uiSecondBaudrate;
							SerialCfg.uart.
								ucFlowCtrl = 1;
						}
						iPortID =
							pSerialIf->
							Open(pPortName,
							     &SerialCfg);
						ucLoadPayload = 1;
					}
				} else	//NAK,TIMEOUT,INVALID COMMAND...
				{
					pSerialIf->Flush(iPortID);
					fw_upload_Send_Ack(V3_TIMEOUT_ACK);
				}
			}
		} else {
			PRINT("Non-empty terminating else statement\n");
		}
		if (ucLoadPayload != 0 || uiReUsedInitBaudrate) {
			waitHeaderSigTime = TIMEOUT_VAL_MILLISEC;
		} else {
			waitHeaderSigTime = 0;
		}
		flag = fw_upload_WaitForHeaderSignature(waitHeaderSigTime);
		if (flag) {
			if (ucLoadPayload) {
				if (uiProVer == Ver3) {
					change_baudrata_buffer_len =
						(uint32) HDR_LEN +
						(uint32) payload_len;
				}
				break;
			}
		} else {
			if (uiReUsedInitBaudrate) {
				ucResult = -2;
				return ucResult;
			}
			if (ucLoadPayload) {
				// If 0xa5 or 0xa7 is not received by using the second baudrate, change baud rate to the first baudrate.
				if (iPortID == OPEN_FAILURE) {
					return RW_FAILURE;
				}
				pSerialIf->Close(iPortID);

				if (pSerialIf->type == UART_IF) {
					SerialCfg.uiSpeed =
						SerialCfg.uart.uiFirstBaudrate;
				}
				iPortID =
					pSerialIf->Open(pPortName, &SerialCfg);
				if (iPortID == OPEN_FAILURE) {
					return RW_FAILURE;
				}
				ucLoadPayload = 0;
				uiReUsedInitBaudrate = TRUE;
				continue;
			}
		}
	}
	return ucResult;
}

/******************************************************************************
 *
 * Name: fw_Change_Timeout
 *
 * Description:
 *   This function changes timeout value of boot loader
 *
 * Conditions For Use:
 *   None.
 *
 * Arguments:
 *   pPortName:       Com port number.

 * Return Value:
 *   the status  of changing timeout value
 *
 * Notes:
 *   None.
 *
 *****************************************************************************/
static int32
fw_Change_Timeout()
{

	int32 Status = -1;
	BOOLEAN bFirst = TRUE;
	BOOLEAN bRetVal = FALSE;
	uint8 reTryNumber = 0;
	int8 bWriteStatus;

	fw_upload_gen_crc_table();

	while (!bRetVal) {
		if (fw_upload_WaitForHeaderSignature(TIMEOUT_VAL_MILLISEC)) {
			if (uiProVer == Ver3) {
				if (uiNewLen != 0) {
					if (uiNewError == 0) {
#ifdef DEBUG_PRINT
						PRINT(" === Succ: REQ = 0xA7, Errcode = 0 \n");
#endif
						if (bFirst ||
						    ulLastOffsetToSend ==
						    ulNewOffset) {
							fw_upload_Send_Ack
								(V3_REQUEST_ACK);
							bWriteStatus =
								fw_upload_ComWriteChars
								(iPortID,
								 m_Buffer_CMD7_ChangeTimeoutValue,
								 uiNewLen);
#ifdef DEBUG_FRAME
							fw_upload_dumpDataFrame
								(TX_DIR,
								 (uint8 *)
								 m_Buffer_CMD7_ChangeTimeoutValue,
								 uiNewLen,
								 (bWriteStatus)
								 ?
								 ("(write ok)")
								 :
								 ("(write error)"));
#endif
							ulLastOffsetToSend =
								ulNewOffset;
							bFirst = FALSE;
						} else {
							bRetVal = TRUE;
							Status = 0;
						}
					} else {
						if (reTryNumber < 6) {
							pSerialIf->
								Flush(iPortID);
							fw_upload_Send_Ack
								(V3_TIMEOUT_ACK);
							reTryNumber++;
						} else {
							bRetVal = TRUE;
						}
					}
				}
			}
			if (uiProVer == Ver1) {
				Status = 1;
				break;
			}
		} else {
			PRINT("Timeout for waiting header signature in fw_Change_Timeout function\n");
			return Status;
		}
	}
	return Status;
}

/******************************************************************************
 *
 * Name: fw_upload_FW
 *
 * Description:
 *   This function performs the task of FW load over UART.
 *
 * Conditions For Use:
 *   None.
 *
 * Arguments:
 *   pPortName:       Com port number.
 *
 * Return Value:
 *   the error code of downloading
 *
 * Notes:
 *   None.
 *
 *****************************************************************************/
static uint32
fw_upload_FW(int8 * pPortName)
{
	uint8 *pFileBuffer = NULL;
	uint16 uiLenToSend = 0;

	BOOLEAN bRetVal = 0;
	BOOLEAN flag = TRUE;
	int32 result = 0;

	uint32 ulReadLen = 0;

	BOOLEAN bFirstWaitHeaderSignature = TRUE;
	// UART specific variables

	if (uiReDownload == FALSE) {
		// Toggle RTS to wake up BT
		SerialCfg.uiSpeed = SerialCfg.uart.uiFirstBaudrate;
		fw_toggle_rts();
		iPortID = pSerialIf->Open(pPortName, &SerialCfg);
	}
	if ((iPortID == OPEN_FAILURE) || (pFile == NULL)) {
		PRINT("Port is not open or file not found\n");
		return OPEN_SERIAL_PORT_OR_FILE_ERROR;
	}

	result = fw_Change_Timeout();
	if (result == 0) {
		cmd7_change_timeout_len = HDR_LEN;
		bFirstWaitHeaderSignature = FALSE;
	} else if (result == -1) {
		return result;
	}

	if (pSerialIf->type == UART_IF && SerialCfg.uart.uiSecondBaudrate) {
		uint32 j = 0;
		result = fw_Change_Baudrate(pPortName,
					    bFirstWaitHeaderSignature);
		switch (result) {
		case -1:
			PRINT("Second baud rate %d is not support\n",
			      SerialCfg.uart.uiSecondBaudrate);
			PRINT("Fw loader only supports the baud rate as");
			for (j = 0;
			     j < sizeof(UartCfgTbl) / sizeof(UART_BAUDRATE);
			     j++) {
				PRINT(" %d ", UartCfgTbl[j].iBaudRate);
			}
			PRINT("\n");
			break;
		case -2:
			PRINT("0xa5 or 0xaa is not received after changing baud rate in 2s.\n");
			break;
		default:
			break;
		}
		if (result != 0) {
			return CHANGE_BAUDRATE_FAIL;
		}
	}

	// Calculate the size of the file to be downloaded. 
	result = fseek(pFile, 0, SEEK_END);
	if (result != 0) {
		PRINT("fseek failed\n");
		return FEEK_SEEK_ERROR;
	}

	ulTotalFileSize = (long)ftell(pFile);
	if (ulTotalFileSize <= 0) {
		PRINT("Error:Download Size is 0\n");
		return FILESIZE_IS_ZERO;
	}
	pFileBuffer = (uint8 *) malloc((size_t)ulTotalFileSize);
	if (pFileBuffer == NULL) {
		PRINT("malloc() returned NULL while allocating size for file\n");
		return MALLOC_RETURNED_NULL;
	}

	result = fseek(pFile, 0, SEEK_SET);
	if (result != 0) {
		PRINT("fseek() failed\n");
		free(pFileBuffer);
		return FEEK_SEEK_ERROR;
	}

	if (pFileBuffer != (void *)0) {
		ulReadLen =
			(size_t)fread((void *)pFileBuffer, (size_t)1,
				      (size_t)ulTotalFileSize, pFile);
		if (ulReadLen != ulTotalFileSize) {
			PRINT("Error:Read File Fail\n");
			free(pFileBuffer);
			return READ_FILE_FAIL;
		}
	}
	ulCurrFileSize = 0;

	while (!bRetVal) {
		if (uiProVer == Ver1) {
			uiLenToSend = uiNewLen;
			fw_upload_v1_xor_pass();
			do {
				uiLenToSend =
					fw_upload_V1SendLenBytes(pFileBuffer,
								 uiLenToSend);
			} while (uiLenToSend != 0);
			// If the Length requested is 0, download is complete.
			if (uiLenToSend == 0) {
				bRetVal = TRUE;
				break;
			}
		} else if (uiProVer == Ver3) {
			if (uiNewLen != 0) {
				if (uiNewError == 0) {
#ifdef DEBUG_PRINT
					PRINT(" === Succ: REQ = 0xA7, Errcode = 0 \n");
#endif
					fw_upload_Send_Ack(V3_REQUEST_ACK);
					fw_upload_V3SendLenBytes(pFileBuffer,
								 uiNewLen,
								 ulNewOffset);

#ifdef DEBUG_PRINT
					PRINT(" sent %d bytes..\n", uiNewLen);
#endif
				} else	//NAK,TIMEOUT,INVALID COMMAND...
				{
#ifdef DEBUG_PRINT
					uint8 i;
					PRINT(" === Fail: REQ = 0xA7, Errcode != 0 \n");
					for (i = 0; i < 7; i++) {
						uiErrCnt[i] +=
							(uiNewError >> i) & 0x1;
					}
#endif
					pSerialIf->Flush(iPortID);
					fw_upload_Send_Ack(V3_TIMEOUT_ACK);
					if (uiNewError & BT_MIC_FAIL_BIT) {
						change_baudrata_buffer_len = 0;
						ulCurrFileSize = 0;
						ulLastOffsetToSend = 0xFFFF;
					}
				}
			} else {
				/* check if download complete */
				if (uiNewError == 0) {
					fw_upload_Send_Ack(V3_REQUEST_ACK);
					bRetVal = TRUE;
					break;
				} else if (uiNewError & BT_MIC_FAIL_BIT) {
#ifdef DEBUG_PRINT
					uiErrCnt[7] += 1;
#endif
					fw_upload_Send_Ack(V3_REQUEST_ACK);
					fseek(pFile, 0, SEEK_SET);
					change_baudrata_buffer_len = 0;
					ulCurrFileSize = 0;
					ulLastOffsetToSend = 0xFFFF;
				} else if (uiNewError & TIMEOUT_REC_ACK_BIT) {
					// Send ACK when Timeout & Len=0 .
					fw_upload_Send_Ack(V3_TIMEOUT_ACK);
				} else {
					PRINT("Non-empty terminating else statement\n");
				}
			}
			PRINT("File downloaded: %8u:%8ld\r", ulCurrFileSize,
			      ulTotalFileSize);
#ifdef DEBUG_FRAME
			if (fw_upload_isDumpFrame()) {
				PRINT("\n");
			}
#endif

		} else {
			PRINT("Not downloaded\n");
		}
		flag = fw_upload_WaitForHeaderSignature(TIMEOUT_VAL_MILLISEC);
		if (!flag) {
			PRINT("0xa5,0xaa,0xab or 0xa7 is not received in %d ms\n", TIMEOUT_VAL_MILLISEC);
			free(pFileBuffer);
			return HEADER_SIGNATURE_TIMEOUT;
		}
	}

	if (pFileBuffer != NULL) {
		free(pFileBuffer);
		pFileBuffer = NULL;
	}
	return DOWNLOAD_SUCCESS;
}

/******************************************************************************
 *
 * Name: closeFileorDescriptor
 *
 * Description:
 *   Closes File* or the descriptor pointing to the file
 *
 * Conditions For Use:
 *   File* and fileDescriptor should be pointing to the same resource
 *
 * Arguments:
 *   fileDescriptor:  integer read only descriptor of the file
 *
 * Return Value:
 *   None 
 *
 * Notes:
 *   None.
 *
 ****************************************************************************/
static void
closeFileorDescriptor(int fileDescriptor)
{
	if (pFile != (void *)0) {
		fclose(pFile);
		pFile = NULL;
	}
	if (fileDescriptor != -1) {
		close(fileDescriptor);
	}
}

/******************************************************************************
 *
 * Name: fw_upload_usage
 *
 * Description:
 *   This function display the usage of this program
 *
 * Conditions For Use:
 *   None.
 *
 * Arguments:
 *   argv:       Arguments table.
 *
 * Return Value:
 *   None.
 *
 * Notes:
 *   None.
 *
 *****************************************************************************/
static int
fw_upload_usage(char *argv[], SERIAL_TYPE type)
{
	PRINT("\n");
	PRINT("Utility for downloading firmware (version %s) using Serial/Spi port\n", VERSION);
	PRINT("\n");
	if (type == UART_IF || type == UNKNOWN_IF) {
		PRINT("UART Usage: %s <ComPort> <BaudRate> <FlowControl> <FileName> [Second BaudRate]\n", argv[0]);
		PRINT("--e.g. %s com3 115200 0 uart8997.bin\n", argv[0]);
		PRINT("--e.g. %s /dev/ttyUSB# 115200 0 uart8997.bin\n",
		      argv[0]);
		PRINT("--e.g. %s com3 115200 0 uart9098.bin 3000000\n",
		      argv[0]);
		PRINT("--e.g. %s /dev/ttyUSB# 115200 0 uart9098.bin 3000000\n",
		      argv[0]);
		PRINT("\n");
	}
#if defined(LINUX_SPI)
	if (type == SPI_IF || type == UNKNOWN_IF) {
		PRINT("SPI Usage: %s <SpiPort> <SpiSpeed> <SpiMode> <FileName> <GpioIntDevice> <GpioIntLine>\n", argv[0]);
		PRINT("--e.g. %s /dev/spidev1.0 4000000 0 uart8997.bin /dev/gpiochip5 12\n", argv[0]);
		PRINT("\n");
	}
#endif
	PRINT("\n");
	PRINT("Env variable supported\n");
	PRINT("  DUMP_FRAME: dumps frames if greater to 0 (V3 only)\n");
	PRINT("Examples:\n");
#ifdef OS_TYPE_FW_LOADER_WIN
	PRINT("  set DUMP_FRAME=0            (default)\n");
	PRINT("  set DUMP_FRAME=1\n");
#endif
#ifdef OS_TYPE_FW_LOADER_LINUX
	PRINT("  export DUMP_FRAME=0         (default)\n");
	PRINT("  export DUMP_FRAME=1\n");
#endif
	PRINT("\n");
	return 1;
}

/******************************************************************************
 *
 * Name: fw_upload_parse_args_uart
 *
 * Description:
 *   This function parse the args for uart
 *
 * Conditions For Use:
 *   None.
 *
 * Arguments:
 *   argc:       Number of arguments.
 *   argv:       Arguments table.
 *
 * Return Value:
 *   pFileName:  pointer to store the name of the file to download
 *
 * Notes:
 *   None.
 *
 *****************************************************************************/
static char *
fw_upload_parse_args_uart(int argc, char *argv[])
{
	char *pFileName;

	/* Parse mandatory args */
	if (argc < 5)		/* fw_loader <ComPort> <BaudRate> <FlowControl> <FileName> */
		return NULL;

	/* Parse Arg2 <BaudRate> */
	SerialCfg.uart.uiFirstBaudrate = atoi(argv[2]);
	/* Parse Arg3 <FlowControl> */
	SerialCfg.uart.ucFlowCtrl = atoi(argv[3]);
	/* Parse Arg4 <FileName> */
	pFileName = argv[4];

	/* Check args */
	if (SerialCfg.uart.ucFlowCtrl != 0 && SerialCfg.uart.ucFlowCtrl != 1) {
		PRINT("Usage: fw_loader <ComPort> <BaudRate> <FlowControl> <FileName>, FlowControl should be 0 or 1\n");
		return NULL;
	}

	/* Fixed option */
	SerialCfg.uart.ucParity = 0;
	SerialCfg.uart.ucStopBits = 1;
	SerialCfg.ucByteSize = sizeof(uint8) * 8;

	/* Parse optional args */
	switch (argc) {
	case 5:		/* fw_loader <ComPort> <BaudRate> <FlowControl> <FileName> */
		/* Ok, we are done */
		break;
	case 6:		/* fw_loader <ComPort> <BaudRate> <FlowControl> <FileName> [Second BaudRate] */
		/* Parse Arg5 [Second BaudRate] */
		SerialCfg.uart.uiSecondBaudrate = atoi(argv[5]);
		break;
	default:
		return NULL;
		break;
	}

	/* Print config2 */
	PRINT("BaudRate:          %d\n", SerialCfg.uart.uiFirstBaudrate);
	PRINT("FlowControl:       %d\n", SerialCfg.uart.ucFlowCtrl);
	PRINT("Filename:          %s\n", pFileName);
	if (SerialCfg.uart.uiSecondBaudrate) {
		PRINT("Second BaudRate:  %d\n",
		      SerialCfg.uart.uiSecondBaudrate);
	}

	/* Ok, we are safe */
	return pFileName;
}

#ifdef LINUX_SPI
/******************************************************************************
 *
 * Name: fw_upload_parse_args_spi
 *
 * Description:
 *   This function parse the args for spi
 *
 * Conditions For Use:
 *   None.
 *
 * Arguments:
 *   argc:       Number of arguments.
 *   argv:       Arguments table.
 *
 * Return Value:
 *   pFileName:  pointer to store the name of the file to download
 *
 * Notes:
 *   None.
 *
 *****************************************************************************/
static char *
fw_upload_parse_args_spi(int argc, char *argv[])
{
	char *pFileName;

	/* Parse mandatory args */
	if (argc < 7)		/* fw_loader <SpiPort> <SpiSpeed> <SpiMode> <FileName> <GpioIntDevice> <GpioIntLine> */
		return NULL;

	/* Parse Arg2 <SpiSpeed> */
	SerialCfg.uiSpeed = atoi(argv[2]);
	/* Parse Arg3 <SpiMode> */
	SerialCfg.spi.ucMode = atoi(argv[3]);
	/* Parse Arg4 <FileName> */
	pFileName = argv[4];
	/* Parse Arg5 <GpioIntDevice> */
	SerialCfg.spi.pGpioIntDev = argv[5];
	/* Parse Arg6 <GpioIntLine> */
	SerialCfg.spi.ucGpioIntLine = atoi(argv[6]);

	/* Fixed option */
	SerialCfg.ucByteSize = sizeof(uint8) * 8;

	/* Parse optional args */
	switch (argc) {
	case 7:		/* fw_loader <SpiPort> <SpiSpeed> <SpiMode> <FileName> <GpioIntDevice> <GpioIntLine> */
		/* Ok, we are done */
		break;
	default:
		return NULL;
		break;
	}

	/* Print config2 */
	PRINT("SpiSpeed:          %d\n", SerialCfg.uiSpeed);
	PRINT("SpiMode:           %d\n", SerialCfg.spi.ucMode);
	PRINT("Filename:          %s\n", pFileName);
	PRINT("GpioInterrupt:     %s line %d\n", SerialCfg.spi.pGpioIntDev,
	      SerialCfg.spi.ucGpioIntLine);

	/* Ok, we are safe */
	return pFileName;
}
#endif

/*******************************************************************************
 *
 * Name: main
 *
 * Description:
 *   Main Entry point of the application.
 *
 * Conditions For Use:
 *   None.
 *
 * Arguments:
 *   None.
 *
 * Return Value:
 *   None 
 *
 * Notes:
 *   None.
 *
 *****************************************************************************/
int
main(int argc, char **argv)
{
	char *pPortName = NULL;
	char *pFileName = NULL;

	const char *pVersionName = VERSION;
	uint64 endTime;
	uint64 cost;
	uint32 ulResult;
	int32 fileDescriptor = -1;

	// Initialize the function pointers.
	fw_upload_io_func_init();
	init_crc8();
	fwLoaderStartTime = fw_upload_GetTime();

	if (sizeof(sRxFrame.ackNakV3) != 1) {
		PRINT("Invalid ackNakV3 pack size %ld\n",
		      sizeof(sRxFrame.ackNakV3));
		return 1;
	}

	if (sizeof(sRxFrame.timeoutV3) != 5) {
		PRINT("Invalid timeoutV3 pack size %ld\n",
		      sizeof(sRxFrame.timeoutV3));
		return 1;
	}

	if (sizeof(sRxFrame.startIndV3) != 4) {
		PRINT("Invalid startIndV3 pack size %ld\n",
		      sizeof(sRxFrame.startIndV3));
		return 1;
	}

	if (sizeof(sRxFrame.dataReqV3) != 9) {
		PRINT("Invalid dataReqV3 pack size %ld\n",
		      sizeof(sRxFrame.dataReqV3));
		return 1;
	}

	/* First get the device name */
	if (argc < 2)
		return fw_upload_usage(argv, UNKNOWN_IF);

	/* Parse Arg1 <Com/Spi Port> */
	pPortName = argv[1];

	/* Next detect the type of device (UART, SPI if needed) */
	pSerialIf = NULL;

#ifdef OS_TYPE_FW_LOADER_WIN

	if (sSerialInterfaceWin.Probe(pPortName))
		pSerialIf = &sSerialInterfaceWin;

#elif defined(OS_TYPE_FW_LOADER_LINUX)

	if (sSerialInterfaceLinuxUart.Probe(pPortName))
		pSerialIf = &sSerialInterfaceLinuxUart;
#if defined(LINUX_SPI)
	else if (sSerialInterfaceLinuxSpi.Probe(pPortName))
		pSerialIf = &sSerialInterfaceLinuxSpi;
#endif

#else
#error Unknown OS_TYPE
#endif

	if (!pSerialIf) {
		PRINT("Invalid device %s\n", pPortName);
		return 1;
	}

	/* Print config1 */
	PRINT("Protocol:          NXP Proprietary\n");
	PRINT("FW Loader Version: %s\n", pVersionName);
	PRINT("Serial:            %s\n", pSerialIf->name);
	PRINT("Device:            %s\n", pPortName);
	PRINT("DumpFrame:         %s\n",
	      (fw_upload_isDumpFrame())? ("Yes") : ("No"));
	PRINT("Timeout:           %d\n", TIMEOUT_VAL_MILLISEC);

	switch (pSerialIf->type) {
	case UART_IF:
		pFileName = fw_upload_parse_args_uart(argc, argv);
		if (!pFileName)
			return fw_upload_usage(argv, pSerialIf->type);
		break;

#ifdef LINUX_SPI
	case SPI_IF:
		pFileName = fw_upload_parse_args_spi(argc, argv);
		if (!pFileName)
			return fw_upload_usage(argv, pSerialIf->type);
		break;
#endif

	default:		/* Unknown pSerialIf->type */
		return fw_upload_usage(argv, pSerialIf->type);
		break;
	}

#if 0				/* Not working on Win10 */
	fileDescriptor = open(pFileName, O_RDONLY);	// open file descriptor.
	if (fileDescriptor == -1) {
		PRINT("\nError in getting file descriptor");
		retun 1;
	}
	pFile = fdopen(fileDescriptor, "rb");	// open streaming handle.
#else
	pFile = fopen(pFileName, "rb");	// open streaming handle.
#endif
	if (NULL == pFile) {
		perror("fdopen failed");
		return 1;
	}

	do {
		gPortName = pPortName;
		ulResult = fw_upload_FW(pPortName);
		if (ulResult == 0) {
			PRINT("Download Complete\n");
			cost = fw_upload_GetTime() - fwLoaderStartTime;
			PRINT("FW Downloading time:%llu\n", cost);
			if (uiProVer == Ver3 && pSerialIf->type == UART_IF &&
			    SerialCfg.uart.uiSecondBaudrate) {
				fw_upload_DelayInMs(100);
				endTime =
					fw_upload_GetTime() +
					2 * MAX_CTS_TIMEOUT;
			} else {
				fw_upload_DelayInMs(500);
				endTime = fw_upload_GetTime() + MAX_CTS_TIMEOUT;
			}
			do {
				if (!pSerialIf->ComGetCTS(iPortID)) {
					cost = fw_upload_GetTime() - fwLoaderStartTime - cost;	// FW init time = (Current Time) - (Fw loader start time) - (FW DLD time)
					switch (pSerialIf->type) {
					case UART_IF:
						PRINT("FW is Active (CTS is low)\n");
						PRINT("FW INIT Time:%llu\n",
						      cost);
						break;
#if defined(LINUX_SPI)
					case SPI_IF:
						PRINT("FW is Active (SPI is idle)\n");
						PRINT("FW INIT Time:%llu\n",
						      cost);
						break;
#endif
					default:
						PRINT("UNKOWN is low\n");
						break;
					}
					closeFileorDescriptor(fileDescriptor);
					exit((int32) ulResult);
				}
			} while (endTime > fw_upload_GetTime());
			cost = fw_upload_GetTime() - fwLoaderStartTime - cost;
			switch (pSerialIf->type) {
			case UART_IF:
				PRINT("FW init failed (CTS is high)\n");
				PRINT("Wait CTS low timeout, Duration:%llu\n",
				      cost);
				break;
#if defined(LINUX_SPI)
			case SPI_IF:
				PRINT("FW init failed (SPI is active)\n");
				PRINT("Wait SPI idle timeout, Duration:%llu\n",
				      cost);
				break;
#endif
			default:
				PRINT("UNKOWN is high\n");
				break;
			}
			PRINT("Error code is %d\n", ulResult);
			uiReDownload = FALSE;
		} else {
			PRINT("Download Error\n");
			PRINT("Error code is %d\n", ulResult);
			uiReDownload = FALSE;
		}
	} while (uiReDownload);

	if (uiReDownload == FALSE) {
		closeFileorDescriptor(fileDescriptor);
		return (int)ulResult;
	}
	return 0;
}
