/** @file   fw_loader_io_win.c
 *
 *  @brief  This file contains the functions that implement the Nxp specific
 *          Helper Protocol for Windows.
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
#include "fw_loader_io_win.h"
/*===================== Macros ===================================================*/
#define TIMEOUT_FOR_READ        2000

/*==================== Typedefs =================================================*/

/*==================== Function Prototypes ======================================*/
static int8 fw_upload_Probe_Win(int8 * dev);
static int8 fw_upload_ComReadChar_Win(int32 iPortID);
static int8 fw_upload_ComWriteChar_Win(int32 iPortID, int8 iChar);
static int8 fw_upload_ComWriteChars_Win(int32 iPortID, int8 * pChBuffer,
					uint32 uiLen);
static int32 fw_upload_ComReadChars_Win(int32 iPortID, int8 * pChBuffer,
					uint32 uiCount);
static int32 fw_upload_init_uart_Win(int8 * pPortName, SERIAL_CONFIG * pConfig);
static int32 fw_upload_ComGetCTS_Win(int32 iPortID);
static int32 fw_upload_ComGetBufferSize_Win(int32 iPortID);
static void fw_upload_Flush_Win(int32 iPortID);
static void fw_upload_CloseUart_Win(int32 iPortID);

/*===================== Global Vars ==============================================*/
SERIAL_INTERFACE sSerialInterfaceWin = {
	.name = "COM interface",
	.type = UART_IF,
	.Probe = fw_upload_Probe_Win,
	.Open = fw_upload_init_uart_Win,
	.Close = fw_upload_CloseUart_Win,
	.ComReadChar = fw_upload_ComReadChar_Win,
	.ComWriteChar = fw_upload_ComWriteChar_Win,
	.ComWriteChars = fw_upload_ComWriteChars_Win,
	.ComReadChars = fw_upload_ComReadChars_Win,
	.ComGetCTS = fw_upload_ComGetCTS_Win,
	.ComGetBufferSize = fw_upload_ComGetBufferSize_Win,
	.Flush = fw_upload_Flush_Win,
};

/*==================== Coded Procedures =========================================*/
/******************************************************************************
 *
 * Name: fw_upload_Probe_Win
 *
 * Description:
 *   This function validates this device belongs to this driver.
 *
 * Conditions For Use:
 *   None.
 *
 * Arguments:
 *   dev - Device name.
 *
 * Return Value:
 *   Returns 1 for success.
 *   Returns 0 for failure.
 *
 * Notes:
 *   None.
 *
 *****************************************************************************/
static int8
fw_upload_Probe_Win(int8 * dev)
{
	int8 result = 0;
	HANDLE myport;
	DCB dcb = { 0 };
	COMMTIMEOUTS timeouts;
	HANDLE iresult = (void *)-1;
	char aucPortName[32];

	//for COM port
	sprintf(aucPortName, "\\\\.\\%s", dev);

	// Open the COM Port
	myport = CreateFile(aucPortName, GENERIC_READ | GENERIC_WRITE,	//access ( read and write)
			    0,	//(share) 0:cannot share the COM port
			    0,	//security  (None)
			    OPEN_EXISTING,	// creation : open_existing
			    FILE_ATTRIBUTE_NORMAL,	// we want overlapped operation
			    0	// no templates file for COM port...
		);
	if (myport == INVALID_HANDLE_VALUE) {
		return 0;
	}

	if (GetCommState(myport, &dcb)) {
		// Speed: dcb.BaudRate.
		result = 1;
//  } else {
//  PRINT("%s is not a COM# (%s)\n", dev, strerror(errno));
	}

	CloseHandle(myport);

	return result;
}

/******************************************************************************
 *
 * Name: fw_upload_DelayInMs_Win
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
fw_upload_DelayInMs_Win(uint32 uiMs)
{
	// Call the Sleep Function provided by Windows
	// Note: The Sleep function takes Millisecond units
	Sleep(uiMs);
}

/******************************************************************************
 *
 * Name: fw_upload_ComReadChar_Win
 *
 * Description:
 *   Read a character from the port specified by iPortID.
 *
 * Conditions For Use:
 *   None.
 *
 * Arguments:
 *   None.
 *
 * Return Value:
 *   Returns the character, if Successful.
 *   Returns RW_FAILURE if no character available (OR TIMED-OUT)
 *
 * Notes:
 *   None.
 *
 *****************************************************************************/
static int8
fw_upload_ComReadChar_Win(int32 iPortID)
{
	uint32 uibuf;
	uint64 uEndTime;
	int32 iResult = 0xFF;
	uint8 ucNumCharToRead = 1;
	uint64 uTimeOut = TIMEOUT_FOR_READ;

	uEndTime = fw_upload_GetTime_Win() + uTimeOut;

	// Read from the com port.
	// Note: Parameters 4 & 5 - uiBuf and NULL are inconsequential - Optional
	// parameters.
	// uiBuf - A pointer to the variable that receives the number of bytes read
	// when using a synchronous hFile parameter.
	// Parameter 5 - A pointer to an  OVERLAPPED structure is required if the iPortID parameter
	// was opened with FILE_FLAG_OVERLAPPED, otherwise it can be NULL.
	do {
		if (ReadFile
		    ((HANDLE) iPortID, &iResult, ucNumCharToRead, &uibuf,
		     NULL)) {
			return (iResult & 0xFF);
		}
	} while (uEndTime > fw_upload_GetTime_Win());
	ClearCommError((HANDLE) iPortID, NULL, NULL);
	return 0xFF;
}

/******************************************************************************
 *
 * Name: fw_upload_ComReadChars_Win
 *
 * Description:
 *   Read iCount characters from the port specified by iPortID.
 *
 * Conditions For Use:
 *   None.
 *
 * Arguments:
 *   pBuffer   : Destination buffer for the characters read
 *   iCount    : Number of Characters to be read.
 *
 * Return Value:
 *   Returns the number of characters read if Successful.
 *   Returns RW_FAILURE if iCount characters could not be read or if Port ID is invalid.
 *
 * Notes:
 *   None.
 *
 *****************************************************************************/
static int32
fw_upload_ComReadChars_Win(int32 iPortID, int8 * pBuffer, uint32 uiCount)
{
	uint64 uEndTime;
	uint32 uiRetBytes = 0;
	uint64 uTimeOut = TIMEOUT_FOR_READ;

	uEndTime = fw_upload_GetTime_Win() + uTimeOut;

	// Read from the com port.
	// Parameter 5 - A pointer to an  OVERLAPPED structure is required if the iPortID parameter
	// was opened with FILE_FLAG_OVERLAPPED, otherwise it can be NULL.
	do {
		if (fw_upload_ComGetBufferSize_Win(iPortID) >= (int32) uiCount) {
			if (ReadFile
			    ((HANDLE) iPortID, pBuffer, uiCount, &uiRetBytes,
			     NULL) && uiRetBytes == uiCount) {
				// Read Successful. Return the number of characters read.
				return uiCount;
			}
		}
	} while (uEndTime > fw_upload_GetTime_Win());

	ClearCommError((HANDLE) iPortID, NULL, NULL);
	return RW_FAILURE;
}

/******************************************************************************
 *
 * Name: fw_upload_ComWriteChar_Win
 *
 * Description:
 *   Write a character to the port specified by iPortID.
 *
 * Conditions For Use:
 *   None.
 *
 * Arguments:
 *   iChar   : Character to be written
 *
 * Return Value:
 *   Returns success or failure of the Write.
 *
 * Notes:
 *   None.
 *
 *****************************************************************************/
static int8
fw_upload_ComWriteChar_Win(int32 iPortID, int8 iChar)
{
	uint32 uibuf;
	uint8 ucNumCharToWrite = 1;

	// Write 1 char to the com port.
	// Note: Parameters 4 & 5 - uiBuf and NULL are inconsequential - Optional
	// parameters.
	// uiBuf - A pointer to the variable that receives the number of bytes written
	// when using a synchronous iPortID parameter. In this case,
	// it will be 1 after the write.
	// Parameter 5 - A pointer to an  OVERLAPPED structure is required if the
	// iPortID parameter was opened with FILE_FLAG_OVERLAPPED, otherwise it can be NULL.
	if (!WriteFile
	    ((HANDLE) iPortID, &iChar, ucNumCharToWrite, &uibuf, NULL)) {
		ClearCommError((HANDLE) iPortID, NULL, NULL);
		return RW_FAILURE;
	}
	return RW_SUCCESSFUL;
}

/******************************************************************************
 *
 * Name: fw_upload_ComWriteChars_Win
 *
 * Description:
 *   Write iLen characters to the port specified by iPortID.
 *
 * Conditions For Use:
 *   None.
 *
 * Arguments:
 *   pBuffer : Buffer where characters are available to be written to the Port.
 *   iLen    : Number of Characters to write.
 *
 * Return Value:
 *   Returns success or failure of the Write.
 *
 * Notes:
 *   None.
 *
 *****************************************************************************/
static int8
fw_upload_ComWriteChars_Win(int32 iPortID, int8 * pBuffer, uint32 uiLen)
{
	uint32 uibuf;

	// Write to the com port.
	// Parameter 5 - A pointer to an  OVERLAPPED structure is required if the iPortID parameter
	// was opened with FILE_FLAG_OVERLAPPED, otherwise it can be NULL.
	if (!WriteFile((HANDLE) iPortID, pBuffer, uiLen, &uibuf, NULL)) {
		ClearCommError((HANDLE) iPortID, NULL, NULL);
		return RW_FAILURE;
	}
	return RW_SUCCESSFUL;
}

/******************************************************************************
 *
 * Name: fw_upload_ComGetCTS_Win
 *
 * Description:
 *   Check CTS status
 *
 * Conditions For Use:
 *   None.
 *
 * Arguments:
 *
 * Return Value:
 *   return the status of CTS
 *
 * Notes:
 *   None.
 *
 
*****************************************************************************/

static int32
fw_upload_ComGetCTS_Win(int32 iPortID)
{
	COMSTAT lpStat;

	ClearCommError((HANDLE) iPortID, NULL, &lpStat);
	if (lpStat.fCtsHold == 1) {
		return 1;
	} else {
		return 0;
	}
}

/******************************************************************************
 *
 * Name: fw_upload_GetTime_Win
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
fw_upload_GetTime_Win(void)
{
	uint64 time;
	time = GetTickCount();
	return time;

}

/******************************************************************************
 *
 * Name: fw_upload_ComGetBufferSize_Win
 *
 * Description:
 *   get buffer size.
 *
 * Conditions For Use:
 *   None.
 *
 * Arguments:
 *   iPortID
 *
 * Return Value:
 *   buffer size.
 * Notes:
 *   None.
 *
 *****************************************************************************/
static int32
fw_upload_ComGetBufferSize_Win(int32 iPortID)
{
	int32 bytes = 0;
	DWORD dwErrors;
	COMSTAT Rcs;

	ClearCommError((HANDLE) iPortID, &dwErrors, &Rcs);
	bytes = Rcs.cbInQue;

	return bytes;
}

/******************************************************************************
 *
 * Name: fw_upload_Flush_Win
 *
 * Description:
 *   flush buffer.
 *
 * Conditions For Use:
 *   None.
 *
 * Arguments:
 *   iPortID
 *
 * Return Value:
 *
 * Notes:
 *   None.
 *
 *****************************************************************************/
static void
fw_upload_Flush_Win(int32 iPortID)
{
	PurgeComm((HANDLE) iPortID, PURGE_RXCLEAR);
}

/******************************************************************************
 *
 * Name: fw_upload_init_uart_Win
 *
 * Description:
 *   Initialize UART.
 *
 * Conditions For Use:
 *   None.
 *
 * Arguments:
 *
 * Return Value:
 *
 * Notes:
 *   None.
 *
 *****************************************************************************/
static int32
fw_upload_init_uart_Win(int8 * pPortName, SERIAL_CONFIG * pConfig)
{
	HANDLE myport;
	DCB dcb = { 0 };
	COMMTIMEOUTS timeouts;
	HANDLE iresult = (void *)-1;
	char aucPortName[32];

	//for COM port
	sprintf(aucPortName, "\\\\.\\%s", pPortName);

	// Open the COM Port
	myport = CreateFile(aucPortName, GENERIC_READ | GENERIC_WRITE,	//access ( read and write)
			    0,	//(share) 0:cannot share the COM port
			    0,	//security  (None)
			    OPEN_EXISTING,	// creation : open_existing
			    FILE_ATTRIBUTE_NORMAL,	// we want overlapped operation
			    0	// no templates file for COM port...
		);
	if (myport == INVALID_HANDLE_VALUE) {
		return OPEN_FAILURE;
	}
	// Now start to read but first we need to set the COM port settings and the timeouts
	if (!SetCommMask(myport, EV_RXCHAR | EV_TXEMPTY)) {
		//ASSERT(0);
		return OPEN_FAILURE;
	}
	// Now we need to set baud rate etc,
	dcb.DCBlength = sizeof(DCB);

	if (!GetCommState(myport, &dcb)) {
		CloseHandle(myport);
		return OPEN_FAILURE;
	}

	dcb.BaudRate = pConfig->uiSpeed;
	dcb.ByteSize = pConfig->ucByteSize;
	dcb.Parity = pConfig->uart.ucParity;
	if (pConfig->uart.ucStopBits == 1) {
		dcb.StopBits = ONESTOPBIT;
	} else if (pConfig->uart.ucStopBits == 2) {
		dcb.StopBits = TWOSTOPBITS;
	} else {
		dcb.StopBits = ONE5STOPBITS;
	}

	dcb.fDsrSensitivity = 0;
	dcb.fDtrControl = DTR_CONTROL_DISABLE;
	if (pConfig->uart.ucFlowCtrl &&
	    pConfig->uiSpeed == pConfig->uart.uiSecondBaudrate) {
		dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;
		dcb.fOutxCtsFlow = 1;
	} else {
		dcb.fRtsControl = RTS_CONTROL_DISABLE;
		dcb.fOutxCtsFlow = 0;
	}
	dcb.fOutxDsrFlow = 0;

	if (!SetCommState(myport, &dcb)) {
		return OPEN_FAILURE;
	}
	// Now set the timeouts ( we control the timeout overselves using WaitForXXX()
	timeouts.ReadIntervalTimeout = MAXDWORD;
	timeouts.ReadTotalTimeoutMultiplier = 0;
	timeouts.ReadTotalTimeoutConstant = 0;
	timeouts.WriteTotalTimeoutMultiplier = 0;
	timeouts.WriteTotalTimeoutConstant = 0;

	if (!SetCommTimeouts(myport, &timeouts)) {
		return OPEN_FAILURE;
	}
	return (int32) myport;
}

/******************************************************************************
 *
 * Name: fw_upload_CloseUart_Win
 *
 * Description:
 *   Close Uart.
 *
 * Conditions For Use:
 *   None.
 *
 * Arguments:
 *
 * Return Value:
 *   None.
 *
 * Notes:
 *   None.
 *
 
*****************************************************************************/
static void
fw_upload_CloseUart_Win(int32 iPortID)
{
	HANDLE myport = (HANDLE) iPortID;
	if (myport) {
		CloseHandle(myport);
	}
}
