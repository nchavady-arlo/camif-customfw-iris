/** @file   fw_loader_io_linux_uart.c
 *
 *  @brief  This file contains the functions that implement the Nxp specific
 *          Helper Protocol for Linux over uart.
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
#include <errno.h>
#include <string.h>
#include <termios.h>
#include "fw_loader_io_linux.h"

/*===================== Macros ===================================================*/
#define TIMEOUT_SEC             6
#define TIMEOUT_FOR_READ        2000
#define PRINT(...)         printWithTimeStamp(__VA_ARGS__)
#define USE_SELECT
/*==================== Typedefs =================================================*/

/*==================== Function Prototypes ======================================*/
static int8 fw_upload_Probe_Linux_Uart(int8 * dev);
static int8 fw_upload_ComReadChar_Linux_Uart(int32 iPortID);
static int8 fw_upload_ComWriteChar_Linux_Uart(int32 iPortID, int8 iChar);
static int8 fw_upload_ComWriteChars_Linux_Uart(int32 iPortID, int8 * pChBuffer,
					       int32 uiLen);
static int32 fw_upload_ComReadChars_Linux_Uart(int32 iPortID, int8 * pChBuffer,
					       int32 uiCount);
static int32 fw_upload_init_uart_Linux_Uart(int8 * pPortName,
					    SERIAL_CONFIG * pConfig);
static int32 fw_upload_ComGetCTS_Linux_Uart(int32 iPortID);
static int32 fw_upload_ComGetBufferSize_Linux_Uart(int32 iPortID);
static void fw_upload_Flush_Linux_Uart(int32 iPortID);
static void fw_upload_CloseUart_Linux_Uart(int32 iPortID);

/*===================== Global Vars ==============================================*/
SERIAL_INTERFACE sSerialInterfaceLinuxUart = {
	.name = "UART interface",
	.type = UART_IF,
	.Probe = fw_upload_Probe_Linux_Uart,
	.Open = fw_upload_init_uart_Linux_Uart,
	.Close = fw_upload_CloseUart_Linux_Uart,
	.ComReadChar = fw_upload_ComReadChar_Linux_Uart,
	.ComWriteChar = fw_upload_ComWriteChar_Linux_Uart,
	.ComWriteChars = fw_upload_ComWriteChars_Linux_Uart,
	.ComReadChars = fw_upload_ComReadChars_Linux_Uart,
	.ComGetCTS = fw_upload_ComGetCTS_Linux_Uart,
	.ComGetBufferSize = fw_upload_ComGetBufferSize_Linux_Uart,
	.Flush = fw_upload_Flush_Linux_Uart,
};

/*==================== Coded Procedures =========================================*/
/******************************************************************************
 *
 * Name: fw_upload_Probe_Linux_Uart
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
fw_upload_Probe_Linux_Uart(int8 * dev)
{
	int fd, ret;
	int8 result = 0;
	struct termios tios;
	speed_t speed;

	fd = open(dev, O_RDWR | O_NOCTTY | O_CLOEXEC);
	if (fd < 0) {
		perror("Can't probe serial port");
		return 0;
	}

	/* Can read the termios, assume it is an uart, othercase we will get the error "Inappropriate ioctl for device" */
	ret = tcgetattr(fd, &tios);
	if (ret == 0) {
		speed = cfgetispeed(&tios);
		if (speed)
			speed = 1;
		result = 1;
//  } else {
//    PRINT("%s is not an UART (%s)\n", dev, strerror(errno));
	}

	close(fd);

	return result;
}

/******************************************************************************
 *
 * Name: fw_upload_ComReadChar_Linux_Uart
 *
 * Description:
 *   Read a character from the port specified by nPortID.     
 *
 * Conditions For Use:
 *   None.
 *
 * Arguments:
 *   nPortID : Port ID.
 *
 * Return Value:
 *   Returns the character, if Successful.
 *   Returns -1 if no character available (OR TIMED-OUT)
 *
 * Notes:
 *   None.
 *
 *****************************************************************************/
static int8
fw_upload_ComReadChar_Linux_Uart(int32 iPortID)
{
	int32 iResult = 0xFF;
	uint8 ucNumCharToRead = 1;

#ifdef USE_SELECT
	fd_set rd;
	struct timeval tv;
#else
	uint64 uTimeOut = TIMEOUT_FOR_READ;
	uint64 uEndTime;
#endif

#ifdef USE_SELECT
	FD_ZERO(&rd);
	FD_SET(iPortID, &rd);
	tv.tv_sec = TIMEOUT_FOR_READ / 1000;
	tv.tv_usec = 0;

	if (select(iPortID + 1, &rd, NULL, NULL, &tv) < 0)
		perror("select error in fw_upload_ComReadChar_Linux!\n");
	else if (FD_ISSET(iPortID, &rd)) {
		if (read(iPortID, &iResult, (size_t)ucNumCharToRead) ==
		    ucNumCharToRead) {
			return (uint8) ((uint8) iResult & (uint8) 0xFF);
		}
	}
#else
	uEndTime = fw_upload_GetTime_Linux() + uTimeOut;
	do {
		if (read(iPortID, &iResult, (size_t)ucNumCharToRead) ==
		    ucNumCharToRead) {
			return (uint8) ((uint8) iResult & (uint8) 0xFF);
		}
	} while (uEndTime > fw_upload_GetTime_Linux());

#endif

	return 0xFF;
}

/******************************************************************************
 *
 * Name: fw_upload_ComReadChars_Linux_Uart
 *
 * Description:
 *   Read iCount characters from the port specified by nPortID.     
 *
 * Conditions For Use:
 *   None.
 *
 * Arguments:
 *   iPortID   : Port ID.
 *   pBuffer : Destination buffer for the characters read 
 *   iCount    : Number of Characters to be read.
 *
 * Return Value:
 *   Returns the number of characters read if Successful.
 *   Returns -1 if iCount characters could not be read or if Port ID is invalid.
 *
 * Notes:
 *   None.
 *
 *****************************************************************************/
static int32
fw_upload_ComReadChars_Linux_Uart(int32 iPortID, int8 * pBuffer, int32 uiCount)
{

#ifdef USE_SELECT
	fd_set rd;
	struct timeval tv;
	int32 rdCount, retCount, nLoop;
	int8 *rdPtr;
	int ret;
#else
	uint64 uEndTime;
#endif

	uint64 uTimeOut = TIMEOUT_FOR_READ;

#ifdef USE_SELECT
	FD_ZERO(&rd);
	FD_SET(iPortID, &rd);
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	rdPtr = pBuffer;
	rdCount = uiCount;
	nLoop = 0;

	while (1) {
		ret = select(iPortID + 1, &rd, NULL, NULL, &tv);
		if (ret < 0) {
			perror("select error in fw_upload_ComReadChars_Linux!\n");
			break;
		}

		/* Select Timeout */
		if (ret == 0) {
			nLoop++;
			if (nLoop > uTimeOut / 1000)
				break;
		}

		if (FD_ISSET(iPortID, &rd)) {
			retCount =
				(int32) read(iPortID, rdPtr, (size_t)rdCount);
			if (retCount < rdCount) {
				rdCount -= retCount;
				rdPtr += retCount;
			} else {
				return uiCount;
			}
		}
	}
#else

	uEndTime = fw_upload_GetTime_Linux() + uTimeOut;

	do {
		if (fw_upload_ComGetBufferSize_Linux(iPortID) >= uiCount) {
			uiCount = read(iPortID, pBuffer, (size_t)uiCount);
			return uiCount;
		}
	} while (uEndTime > fw_upload_GetTime_Linux());
#endif
	return RW_FAILURE;
}

/******************************************************************************
 *
 * Name: fw_upload_ComWriteChar_Linux_Uart
 *
 * Description:
 *   Write a character to the port specified by iPortID.     
 *
 * Conditions For Use:
 *   None.
 *
 * Arguments:
 *   iPortID : Port ID.
 *   iChar   : Character to be written
 *
 * Return Value:
 *   Returns TRUE, if write is Successful.
 *   Returns FALSE if write is a failure.
 *
 * Notes:
 *   None.
 *
 *****************************************************************************/
static int8
fw_upload_ComWriteChar_Linux_Uart(int32 iPortID, int8 iChar)
{
	uint8 ucNumCharToWrite = 1;

	if (iPortID > 1000000) {
		return (int8) RW_FAILURE;
	}
	if (write(iPortID, &iChar, ucNumCharToWrite) == ucNumCharToWrite) {
		return (int8) RW_SUCCESSFUL;
	} else {
		return (int8) RW_FAILURE;
	}
}

/******************************************************************************
 *
 * Name: fw_upload_ComWriteChars_Linux_Uart
 *
 * Description:
 *   Write iLen characters to the port specified by iPortID.     
 *
 * Conditions For Use:
 *   None.
 *
 * Arguments:
 *   iPortID : Port ID.
 *   pBuffer : Buffer where characters are available to be written to the Port.
 *   iLen    : Number of Characters to write.
 *
 * Return Value:
 *   Returns TRUE, if write is Successful.
 *   Returns FALSE if write is a failure.
 *
 * Notes:
 *   None.
 *
 *****************************************************************************/
static int8
fw_upload_ComWriteChars_Linux_Uart(int32 iPortID, int8 * pBuffer, int32 uiLen)
{
	if (write(iPortID, pBuffer, uiLen) == uiLen) {
		return (int8) RW_SUCCESSFUL;
	} else {
		return (int8) RW_FAILURE;
	}
}

// UART Set up Related Functions.
// adapted from hciattach.c (bluez-util-3.7)
/******************************************************************************
 *
 * Name: fw_upload_uart_speed
 *
 * Description:
 *   Return the baud rate corresponding to the frequency.     
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

static speed_t
fw_upload_uart_speed(uint32 u32Rate)
{
	speed_t ulBaudrate = 0;
	switch (u32Rate) {
	case 9600:
		ulBaudrate = B9600;
		break;
	case 19200:
		ulBaudrate = B19200;
		break;
	case 38400:
		ulBaudrate = B38400;
		break;
	case 57600:
		ulBaudrate = B57600;
		break;
	case 115200:
		ulBaudrate = B115200;
		break;
#ifdef B230400
	case 230400:
		ulBaudrate = B230400;
		break;
#endif // B230400
#ifdef B460800
	case 460800:
		ulBaudrate = B460800;
		break;
#endif // B460800
#ifdef B500000
	case 500000:
		ulBaudrate = B500000;
		break;
#endif // B500000
#ifdef B576000
	case 576000:
		ulBaudrate = B576000;
		break;
#endif // B576000
#ifdef B921600
	case 921600:
		ulBaudrate = B921600;
		break;
#endif // B921600
#ifdef B1000000
	case 1000000:
		ulBaudrate = B1000000;
		break;
#endif // B1000000
#ifdef B1152000
	case 1152000:
		ulBaudrate = B1152000;
		break;
#endif // B1152000
#ifdef B1500000
	case 1500000:
		ulBaudrate = B1500000;
		break;
#endif // B1500000
#ifdef B3000000
	case 3000000:
		ulBaudrate = B3000000;
		break;
#endif // B3000000

#ifdef B4000000
	case 4000000:
		ulBaudrate = B4000000;
		break;
#endif // B4000000

	default:
		ulBaudrate = B0;
		break;
	}
	return ulBaudrate;
}

// adapted from hciattach.c (bluez-util-3.7)
/******************************************************************************
 *
 * Name: fw_upload_set_speed
 *
 * Description:
 *   Set the baud rate speed.
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
fw_upload_set_speed(int32 fd, struct termios *ti, int32 speed)
{
	cfsetospeed(ti, (speed_t) fw_upload_uart_speed(speed));
	cfsetispeed(ti, (speed_t) fw_upload_uart_speed(speed));
	return tcsetattr(fd, TCSANOW, ti);
}

/******************************************************************************
 *
 * Name: fw_upload_ComGetCTS_Linux_Uart
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
 *
 * Notes:
 *   None.
 *
 *****************************************************************************/
static int32
fw_upload_ComGetCTS_Linux_Uart(int32 iPortID)
{
	int32 status = 0;
	ioctl(iPortID, TIOCMGET, &status);
	if (status & TIOCM_CTS) {
		return 0;
	} else {
		return 1;
	}
}

/******************************************************************************
 *
 * Name: fw_upload_ComGetBufferSize_Linux_Uart
 *
 * Description:
 *   Check buffer size
 *
 * Conditions For Use:
 *   None.
 *
 * Arguments:
 *   iPortID
 *
 * Return Value:
 *   size in buffer
 *
 * Notes:
 *   None.
 *
 *****************************************************************************/
static int32
fw_upload_ComGetBufferSize_Linux_Uart(int32 iPortID)
{
	int32 bytes = 0;
	ioctl(iPortID, FIONREAD, &bytes);
	return bytes;
}

/******************************************************************************
 *
 * Name: fw_upload_Flush_Linux_Uart
 *
 * Description:
 *   flush buffer
 *
 * Conditions For Use:
 *   None.
 *
 * Arguments:
 *   iPortID
 *
 * Return Value:
 *   None.
 *
 * Notes:
 *   None.
 *
 *****************************************************************************/
static void
fw_upload_Flush_Linux_Uart(int32 iPortID)
{
	tcflush(iPortID, TCIFLUSH);
}

// adapted from hciattach.c (bluez-util-3.7)
/******************************************************************************
 *
 * Name: fw_upload_init_uart_Linux_Uart
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
fw_upload_init_uart_Linux_Uart(int8 * dev, SERIAL_CONFIG * pConfig)
{
	struct termios ti;
	int32 fd = open(dev, O_RDWR | O_NOCTTY);

	if (fd < 0) {
		perror("Can't open serial port");
		return OPEN_FAILURE;
	}

#if defined(W9098)
	tcsendbreak(fd, 0);
	usleep(500000);
#endif
	//tcflush(fd, TCIOFLUSH);

	if (tcgetattr(fd, &ti) < 0) {
		perror("Can't get port settings");
		close(fd);
		return OPEN_FAILURE;
	}
	tcflush(fd, TCIOFLUSH);
	cfmakeraw(&ti);
	ti.c_cflag |= (tcflag_t) (CLOCAL | CREAD);

	// Set 1 stop bit & no parity (8-bit data already handled by cfmakeraw)
	ti.c_cflag &= ~((tcflag_t) (CSTOPB | PARENB));

#ifdef CRTSCTS
	if (pConfig->uart.ucFlowCtrl ||
	    pConfig->uart.uiSecondBaudrate == pConfig->uiSpeed) {
		ti.c_cflag |= (tcflag_t) CRTSCTS;
	} else {
		ti.c_cflag &= (tcflag_t) (~CRTSCTS);
	}
#else
	if (pConfig->uart.ucFlowCtrl ||
	    pConfig->uart.uiSecondBaudrate == pConfig->uiSpeed) {
		ti.c_cflag |= (tcflag_t) IHFLOW;
		ti.c_cflag |= (tcflag_t) OHFLOW;
	} else {
		ti.c_cflag &= (tcflag_t) (~IHFLOW);
		ti.c_cflag &= (tcflag_t) (~OHFLOW);
	}
#endif

	//FOR READS:  set timeout time w/ no minimum characters needed (since we read only 1 at at time...)
	ti.c_cc[VMIN] = (cc_t) 0;
	ti.c_cc[VTIME] = (cc_t) TIMEOUT_SEC *10;

	tcflush(fd, TCIOFLUSH);

	if (tcsetattr(fd, TCSANOW, &ti) < 0) {
		perror("Can't set port settings");
		close(fd);
		return OPEN_FAILURE;
	}
	tcflush(fd, TCIOFLUSH);

	/* Set actual baudrate */
	if (fw_upload_set_speed(fd, &ti, pConfig->uiSpeed) < 0) {
		perror("Can't set baud rate");
		close(fd);
		return OPEN_FAILURE;
	}

	return fd;
}

/******************************************************************************
 *
 * Name: fw_upload_CloseUart_Linux_Uart
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
fw_upload_CloseUart_Linux_Uart(int32 iPortID)
{
	if (iPortID > 1000000) {
		exit(-1);
	}

	close(iPortID);
}
